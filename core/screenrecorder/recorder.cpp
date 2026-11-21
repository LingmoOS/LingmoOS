#include "recorder.h"
#include "audiocapture.h"
#include "capture_client.h"
#include "encoder.h"
#include "monotonic.h"

#include "base/event_loop.h"

#include <cstdio>
#include <filesystem>

#include <sys/epoll.h>

#include <wayland-client.h>

using Lingmo::Base::Rect;
using Lingmo::Base::RectF;
using Lingmo::Base::Size;

// Buffer ids travel to the encoder with the session they belong to.
static constexpr int kSessionStride = 1 << 16;

Recorder::Recorder(const Options &options, Lingmo::Base::EventLoop &loop)
    : m_options(options)
    , m_loop(loop)
{
}

Recorder::~Recorder()
{
    if (m_audio)
        m_audio->stop();
    m_encoderThread.stop();
    m_encoder.reset();
    // Replaced sessions may still be on their way back from the encoder.
    m_replaced.clear();
    m_session.reset();
    if (m_prepareHook)
        m_loop.removePrepareHook(m_prepareHook);
    if (m_displayWatch)
        m_loop.unwatch(m_displayWatch);
    m_client.reset();
    if (m_display)
        wl_display_disconnect(m_display);
}

void Recorder::toMain(std::function<void()> task)
{
    m_loop.post([lifetime = m_lifetime, task = std::move(task)] {
        if (lifetime.lock())
            task();
    });
}

bool Recorder::start(std::string *error)
{
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        *error = "cannot connect to the compositor";
        return false;
    }
    m_client = std::make_unique<CaptureClient>(m_display);
    if (!m_client->initialize()) {
        *error = "the compositor does not let this program capture the screen";
        return false;
    }

    if (m_options.window.empty()) {
        if (const CaptureClient::Output *output = m_client->outputNamed(m_options.output))
            m_outputGeometry = output->logicalGeometry;
    }

    m_displayWatch = m_loop.watch(wl_display_get_fd(m_display), EPOLLIN, [this](uint32_t) { dispatch(); });
    // Events a round trip already read sit in the queue without the socket
    // becoming readable again.
    m_prepareHook = m_loop.addPrepareHook([this] {
        wl_display_dispatch_pending(m_display);
        wl_display_flush(m_display);
    });

    m_encoder = std::make_unique<Encoder>();
    m_encoder->onFrameDone = [this](int token) {
        toMain([this, token] {
            // A buffer of a session replaced since is already gone.
            if (token >= 0 && m_session && token / kSessionStride == m_generation)
                m_session->releaseBuffer(token % kSessionStride);
            captureNext();
        });
    };
    m_encoder->onNeedsSharedMemory = [this] {
        toMain([this] {
            if (m_memory == CaptureSession::Memory::DmaBuf)
                startSession(CaptureSession::Memory::Shm);
        });
    };
    m_encoder->onStarted = [this] {
        toMain([this] {
            if (onStarted)
                onStarted();
        });
    };
    m_encoder->onFinished = [this](bool ok, const std::string &error) {
        toMain([this, ok, error] { onEncoderFinished(ok, error); });
    };
    m_encoderThread.start();
    m_encoderThread.post([encoder = m_encoder.get(), path = partialPath(), audio = m_options.microphone] {
        encoder->open(path, audio);
    });

    // Frames on the GPU first; the encoder asks for shared memory without it.
    if (!startSession(CaptureSession::Memory::DmaBuf)) {
        *error = "there is nothing to record";
        return false;
    }

    if (m_options.microphone) {
        m_audio = std::make_unique<AudioCapture>();
        m_audio->onSamples = [this](std::vector<float> samples, int64_t capturedNs) {
            m_encoderThread.post([encoder = m_encoder.get(), samples = std::move(samples), capturedNs] {
                encoder->encodeAudio(samples, capturedNs);
            });
        };
        std::string audioError;
        if (!m_audio->start(m_options.microphoneTarget, &audioError))
            fprintf(stderr, "lingmo-screenrecorder: %s\n", audioError.c_str());
    }
    return true;
}

