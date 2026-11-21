#include "encoder.h"

#include "base/logging.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include <drm_fourcc.h>
#include <unistd.h>
#include <va/va.h>
#include <va/va_drmcommon.h>
#include <va/va_vpp.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/channel_layout.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_vaapi.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

using Lingmo::Base::Point;
using Lingmo::Base::Rect;
using Lingmo::Base::Size;

static const Lingmo::Base::LogCategory cfRecorder("lingmo.recorder");

static constexpr AVRational kVideoTimeBase {1, 90000};
// H.264 levels and the GPU encoders stop around here.
static constexpr int kMaximumDimension = 4096;
// No more than this is held back waiting for the first frame.
static constexpr int64_t kEarlyAudioLimitNs = 5'000'000'000;
// Sound this many samples away from where its capture time puts it is moved
// back there: well above the jitter of the capture times.
static constexpr int64_t kAudioDriftLimit = Encoder::kSampleRate / 5;
static constexpr int kSilenceChunk = 4800;
// Constant quality: screen text stays sharp, and a still screen costs nothing.
static constexpr int kVaapiQp = 18;
static constexpr char kX264Crf[] = "18";
// Enough surfaces for what the encoder holds on to, the frame waiting for its
// duration and the one being converted.
static constexpr int kSurfacePool = 16;

static std::string avError(int code)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(code, buffer, sizeof(buffer));
    return buffer;
}

static int evenFloor(int value)
{
    return std::max(2, value & ~1);
}

static int64_t toVideoPts(int64_t ns)
{
    return av_rescale_q(ns, AVRational {1, 1000000000}, kVideoTimeBase);
}

// The size a video of this crop gets: even, and within what encoders take.
// The largest size of @p size's aspect ratio within @p bounds.
static Size scaledToFit(const Size &size, const Size &bounds)
{
    if (size.isEmpty())
        return bounds;
    const int64_t width = int64_t(bounds.height()) * size.width() / size.height();
    if (width <= bounds.width())
        return Size(int(width), bounds.height());
    return Size(bounds.width(), int(int64_t(bounds.width()) * size.height() / size.width()));
}

static Size videoSize(const Rect &crop)
{
    Size size(evenFloor(crop.width()), evenFloor(crop.height()));
    if (size.width() > kMaximumDimension || size.height() > kMaximumDimension) {
        size = scaledToFit(size, Size(kMaximumDimension, kMaximumDimension));
        size = Size(evenFloor(size.width()), evenFloor(size.height()));
    }
    return size;
}

static int64_t takeDuration(std::unordered_map<int64_t, int64_t> &durations, int64_t pts)
{
    const auto it = durations.find(pts);
    if (it == durations.end())
        return 0;
    const int64_t duration = it->second;
    durations.erase(it);
    return duration;
}

static AVCodecContext *videoContext(const AVCodec *codec, const Size &size, const AVFormatContext *format)
{
    AVCodecContext *context = avcodec_alloc_context3(codec);
    context->width = size.width();
    context->height = size.height();
    context->time_base = kVideoTimeBase;
    context->framerate = AVRational {60, 1};
    context->gop_size = 120;
    context->color_range = AVCOL_RANGE_MPEG;
    context->colorspace = AVCOL_SPC_BT709;
    context->color_primaries = AVCOL_PRI_BT709;
    context->color_trc = AVCOL_TRC_BT709;
    if (format->oformat->flags & AVFMT_GLOBALHEADER)
        context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    return context;
}

Encoder::Encoder() = default;

Encoder::~Encoder()
{
    close();
}

void Encoder::open(const std::string &path, bool audio)
{
    m_path = path;

    if (avformat_alloc_output_context2(&m_format, nullptr, "mp4", path.c_str()) < 0
        || !m_format) {
        fail("cannot create an MP4 file");
        return;
    }

    std::string error;
    if (audio && !openAudio(&error)) {
        // A recording without sound beats no recording.
        LM_WARNING(cfRecorder) << "recording without audio:" << error;
    }
}

