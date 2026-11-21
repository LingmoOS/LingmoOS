#include "screencaststream.h"

#include "pipewirecore.h"
#include "requestobject.h"
#include "waylandcapture.h"

#include "base/logging.h"

#include <algorithm>
#include <dirent.h>
#include <drm_fourcc.h>
#include <ext-image-copy-capture-v1-client-protocol.h>
#include <fcntl.h>
#include <gbm.h>
#include <pipewire/pipewire.h>
#include <spa/buffer/meta.h>
#include <spa/param/video/format-utils.h>
#include <spa/pod/builder.h>
#include <spa/pod/dynamic.h>
#include <spa/utils/result.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

using namespace Lingmo;

struct ScreenCastStream::Buffer {
    ~Buffer()
    {
        *alive = false;
        if (wlBuffer)
            wl_buffer_destroy(wlBuffer);
        for (const int planeFd : planeFds)
            close(planeFd);
        if (bo)
            gbm_bo_destroy(bo);
        if (fd >= 0)
            close(fd);
    }

    // memfd-backed wl_shm buffers own one fd; DMA-BUFs own the bo and one
    // exported fd per plane.
    int fd = -1;
    gbm_bo *bo = nullptr;
    std::vector<int> planeFds;
    wl_buffer *wlBuffer = nullptr;
    // The bounding box of what changed since this buffer was last filled,
    // for damage_buffer; the compositor paints whole frames, so a box is
    // as good as the exact region here.
    base::Rect missed;
    bool filled = false;
    // DMA-BUF import is asynchronous; a capture waits for it, a refusal
    // withdraws DMA-BUFs from the stream.
    bool importPending = false;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
};

struct ScreenCastStreamEvents {
    static void stateChanged(void *data, pw_stream_state previous, pw_stream_state state,
                             const char *error)
    {
        static_cast<ScreenCastStream *>(data)->handleStateChanged(previous, state, error);
    }

    static void paramChanged(void *data, uint32_t id, const spa_pod *param)
    {
        if (id == SPA_PARAM_Format && param)
            static_cast<ScreenCastStream *>(data)->handleFormat(param);
    }

    static void addBuffer(void *data, pw_buffer *buffer)
    {
        static_cast<ScreenCastStream *>(data)->addBuffer(buffer);
    }

    static void removeBuffer(void *data, pw_buffer *buffer)
    {
        static_cast<ScreenCastStream *>(data)->removeBuffer(buffer);
    }

    static void process(void *data)
    {
        static_cast<ScreenCastStream *>(data)->captureNext();
    }
};

static const pw_stream_events streamEvents = [] {
    pw_stream_events events {};
    events.version = PW_VERSION_STREAM_EVENTS;
    events.state_changed = ScreenCastStreamEvents::stateChanged;
    events.param_changed = ScreenCastStreamEvents::paramChanged;
    events.add_buffer = ScreenCastStreamEvents::addBuffer;
    events.remove_buffer = ScreenCastStreamEvents::removeBuffer;
    events.process = ScreenCastStreamEvents::process;
    return events;
}();

static constexpr int bytesPerPixel = 4;
static constexpr int damageRegions = 16;
static constexpr int dmabufFailureLimit = 3;

// ARGB8888 little-endian memory order is what SPA calls BGRA.
static spa_video_format spaFormat(uint32_t drmFormat)
{
    switch (drmFormat) {
    case DRM_FORMAT_ARGB8888:
        return SPA_VIDEO_FORMAT_BGRA;
    case DRM_FORMAT_XRGB8888:
        return SPA_VIDEO_FORMAT_BGRx;
    default:
        return SPA_VIDEO_FORMAT_UNKNOWN;
    }
}

static uint32_t drmFormat(spa_video_format format)
{
    switch (format) {
    case SPA_VIDEO_FORMAT_BGRA:
        return DRM_FORMAT_ARGB8888;
    case SPA_VIDEO_FORMAT_BGRx:
        return DRM_FORMAT_XRGB8888;
    default:
        return DRM_FORMAT_INVALID;
    }
}

