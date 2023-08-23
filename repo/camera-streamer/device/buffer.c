#include "device/buffer.h"
#include "device/buffer_list.h"
#include "device/device.h"
#include "util/opts/log.h"

buffer_t *buffer_open(const char *name, buffer_list_t *buf_list, int index) {
  buffer_t *buf = calloc(1, sizeof(buffer_t));
  device_t *dev = buf_list->dev;

  buf->name = strdup(name);
  buf->index = index;
  buf->buf_list = buf_list;
  buf->dma_fd = -1;
  buf->mmap_reflinks = 1;
  buf->used = 0;

  if (dev->hw->buffer_open(buf) < 0) {
    goto error;
  }

  return buf;

error:
  buffer_close(buf);
  return NULL;
}

void buffer_close(buffer_t *buf)
{
  if (buf == NULL) {
    return;
  }

  buf->buf_list->dev->hw->buffer_close(buf);
  free(buf->name);
  free(buf);
}
