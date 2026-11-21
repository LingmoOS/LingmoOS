// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Test sources retain the lower-case component spelling, including qualified
// Lingmo::core and Lingmo::test names that global aliases cannot satisfy.
namespace Lingmo {
namespace Core {}
namespace Test {}
namespace Testsupport {}
namespace core = Core;
namespace Input {}
namespace input = Input;
namespace test = Test;
namespace testsupport = Testsupport;
}
namespace test = Lingmo::Test;
namespace testsupport = Lingmo::Testsupport;
namespace Lingmo::Shell {
namespace ClockFormat {}
namespace clock_format = ClockFormat;
}
namespace Lingmo {
namespace Wm = WM;
}