// The DRM node with this st_rdev, which is what the compositor names.
static std::string drmNodeFor(dev_t device)
{
    DIR *nodes = opendir("/dev/dri");
    if (!nodes)
        return {};
    std::string result;
    while (const dirent *entry = readdir(nodes)) {
        const std::string path = std::string("/dev/dri/") + entry->d_name;
        struct stat info = {};
        if (stat(path.c_str(), &info) == 0 && S_ISCHR(info.st_mode) && info.st_rdev == device) {
            result = path;
            break;
        }
    }
    closedir(nodes);
    return result;
}

// A buffer with exactly this modifier. Some GBM backends (virgl among them)
// refuse gbm_bo_create_with_modifiers even for LINEAR, while the flag form
// allocates a linear buffer fine; both forms are tried so the buffer path
// accepts whatever the modifier probe accepted.
static gbm_bo *allocateDmaBuf(gbm_device *gbm, const base::Size &size, uint32_t format, uint64_t modifier)
{
    const auto width = uint32_t(size.width());
    const auto height = uint32_t(size.height());
    if (modifier != DRM_FORMAT_MOD_INVALID) {
        if (gbm_bo *bo = gbm_bo_create_with_modifiers2(gbm, width, height, format, &modifier, 1,
                                                       GBM_BO_USE_RENDERING))
            return bo;
        if (modifier != DRM_FORMAT_MOD_LINEAR)
            return nullptr;
    }
    uint32_t flags = GBM_BO_USE_RENDERING;
    if (modifier == DRM_FORMAT_MOD_LINEAR)
        flags |= GBM_BO_USE_LINEAR;
    gbm_bo *bo = gbm_bo_create(gbm, width, height, format, flags);
    if (bo && modifier != DRM_FORMAT_MOD_INVALID && gbm_bo_get_modifier(bo) != modifier) {
        gbm_bo_destroy(bo);
        return nullptr;
    }
    return bo;
}

ScreenCastStream::ScreenCastStream(WaylandCapture *wayland, PipeWireCore *pipewire,
                                   const CaptureTarget &target, bool cursor, int maxFramerate)
    : m_wayland(wayland)
    , m_pipewire(pipewire)
    , m_target(target)
    , m_cursor(cursor)
    , m_maxFramerate(std::clamp(maxFramerate, 1, 240))
{
    m_throttle.setSingleShot(true);
    m_throttle.onTimeout = [this] { beginCapture(); };
    // Emitted outside PipeWire callbacks, so the stream can go right away, and
    // it must: the core is destroyed as soon as the signal returns.
    m_pipewireDisconnected = pipewire->disconnected.connect([this] {
        destroyStream();
        finish();
    });
    // Likewise the wl_buffers must be destroyed while the display exists.
    m_waylandDisconnected = wayland->disconnected.connect([this] {
        destroyStream();
        finish();
    });
}

ScreenCastStream::~ScreenCastStream()
{
    m_finished = true;
    m_throttle.stop();
    if (m_session)
        m_session->cancel();
    destroyStream();
    if (m_gbm)
        gbm_device_destroy(m_gbm);
    if (m_gbmFd >= 0)
        close(m_gbmFd);
}

void ScreenCastStream::destroyStream()
{
    if (m_stream) {
        pw_stream_disconnect(m_stream);
        pw_stream_destroy(m_stream);
        m_stream = nullptr;
    }
    delete m_listener;
    m_listener = nullptr;
}