void Encoder::encodeVideo(const Image &image, const Rect &crop, int64_t presentedNs, int buffer)
{
    if (m_failed || !m_format) {
        frameDone(buffer);
        return;
    }

    if (m_originNs < 0) {
        std::string error;
        if (!openVideo(videoSize(crop), std::string(), &error) || !startVideo(&error)) {
            frameDone(buffer);
            fail(error);
            return;
        }
        m_originNs = presentedNs;
    }

    if (!sendPending(presentedNs)) {
        frameDone(buffer);
        return;
    }
    const bool converted = convert(image, crop);
    frameDone(buffer);
    if (!converted) {
        fail("cannot convert a frame");
        return;
    }
    accepted(presentedNs);
}

void Encoder::encodeDmaBuf(const DmaBuf &dmabuf, const std::string &node, const Rect &crop,
                           int64_t presentedNs, int buffer)
{
    if (m_failed || !m_format) {
        frameDone(buffer);
        return;
    }

    if (m_originNs < 0) {
        // The GPU path is only tried once, on the first frame; without it the
        // frames have to come in shared memory.
        std::string error;
        if (!openVideo(videoSize(crop), node, &error) || !openVpp()
            || !convertDmaBuf(dmabuf, buffer, crop)) {
            closeVideo();
            frameDone(buffer);
            if (onNeedsSharedMemory)
                onNeedsSharedMemory();
            return;
        }
        frameDone(buffer);
        if (!startVideo(&error)) {
            fail(error);
            return;
        }
        m_originNs = presentedNs;
        accepted(presentedNs);
        return;
    }

    if (!m_vppContext || !sendPending(presentedNs)) {
        frameDone(buffer);
        if (!m_vppContext && onNeedsSharedMemory)
            onNeedsSharedMemory();
        return;
    }
    const bool converted = convertDmaBuf(dmabuf, buffer, crop);
    frameDone(buffer);
    if (!converted) {
        fail("cannot convert a frame on the GPU");
        return;
    }
    accepted(presentedNs);
}

void Encoder::forgetBuffers()
{
    // An import keeps its buffer's GPU memory alive after the session freed it.
    // convertDmaBuf waits for the GPU, so no import is in use between frames.
    for (auto &[buffer, surface] : m_imported)
        vaDestroySurfaces(m_display, &surface, 1);
    m_imported.clear();
}

bool Encoder::sendPending(int64_t presentedNs)
{
    // A frame lasts until the next one, so the previous frame goes out now.
    if (m_pendingPts < 0)
        return true;
    const int64_t pts = std::max(toVideoPts(presentedNs - m_originNs), m_pendingPts + 1);
    return sendVideo(m_pendingPts, pts - m_pendingPts);
}

void Encoder::accepted(int64_t presentedNs)
{
    const bool first = m_pendingPts < 0;
    m_pendingPts = first ? 0 : std::max(toVideoPts(presentedNs - m_originNs), m_pendingPts + 1);

    if (first) {
        if (onStarted)
            onStarted();
        // Whatever the microphone caught before this frame lines up with it now.
        while (!m_failed && !m_earlyAudio.empty()) {
            auto [samples, capturedNs] = std::move(m_earlyAudio.front());
            m_earlyAudio.pop_front();
            queueAudio(samples, capturedNs);
        }
    }
}

void Encoder::encodeAudio(const std::vector<float> &samples, int64_t capturedNs)
{
    if (m_failed || !m_audio)
        return;
    if (m_originNs < 0) {
        m_earlyAudio.emplace_back(samples, capturedNs);
        while (!m_earlyAudio.empty() && capturedNs - m_earlyAudio.front().second > kEarlyAudioLimitNs)
            m_earlyAudio.pop_front();
        return;
    }
    queueAudio(samples, capturedNs);
}

void Encoder::finish(int64_t stopNs)
{
    if (m_failed)
        return;
    if (m_pendingPts < 0) {
        fail("nothing was recorded");
        return;
    }

    // A still screen at the end is one long frame, until the stop.
    const int64_t endPts = std::max(toVideoPts(stopNs - m_originNs),
                               m_pendingPts + kVideoTimeBase.den / 60);
    if (!sendVideo(m_pendingPts, endPts - m_pendingPts))
        return;

    int result = avcodec_send_frame(m_video, nullptr);
    if (result < 0 || !writePackets(m_video, m_videoStream)) {
        fail("cannot finish the video: " + avError(result));
        return;
    }
    if (m_audio && m_audioAligned) {
        if (!drainAudio(true))
            return;
        result = avcodec_send_frame(m_audio, nullptr);
        if (result < 0 || !writePackets(m_audio, m_audioStream)) {
            fail("cannot finish the audio: " + avError(result));
            return;
        }
    }

    result = av_write_trailer(m_format);
    close();
    if (result < 0) {
        fail("cannot complete the file: " + avError(result));
        return;
    }
    if (onFinished)
        onFinished(true, std::string());
}

