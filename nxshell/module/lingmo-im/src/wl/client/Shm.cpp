#include "Shm.h"

using namespace wl::client;

Shm::Shm(wl_shm *val)
    : Type(val)
{
}

Shm::~Shm() = default;