bool ScreenCastStream::start()
{
    if (!m_wayland->connectToDisplay() || !m_pipewire->connectToDaemon())
        return false;
    m_session = m_target.kind == CaptureTarget::Kind::Window
            ? m_wayland->createToplevelSession(m_target.name, m_cursor)
            : m_wayland->createSession(m_target.name, m_cursor);
    if (!m_session)
        return false;
    m_session->onStopped = [this] {
        finish();
    };
    m_session->onFrameReady = [this](uint64_t presentationNs, uint32_t transform,
                                     const std::vector<base::Rect> &damage) {
        frameReady(presentationNs, transform, damage);
    };
    m_session->onFrameFailed = [this](uint32_t reason) {
        frameFailed(reason);
    };
    m_session->onConstraintsChanged = [this] {
        if (m_finished)
            return;
        if (m_session->bufferSize().isEmpty() || !m_session->argbSupported()) {
            LM_WARNING(base::LogCategory::defaultCategory()) << "Unsupported capture constraints for" << m_target.name;
            finish();
            return;
        }
        if (!m_stream) {
            m_size = m_session->bufferSize();
            if (!connectStream())
                finish();
        } else if (m_session->bufferSize() != m_size) {
            m_size = m_session->bufferSize();
            updateParams();
        }
    };
    return true;
}

bool ScreenCastStream::openGbm()
{
    if (m_gbm)
        return true;
    if (!m_wayland->dmabufAvailable() || !m_session->dmabufDevice()
        || m_session->dmabufFormats().empty())
        return false;
    const std::string node = drmNodeFor(m_session->dmabufDevice());
    if (node.empty())
        return false;
    m_gbmFd = open(node.c_str(), O_RDWR | O_CLOEXEC);
    if (m_gbmFd < 0)
        return false;
    m_gbm = gbm_create_device(m_gbmFd);
    if (!m_gbm) {
        close(m_gbmFd);
        m_gbmFd = -1;
        return false;
    }
    return true;
}

// DMA-BUF formats first, each with the compositor's modifiers to choose from,
// then plain memory. `fixate` puts the modifier settled on ahead of them so
// PipeWire's next format is that one.
void ScreenCastStream::buildFormats(spa_pod_builder *builder,
                                    std::vector<const spa_pod *> &params, bool fixate) const
{
    const spa_rectangle size {uint32_t(m_size.width()), uint32_t(m_size.height())};
    const spa_fraction variable {0, 1};
    const spa_fraction minimum {1, 1};
    const spa_fraction maximum {uint32_t(m_maxFramerate), 1};
    const auto timing = [&](spa_pod_frame *frame) {
        spa_pod_builder_add(builder, SPA_FORMAT_VIDEO_size, SPA_POD_Rectangle(&size),
                            SPA_FORMAT_VIDEO_framerate, SPA_POD_Fraction(&variable),
                            SPA_FORMAT_VIDEO_maxFramerate,
                            SPA_POD_CHOICE_RANGE_Fraction(&maximum, &minimum, &maximum), 0);
        params.push_back(static_cast<const spa_pod *>(spa_pod_builder_pop(builder, frame)));
    };
    const auto begin = [&](spa_pod_frame *frame, spa_video_format format) {
        spa_pod_builder_push_object(builder, frame, SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat);
        spa_pod_builder_add(builder, SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
                            SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
                            SPA_FORMAT_VIDEO_format, SPA_POD_Id(format), 0);
    };

    if (fixate) {
        spa_pod_frame frame;
        begin(&frame, spaFormat(m_fixateFormat));
        spa_pod_builder_prop(builder, SPA_FORMAT_VIDEO_modifier, SPA_POD_PROP_FLAG_MANDATORY);
        spa_pod_builder_long(builder, int64_t(m_fixateModifier));
        timing(&frame);
    }
    if (!m_avoidDmaBuf && m_gbm) {
        for (const auto &[format, modifiers] : m_session->dmabufFormats()) {
            if (spaFormat(format) == SPA_VIDEO_FORMAT_UNKNOWN || modifiers.empty())
                continue;
            spa_pod_frame frame, choice;
            begin(&frame, spaFormat(format));
            spa_pod_builder_prop(builder, SPA_FORMAT_VIDEO_modifier,
                                 SPA_POD_PROP_FLAG_MANDATORY | SPA_POD_PROP_FLAG_DONT_FIXATE);
            spa_pod_builder_push_choice(builder, &choice, SPA_CHOICE_Enum, 0);
            // The first value is the default and repeats as the first choice.
            spa_pod_builder_long(builder, int64_t(modifiers.front()));
            for (const uint64_t modifier : modifiers)
                spa_pod_builder_long(builder, int64_t(modifier));
            spa_pod_builder_pop(builder, &choice);
            timing(&frame);
        }
    }
    {
        spa_pod_frame frame;
        spa_pod_builder_push_object(builder, &frame, SPA_TYPE_OBJECT_Format,
                                    SPA_PARAM_EnumFormat);
        spa_pod_builder_add(builder, SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
                            SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
                            SPA_FORMAT_VIDEO_format,
                            SPA_POD_CHOICE_ENUM_Id(3, SPA_VIDEO_FORMAT_BGRA, SPA_VIDEO_FORMAT_BGRA,
                                                   SPA_VIDEO_FORMAT_BGRx),
                            0);
        timing(&frame);
    }
}

