#include "ctx.h"
#include "cairo.h"
#include "log.h"
#include "types.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-client.h>

#ifndef SYS_memfd_create
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#endif

static void wl_release(void *data, struct wl_buffer *wl_buffer) {
  (void)wl_buffer;
  ctx_t *ctx = data;
  ctx->busy = false;
}
static const struct wl_buffer_listener wl_buffer_listener = {.release =
                                                                 wl_release};

static int create_shm(void) {
#ifdef SYS_memfd_create
  return (int)syscall(SYS_memfd_create, "dewsesh", 0);
#else
  int retries = 100;

  do {
    // try a probably-unique name
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    pid_t pid = getpid();
    char name[50];
    snprintf(name, sizeof(name), "/dewsesh-%x-%x", (unsigned int)pid,
             (unsigned int)ts.tv_nsec);

    // shm_open guarantees that O_CLOEXEC is set
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }

    --retries;
  } while (retries > 0 && errno == EEXIST);

  return -1;
#endif
}

result_t ctx_create(ctx_t *c, size_t width, size_t height,
                    struct wl_shm *wl_shm) {
  size_t stride = width * 4;
  size_t size = stride * height;

  int fd = create_shm();
  if (fd < 0) {
    log_error("cannot allocate shared memory", NULL);
    return ERR_CTX_ALLOCATION;
  }

  if (ftruncate(fd, (long)size) < 0) {
    log_error("cannot truncate shared memory %s", strerror(errno));
    close(fd);
    return ERR_CTX_ALLOCATION;
  }

  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    log_error("mmap failed: %s\n", strerror(errno));
    close(fd);
    return ERR_CTX_ALLOCATION;
  }

  struct wl_shm_pool *wl_shm_pool =
      wl_shm_create_pool(wl_shm, fd, (int32_t)size);
  struct wl_buffer *wl_buffer =
      wl_shm_pool_create_buffer(wl_shm_pool, 0, (int32_t)width, (int32_t)height,
                                (int32_t)stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(wl_shm_pool);
  close(fd);

  cairo_surface_t *target = cairo_image_surface_create_for_data(
      data, CAIRO_FORMAT_ARGB32, (int)width, (int)height, (int)stride);
  cairo_status_t status = cairo_surface_status(target);
  if (status != CAIRO_STATUS_SUCCESS) {
    log_error("cairo error: %s", cairo_status_to_string(status));
    return ERR_CTX_CAIRO;
  }

  c->busy = false;
  c->wl.buffer = wl_buffer;
  c->height = height;
  c->width = width;
  c->shm.buf = data;
  c->shm.len = size;
  c->cairo.target = target;
  c->cairo.ctx = cairo_create(target);

  wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, c);

  return OK;
}

void ctx_destroy(ctx_t **c) {
  ctx_t *self = *c;

  if (self->wl.buffer)
    wl_buffer_destroy(self->wl.buffer);

  if (self->cairo.ctx)
    cairo_destroy(self->cairo.ctx);

  if (self->cairo.target)
    cairo_surface_destroy(self->cairo.target);

  memset(self, 0, sizeof(ctx_t));
  *c = NULL;
}
