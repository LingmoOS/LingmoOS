#include "encoder.h"

#include "base/file.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <filesystem>
#include <vector>

#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>
#include <va/va.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

using Lingmo::base::Rect;
using Lingmo::base::Size;

static constexpr int64_t kOriginNs = 1'000'000'000'000;
static constexpr int64_t kFrameNs = 33'333'333;
static constexpr char kRenderNode[] = "/dev/dri/renderD128";

static void fill(Image &image, uint32_t argb)
{
    for (int y = 0; y < image.height; ++y) {
        auto *row = reinterpret_cast<uint32_t *>(image.mutableBits() + size_t(y) * image.stride);
        std::fill(row, row + image.width, argb);
    }
}

static Image frame(const Size &size, int index)
{
    Image image = Image::allocate(size.width(), size.height());
    // A different opaque colour for every frame.
    const auto channel = [index](int shift) { return uint32_t((index * 37 + shift) % 200 + 40); };
    fill(image, 0xff000000 | channel(0) << 16 | channel(70) << 8 | channel(140));
    return image;
}

static std::vector<float> tone(int frames, int offset)
{
    std::vector<float> samples(size_t(frames) * Encoder::kChannels);
    for (int i = 0; i < frames; ++i) {
        const float value = 0.3f * std::sin(2.0f * float(M_PI) * 440.0f * float(offset + i) / Encoder::kSampleRate);
        samples[size_t(i) * 2] = value;
        samples[size_t(i) * 2 + 1] = value;
    }
    return samples;
}

namespace {

struct Probe {
    int videoStreams = 0;
    int audioStreams = 0;
    Size size;
    int sampleRate = 0;
    double duration = 0;
    double audioDuration = 0;
    AVCodecID videoCodec = AV_CODEC_ID_NONE;
    AVCodecID audioCodec = AV_CODEC_ID_NONE;
};

struct Luma {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;

    int at(int x, int y) const { return pixels[size_t(y) * width + x]; }
};

} // namespace

static Probe probe(const std::string &path)
{
    Probe result;
    AVFormatContext *format = nullptr;
    if (avformat_open_input(&format, path.c_str(), nullptr, nullptr) < 0)
        return result;
    avformat_find_stream_info(format, nullptr);
    for (unsigned i = 0; i < format->nb_streams; ++i) {
        const AVCodecParameters *codec = format->streams[i]->codecpar;
        if (codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            ++result.videoStreams;
            result.size = Size(codec->width, codec->height);
            result.videoCodec = codec->codec_id;
        } else if (codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            ++result.audioStreams;
            result.sampleRate = codec->sample_rate;
            result.audioCodec = codec->codec_id;
            result.audioDuration = double(format->streams[i]->duration) * av_q2d(format->streams[i]->time_base);
        }
    }
    result.duration = double(format->duration) / AV_TIME_BASE;
    avformat_close_input(&format);
    return result;
}

// The luma plane of the last frame in the file.
static Luma lastLuma(const std::string &path)
{
    Luma result;
    AVFormatContext *format = nullptr;
    if (avformat_open_input(&format, path.c_str(), nullptr, nullptr) < 0)
        return result;
    avformat_find_stream_info(format, nullptr);
    const int stream = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    const AVCodec *decoder = avcodec_find_decoder(format->streams[stream]->codecpar->codec_id);
    AVCodecContext *context = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(context, format->streams[stream]->codecpar);
    avcodec_open2(context, decoder, nullptr);
    AVPacket *packet = av_packet_alloc();
    AVFrame *decoded = av_frame_alloc();
    const auto take = [&] {
        while (avcodec_receive_frame(context, decoded) >= 0) {
            result.width = decoded->width;
            result.height = decoded->height;
            result.pixels.resize(size_t(decoded->width) * decoded->height);
            for (int y = 0; y < decoded->height; ++y)
                memcpy(result.pixels.data() + size_t(y) * decoded->width,
                       decoded->data[0] + y * decoded->linesize[0], size_t(decoded->width));
        }
    };
    while (av_read_frame(format, packet) >= 0) {
        if (packet->stream_index == stream && avcodec_send_packet(context, packet) >= 0)
            take();
        av_packet_unref(packet);
    }
    avcodec_send_packet(context, nullptr);
    take();
    av_frame_free(&decoded);
    av_packet_free(&packet);
    avcodec_free_context(&context);
    avformat_close_input(&format);
    return result;
}

