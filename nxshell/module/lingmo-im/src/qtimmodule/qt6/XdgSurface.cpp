#include "XdgSurface.h"

#include "common/shm_open_anon.h"
#include "wl/client/Shm.h"

#include <QDebug>

#include <sys/mman.h>
#include <unistd.h>

XdgSurface::XdgSurface(xdg_surface *val,
                       wl_surface *surface,
                       const std::shared_ptr<wl::client::Shm> &shm)
    : wl::client::XdgSurface(val)
    , surface_(surface)
    , shm_(shm)
{
}

XdgSurface::~XdgSurface() = default;

void XdgSurface::xdg_surface_configure(uint32_t serial)
{
    xdg_surface_ack_configure(get(), serial);

    createBuffer(1, 1);
}

void XdgSurface::createBuffer(int width, int height)
{
    int stride = width * 4; // 4 bytes per pixel
    int size = stride * height;

    int fd = shm_open_anon();
    if (fd < 0) {
        // todo: error handling
        return;
    }
    if (ftruncate(fd, size) != 0) {
        qWarning() << "ftruncate failed";
        return;
    }

    uint32_t *data =
        static_cast<uint32_t *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        // fprintf(stderr, "mmap fd %d failed, size=%d\n", fd, size);
        close(fd);
        return;
    }

    wl_shm_pool *pool = wl_shm_create_pool(shm_->get(), fd, size);
    wl_buffer *buffer =
        wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    memset(data, 0, size);

    munmap(data, size);
    close(fd);

    wl_surface_attach(surface_, buffer, 0, 0);
    wl_surface_commit(surface_);
}
