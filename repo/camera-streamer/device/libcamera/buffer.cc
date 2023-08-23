#ifdef USE_LIBCAMERA
#include "libcamera.hh"

#include <inttypes.h>

int libcamera_buffer_open(buffer_t *buf)
{
  buf->libcamera = new buffer_libcamera_t{};
  buf->libcamera->request = buf->buf_list->dev->libcamera->camera->createRequest(buf->index);
  if (!buf->libcamera->request) {
    LOG_INFO(buf, "Can't create request");
    return -1;
  }

  auto &configurations = buf->buf_list->dev->libcamera->configuration;
  auto &configuration = configurations->at(buf->buf_list->index);
  auto stream = configuration.stream();
  const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers =
      buf->buf_list->dev->libcamera->allocator->buffers(stream);
  auto const &buffer = buffers[buf->index];

  if (buf->libcamera->request->addBuffer(stream, buffer.get()) < 0) {
    LOG_ERROR(buf, "Can't set buffer for request");
  }
  if (buf->start) {
    LOG_ERROR(buf, "Too many streams.");
  }

  if (buffer->planes().empty()) {
    LOG_ERROR(buf, "No planes allocated");
  }

  {
    uint64_t offset = buffer->planes()[0].offset;
    uint64_t length = 0;
    libcamera::SharedFD dma_fd = buffer->planes()[0].fd;

    // Require that planes are continuous
    for (auto const &plane : buffer->planes()) {
      if (plane.fd != dma_fd) {
        LOG_ERROR(buf, "Plane does not share FD: fd=%d, expected=%d", plane.fd.get(), dma_fd.get());
      }

      if (offset + length != plane.offset) {
        LOG_ERROR(buf, "Plane is not continuous: offset=%u, expected=%" PRIu64, plane.offset, offset + length);
      }

      length += plane.length;
    }

    buf->start = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd.get(), offset);
    if (!buf->start || buf->start == MAP_FAILED) {
      LOG_ERROR(buf, "Failed to mmap DMA buffer");
    }

    buf->dma_fd = dma_fd.get();
    buf->length = length;

    LOG_DEBUG(buf, "Mapped buffer: start=%p, length=%zu, fd=%d, planes=%zu",
      buf->start, buf->length, buf->dma_fd, buffer->planes().size());
  }

  return 0;

error:
  return -1;
}

void libcamera_buffer_close(buffer_t *buf)
{
  if (buf->libcamera) {
    delete buf->libcamera;
    buf->libcamera = NULL;
  }
}

int libcamera_buffer_enqueue(buffer_t *buf, const char *who)
{
  auto &request = buf->libcamera->request;
  auto const &camera = buf->buf_list->dev->libcamera->camera;

  request->reuse(libcamera::Request::ReuseBuffers);
  request->controls() = std::move(buf->buf_list->dev->libcamera->controls);

  if (camera->queueRequest(buf->libcamera->request.get()) < 0) {
    LOG_ERROR(buf, "Can't queue buffer.");
  }
  buf->buf_list->dev->libcamera->controls.clear();
  return 0;

error:
  return -1;
}

void libcamera_buffer_dump_metadata(buffer_t *buf)
{
  auto &metadata = buf->libcamera->request->metadata();
  auto idMap = metadata.idMap();

  for (auto const &control : metadata) {
    if (!control.first)
      continue;

    auto control_id = control.first;
    auto control_value = control.second;
    std::string control_id_name = "";

    if (auto control_id_info = idMap ? idMap->at(control_id) : NULL) {
      control_id_name = control_id_info->name();
    }

    LOG_VERBOSE(buf, "Metadata: %s (%08x, type=%d): %s",
      control_id_name.c_str(), control_id, control_value.type(), control_value.toString().c_str());
  }
}

void buffer_list_libcamera_t::libcamera_buffer_list_dequeued(libcamera::Request *request)
{
  if (request->status() == libcamera::Request::RequestComplete) {
    unsigned index = request->cookie();
    if (write(buf_list->libcamera->fds[1], &index, sizeof(index)) == sizeof(index)) {
      return;
    }
  }

  // put back into queue, as it failed
  buf_list->dev->libcamera->camera->queueRequest(request);
}

int libcamera_buffer_list_dequeue(buffer_list_t *buf_list, buffer_t **bufp)
{
  unsigned index = 0;
  int n = read(buf_list->libcamera->fds[0], &index, sizeof(index));
  if (n != sizeof(index)) {
    LOG_INFO(buf_list, "Received invalid result from `read`: %d", n);
    return -1;
  }

  if (index >= (unsigned)buf_list->nbufs) {
    LOG_INFO(buf_list, "Received invalid index from `read`: %d >= %d", index, buf_list->nbufs);
    return -1;
  }

  *bufp = buf_list->bufs[index];

  std::optional<int64_t> sensor_timestamp((*bufp)->libcamera->request->metadata().
    get<int64_t>(libcamera::controls::SensorTimestamp));
  uint64_t sensor_timestamp_us = sensor_timestamp.value_or(0) / 1000;
  uint64_t boot_time_us = get_time_us(CLOCK_BOOTTIME, NULL, NULL, 0);

  uint64_t now_us = get_monotonic_time_us(NULL, NULL);

  (*bufp)->captured_time_us = now_us - (boot_time_us - sensor_timestamp_us);
  (*bufp)->used = 0;

  for (auto &bufferMap : (*bufp)->libcamera->request->buffers()) {
    auto frameBuffer = bufferMap.second;

    for (auto const &plane : frameBuffer->metadata().planes()) {
      (*bufp)->used += plane.bytesused;
    }
  }

  if (index == 0) {
    libcamera_buffer_dump_metadata(*bufp);
  }
  return 0;
}

int libcamera_buffer_list_pollfd(buffer_list_t *buf_list, struct pollfd *pollfd, bool can_dequeue)
{
  int count_enqueued = buffer_list_count_enqueued(buf_list);
  pollfd->fd = buf_list->libcamera->fds[0]; // write end
  pollfd->events = POLLHUP;
  if (can_dequeue && count_enqueued > 0) {
    pollfd->events |= POLLIN;
  }
  pollfd->revents = 0;
  return 0;
}
#endif // USE_LIBCAMERA
