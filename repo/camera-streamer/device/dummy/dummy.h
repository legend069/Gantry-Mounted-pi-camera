#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct buffer_s buffer_t;
typedef struct buffer_list_s buffer_list_t;
typedef struct device_s device_t;
struct pollfd;

typedef struct device_dummy_s {
} device_dummy_t;

typedef struct buffer_list_dummy_s {
  int fds[2];
  void *data;
  size_t length;
} buffer_list_dummy_t;

typedef struct buffer_dummy_s {
} buffer_dummy_t;

int dummy_device_open(device_t *dev);
void dummy_device_close(device_t *dev);
int dummy_device_video_force_key(device_t *dev);
int dummy_device_set_fps(device_t *dev, int desired_fps);
int dummy_device_set_option(device_t *dev, const char *key, const char *value);

int dummy_buffer_open(buffer_t *buf);
void dummy_buffer_close(buffer_t *buf);
int dummy_buffer_enqueue(buffer_t *buf, const char *who);
int dummy_buffer_list_dequeue(buffer_list_t *buf_list, buffer_t **bufp);
int dummy_buffer_list_pollfd(buffer_list_t *buf_list, struct pollfd *pollfd, bool can_dequeue);

int dummy_buffer_list_open(buffer_list_t *buf_list);
void dummy_buffer_list_close(buffer_list_t *buf_list);
int dummy_buffer_list_set_stream(buffer_list_t *buf_list, bool do_on);
