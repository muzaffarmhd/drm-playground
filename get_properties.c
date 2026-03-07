#define _GNU_SOURCE
#include "fcntl.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "unistd.h"

struct display_limits {
    uint32_t height;
    uint32_t width;
};

int setup_device(char* path, int* device_fd) {
    uint64_t hasDumb;
    const int fd = open(path, O_RDWR);
    if (fd <= -1) {
        fprintf(stderr, "Cannot open device %s\n", path);
        return -errno;
    }
    // here we would need to check if the drm device has capability for dumb buffers
    if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &hasDumb) < 0 || !hasDumb) {
        fprintf(stderr, "The device doesn't support dumb buffers\n");
        return -errno;
    }
    *device_fd = fd;
    return 0;
}

struct display_config {
    uint32_t fb_id;
    uint32_t connector_id;
    uint32_t crtc_id;
    uint32_t encoder_id;
    uint32_t plane_id;
    uint32_t mode_blob_id;
    drmModeModeInfo mode;
};

bool is_primary_plane(int fd, uint32_t plane_id) {
    int is_primary = 0;
    drmModeObjectProperties *props = drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props) return 0;
    for (int i=0; i < props->count_props; i++) {
        drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[i]);
        if (!prop) continue;
        if (strcmp(prop->name, "type") == 0) {
            if (props->prop_values[i] == DRM_PLANE_TYPE_PRIMARY) {
                is_primary = 1;
            }
            drmModeFreeProperty(prop);
            break;
        }
    }
    drmModeFreeObjectProperties(props);
    return is_primary;
}

/*
 * Loop through the connectors, and find the connector which is connected,
 * then find the first CRTC that is compatible with the connector's encoder,
 * then find the first primary plane that is compatible with the CRTC
 * store the connector_id, plane_id, crtc_id
*/

int setup_ccp(struct display_config *config, int device_fd) {
    const int fd = device_fd;
    drmModeRes* res = drmModeGetResources(fd);
    for (int i=0; i<res->count_connectors; i++) {
        drmModeConnector* conn = drmModeGetConnector(fd, res->connectors[i]);
        if (conn) {
            if (conn->connection == DRM_MODE_CONNECTED) {
                config->connector_id = conn->connector_id;
                config->encoder_id = conn->encoder_id;
                config->mode = conn->modes[0];
                drmModeFreeConnector(conn);
                break;
            }
        }
        drmModeFreeConnector(conn);
    }
    if (!config->connector_id) return 0;
    drmModeEncoder *enc = drmModeGetEncoder(fd, config->encoder_id);
    int crtc_idx = -1;
    for (int i=0; i<res->count_crtcs; i++) {
        if (enc->possible_crtcs & (1 << i)) {
            config->crtc_id = res->crtcs[i];
            crtc_idx = i;
            break;
        }
    }
    drmModeFreeEncoder(enc);
    drmModePlaneRes* pres = drmModeGetPlaneResources(fd);
    for (int i=0; i<pres->count_planes; i++) {
        drmModePlane* plane = drmModeGetPlane(fd, pres->planes[i]);
        if (plane) {
            if (plane->possible_crtcs & (1 << crtc_idx)) {
                if (is_primary_plane(fd, plane->plane_id)) {
                    config->plane_id = plane->plane_id;
                    drmModeFreePlane(plane);
                    break;
                }
            }
        }
        drmModeFreePlane(plane);
    }
    drmModeFreePlaneResources(pres);
    drmModeFreeResources(res);
}

int main() {
    int device_fd;
    struct display_config config;
    setup_device("/dev/dri/card1", &device_fd);
    setup_ccp(&config, device_fd);
    // if (!config.connector_id || !config.crtc_id || !config.plane_id) return 0;
    printf("connector_id: %d, crtc_id: %d, plane_id: %d", config.connector_id, config.crtc_id, config.plane_id);
}




