// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The toolkit's declarations live in Lingmo::base, Lingmo::base::dbus,
// Lingmo::ui and Lingmo::I18n. The desktop's sources were ported from the
// Cutefish toolkit and spell them base::, dbus::, ui:: and i18n::; the
// framework, the core and the compositor also name them Lingmo::Base,
// Lingmo::Base::DBus and Lingmo::UI. This header aliases both older
// spellings.
//
// The toolkit's CMake target force-includes it, so it comes before any
// header that relies on the names, without every source having to include
// it: the target's interface carries -include base/compat.h.
#pragma once

#ifdef __cplusplus

namespace Lingmo::base {}
namespace Lingmo::base::dbus {}
namespace Lingmo::ui {}
namespace Lingmo::I18n {}

// The capitalised spellings the framework, core and compositor use.
namespace Lingmo {
namespace Base = base;
namespace UI = ui;
namespace i18n = I18n;
} // namespace Lingmo

// Lingmo::Base::DBus is how the core names the bus namespace.
namespace Lingmo::base {
namespace DBus = dbus;
} // namespace Lingmo::base

namespace base = Lingmo::base;
namespace dbus = Lingmo::base::dbus;
namespace ui = Lingmo::ui;
namespace i18n = Lingmo::I18n;

#endif // __cplusplus