namespace {

// DMA-BUFs on a render node, allocated the way the capture session does.
class RenderNode
{
public:
    RenderNode()
        : m_fd(open(kRenderNode, O_RDWR | O_CLOEXEC))
    {
        if (m_fd >= 0)
            m_gbm = gbm_create_device(m_fd);
    }

    ~RenderNode()
    {
        freeBuffers();
        if (m_gbm)
            gbm_device_destroy(m_gbm);
        if (m_fd >= 0)
            close(m_fd);
    }

    bool isValid() const { return m_gbm != nullptr; }

    DmaBuf allocate(const Size &size)
    {
        const auto width = uint32_t(size.width());
        const auto height = uint32_t(size.height());
        const uint64_t linear = DRM_FORMAT_MOD_LINEAR;
        gbm_bo *bo = gbm_bo_create_with_modifiers2(m_gbm, width, height, DRM_FORMAT_XRGB8888, &linear, 1,
                                                   GBM_BO_USE_RENDERING);
        if (!bo)
            bo = gbm_bo_create(m_gbm, width, height, DRM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
        DmaBuf dmabuf;
        if (!bo)
            return dmabuf;
        dmabuf.fd = gbm_bo_get_fd(bo);
        dmabuf.format = DRM_FORMAT_XRGB8888;
        dmabuf.modifier = gbm_bo_get_modifier(bo);
        dmabuf.offset = gbm_bo_get_offset(bo, 0);
        dmabuf.stride = gbm_bo_get_stride_for_plane(bo, 0);
        dmabuf.size = size;
        m_buffers.emplace_back(bo, dmabuf.fd);
        return dmabuf;
    }

    // What the session does with its buffers when the size changes.
    void freeBuffers()
    {
        for (const auto &[bo, fd] : m_buffers) {
            if (fd >= 0)
                close(fd);
            gbm_bo_destroy(bo);
        }
        m_buffers.clear();
    }

private:
    int m_fd = -1;
    gbm_device *m_gbm = nullptr;
    std::vector<std::pair<gbm_bo *, int>> m_buffers;
};

// What the encoder reported through its callbacks.
struct Reports {
    int started = 0;
    std::vector<int> done;
    int needsSharedMemory = 0;
    std::vector<std::pair<bool, std::string>> finished;

    void watch(Encoder &encoder)
    {
        encoder.onStarted = [this] { ++started; };
        encoder.onFrameDone = [this](int buffer) { done.push_back(buffer); };
        encoder.onNeedsSharedMemory = [this] { ++needsSharedMemory; };
        encoder.onFinished = [this](bool ok, const std::string &error) { finished.emplace_back(ok, error); };
    }
};

} // namespace

class ScreenRecorderEncoderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_directory.isValid());
    }

    std::string filePath(const std::string &name) const { return m_directory.path() + '/' + name; }

    static const std::map<int, unsigned int> &imported(const Encoder &encoder) { return encoder.m_imported; }
    static void *display(const Encoder &encoder) { return encoder.m_display; }

    Lingmo::base::file::TemporaryDirectory m_directory{std::filesystem::temp_directory_path().string(),
                                                         "recorder-test-"};
};

