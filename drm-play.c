#define _GNU_SOURCE
#include "fcntl.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include <errno.h>
#include "stdio.h"
#include "unistd.h"

int get_drm_device(int* out, char* node) {
  int ret;
  int fd = open(node, O_RDWR);
  uint64_t hasDumb;
  if (fd <= -1) {
    ret = -errno;
    fprintf(stderr, "failed to grab the drm device %d", fd);
    return ret;
  }
  if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &hasDumb) < 0 || !hasDumb) {
    fprintf (stderr, "the device %s doesn't support dumb buffers", node);
    close(fd);
    return -EOPNOTSUPP;
  };
  *out = fd;
  return 0;
}

int main() {
  int fd;
  char* node = "/dev/dri/card1";
  get_drm_device (&fd, node);
  printf("got the drm device at %d\n", fd);
}

