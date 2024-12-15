#ifndef DIM_XDGSURFACE_H
#define DIM_XDGSURFACE_H

#include "wl/client/XdgSurface.h"

#include <memory>

namespace wl::client {

class Shm;
}

class XdgSurface : public wl::client::XdgSurface
{
public:
    explicit XdgSurface(xdg_surface *val,
                        wl_surface *surface,
                        const std::shared_ptr<wl::client::Shm> &shm);
    ~XdgSurface() override;

protected:
    void xdg_surface_configure(uint32_t serial) override;

private:
    void createBuffer(int width, int height);

private:
    wl_surface *surface_;
    std::shared_ptr<wl::client::Shm> shm_;
};

#endif // !DIM_XDGSURFACE_H
