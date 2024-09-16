#ifndef WL_CLIENT_SHM_H
#define WL_CLIENT_SHM_H

#include "Type.h"

namespace wl::client {

class Shm : public Type<wl_shm>
{
public:
    explicit Shm(wl_shm *val);
    ~Shm();
};

} // namespace wl::client

#endif // !WL_CLIENT_SHM_H
