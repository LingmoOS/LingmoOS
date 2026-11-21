#ifndef LINGMO_RECORDER_AUDIOCAPTURE_H
#define LINGMO_RECORDER_AUDIOCAPTURE_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <pipewire/pipewire.h>

// The microphone as interleaved float stereo at the encoder's rate; onSamples
// is called on PipeWire's thread.
class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();

    AudioCapture(const AudioCapture &) = delete;
    AudioCapture &operator=(const AudioCapture &) = delete;

    //! @p target is a node name; empty records the default input.
    bool start(const std::string &target, std::string *error);
    void stop();

    //! @p capturedNs is when the first sample was taken, on CLOCK_MONOTONIC.
    //! Set before start().
    std::function<void(std::vector<float> interleaved, int64_t capturedNs)> onSamples;

private:
    static const pw_stream_events s_events;
    void process();

    pw_thread_loop *m_loop = nullptr;
    pw_stream *m_stream = nullptr;
};

#endif // LINGMO_RECORDER_AUDIOCAPTURE_H
