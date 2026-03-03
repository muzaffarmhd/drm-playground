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

int get_connector(int* fd, drmModeModeInfo mode_info, uint32_t* connector_id) {
  drmModeResPtr res= drmModeGetResources (*fd);
  for(int i=0; i<res->count_connectors; i++) {
    printf("got connector id %d\n", res->connectors[i]);
    drmModeConnectorPtr connection = drmModeGetConnector (*fd, res->connectors[i]);
    if (connection->connection == DRM_MODE_CONNECTED) {
      mode_info = connection->modes[0];
      *connector_id = connection->connector_id;
      break;
    }
    drmModeFreeConnector (connection);
  }
  drmModeFreeResources (res);
}

int main() {
  uint32_t connector_id;
  drmModeModeInfo mode_info;
  int fd;
  char* node = "/dev/dri/card1";
  get_drm_device (&fd, node);
  printf("got the drm device at %d\n", fd);
  get_connector (&fd, mode_info, &connector_id);
  printf("decided on the connector id%d\n",connector_id );
}