TEST_F(ScreenRecorderEncoderTest, WritesVideoAndSoundUntilTheStop)
{
    const std::string path = filePath("recording.mp4");
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);

    encoder.open(path, true);
    // Sound from before the first frame is cut at it.
    for (int chunk = -5; chunk < 100; ++chunk)
        encoder.encodeAudio(tone(480, chunk * 480), kOriginNs + chunk * 10'000'000);
    for (int i = 0; i < 30; ++i) {
        const Image image = frame(Size(320, 240), i);
        encoder.encodeVideo(image, image.rect(), kOriginNs + i * kFrameNs, i);
    }
    // The screen stood still for half a second before the stop.
    encoder.finish(kOriginNs + 1'500'000'000);

    EXPECT_EQ(reports.started, 1);
    ASSERT_EQ(reports.done.size(), 30u);
    EXPECT_EQ(reports.done.back(), 29);
    ASSERT_EQ(reports.finished.size(), 1u);
    ASSERT_TRUE(reports.finished.front().first) << reports.finished.front().second;

    const Probe file = probe(path);
    EXPECT_EQ(file.videoStreams, 1);
    EXPECT_EQ(file.audioStreams, 1);
    EXPECT_EQ(file.videoCodec, AV_CODEC_ID_H264);
    EXPECT_EQ(file.audioCodec, AV_CODEC_ID_AAC);
    EXPECT_EQ(file.size, Size(320, 240));
    EXPECT_EQ(file.sampleRate, Encoder::kSampleRate);
    EXPECT_LT(std::abs(file.duration - 1.5), 0.1) << file.duration;
}

TEST_F(ScreenRecorderEncoderTest, SoundAfterAGapKeepsItsPlace)
{
    const std::string path = filePath("gap.mp4");
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);

    encoder.open(path, true);
    for (int i = 0; i < 45; ++i) {
        const Image image = frame(Size(320, 240), i);
        encoder.encodeVideo(image, image.rect(), kOriginNs + i * kFrameNs, i);
    }
    // The microphone stalled for half a second in the middle.
    for (int chunk = 0; chunk < 150; ++chunk) {
        if (chunk < 50 || chunk >= 100)
            encoder.encodeAudio(tone(480, chunk * 480), kOriginNs + chunk * 10'000'000);
    }
    encoder.finish(kOriginNs + 1'500'000'000);

    ASSERT_EQ(reports.finished.size(), 1u);
    ASSERT_TRUE(reports.finished.front().first) << reports.finished.front().second;
    const Probe file = probe(path);
    EXPECT_EQ(file.audioStreams, 1);
    EXPECT_LT(std::abs(file.audioDuration - 1.5), 0.05) << file.audioDuration;
}

TEST_F(ScreenRecorderEncoderTest, SoundAheadOfItsTimesIsTrimmed)
{
    const std::string path = filePath("ahead.mp4");
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);

    encoder.open(path, true);
    for (int i = 0; i < 45; ++i) {
        const Image image = frame(Size(320, 240), i);
        encoder.encodeVideo(image, image.rect(), kOriginNs + i * kFrameNs, i);
    }
    // Two seconds of samples captured within a second and a half.
    for (int chunk = 0; chunk < 200; ++chunk)
        encoder.encodeAudio(tone(480, chunk * 480), kOriginNs + chunk * 7'500'000);
    encoder.finish(kOriginNs + 1'500'000'000);

    ASSERT_EQ(reports.finished.size(), 1u);
    ASSERT_TRUE(reports.finished.front().first) << reports.finished.front().second;
    const Probe file = probe(path);
    EXPECT_EQ(file.audioStreams, 1);
    EXPECT_LT(file.audioDuration, 1.5 + 0.25) << file.audioDuration;
}

TEST_F(ScreenRecorderEncoderTest, LaterSizesAreScaledIntoTheFirst)
{
    const std::string path = filePath("window.mp4");
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);

    encoder.open(path, false);
    const Image first = frame(Size(320, 240), 0);
    encoder.encodeVideo(first, first.rect(), kOriginNs, -1);
    // The window became twice as wide: it is shown whole, with bars above and below.
    Image wide = Image::allocate(640, 240);
    fill(wide, 0xffffffff);
    encoder.encodeVideo(wide, wide.rect(), kOriginNs + kFrameNs, -1);
    encoder.finish(kOriginNs + 2 * kFrameNs);

    ASSERT_EQ(reports.finished.size(), 1u);
    ASSERT_TRUE(reports.finished.front().first) << reports.finished.front().second;
    EXPECT_EQ(probe(path).size, Size(320, 240));

    const Luma luma = lastLuma(path);
    ASSERT_EQ(luma.width, 320);
    ASSERT_EQ(luma.height, 240);
    EXPECT_LT(luma.at(160, 10), 40);
    EXPECT_LT(luma.at(160, 230), 40);
    EXPECT_GT(luma.at(160, 120), 200);
}