bool Encoder::openVideo(const Size &size, const std::string &node, std::string *error)
{
    m_size = size;
    // Frames on the GPU need the GPU's encoder, on the device they are on.
    if (!openVaapi(size, node) && (!node.empty() || !openX264(size))) {
        *error = "no H.264 encoder is available";
        return false;
    }
    return true;
}

bool Encoder::startVideo(std::string *error)
{
    m_videoStream = avformat_new_stream(m_format, nullptr);
    if (!m_videoStream) {
        *error = "cannot add the video stream";
        return false;
    }
    m_videoStream->time_base = m_video->time_base;
    avcodec_parameters_from_context(m_videoStream->codecpar, m_video);
    return writeHeader(error);
}

void Encoder::closeVideo()
{
    forgetBuffers();
    if (m_vppContext)
        vaDestroyContext(m_display, m_vppContext);
    if (m_vppConfig)
        vaDestroyConfig(m_display, m_vppConfig);
    m_vppContext = 0;
    m_vppConfig = 0;
    m_display = nullptr;
    av_frame_free(&m_pendingHw);
    av_frame_free(&m_frame);
    av_frame_free(&m_hwFrame);
    avcodec_free_context(&m_video);
    av_buffer_unref(&m_hwDevice);
    m_scaledFrom = Size();
}

bool Encoder::openVaapi(const Size &size, const std::string &node)
{
    const AVCodec *codec = avcodec_find_encoder_by_name("h264_vaapi");
    if (!codec
        || av_hwdevice_ctx_create(&m_hwDevice, AV_HWDEVICE_TYPE_VAAPI,
                                  node.empty() ? nullptr : node.c_str(), nullptr, 0)
                < 0)
        return false;

    AVBufferRef *frames = av_hwframe_ctx_alloc(m_hwDevice);
    auto *framesContext = reinterpret_cast<AVHWFramesContext *>(frames->data);
    framesContext->format = AV_PIX_FMT_VAAPI;
    framesContext->sw_format = AV_PIX_FMT_NV12;
    framesContext->width = size.width();
    framesContext->height = size.height();
    framesContext->initial_pool_size = kSurfacePool;
    if (av_hwframe_ctx_init(frames) < 0) {
        av_buffer_unref(&frames);
        av_buffer_unref(&m_hwDevice);
        return false;
    }

    m_video = videoContext(codec, size, m_format);
    m_video->pix_fmt = AV_PIX_FMT_VAAPI;
    m_video->hw_frames_ctx = frames;
    m_video->max_b_frames = 0;
    m_video->global_quality = kVaapiQp;
    av_opt_set(m_video->priv_data, "rc_mode", "CQP", 0);
    if (avcodec_open2(m_video, codec, nullptr) < 0) {
        avcodec_free_context(&m_video);
        av_buffer_unref(&m_hwDevice);
        return false;
    }
    m_swFormat = AV_PIX_FMT_NV12;
    const auto *hardware = reinterpret_cast<AVHWDeviceContext *>(m_hwDevice->data);
    m_display = static_cast<AVVAAPIDeviceContext *>(hardware->hwctx)->display;
    LM_INFO(cfRecorder).format("encoding with h264_vaapi at {}x{}", size.width(), size.height());
    return true;
}

bool Encoder::openVpp()
{
    if (!m_display)
        return false;
    if (vaCreateConfig(m_display, VAProfileNone, VAEntrypointVideoProc, nullptr, 0, &m_vppConfig)
        != VA_STATUS_SUCCESS) {
        m_vppConfig = 0;
        return false;
    }
    if (vaCreateContext(m_display, m_vppConfig, m_size.width(), m_size.height(), VA_PROGRESSIVE,
                        nullptr, 0, &m_vppContext)
        != VA_STATUS_SUCCESS) {
        m_vppContext = 0;
        return false;
    }
    LM_INFO(cfRecorder) << "converting frames on the GPU";
    return true;
}

