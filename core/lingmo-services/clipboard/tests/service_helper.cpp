#include <chrono>

#include "base/event_loop.h"
#include "base/timer.h"

#include "../clipboard.h"

// Keeps the clipboard for 15 seconds, for checks by hand.
int main()
{
    Lingmo::Base::EventLoop loop;
    Clipboard clipboard;
    Lingmo::Base::Timer quit;
    quit.setSingleShot(true);
    quit.onTimeout = [&loop] {
        loop.quit();
    };
    quit.start(std::chrono::seconds(15));
    return loop.run();
}