bool ScreenCastStream::connectStream()
{
    openGbm();
    auto *properties = pw_properties_new(PW_KEY_MEDIA_CLASS, "Video/Source", PW_KEY_MEDIA_ROLE,
                                         "Screen", PW_KEY_NODE_NAME, "lingmo-screencast",
                                         PW_KEY_NODE_DESCRIPTION, m_target.name.c_str(),
                                         nullptr);
    m_stream = pw_stream_new(m_pipewire->core(), "lingmo-screencast", properties);
    if (!m_stream)
        return false;
    m_listener = new spa_hook {};
    pw_stream_add_listener(m_stream, m_listener, &streamEvents, this);

    uint8_t storage[4096];
    spa_pod_dynamic_builder builder;
    spa_pod_dynamic_builder_init(&builder, storage, sizeof(storage), 4096);
    std::vector<const spa_pod *> params;
    buildFormats(&builder.b, params, false);
    const int result = pw_stream_connect(
            m_stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
            static_cast<pw_stream_flags>(PW_STREAM_FLAG_DRIVER | PW_STREAM_FLAG_ALLOC_BUFFERS),
            params.data(), uint32_t(params.size()));
    spa_pod_dynamic_builder_clean(&builder);
    if (result < 0) {
        LM_WARNING(base::LogCategory::defaultCategory())
                << "Cannot connect the PipeWire stream for" << m_target.name << spa_strerror(result);
        return false;
    }
    return true;
}

void ScreenCastStream::updateParams(bool fixate)
{
    uint8_t storage[4096];
    spa_pod_dynamic_builder builder;
    spa_pod_dynamic_builder_init(&builder, storage, sizeof(storage), 4096);
    std::vector<const spa_pod *> params;
    buildFormats(&builder.b, params, fixate);
    pw_stream_update_params(m_stream, params.data(), uint32_t(params.size()));
    spa_pod_dynamic_builder_clean(&builder);
}

void ScreenCastStream::handleStateChanged(int previous, int state, const char *error)
{
    switch (state) {
    case PW_STREAM_STATE_ERROR:
        LM_WARNING(base::LogCategory::defaultCategory()) << "PipeWire stream error for" << m_target.name << error;
        finish();
        break;
    case PW_STREAM_STATE_UNCONNECTED:
        if (previous != PW_STREAM_STATE_CONNECTING)
            finish();
        break;
    case PW_STREAM_STATE_PAUSED:
        m_streaming = false;
        m_throttle.stop();
        if (m_current)
            returnCurrent();
        if (!m_announced && pw_stream_get_node_id(m_stream) != SPA_ID_INVALID) {
            m_nodeId = pw_stream_get_node_id(m_stream);
            m_announced = true;
            if (auto callback = onReady)
                callback();
        }
        break;
    case PW_STREAM_STATE_STREAMING:
        m_streaming = true;
        captureNext();
        break;
    default:
        break;
    }
}