unsigned int Encoder::importSurface(const DmaBuf &dmabuf, int buffer)
{
    if (const auto it = m_imported.find(buffer); it != m_imported.end())
        return it->second;

    VADRMPRIMESurfaceDescriptor descriptor = {};
    descriptor.fourcc = dmabuf.format == DRM_FORMAT_ARGB8888 ? VA_FOURCC_BGRA : VA_FOURCC_BGRX;
    descriptor.width = uint32_t(dmabuf.size.width());
    descriptor.height = uint32_t(dmabuf.size.height());
    descriptor.num_objects = 1;
    descriptor.objects[0].fd = dmabuf.fd;
    descriptor.objects[0].size = uint32_t(lseek(dmabuf.fd, 0, SEEK_END));
    descriptor.objects[0].drm_format_modifier = dmabuf.modifier;
    descriptor.num_layers = 1;
    descriptor.layers[0].drm_format = dmabuf.format;
    descriptor.layers[0].num_planes = 1;
    descriptor.layers[0].object_index[0] = 0;
    descriptor.layers[0].offset[0] = dmabuf.offset;
    descriptor.layers[0].pitch[0] = dmabuf.stride;

    VASurfaceAttrib attributes[2] = {};
    attributes[0].type = VASurfaceAttribMemoryType;
    attributes[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attributes[0].value.type = VAGenericValueTypeInteger;
    attributes[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2;
    attributes[1].type = VASurfaceAttribExternalBufferDescriptor;
    attributes[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attributes[1].value.type = VAGenericValueTypePointer;
    attributes[1].value.value.p = &descriptor;

    VASurfaceID surface = VA_INVALID_SURFACE;
    if (vaCreateSurfaces(m_display, VA_RT_FORMAT_RGB32, descriptor.width, descriptor.height,
                         &surface, 1, attributes, 2)
        != VA_STATUS_SUCCESS)
        return VA_INVALID_SURFACE;
    m_imported.emplace(buffer, surface);
    return surface;
}

bool Encoder::convertDmaBuf(const DmaBuf &dmabuf, int buffer, const Rect &crop)
{
    const VASurfaceID source = importSurface(dmabuf, buffer);
    if (source == VA_INVALID_SURFACE)
        return false;
    const Rect region = fit(crop & Rect(dmabuf.size));
    if (region.isEmpty())
        return false;

    m_pendingHw = av_frame_alloc();
    if (av_hwframe_get_buffer(m_video->hw_frames_ctx, m_pendingHw, 0) < 0)
        return false;
    const auto target = VASurfaceID(reinterpret_cast<uintptr_t>(m_pendingHw->data[3]));

    // The crop, scaled into the video's frame and converted to BT.709 video range.
    VARectangle input {int16_t(region.x()), int16_t(region.y()), uint16_t(region.width()),
                       uint16_t(region.height())};
    VARectangle output {int16_t(m_fit.x()), int16_t(m_fit.y()), uint16_t(m_fit.width()),
                        uint16_t(m_fit.height())};
    VAProcPipelineParameterBuffer parameters = {};
    parameters.surface = source;
    parameters.surface_region = &input;
    parameters.output_region = &output;
    parameters.output_background_color = 0xff000000;
    parameters.surface_color_standard = VAProcColorStandardBT709;
    parameters.output_color_standard = VAProcColorStandardBT709;
    parameters.input_color_properties.color_range = VA_SOURCE_RANGE_FULL;
    parameters.output_color_properties.color_range = VA_SOURCE_RANGE_REDUCED;
    parameters.filter_flags = VA_FILTER_SCALING_HQ;

    VABufferID parameterBuffer = VA_INVALID_ID;
    if (vaCreateBuffer(m_display, m_vppContext, VAProcPipelineParameterBufferType,
                       sizeof(parameters), 1, &parameters, &parameterBuffer)
        != VA_STATUS_SUCCESS)
        return false;
    VAStatus status = vaBeginPicture(m_display, m_vppContext, target);
    if (status == VA_STATUS_SUCCESS)
        status = vaRenderPicture(m_display, m_vppContext, &parameterBuffer, 1);
    const VAStatus ended = vaEndPicture(m_display, m_vppContext);
    vaDestroyBuffer(m_display, parameterBuffer);
    // The capture buffer goes back to the compositor once the GPU has read it.
    return status == VA_STATUS_SUCCESS && ended == VA_STATUS_SUCCESS
            && vaSyncSurface(m_display, target) == VA_STATUS_SUCCESS;
}

Rect Encoder::fit(Rect source)
{
    if (source.isEmpty())
        return Rect();
    // Odd sizes were rounded down to what YUV 4:2:0 can hold; trim rather than scale.
    const int extraWidth = source.width() - m_size.width();
    const int extraHeight = source.height() - m_size.height();
    if (extraWidth >= 0 && extraWidth <= 1 && extraHeight >= 0 && extraHeight <= 1)
        source.setSize(m_size);
    if (source.size() != m_scaledFrom) {
        Size fitted = scaledToFit(source.size(), m_size);
        fitted = Size(evenFloor(fitted.width()), evenFloor(fitted.height()));
        m_fit = Rect(Point(((m_size.width() - fitted.width()) / 2) & ~1,
                           ((m_size.height() - fitted.height()) / 2) & ~1),
                     fitted);
        m_scaledFrom = source.size();
        m_fitChanged = true;
    }
    return source;
}

bool Encoder::openX264(const Size &size)
{
    const AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec)
        return false;

    m_video = videoContext(codec, size, m_format);
    m_video->pix_fmt = AV_PIX_FMT_YUV420P;
    m_video->thread_count = 0;
    av_opt_set(m_video->priv_data, "preset", "veryfast", 0);
    av_opt_set(m_video->priv_data, "crf", kX264Crf, 0);
    if (avcodec_open2(m_video, codec, nullptr) < 0) {
        avcodec_free_context(&m_video);
        return false;
    }
    m_swFormat = AV_PIX_FMT_YUV420P;
    LM_INFO(cfRecorder).format("encoding with libx264 at {}x{}", size.width(), size.height());
    return true;
}

bool Encoder::openAudio(std::string *error)
{
    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
        *error = "no AAC encoder";
        return false;
    }
    m_audio = avcodec_alloc_context3(codec);
    m_audio->sample_fmt = AV_SAMPLE_FMT_FLTP;
    m_audio->sample_rate = kSampleRate;
    m_audio->time_base = AVRational {1, kSampleRate};
    m_audio->bit_rate = 160'000;
    av_channel_layout_default(&m_audio->ch_layout, kChannels);
    if (m_format->oformat->flags & AVFMT_GLOBALHEADER)
        m_audio->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    if (int result = avcodec_open2(m_audio, codec, nullptr); result < 0) {
        *error = avError(result);
        avcodec_free_context(&m_audio);
        return false;
    }

    m_audioStream = avformat_new_stream(m_format, nullptr);
    if (!m_audioStream) {
        *error = "cannot add the audio stream";
        avcodec_free_context(&m_audio);
        return false;
    }
    m_audioStream->time_base = m_audio->time_base;
    avcodec_parameters_from_context(m_audioStream->codecpar, m_audio);
    m_fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLTP, kChannels, kSampleRate);
    m_audioFrame = av_frame_alloc();
    return true;
}

