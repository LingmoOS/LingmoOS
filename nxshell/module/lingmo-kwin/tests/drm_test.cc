#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>

int main(int argc, char *argv[])
{
    drmModeGetConnectorTypeName(DRM_MODE_CONNECTOR_USB);
    return 0;
}