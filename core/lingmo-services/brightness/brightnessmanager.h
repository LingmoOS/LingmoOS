
#include "ddcworker.h"

#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/timer.h"

class BrightnessManager
{
public:
    explicit BrightnessManager(Lingmo::Base::DBus::Connection *session,
                               Lingmo::Base::DBus::Connection *system);
    ~BrightnessManager();

    BrightnessManager(const BrightnessManager &) = delete;
    BrightnessManager &operator=(const BrightnessManager &) = delete;

    // org.lingmo.Services.Brightness at /org/lingmo/Services/Brightness.
    bool publish();

    // Legacy single-display API; acts on the internal panel, else the first adjustable display.
    int maxBrightness();
    int brightness();
    bool brightnessEnabled();
    void setValue(int value);

    // Displays are sysfs DRM connector names, e.g. "card0-eDP-1". Values are percentages.
    std::vector<std::string> displays() const;
    int displayBrightness(const std::string &display) const;
    void setDisplayBrightness(const std::string &display, int value);

private:
    enum class Kind {
        Backlight,
        Ddc
    };

    struct Display {
        std::string connector;
        Kind kind = Kind::Backlight;
        std::string device; // backlight name or /dev/i2c-N
        int maximum = 0;
        int raw = 0;
    };

    void rescan();
    std::vector<Display> scanBacklights(const std::vector<std::string> &connected) const;
    std::vector<DdcDisplay> ddcCandidates(const std::vector<std::string> &connected,
                                          const std::vector<Display> &backlights) const;
    void publish(std::vector<Display> displays);
    void readBacklight(Display &display) const;
    Display *find(const std::string &display);
    const Display *find(const std::string &display) const;
    int percent(const Display &display) const;
    void notifyIfChanged(const Display &display, int oldPercent);
    void emitBrightnessChanged();
    void emitDisplayBrightnessChanged(const std::string &display, int value);

    // The brightness files of the backlights in use, through inotify.
    void watchFiles();
    void readInotify();
    void handleFileChanged(const std::string &path);
    void handleDdcProbed(uint64_t generation, const std::vector<DdcDisplay> &displays);

    Lingmo::Base::DBus::Connection *m_session;
    Lingmo::Base::DBus::Connection *m_system;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_object;
    Lingmo::Base::DBus::Match m_displayStateChanged;
    int m_inotify = -1;
    int m_inotifyWatch = 0;
    // Watch descriptors by file.
    std::map<std::string, int> m_watches;
    Lingmo::Base::Timer m_rescanTimer;
    DdcWorker m_ddcWorker;
    uint64_t m_generation = 0;
    std::vector<Display> m_displays;
};