bool Encoder::writeHeader(std::string *error)
{
    if (int result = avio_open(&m_format->pb, m_path.c_str(), AVIO_FLAG_WRITE); result < 0) {
        *error = "cannot write " + m_path + ": " + avError(result);
        return false;
    }
    AVDictionary *options = nullptr;
    // The index goes first, so the file plays while it is still being read.
    av_dict_set(&options, "movflags", "+faststart", 0);
    const int result = avformat_write_header(m_format, &options);
    av_dict_free(&options);
    if (result < 0) {
        *error = "cannot start the file: " + avError(result);
        return false;
    }
    return true;
}

bool Encoder::convert(const Image &image, const Rect &crop)
{
    const Rect source = fit(crop & image.rect());
    if (source.isEmpty())
        return false;

    if (!m_frame) {
        m_frame = av_frame_alloc();
        m_frame->format = m_swFormat;
        m_frame->width = m_size.width();
        m_frame->height = m_size.height();
        if (av_frame_get_buffer(m_frame, 0) < 0)
            return false;
        m_hwFrame = av_frame_alloc();
        m_fitChanged = true;
    }

    const bool refill = std::exchange(m_fitChanged, false);
    if (refill) {
        const Size fitted = m_fit.size();
        m_sws = sws_getCachedContext(m_sws, source.width(), source.height(), AV_PIX_FMT_BGRA,
                                     fitted.width(), fitted.height(),
                                     AVPixelFormat(m_swFormat),
                                     fitted == source.size() ? SWS_POINT : SWS_BICUBIC, nullptr,
                                     nullptr, nullptr);
        if (!m_sws)
            return false;
        sws_setColorspaceDetails(m_sws, sws_getCoefficients(SWS_CS_DEFAULT), 1,
                                 sws_getCoefficients(SWS_CS_ITU709), 0, 0, 1 << 16, 1 << 16);
    }

    // The encoder may still hold the previous frame.
    if (av_frame_make_writable(m_frame) < 0)
        return false;
    if (refill && m_fit.size() != m_size) {
        const int chromaHeight = m_size.height() / 2;
        for (int y = 0; y < m_size.height(); ++y)
            memset(m_frame->data[0] + y * m_frame->linesize[0], 16, m_size.width());
        for (int plane = 1; plane < (m_swFormat == AV_PIX_FMT_NV12 ? 2 : 3); ++plane) {
            const int width = m_swFormat == AV_PIX_FMT_NV12 ? m_size.width() : m_size.width() / 2;
            for (int y = 0; y < chromaHeight; ++y)
                memset(m_frame->data[plane] + y * m_frame->linesize[plane], 128, width);
        }
    }

    const uint8_t *sourcePlanes[4] = {image.bits + source.y() * image.stride + source.x() * 4, nullptr,
                                      nullptr, nullptr};
    const int sourceStrides[4] = {image.stride, 0, 0, 0};
    uint8_t *planes[4] = {};
    int strides[4] = {};
    const int x = m_fit.x();
    const int y = m_fit.y();
    planes[0] = m_frame->data[0] + y * m_frame->linesize[0] + x;
    strides[0] = m_frame->linesize[0];
    if (m_swFormat == AV_PIX_FMT_NV12) {
        planes[1] = m_frame->data[1] + (y / 2) * m_frame->linesize[1] + x;
        strides[1] = m_frame->linesize[1];
    } else {
        for (int plane = 1; plane < 3; ++plane) {
            planes[plane] = m_frame->data[plane] + (y / 2) * m_frame->linesize[plane] + x / 2;
            strides[plane] = m_frame->linesize[plane];
        }
    }
    return sws_scale(m_sws, sourcePlanes, sourceStrides, 0, source.height(), planes, strides) > 0;
}

