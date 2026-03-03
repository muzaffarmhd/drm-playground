#define _GNU_SOURCE
#include "fcntl.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include "stdio.h"

int get_drm_device(int* out, char* node) {
  int fd = open(node, O_RDWR);
  if (fd <= -1) {
    printf ("failed to grab the drm device %d", fd);
  }
  *out = fd;
}

int main() {
  int fd;
  char* node = "/dev/dri/card1";
  get_drm_device (&fd, node);
  printf("got the drm device at %d", fd);
}

