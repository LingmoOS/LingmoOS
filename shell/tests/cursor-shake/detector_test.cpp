// SPDX-License-Identifier: GPL-3.0-or-later
#include "cursor/shake_detector.h"
#include <cstdio>
#include <stdexcept>
#include <limits>
using Lingmo::Cursor::ShakeDetector;
using Lingmo::Cursor::ShakeAnimation;
void check(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
int main() {
    try {
        ShakeDetector detector;
        // Normal travel, including a fast sweep and low amplitude vibration.
        for (int i = 0; i < 100; ++i) check(!detector.move({double(i*10), 100}, i*8), "straight motion triggered");
        detector.reset();
        for (int i = 0; i < 100; ++i) check(!detector.move({100.0 + i%2*2, 100}, i*8), "jitter triggered");
        detector.reset();
        for (int i = 0; i < 10; ++i) check(!detector.move({double(i%2*80), 0}, i*400), "slow reversal triggered");
        detector.reset();
        for (int i = 0; i < 10; ++i) check(!detector.move({double(i%2*1000), 0}, i*20), "warp triggered");
        for (bool vertical : {false, true}) {
            for (int samples : {1, 8, 64}) {
                detector.reset(); bool triggered = false;
                for (int leg = 0; leg < 7; ++leg) {
                    for (int sample = 0; sample < samples; ++sample) {
                        const double x = (leg%2 ? 80 : 0) + (leg%2 ? -1 : 1) * 80.0 * sample / samples;
                        const int now = leg*80 + sample*80/samples;
                        triggered = detector.move(vertical ? Lingmo::base::PointF(100, x)
                                                            : Lingmo::base::PointF(x, 100), now) || triggered;
                    }
                }
                check(triggered, "intentional shake not detected across input rates");
            }
        }
        detector.move({std::numeric_limits<double>::infinity(), 0}, 1000);
        check(!detector.move({10, 10}, 1010), "invalid sample failed to reset");
        ShakeAnimation animation;
        animation.trigger(1000);
        check(animation.advance(1000) == 1, "growth starts at normal size");
        check(animation.advance(1050) > 1 && animation.advance(1050) < 2.5, "smooth growth");
        check(animation.advance(1100) == 2.5, "peak size");
        check(animation.advance(1800) == 2.5, "hold");
        check(animation.advance(2000) > 1 && animation.advance(2000) < 2.5, "smooth return");
        animation.trigger(2000);
        check(animation.advance(2100) == 2.5, "repeat extends locator");
        check(animation.advance(3200) == 1 && !animation.active(), "eventual return");
        animation.trigger(4000); animation.reset();
        check(animation.advance(4050) == 1 && !animation.active(), "disable clears effect");
        std::puts("Cursor shake detector and animation tests passed");
    } catch (const std::exception &e) { std::fprintf(stderr, "%s\n", e.what()); return 1; }
}
