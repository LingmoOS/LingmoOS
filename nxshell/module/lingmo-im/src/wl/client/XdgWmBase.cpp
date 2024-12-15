#include "XdgWmBase.h"

using namespace wl::client;

XdgWmBase::XdgWmBase(xdg_wm_base *val)
    : Type(val)
{
}

XdgWmBase::~XdgWmBase() = default;
