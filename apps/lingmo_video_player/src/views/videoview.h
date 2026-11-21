// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "base/weak_ptr.h"
#include "ui/view.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

class MainWindow;
class Player;
class SoftwareVideo;
struct MpvHandle;
struct mpv_render_context;

namespace Lingmo::ui {
class Subsurface;
}

// mpv's OpenGL output on a subsurface below the window, drawn by a thread of
// its own: desynchronised while playing, synchronised while the window
// changes size so the picture and the window land in one commit.
class VideoSurface
{
public:
    VideoSurface(Player& player, Lingmo::ui::Window* window);
    // Stops the thread; the player hears its picture is gone.
    ~VideoSurface();

    bool isValid() const;
    void setGeometry(const Lingmo::ui::RectF& rect);

    // On the window's loop: the thread could not set up mpv's OpenGL output.
    std::function<void()> onFailed;

private:
    void run();
    void releaseOutput(mpv_render_context* context);
    void stopThread();
    void fail();

    Player& m_player;
    Lingmo::base::WeakPtr<Player> m_playerWeak;
    std::shared_ptr<MpvHandle> m_handle;
    std::unique_ptr<Lingmo::ui::Subsurface> m_surface;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_wake;
    bool m_stopping = false;
    bool m_dirty = true;
    std::atomic<bool> m_contextReady = false;
    // Taken on the window's loop for the thread to hand back.
    Lingmo::base::WeakPtr<VideoSurface> m_selfWeak;
    Lingmo::base::WeakPtrFactory<VideoSurface> m_weakFactory{this};
};

// The video under everything else: on Wayland a hole the subsurface shows
// through, headless the software picture.
class VideoView : public Lingmo::ui::View
{
public:
    explicit VideoView(MainWindow& owner);
    ~VideoView() override;

    // mpv started: make the output.
    void attach();
    bool usesSurface() const { return m_surface != nullptr; }
    // The output, to outlive the window it shows in.
    std::unique_ptr<VideoSurface> takeSurface();

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    Lingmo::ui::RectF windowRect() const;
    void attachSoftware();

    MainWindow& m_owner;
    std::unique_ptr<VideoSurface> m_surface;
    std::unique_ptr<SoftwareVideo> m_software;
    Lingmo::base::Subscription m_frames;
};
