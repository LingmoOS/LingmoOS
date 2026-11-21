// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/event_loop.h"
#include "base/signal.h"
#include "base/weak_ptr.h"
#include "ui/image.h"

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

struct mpv_handle;

// Seek-bar previews: frames of a local file decoded by a second, silent mpv
// on a thread of its own, so the player itself never seeks. Frames are taken
// at keyframes in steps of about 1/150 of the length, and kept per file.
// It also takes the cover media controls show for the file.
class Thumbnailer
{
public:
    // Covers go in coverFolder; empty means the player's cache folder.
    explicit Thumbnailer(std::string coverFolder = {});
    ~Thumbnailer();

    Thumbnailer(const Thumbnailer&) = delete;
    Thumbnailer& operator=(const Thumbnailer&) = delete;

    // A URL.
    const std::string& source() const { return m_source; }
    void setSource(const std::string& source);
    double duration() const { return m_duration; }
    void setDuration(double duration);
    // Bumped whenever frames are added or dropped.
    int revision() const { return m_revision; }
    // A file URL of a square frame from a tenth of the way in, or of an
    // audio file's embedded picture; empty until taken.
    const std::string& cover() const { return m_cover; }

    // The step a time falls in, and the time its frame shows.
    int bucket(double seconds) const;
    double bucketTime(int bucket) const;
    bool has(int bucket) const;
    // Asks for the frame of bucket; only the latest request is worked on.
    void request(int bucket);
    // Opaque, 240 px wide; null when not taken yet.
    Lingmo::ui::Image image(int bucket) const;

    static std::string defaultCoverFolder();

    Lingmo::base::Signal<> sourceChanged;
    Lingmo::base::Signal<> durationChanged;
    Lingmo::base::Signal<> revisionChanged;
    Lingmo::base::Signal<> coverChanged;

private:
    void run();
    void addFrame(int bucket, Lingmo::ui::Image image, uint64_t generation, double step);
    // Asks the worker for the cover once the file and its length are known.
    void askCover();
    void addCover(const Lingmo::ui::Image& image, uint64_t generation);
    void setCover(const std::string& cover);
    // Whether the worker should drop what it is doing: quitting or a new file.
    bool cancelled(uint64_t generation) const;
    double step() const;

    std::string m_coverFolder;
    std::string m_source;
    double m_duration = 0;
    int m_revision = 0;
    std::string m_cover;
    bool m_coverAsked = false;
    bool m_staleCoversRemoved = false;

    Lingmo::base::LoopHandle m_loop;
    std::thread m_thread;
    mutable std::mutex m_mutex;
    std::condition_variable m_wake;
    // Guarded by m_mutex.
    bool m_stopping = false;
    std::string m_file;
    double m_sharedDuration = 0;
    uint64_t m_generation = 0;
    double m_step = 1;
    int m_pending = -1;
    bool m_coverWanted = false;
    std::unordered_map<int, Lingmo::ui::Image> m_frames;
    // The worker's mpv, woken to cut a wait short.
    mpv_handle* m_activeMpv = nullptr;

    Lingmo::base::WeakPtr<Thumbnailer> m_weak;
    Lingmo::base::WeakPtrFactory<Thumbnailer> m_weakFactory{this};
};
