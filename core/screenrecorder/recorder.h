#ifndef LINGMO_RECORDER_RECORDER_H
#define LINGMO_RECORDER_RECORDER_H

#include "capture_session.h"
#include "task_thread.h"

#include "base/geometry.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::base {
class EventLoop;
}

class AudioCapture;
class CaptureClient;
class Encoder;
struct wl_display;

// One recording until stop(), written under a hidden name next to the file and
// renamed once it is complete. The encoder works on a thread of its own.
class Recorder
{
public:
    struct Options {
        //! The output to record, by wl_output name.
        std::string output;
        //! Part of that output in global logical coordinates; empty for all of it.
        Lingmo::Base::Rect region;
        //! Or a window, by its ext_foreign_toplevel_handle_v1 identifier.
        std::string window;
        bool cursor = false;
        bool microphone = false;
        //! A PipeWire node name; empty for the default input.
        std::string microphoneTarget;
        std::string path;
    };

    Recorder(const Options &options, Lingmo::Base::EventLoop &loop);
    ~Recorder();

    Recorder(const Recorder &) = delete;
    Recorder &operator=(const Recorder &) = delete;

    bool start(std::string *error);
    void stop();

    std::function<void()> onStarted;
    std::function<void(bool ok, const std::string &error)> onFinished;

private:
    //! A session for the source, replacing the one there is.
    bool startSession(CaptureSession::Memory memory);
    void dispatch();
    void captureNext();
    //! The part of a frame of this size that is recorded.
    Lingmo::Base::Rect cropFor(const Lingmo::Base::Size &frame) const;
    void onFrame(const CaptureSession::Frame &frame);
    void onEncoderFinished(bool ok, const std::string &error);
    std::string partialPath() const;
    //! Runs @p task on this thread's loop, unless the recorder is gone by then.
    void toMain(std::function<void()> task);

    Options m_options;
    Lingmo::Base::EventLoop &m_loop;
    std::shared_ptr<int> m_alive = std::make_shared<int>(0);
    const std::weak_ptr<int> m_lifetime = m_alive;
    wl_display *m_display = nullptr;
    int m_displayWatch = 0;
    int m_prepareHook = 0;
    std::unique_ptr<CaptureClient> m_client;
    std::unique_ptr<CaptureSession> m_session;
    //! Replaced sessions, until the encoder is done with their buffers.
    std::vector<std::unique_ptr<CaptureSession>> m_replaced;
    CaptureSession::Memory m_memory = CaptureSession::Memory::DmaBuf;
    int m_generation = 0;
    std::unique_ptr<AudioCapture> m_audio;
    TaskThread m_encoderThread;
    std::unique_ptr<Encoder> m_encoder;
    Lingmo::Base::Rect m_outputGeometry;
    bool m_stopping = false;
    bool m_done = false;
};

#endif // LINGMO_RECORDER_RECORDER_H