bool Recorder::startSession(CaptureSession::Memory memory)
{
    ext_image_capture_source_v1 *source = m_options.window.empty()
            ? m_client->createOutputSource(m_options.output)
            : m_client->createWindowSource(m_options.window);
    if (!source)
        return false;

    if (m_session) {
        CaptureSession *replaced = m_session.get();
        replaced->onConstraintsChanged = nullptr;
        replaced->onFrameReady = nullptr;
        replaced->onFrameFailed = nullptr;
        replaced->onStopped = nullptr;
        m_replaced.push_back(std::move(m_session));
        // Its DMA-BUF fds close with it, and encodeDmaBuf calls for its
        // buffers may still be queued: the deletion goes through the encoder's
        // queue and back, so it lands after every one of them.
        m_encoderThread.post([this, replaced] {
            toMain([this, replaced] {
                std::erase_if(m_replaced, [replaced](const auto &session) { return session.get() == replaced; });
            });
        });
    }
    ++m_generation;
    m_memory = memory;
    // Three buffers: one being captured, one being encoded and one to spare.
    m_session = std::make_unique<CaptureSession>(m_client.get(), source, m_options.cursor, 3, memory);
    m_session->onConstraintsChanged = [this] {
        // New buffers replace the old ones, which the encoder may still have
        // imported; queued behind the frames it has yet to encode from them.
        m_encoderThread.post([encoder = m_encoder.get()] { encoder->forgetBuffers(); });
        captureNext();
    };
    m_session->onFrameReady = [this](const CaptureSession::Frame &frame) { onFrame(frame); };
    m_session->onFrameFailed = [this] { toMain([this] { captureNext(); }); };
    // A window that closes ends its recording with what there is.
    m_session->onStopped = [this] { stop(); };
    return true;
}

void Recorder::stop()
{
    if (m_stopping || m_done)
        return;
    m_stopping = true;
    const int64_t stopNs = monotonicNs();
    if (m_audio)
        m_audio->stop();
    m_encoderThread.post([encoder = m_encoder.get(), stopNs] { encoder->finish(stopNs); });
}

void Recorder::dispatch()
{
    if (wl_display_dispatch(m_display) < 0) {
        m_loop.unwatch(m_displayWatch);
        m_displayWatch = 0;
        stop();
        return;
    }
    wl_display_flush(m_display);
}

void Recorder::captureNext()
{
    if (!m_stopping && m_session)
        m_session->capture();
}

void Recorder::onFrame(const CaptureSession::Frame &frame)
{
    if (m_stopping) {
        m_session->releaseBuffer(frame.buffer);
        return;
    }

    const int token = m_generation * kSessionStride + frame.buffer;
    if (frame.dmabuf.isValid()) {
        // The GPU path takes the pixels as they are; a rotated output goes
        // through shared memory, where the copy can be turned upright.
        if (frame.transform != 0) {
            m_session->releaseBuffer(frame.buffer);
            startSession(CaptureSession::Memory::Shm);
            return;
        }
        m_encoderThread.post([encoder = m_encoder.get(), dmabuf = frame.dmabuf, node = m_session->renderNode(),
                              crop = cropFor(frame.dmabuf.size), presentedNs = frame.presentedNs, token] {
            encoder->encodeDmaBuf(dmabuf, node, crop, presentedNs, token);
        });
        captureNext();
        return;
    }

    // A rotated output gets an upright copy, but its buffer stays held until the
    // encoder is done, so the pool still bounds the frames in flight.
    const Image image = upright(frame.image, frame.transform);
    m_encoderThread.post([encoder = m_encoder.get(), image, crop = cropFor(image.size()),
                          presentedNs = frame.presentedNs, token] {
        encoder->encodeVideo(image, crop, presentedNs, token);
    });
    // The other buffers are free while this one is encoded.
    captureNext();
}

Rect Recorder::cropFor(const Size &frame) const
{
    const Rect whole(frame);
    if (!m_options.window.empty() || m_options.region.isEmpty() || m_outputGeometry.isEmpty())
        return whole;
    const double scale = double(frame.width()) / m_outputGeometry.width();
    const Rect logical = m_options.region.translated(-m_outputGeometry.x(), -m_outputGeometry.y());
    return RectF(logical.x() * scale, logical.y() * scale, logical.width() * scale, logical.height() * scale)
                   .toEnclosingRect()
            & whole;
}

void Recorder::onEncoderFinished(bool ok, const std::string &error)
{
    if (m_done)
        return;
    m_done = true;
    m_stopping = true;
    if (m_audio)
        m_audio->stop();

    std::string message = error;
    std::error_code ignored;
    if (ok) {
        std::filesystem::remove(m_options.path, ignored);
        std::error_code renamed;
        std::filesystem::rename(partialPath(), m_options.path, renamed);
        if (renamed) {
            ok = false;
            message = "cannot name the recording " + m_options.path;
        }
    }
    if (!ok)
        std::filesystem::remove(partialPath(), ignored);
    if (onFinished)
        onFinished(ok, message);
}

std::string Recorder::partialPath() const
{
    const std::filesystem::path path(m_options.path);
    const std::filesystem::path directory = path.has_parent_path() ? path.parent_path() : ".";
    return (directory / ("." + path.filename().string() + ".part")).string();
}