TEST_F(ScreenRecorderEncoderTest, OddSizesAreTrimmedToEven)
{
    const std::string path = filePath("portion.mp4");
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);

    encoder.open(path, false);
    const Image screen = frame(Size(400, 300), 3);
    encoder.encodeVideo(screen, Rect(11, 21, 201, 151), kOriginNs, -1);
    encoder.finish(kOriginNs + kFrameNs);

    ASSERT_EQ(reports.finished.size(), 1u);
    ASSERT_TRUE(reports.finished.front().first) << reports.finished.front().second;
    EXPECT_EQ(probe(path).size, Size(200, 150));
}

TEST_F(ScreenRecorderEncoderTest, ARecordingWithoutFramesFails)
{
    const std::string path = filePath("empty.mp4");
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);

    encoder.open(path, true);
    encoder.encodeAudio(tone(480, 0), kOriginNs);
    encoder.finish(kOriginNs + kFrameNs);

    ASSERT_EQ(reports.finished.size(), 1u);
    EXPECT_FALSE(reports.finished.front().first);
    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST_F(ScreenRecorderEncoderTest, ReplacedBuffersLoseTheirImports)
{
    RenderNode gpu;
    if (!gpu.isValid())
        GTEST_SKIP() << "no render node to allocate DMA-BUFs on";

    const std::string path = filePath("resized.mp4");
    const std::string node = kRenderNode;
    Encoder encoder;
    Reports reports;
    reports.watch(encoder);
    encoder.open(path, false);

    // Three buffers taking turns, the way a session hands them out.
    int64_t presentedNs = kOriginNs;
    const auto record = [&](const Size &size, int firstBuffer) {
        DmaBuf buffers[3];
        for (DmaBuf &buffer : buffers)
            buffer = gpu.allocate(size);
        for (int i = 0; i < 6; ++i) {
            encoder.encodeDmaBuf(buffers[i % 3], node, Rect(size), presentedNs, firstBuffer + i % 3);
            presentedNs += kFrameNs;
        }
        return buffers[0].isValid() && buffers[1].isValid() && buffers[2].isValid();
    };

    ASSERT_TRUE(record(Size(320, 240), 0));
    if (reports.needsSharedMemory)
        GTEST_SKIP() << "this device cannot convert DMA-BUF frames with VA-API";
    ASSERT_EQ(imported(encoder).size(), 3u);
    std::vector<unsigned int> surfaces;
    for (const auto &[buffer, surface] : imported(encoder))
        surfaces.push_back(surface);

    // The window was resized: the session frees its buffers for three new ones.
    gpu.freeBuffers();
    encoder.forgetBuffers();
    EXPECT_TRUE(imported(encoder).empty());
    for (unsigned int surface : surfaces) {
        VASurfaceStatus status = VASurfaceReady;
        EXPECT_NE(vaQuerySurfaceStatus(display(encoder), surface, &status), VA_STATUS_SUCCESS);
    }

    ASSERT_TRUE(record(Size(640, 240), 3));
    EXPECT_EQ(reports.needsSharedMemory, 0);
    EXPECT_EQ(imported(encoder).size(), 3u);
    encoder.finish(presentedNs);

    EXPECT_EQ(reports.done.size(), 12u);
    ASSERT_EQ(reports.finished.size(), 1u);
    ASSERT_TRUE(reports.finished.front().first) << reports.finished.front().second;
    EXPECT_EQ(probe(path).size, Size(320, 240));
}
