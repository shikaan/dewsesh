#include "ctx.h"
#include "cairo.h"
#include "log.h"
#include "result.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
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

static ctx_t pool[2] = {0};

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

static result_t ctx_create(ctx_t *c, uint32_t w, uint32_t h,
                           struct wl_shm *shm) {
  uint32_t stride = w * 4;
  uint32_t size = stride * h;

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

  struct wl_shm_pool *wl_shm_pool = wl_shm_create_pool(shm, fd, (int32_t)size);
  struct wl_buffer *wl_buffer =
      wl_shm_pool_create_buffer(wl_shm_pool, 0, (int32_t)w, (int32_t)h,
                                (int32_t)stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(wl_shm_pool);
  close(fd);

  cairo_surface_t *target = cairo_image_surface_create_for_data(
      data, CAIRO_FORMAT_ARGB32, (int)w, (int)h, (int)stride);
  cairo_status_t status = cairo_surface_status(target);
  if (status != CAIRO_STATUS_SUCCESS) {
    log_error("cairo error: %s", cairo_status_to_string(status));
    return ERR_CTX_CAIRO;
  }

  c->busy = false;
  c->wl.buffer = wl_buffer;
  c->height = h;
  c->width = w;
  c->shm.buf = data;
  c->shm.len = size;
  c->cairo.target = target;
  c->cairo.ctx = cairo_create(target);

  wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, c);

  return OK;
}

static void ctx_deinit(ctx_t *self) {
  if (self->wl.buffer)
    wl_buffer_destroy(self->wl.buffer);

  if (self->cairo.ctx)
    cairo_destroy(self->cairo.ctx);

  if (self->cairo.target)
    cairo_surface_destroy(self->cairo.target);

  memset(self, 0, sizeof(ctx_t));
}

result_t ctx_init(uint32_t w, uint32_t h, struct wl_shm *shm) {
  result_t res;

  ctx_deinit(&pool[0]);
  res = ctx_create(&pool[0], w, h, shm);
  if (res != OK)
    return res;

  ctx_deinit(&pool[1]);
  res = ctx_create(&pool[1], w, h, shm);
  if (res != OK)
    return res;

  return OK;
}

result_t ctx_get(uint32_t w, uint32_t h, ctx_t **ctx) {
  ctx_t *selected = pool[0].busy ? pool[1].busy ? NULL : &pool[1] : &pool[0];
  if (!selected) {
    *ctx = NULL;
    return ERR_CTX_NO_BUFFERS;
  }

  if (selected->height != h || selected->width != w) {
    *ctx = NULL;
    return ERR_CTX_BUFFER_MISMATCH;
  }

  *ctx = selected;
  return OK;
}