bool Encoder::sendVideo(int64_t pts, int64_t duration)
{
    AVFrame *frame = m_pendingHw ? m_pendingHw : m_frame;
    if (!m_pendingHw && m_video->hw_frames_ctx) {
        av_frame_unref(m_hwFrame);
        if (av_hwframe_get_buffer(m_video->hw_frames_ctx, m_hwFrame, 0) < 0
            || av_hwframe_transfer_data(m_hwFrame, m_frame, 0) < 0) {
            fail("cannot upload a frame to the GPU");
            return false;
        }
        frame = m_hwFrame;
    }
    frame->pts = pts;
    // Encoders do not all carry a frame's duration over to its packet.
    m_durations[pts] = duration;
    const int result = avcodec_send_frame(m_video, frame);
    // The encoder keeps its own reference to a GPU frame.
    av_frame_free(&m_pendingHw);
    if (result < 0) {
        fail("cannot encode a frame: " + avError(result));
        return false;
    }
    return writePackets(m_video, m_videoStream);
}

void Encoder::queueAudio(const std::vector<float> &samples, int64_t capturedNs)
{
    int frames = int(samples.size() / kChannels);
    int skip = 0;
    const int64_t offset = av_rescale(capturedNs - m_originNs, kSampleRate, 1000000000);
    if (!m_audioAligned) {
        if (offset < 0) {
            skip = int(std::min<int64_t>(-offset, frames));
            if (skip == frames)
                return;
        } else {
            m_audioSamples = offset;
        }
        m_audioAligned = true;
    } else {
        // A stalled microphone or a lost buffer leaves a hole, which stays
        // silent; a sample count running ahead of the clock loses the overlap.
        const int64_t drift = offset - (m_audioSamples + av_audio_fifo_size(m_fifo));
        if (drift > kAudioDriftLimit) {
            if (!queueSilence(drift))
                return;
        } else if (drift < -kAudioDriftLimit) {
            skip = int(std::min<int64_t>(-drift, frames));
            if (skip == frames)
                return;
        }
    }
    frames -= skip;

    // The encoder takes one plane per channel.
    std::vector<float> left(frames);
    std::vector<float> right(frames);
    const float *interleaved = samples.data() + skip * kChannels;
    for (int i = 0; i < frames; ++i) {
        left[i] = interleaved[i * kChannels];
        right[i] = interleaved[i * kChannels + 1];
    }
    void *planes[2] = {left.data(), right.data()};
    if (av_audio_fifo_write(m_fifo, planes, frames) < frames) {
        fail("cannot buffer audio");
        return;
    }
    drainAudio(false);
}

