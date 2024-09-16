#include "WLFrontend_public.h"

#include "InputMethodV2.h"
#include "VirtualInputContext.h"
#include "WLFrontend.h"
#include "WaylandInputContext.h"
#include "wl/client/ConnectionBase.h"
#include "wl/client/ZwpInputMethodV2.h"

namespace org::deepin::dim::wlfrontend {

wl::client::ZwpInputMethodV2 *getInputMethodV2(InputContext *ic)
{
    auto *vic = qobject_cast<VirtualInputContext *>(ic);
    if (!vic) {
        return nullptr;
    }

    auto *wic = qobject_cast<WaylandInputContext *>(vic->parentIC());
    if (!wic) {
        return nullptr;
    }

    return dynamic_cast<wl::client::ZwpInputMethodV2 *>(wic->getInputMethodV2());
}

wl::client::ConnectionBase *getWl(Addon *addon)
{
    auto *wlf = qobject_cast<WLFrontend *>(addon);
    return wlf->getWl();
}

} // namespace org::deepin::dim::wlfrontend
