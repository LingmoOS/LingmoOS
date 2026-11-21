// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/thread_pool.h"
#include "shell/common/pending_calls.h"
#include "ui/bitmap.h"

namespace Lingmo::Shell {

// What a capture tells the user: the
// shutter sound, and a notification of the file saved or the picture copied,
// under the Screenshot application, with Open and Show in Folder.
class CaptureNotifier
{
public:
    explicit CaptureNotifier(base::dbus::Connection *bus);
    ~CaptureNotifier();

    // Ask the session sound service to play the screen-capture event.
    void playShutter();
    void notifySaved(const std::string &path, bool video);
    // A copy has no file to name or show; its picture says what was taken.
    void notifyCopied(const ui::Bitmap &image);
    // A picture another program asked for and saves itself.
    void notifyTaken(const ui::Bitmap &image);
    void notifyFailed(const std::string &summary, const std::string &reason);

    // Tests: what playShutter() sends instead of the service.
    std::function<void(const std::string &event)> playSound;

    // The thumbnail a copied picture is shown with: at least 132 px both
    // ways, as the notification centre crops it, unless the picture is
    // smaller.
    static ui::Bitmap thumbnail(const ui::Bitmap &image);

private:
    void notify(const std::string &summary, const std::string &body, const std::string &image,
                const std::string &path, const ui::Bitmap &thumbnail = {}, bool suppressSound = true);
    void notifyPicture(const std::string &body, const ui::Bitmap &image);
    void actionInvoked(uint32_t id, const std::string &action);

    base::dbus::Connection *m_bus;
    base::dbus::Proxy m_notifications;
    base::dbus::Match m_actionInvoked;
    base::dbus::Match m_closed;
    // The files of the notifications still up, by id.
    std::map<uint32_t, std::string> m_paths;
    std::vector<base::Job> m_thumbnails;
    PendingCalls m_calls;
};

} // namespace Lingmo::Shell