bool Encoder::queueSilence(int64_t frames)
{
    static const std::vector<float> silence(kSilenceChunk);
    void *planes[2] = {const_cast<float *>(silence.data()), const_cast<float *>(silence.data())};
    while (frames > 0) {
        const int count = int(std::min<int64_t>(frames, kSilenceChunk));
        if (av_audio_fifo_write(m_fifo, planes, count) < count) {
            fail("cannot buffer audio");
            return false;
        }
        frames -= count;
        if (!drainAudio(false))
            return false;
    }
    return true;
}

bool Encoder::drainAudio(bool flush)
{
    const int frameSize = m_audio->frame_size > 0 ? m_audio->frame_size : 1024;
    while (av_audio_fifo_size(m_fifo) >= frameSize || (flush && av_audio_fifo_size(m_fifo) > 0)) {
        const int count = std::min(frameSize, av_audio_fifo_size(m_fifo));
        av_frame_unref(m_audioFrame);
        m_audioFrame->nb_samples = count;
        m_audioFrame->format = m_audio->sample_fmt;
        m_audioFrame->sample_rate = kSampleRate;
        av_channel_layout_copy(&m_audioFrame->ch_layout, &m_audio->ch_layout);
        if (av_frame_get_buffer(m_audioFrame, 0) < 0
            || av_audio_fifo_read(m_fifo, reinterpret_cast<void **>(m_audioFrame->data), count)
                    < count) {
            fail("cannot read buffered audio");
            return false;
        }
        m_audioFrame->pts = m_audioSamples;
        m_audioSamples += count;
        if (int result = avcodec_send_frame(m_audio, m_audioFrame); result < 0) {
            fail("cannot encode audio: " + avError(result));
            return false;
        }
        if (!writePackets(m_audio, m_audioStream))
            return false;
    }
    return true;
}

bool Encoder::writePackets(AVCodecContext *codec, AVStream *stream)
{
    AVPacket *packet = av_packet_alloc();
    int result = 0;
    while ((result = avcodec_receive_packet(codec, packet)) >= 0) {
        if (codec == m_video)
            packet->duration = takeDuration(m_durations, packet->pts);
        av_packet_rescale_ts(packet, codec->time_base, stream->time_base);
        packet->stream_index = stream->index;
        if (int written = av_interleaved_write_frame(m_format, packet); written < 0) {
            av_packet_free(&packet);
            fail("cannot write to " + m_path + ": " + avError(written));
            return false;
        }
    }
    av_packet_free(&packet);
    if (result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
        fail("the encoder failed: " + avError(result));
        return false;
    }
    return true;
}

void Encoder::frameDone(int buffer)
{
    if (onFrameDone)
        onFrameDone(buffer);
}

void Encoder::fail(const std::string &error)
{
    if (m_failed)
        return;
    m_failed = true;
    LM_WARNING(cfRecorder) << error;
    close();
    if (onFinished)
        onFinished(false, error);
}

void Encoder::close()
{
    if (m_format && m_format->pb)
        avio_closep(&m_format->pb);
    if (m_format)
        avformat_free_context(std::exchange(m_format, nullptr));
    closeVideo();
    avcodec_free_context(&m_audio);
    av_frame_free(&m_audioFrame);
    if (m_fifo)
        av_audio_fifo_free(std::exchange(m_fifo, nullptr));
    if (m_sws)
        sws_freeContext(std::exchange(m_sws, nullptr));
    m_videoStream = nullptr;
    m_audioStream = nullptr;
}