// Settles a DONT_FIXATE modifier choice the way xdg-desktop-portal-wlr does:
// whichever of the consumer's modifiers GBM will allocate wins. False means
// none does, and DMA-BUFs are withdrawn from the offer.
bool ScreenCastStream::chooseModifier(uint32_t format, const std::vector<uint64_t> &modifiers)
{
    if (!m_gbm || modifiers.empty())
        return false;
    gbm_bo *bo = gbm_bo_create_with_modifiers2(m_gbm, uint32_t(m_size.width()),
                                               uint32_t(m_size.height()), format,
                                               modifiers.data(), unsigned(modifiers.size()),
                                               GBM_BO_USE_RENDERING);
    for (auto it = modifiers.begin(); !bo && it != modifiers.end(); ++it)
        bo = allocateDmaBuf(m_gbm, m_size, format, *it);
    if (!bo)
        return false;
    m_fixateModifier = gbm_bo_get_modifier(bo);
    gbm_bo_destroy(bo);
    m_fixateFormat = format;
    return true;
}

void ScreenCastStream::handleFormat(const spa_pod *param)
{
    spa_video_info_raw info {};
    if (spa_format_video_raw_parse(param, &info) < 0)
        return;
    m_formatSize = base::Size(int(info.size.width), int(info.size.height));
    m_framerate = info.max_framerate.denom > 0
            ? int(info.max_framerate.num / info.max_framerate.denom)
            : 0;
    if (m_framerate <= 0)
        m_framerate = m_maxFramerate;

    const spa_pod_prop *modifier = spa_pod_find_prop(param, nullptr, SPA_FORMAT_VIDEO_modifier);
    uint32_t dataType = 1 << SPA_DATA_MemFd;
    int blocks = 1;
    m_dmabuf = false;
    if (modifier) {
        const uint32_t format = drmFormat(info.format);
        if (modifier->flags & SPA_POD_PROP_FLAG_DONT_FIXATE) {
            const spa_pod *choice = &modifier->value;
            const uint32_t count = SPA_POD_CHOICE_N_VALUES(choice);
            const auto *values = static_cast<const int64_t *>(SPA_POD_CHOICE_VALUES(choice));
            // The first value is the default; the alternatives follow it.
            std::vector<uint64_t> modifiers;
            for (uint32_t index = 1; index < count; ++index)
                modifiers.push_back(uint64_t(values[index]));
            if (chooseModifier(format, modifiers)) {
                updateParams(true);
            } else {
                LM_WARNING(base::LogCategory::defaultCategory())
                        << "No DMA-BUF modifier can be allocated; falling back to memory";
                m_avoidDmaBuf = true;
                updateParams();
            }
            return;
        }
        m_dmabuf = true;
        m_drmFormat = format;
        m_modifier = uint64_t(info.modifier);
        dataType = 1 << SPA_DATA_DmaBuf;
        if (m_modifier != DRM_FORMAT_MOD_INVALID && m_gbm)
            blocks = std::max(1, gbm_device_get_format_modifier_plane_count(m_gbm, m_drmFormat,
                                                                              m_modifier));
    }

    const int stride = int(info.size.width) * bytesPerPixel;
    const int size = stride * int(info.size.height);
    uint8_t storage[2048];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(storage, sizeof(storage));
    std::vector<const spa_pod *> params;
    {
        spa_pod_frame frame;
        spa_pod_builder_push_object(&builder, &frame, SPA_TYPE_OBJECT_ParamBuffers,
                                    SPA_PARAM_Buffers);
        spa_pod_builder_add(&builder, SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(4, 2, 8),
                            SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(blocks), 0);
        if (!m_dmabuf)
            spa_pod_builder_add(&builder, SPA_PARAM_BUFFERS_size, SPA_POD_Int(size),
                                SPA_PARAM_BUFFERS_stride, SPA_POD_Int(stride), 0);
        spa_pod_builder_add(&builder, SPA_PARAM_BUFFERS_align, SPA_POD_Int(16),
                            SPA_PARAM_BUFFERS_dataType, SPA_POD_CHOICE_FLAGS_Int(dataType), 0);
        params.push_back(static_cast<const spa_pod *>(spa_pod_builder_pop(&builder, &frame)));
    }
    params.push_back(static_cast<const spa_pod *>(spa_pod_builder_add_object(
            &builder, SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
            SPA_POD_Id(SPA_META_Header), SPA_PARAM_META_size,
            SPA_POD_Int(sizeof(spa_meta_header)))));
    params.push_back(static_cast<const spa_pod *>(spa_pod_builder_add_object(
            &builder, SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
            SPA_POD_Id(SPA_META_VideoTransform), SPA_PARAM_META_size,
            SPA_POD_Int(sizeof(spa_meta_videotransform)))));
    params.push_back(static_cast<const spa_pod *>(spa_pod_builder_add_object(
            &builder, SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
            SPA_POD_Id(SPA_META_VideoDamage), SPA_PARAM_META_size,
            SPA_POD_CHOICE_RANGE_Int(sizeof(spa_meta_region) * damageRegions,
                                     sizeof(spa_meta_region),
                                     sizeof(spa_meta_region) * damageRegions))));
    pw_stream_update_params(m_stream, params.data(), uint32_t(params.size()));
}

