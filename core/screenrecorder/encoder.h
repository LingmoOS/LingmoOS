#ifndef LINGMO_RECORDER_ENCODER_H
#define LINGMO_RECORDER_ENCODER_H

#include "image.h"

#include "base/geometry.h"

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct AVAudioFifo;
struct AVBufferRef;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVStream;
struct SwsContext;

// H.264 (VA-API, else x264) and AAC into an MP4. Every call runs on the
// caller's thread, and so do the callbacks. Frames keep the compositor's
// times; later sizes are letterboxed into the first one. DMA-BUF frames stay
// on the GPU: VA-API crops and converts them for the encoder.
class Encoder
{
public:
    static constexpr int kSampleRate = 48000;
    static constexpr int kChannels = 2;

    Encoder();
    ~Encoder();

    Encoder(const Encoder &) = delete;
    Encoder &operator=(const Encoder &) = delete;

    //! Writes to @p path. The video's size is fixed by the first frame.
    void open(const std::string &path, bool audio);

    //! @p crop of @p image, presented at @p presentedNs on CLOCK_MONOTONIC.
    //! @p buffer goes back through onFrameDone once the pixels are read.
    void encodeVideo(const Image &image, const Lingmo::Base::Rect &crop, int64_t presentedNs, int buffer);
    //! The same from a DMA-BUF on the render node @p node. onNeedsSharedMemory
    //! when the GPU cannot take it.
    void encodeDmaBuf(const DmaBuf &dmabuf, const std::string &node, const Lingmo::Base::Rect &crop,
                      int64_t presentedNs, int buffer);
    //! The capture buffers handed over so far are replaced, after a size change
    //! or by a new session; their GPU imports go with them.
    void forgetBuffers();

    //! Interleaved samples whose first one was captured at @p capturedNs.
    void encodeAudio(const std::vector<float> &samples, int64_t capturedNs);

    //! Ends the video at @p stopNs and completes the file.
    void finish(int64_t stopNs);

    std::function<void(int buffer)> onFrameDone;
    //! Frames have to come in shared memory from now on.
    std::function<void()> onNeedsSharedMemory;
    //! The first frame is in the file.
    std::function<void()> onStarted;
    std::function<void(bool ok, const std::string &error)> onFinished;

private:
    friend class ScreenRecorderEncoderTest;

    bool openVideo(const Lingmo::Base::Size &size, const std::string &node, std::string *error);
    bool startVideo(std::string *error);
    void closeVideo();
    bool openVaapi(const Lingmo::Base::Size &size, const std::string &node);
    bool openVpp();
    unsigned int importSurface(const DmaBuf &dmabuf, int buffer);
    bool convertDmaBuf(const DmaBuf &dmabuf, int buffer, const Lingmo::Base::Rect &crop);
    //! @p source trimmed to the video, and where in the frame it goes (m_fit).
    Lingmo::Base::Rect fit(Lingmo::Base::Rect source);
    bool sendPending(int64_t presentedNs);
    void accepted(int64_t presentedNs);
    bool openX264(const Lingmo::Base::Size &size);
    bool openAudio(std::string *error);
    bool writeHeader(std::string *error);
    bool convert(const Image &image, const Lingmo::Base::Rect &crop);
    bool sendVideo(int64_t pts, int64_t duration);
    bool queueSilence(int64_t frames);
    bool drainAudio(bool flush);
    bool writePackets(AVCodecContext *codec, AVStream *stream);
    void queueAudio(const std::vector<float> &samples, int64_t capturedNs);
    void frameDone(int buffer);
    void fail(const std::string &error);
    void close();

    std::string m_path;
    bool m_failed = false;

    AVFormatContext *m_format = nullptr;
    AVCodecContext *m_video = nullptr;
    AVCodecContext *m_audio = nullptr;
    AVStream *m_videoStream = nullptr;
    AVStream *m_audioStream = nullptr;
    AVBufferRef *m_hwDevice = nullptr;
    AVFrame *m_frame = nullptr;
    AVFrame *m_hwFrame = nullptr;
    AVFrame *m_audioFrame = nullptr;
    //! A converted GPU frame waiting for its duration.
    AVFrame *m_pendingHw = nullptr;
    void *m_display = nullptr;
    unsigned int m_vppConfig = 0;
    unsigned int m_vppContext = 0;
    //! Capture buffer id -> the VA surface it was imported as.
    std::map<int, unsigned int> m_imported;
    bool m_fitChanged = false;
    AVAudioFifo *m_fifo = nullptr;
    SwsContext *m_sws = nullptr;
    int m_swFormat = -1;
    Lingmo::Base::Size m_size;
    Lingmo::Base::Rect m_fit;
    Lingmo::Base::Size m_scaledFrom;

    //! The first frame's presentation time; everything is relative to it.
    int64_t m_originNs = -1;
    //! The frame converted into m_frame, sent once the next one says how
    //! long it was on screen.
    int64_t m_pendingPts = -1;
    std::unordered_map<int64_t, int64_t> m_durations;
    int64_t m_audioSamples = 0;
    bool m_audioAligned = false;
    //! Audio that arrived before the first frame fixed the origin.
    std::deque<std::pair<std::vector<float>, int64_t>> m_earlyAudio;
};

#endif // LINGMO_RECORDER_ENCODER_H