void ScreenCastStream::addBuffer(pw_buffer *buffer)
{
    spa_data *data = buffer->buffer->datas;
    const int stride = m_formatSize.width() * bytesPerPixel;
    const int size = stride * m_formatSize.height();
    if (buffer->buffer->n_datas < 1 || m_formatSize.isEmpty()) {
        finish();
        return;
    }
    auto entry = std::make_unique<Buffer>();
    if (m_dmabuf && (data[0].type & (1 << SPA_DATA_DmaBuf))) {
        entry->bo = allocateDmaBuf(m_gbm, m_formatSize, m_drmFormat, m_modifier);
        if (!entry->bo) {
            // Withdraw DMA-BUFs and let the renegotiation replace this buffer.
            LM_WARNING(base::LogCategory::defaultCategory())
                    << "Cannot allocate a DMA-BUF for the stream; falling back to memory";
            m_avoidDmaBuf = true;
            m_buffers.push_back(entry.get());
            buffer->user_data = entry.release();
            updateParams();
            return;
        }
        const int planes = gbm_bo_get_plane_count(entry->bo);
        if (planes < 1 || uint32_t(planes) > buffer->buffer->n_datas) {
            finish();
            return;
        }
        std::vector<DmaBufPlane> description;
        for (int plane = 0; plane < planes; ++plane) {
            const int fd = gbm_bo_get_fd_for_plane(entry->bo, plane);
            if (fd < 0) {
                finish();
                return;
            }
            entry->planeFds.push_back(fd);
            description.push_back({fd, gbm_bo_get_offset(entry->bo, plane),
                                   gbm_bo_get_stride_for_plane(entry->bo, plane)});
            data[plane].type = SPA_DATA_DmaBuf;
            data[plane].fd = fd;
            data[plane].mapoffset = 0;
            data[plane].flags = SPA_DATA_FLAG_READWRITE;
            data[plane].data = nullptr;
            data[plane].chunk->offset = description.back().offset;
            data[plane].chunk->stride = int32_t(description.back().stride);
            data[plane].chunk->size = description.back().stride * uint32_t(m_formatSize.height());
            data[plane].maxsize = data[plane].chunk->offset + data[plane].chunk->size;
        }
        Buffer *raw = entry.get();
        entry->importPending = m_wayland->createDmaBufBuffer(
                m_formatSize, m_drmFormat, gbm_bo_get_modifier(entry->bo), description,
                [this, raw, alive = entry->alive](wl_buffer *imported) {
                    if (!*alive) {
                        // Removed before the reply; the compositor pins the planes until this goes.
                        if (imported)
                            wl_buffer_destroy(imported);
                        return;
                    }
                    raw->importPending = false;
                    raw->wlBuffer = imported;
                    if (!imported) {
                        LM_WARNING(base::LogCategory::defaultCategory())
                                << "The compositor refused a DMA-BUF; falling back to memory";
                        m_avoidDmaBuf = true;
                        if (m_stream)
                            updateParams();
                    } else if (m_current && m_current->user_data == raw) {
                        beginCapture();
                    }
                });
        if (!entry->importPending) {
            finish();
            return;
        }
    } else if (!m_dmabuf && (data[0].type & (1 << SPA_DATA_MemFd))) {
        entry->fd = memfd_create("lingmo-screencast", MFD_CLOEXEC | MFD_ALLOW_SEALING);
        if (entry->fd < 0 || ftruncate(entry->fd, size) != 0) {
            finish();
            return;
        }
        // The compositor maps this through wl_shm; a shrinking file would fault it.
        fcntl(entry->fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
        entry->wlBuffer = m_wayland->createShmBuffer(entry->fd, m_formatSize, stride);
        data[0].type = SPA_DATA_MemFd;
        data[0].fd = entry->fd;
        data[0].maxsize = uint32_t(size);
        data[0].mapoffset = 0;
        data[0].flags = SPA_DATA_FLAG_READABLE | SPA_DATA_FLAG_MAPPABLE;
        data[0].data = nullptr;
        data[0].chunk->offset = 0;
        data[0].chunk->stride = stride;
        data[0].chunk->size = uint32_t(size);
    } else {
        finish();
        return;
    }
    if (!entry->wlBuffer && !entry->importPending) {
        buffer->user_data = entry.release();
        removeBuffer(buffer);
        finish();
        return;
    }
    m_buffers.push_back(entry.get());
    buffer->user_data = entry.release();
}

void ScreenCastStream::removeBuffer(pw_buffer *buffer)
{
    auto *entry = static_cast<Buffer *>(buffer->user_data);
    if (buffer == m_current) {
        if (m_session)
            m_session->cancel();
        m_current = nullptr;
    }
    if (entry) {
        m_buffers.erase(std::remove(m_buffers.begin(), m_buffers.end(), entry), m_buffers.end());
        delete entry;
    }
    buffer->user_data = nullptr;
    for (uint32_t plane = 0; plane < buffer->buffer->n_datas; ++plane)
        buffer->buffer->datas[plane].fd = -1;
}

void ScreenCastStream::captureNext()
{
    if (m_finished || !m_streaming || m_current || m_throttle.isActive() || !m_session
        || m_session->capturing())
        return;
    m_current = pw_stream_dequeue_buffer(m_stream);
    const int interval = 1000 / std::max(m_framerate, 1);
    if (!m_current) {
        // Every buffer is with the consumer; look again after one frame period.
        m_throttle.start(std::chrono::milliseconds(interval));
        return;
    }
    const int wait = m_lastCapture.isValid() ? interval - int(m_lastCapture.elapsedMs()) : 0;
    if (wait > 0)
        m_throttle.start(std::chrono::milliseconds(wait));
    else
        beginCapture();
}

void ScreenCastStream::beginCapture()
{
    if (m_finished || !m_streaming)
        return;
    if (!m_current) {
        captureNext();
        return;
    }
    auto *entry = static_cast<Buffer *>(m_current->user_data);
    if (entry && entry->importPending)
        return; // The import's completion resumes this capture.
    m_lastCapture.restart();
    // A buffer filled before only needs what it missed since; a fresh one,
    // all of it.
    std::vector<base::Rect> damage;
    if (entry && entry->filled)
        damage.push_back(!entry->missed.isEmpty() ? entry->missed : base::Rect(0, 0, 0, 0));
    if (!entry || !entry->wlBuffer || !m_session->capture(entry->wlBuffer, damage))
        returnCurrent();
}

void ScreenCastStream::frameReady(uint64_t presentationNs, uint32_t transform,
                                  const std::vector<base::Rect> &damage)
{
    if (!m_current)
        return;
    m_failures = 0;
    spa_buffer *buffer = m_current->buffer;
    if (auto *header = static_cast<spa_meta_header *>(
                spa_buffer_find_meta_data(buffer, SPA_META_Header, sizeof(spa_meta_header)))) {
        header->pts = int64_t(presentationNs);
        header->flags = 0;
        header->seq = m_sequence++;
        header->dts_offset = 0;
    }
    // wl_output transforms and SPA_META_TRANSFORMATION_* share their numbering.
    if (auto *videoTransform = static_cast<spa_meta_videotransform *>(spa_buffer_find_meta_data(
                buffer, SPA_META_VideoTransform, sizeof(spa_meta_videotransform))))
        videoTransform->transform = transform;
    base::Rect changed;
    for (const base::Rect &rect : damage)
        changed |= rect;
    // Damage since the previous frame, as the compositor tracked it. More
    // rectangles than fit collapse into their bounding box; a zero-sized
    // region ends the list.
    if (spa_meta *meta = spa_buffer_find_meta(buffer, SPA_META_VideoDamage)) {
        auto *region = static_cast<spa_meta_region *>(spa_meta_first(meta));
        const uint32_t capacity = meta->size / sizeof(spa_meta_region);
        const std::vector<base::Rect> rects =
                uint32_t(damage.size()) > capacity ? std::vector<base::Rect> {changed} : damage;
        uint32_t index = 0;
        for (const base::Rect &rect : rects) {
            region[index].region =
                    SPA_REGION(rect.x(), rect.y(), uint32_t(rect.width()), uint32_t(rect.height()));
            ++index;
        }
        if (index < capacity)
            region[index].region = SPA_REGION(0, 0, 0, 0);
    }
    // Every other buffer has now missed this change too; this one is current.
    auto *current = static_cast<Buffer *>(m_current->user_data);
    for (Buffer *other : m_buffers) {
        if (other != current)
            other->missed |= changed;
    }
    if (current) {
        current->missed = base::Rect();
        current->filled = true;
    }
    buffer->datas[0].chunk->flags = SPA_CHUNK_FLAG_NONE;
    pw_stream_queue_buffer(m_stream, m_current);
    m_current = nullptr;
    pw_stream_trigger_process(m_stream);
}

void ScreenCastStream::frameFailed(uint32_t reason)
{
    if (reason == EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_STOPPED) {
        finish();
        return;
    }
    returnCurrent();
    // A discarded frame or a stale buffer size is transient; renegotiation, if
    // any, arrives through the session constraints. Nothing but the buffers
    // is different on the DMA-BUF path, so failing every capture there means
    // the compositor cannot render into them after all.
    if (m_dmabuf && reason != EXT_IMAGE_COPY_CAPTURE_FRAME_V1_FAILURE_REASON_BUFFER_CONSTRAINTS
        && ++m_failures >= dmabufFailureLimit) {
        LM_WARNING(base::LogCategory::defaultCategory())
                << "The compositor keeps failing DMA-BUF captures for" << m_target.name << "(reason" << reason
                << "); falling back to memory";
        m_avoidDmaBuf = true;
        updateParams();
        return;
    }
    m_throttle.start(std::chrono::milliseconds(1000 / std::max(m_framerate, 1)));
}

// Hands an unfilled buffer back for the next dequeue. Queuing it instead
// would submit it as output: with the stream driving the graph nothing
// consumes it until the next trigger, and after enough failures every buffer
// sits in that queue and none can be dequeued.
void ScreenCastStream::returnCurrent()
{
    if (!m_current)
        return;
    if (m_session)
        m_session->cancel();
    pw_stream_return_buffer(m_stream, m_current);
    m_current = nullptr;
}

void ScreenCastStream::finish()
{
    if (m_finished)
        return;
    m_finished = true;
    m_throttle.stop();
    if (m_session)
        m_session->cancel();
    // Callers may be PipeWire callbacks, where the stream cannot be destroyed.
    later(m_alive, [this] {
        destroyStream();
        m_session.reset();
        if (auto callback = m_announced ? onStopped : onFailed)
            callback();
    });
}
