// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "movieservice.h"
#include "unionimage/unionimage.h"
#include <QMetaType>
#include <QDirIterator>
#include <QStandardPaths>
#include <QLibrary>
#include <QLibraryInfo>
#include <memory>
#include <QtDebug>
#include <QGuiApplication>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/avutil.h>
}

#define THUMBNAIL_SIZE 200
#define SEEK_TIME "00:00:01"

MovieService *MovieService::m_movieService = nullptr;
std::once_flag MovieService::instanceFlag;

typedef int (*mvideo_avformat_open_input)(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
typedef int (*mvideo_avformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
typedef int (*mvideo_av_find_best_stream)(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags);
typedef AVDictionaryEntry *(*mvideo_av_dict_get)(const AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags);
typedef void (*mvideo_avformat_close_input)(AVFormatContext **s);

static mvideo_avformat_open_input g_mvideo_avformat_open_input = nullptr;
static mvideo_avformat_find_stream_info g_mvideo_avformat_find_stream_info = nullptr;
static mvideo_av_find_best_stream g_mvideo_av_find_best_stream = nullptr;
static mvideo_av_dict_get g_mvideo_av_dict_get = nullptr;
static mvideo_avformat_close_input g_mvideo_avformat_close_input = nullptr;

static QString videoIndex2str(const int index)
{
    QStringList videoList = {"none", "mpeg1video", "mpeg2video", "h261", "h263", "rv10", "rv20",
                             "mjpeg", "mjpegb", "ljpeg", "sp5x", "jpegls", "mpeg4", "rawvideo", "msmpeg4v1",
                             "msmpeg4v2", "msmpeg4v3", "wmv1", "wmv2", "h263p", "h263i", "flv1", "svq1",
                             "svq3", "dvvideo", "huffyuv", "cyuv", "h264", "indeo3", "vp3", "theora",
                             "asv1", "asv2", "ffv1", "4xm", "vcr1", "cljr", "mdec", "roq", "interplay_video",
                             "xan_wc3", "xan_wc4", "rpza", "cinepak", "ws_vqa", "msrle", "msvideo1", "idcin",
                             "8bps", "smc", "flic", "truemotion1", "vmdvideo", "mszh", "zlib", "qtrle", "tscc",
                             "ulti", "qdraw", "vixl", "qpeg", "png", "ppm", "pbm", "pgm", "pgmyuv", "pam", "ffvhuff",
                             "rv30", "rv40", "vc1", "wmv3", "loco", "wnv1", "aasc", "indeo2", "fraps", "truemotion2",
                             "bmp", "cscd", "mmvideo", "zmbv", "avs", "smackvideo", "nuv", "kmvc", "flashsv",
                             "cavs", "jpeg2000", "vmnc", "vp5", "vp6", "vp6f", "targa", "dsicinvideo", "tiertexseqvideo",
                             "tiff", "gif", "dxa", "dnxhd", "thp", "sgi", "c93", "bethsoftvid", "ptx", "txd", "vp6a",
                             "amv", "vb", "pcx", "sunrast", "indeo4", "indeo5", "mimic", "rl2", "escape124", "dirac", "bfi",
                             "cmv", "motionpixels", "tgv", "tgq", "tqi", "aura", "aura2", "v210x", "tmv", "v210", "dpx",
                             "mad", "frwu", "flashsv2", "cdgraphics", "r210", "anm", "binkvideo", "iff_ilbm", "kgv1",
                             "yop", "vp8", "pictor", "ansi", "a64_multi", "a64_multi5", "r10k", "mxpeg", "lagarith",
                             "prores", "jv", "dfa", "wmv3image", "vc1image", "utvideo", "bmv_video", "vble", "dxtory",
                             "v410", "xwd", "cdxl", "xbm", "zerocodec", "mss1", "msa1", "tscc2", "mts2", "cllc", "mss2",
                             "vp9", "aic", "escape130", "g2m", "webp", "hnm4_video", "hevc", "fic", "alias_pix",
                             "brender_pix", "paf_video", "exr", "vp7", "sanm", "sgirle", "mvc1", "mvc2", "hqx", "tdsc",
                             "hq_hqa", "hap", "dds", "dxv", "screenpresso", "rscc", "avs2"
                            };
    QStringList PCMList = {"pcm_s16le", "pcm_s16be", "pcm_u16le", "pcm_u16be", "pcm_s8", "pcm_u8", "pcm_mulaw"
                           "pcm_alaw", "pcm_s32le", "pcm_s32be", "pcm_u32le", "pcm_u32be", "pcm_s24le", "pcm_s24be"
                           "pcm_u24le", "pcm_u24be", "pcm_s24daud", "pcm_zork", "pcm_s16le_planar", "pcm_dvd"
                           "pcm_f32be", "pcm_f32le", "pcm_f64be", "pcm_f64le", "pcm_bluray", "pcm_lxf", "s302m"
                           "pcm_s8_planar", "pcm_s24le_planar", "pcm_s32le_planar", "pcm_s16be_planar"
                          };
    QStringList ADPCMList = {"adpcm_ima_qt", "adpcm_ima_wav", "adpcm_ima_dk3", "adpcm_ima_dk4"
                             "adpcm_ima_ws", "adpcm_ima_smjpeg", "adpcm_ms", "adpcm_4xm", "adpcm_xa", "adpcm_adx"
                             "adpcm_ea", "adpcm_g726", "adpcm_ct", "adpcm_swf", "adpcm_yamaha", "adpcm_sbpro_4"
                             "adpcm_sbpro_3", "adpcm_sbpro_2", "adpcm_thp", "adpcm_ima_amv", "adpcm_ea_r1"
                             "adpcm_ea_r3", "adpcm_ea_r2", "adpcm_ima_ea_sead", "adpcm_ima_ea_eacs", "adpcm_ea_xas"
                             "adpcm_ea_maxis_xa", "adpcm_ima_iss", "adpcm_g722", "adpcm_ima_apc", "adpcm_vima"
                            };
    QStringList AMRList = {"amr_nb", "amr_wb"};
    QStringList realAudioList = {"ra_144", "ra_288" };
    QMap<int, QString> codecMap;
    for (int i = 0; i < videoList.size(); i++) {
        codecMap.insert(i, videoList[i]);
    }
    for (int i = 0; i < PCMList.size(); i++) {
        codecMap.insert(i + 65536, PCMList[i]);
    }
    for (int i = 0; i < ADPCMList.size(); i++) {
        codecMap.insert(i + 69632, ADPCMList[i]);
    }
    codecMap.insert(73728, "amr_nb");
    codecMap.insert(73729, "amr_wb");
    codecMap.insert(77824, "ra_144");
    codecMap.insert(77825, "ra_288");
    QString aa = codecMap[index];
    return aa;
}

static QString audioIndex2str(const int &index)
{
    QStringList audioList = {"mp2", "mp3", "aac", "ac3", "dts", "vorbis", "dvaudio", "wmav1", "wmav2", "mace3", "mace6",
                             "vmdaudio", "flac", "mp3adu", "mp3on4", "shorten", "alac", "westwood_snd1", "gsm", "qdm2",
                             "cook", "truespeech", "tta", "smackaudio", "qcelp", "wavpack", "dsicinaudio", "imc",
                             "musepack7", "mlp", "gsm_ms", "atrac3", "ape", "nellymoser", "musepack8", "speex", "wmavoice",
                             "wmapro", "wmalossless", "atrac3p", "eac3", "sipr", "mp1", "twinvq", "truehd", "mp4als",
                             "atrac1", "binkaudio_rdft", "binkaudio_dct", "aac_latm", "qdmc", "celt", "g723_1", "g729",
                             "8svx_exp", "8svx_fib", "bmv_audio", "ralf", "iac", "ilbc", "opus", "comfort_noise", "tak",
                             "metasound", "paf_audio", "on2avc", "dss_sp", "codec2", "ffwavesynth", "sonic", "sonic_ls",
                             "evrc", "smv", "dsd_lsbf", "dsd_msbf", "dsd_lsbf_planar", "dsd_msbf_planar", "4gv",
                             "interplay_acm", "xma1", "xma2", "dst", "atrac3al", "atrac3pal", "dolby_e", "aptx", "aptx_hd",
                             "sbc", "atrac9"
                            };
    QMap<int, QString> codecMap;
    for (int i = 0; i < audioList.size(); i++) {
        codecMap.insert(i + 86016, audioList[i]);
    }
    QString aa = codecMap[index];
    return aa;
}

static QString Time2str(const qint64 &seconds)
{
    QTime d(0, 0, 0);
    if (seconds < 86400) {
        d = d.addSecs(static_cast<int>(seconds));
        return d.toString("hh:mm:ss");
    } else {
        d = d.addSecs(static_cast<int>(seconds));
        int add = static_cast<int>(seconds / 86400) * 24;
        QString dayOut =  d.toString("hh:mm:ss");
        dayOut.replace(0, 2, QString::number(add + dayOut.left(2).toInt()));
        return dayOut;
    }
}

MovieService *MovieService::instance(QObject *parent)
{
    Q_UNUSED(parent)
    //线程安全单例
    std::call_once(instanceFlag, [&]() {
        m_movieService = new MovieService;
    });
    return m_movieService;
}

MovieInfo MovieService::getMovieInfo(const QUrl &url)
{
    MovieInfo result;

    m_bufferMutex.lock();
    auto iter = std::find_if(m_movieInfoBuffer.begin(), m_movieInfoBuffer.end(), [url](const std::pair<QUrl, MovieInfo> &data) {
        return data.first == url;
    });
    if (iter != m_movieInfoBuffer.end()) {
        m_bufferMutex.unlock();
        return iter->second;
    }
    m_bufferMutex.unlock();

    if (url.isLocalFile()) {
        QFileInfo fi(LibUnionImage_NameSpace::localPath(url));

        if (fi.exists()) {
            auto filePath = fi.filePath();
            result = parseFromFile(fi);
        }
    }

    m_bufferMutex.lock();
    m_movieInfoBuffer.push_back(std::make_pair(url, result));
    if (m_movieInfoBuffer.size() > 30) {
        m_movieInfoBuffer.pop_front();
    }
    m_bufferMutex.unlock();

    return result;
}

QImage MovieService::getMovieCover(const QUrl &url)
{
    QMutexLocker locker(&m_queuqMutex);
    if (!m_bInitThumb) {
        initThumb();
        m_mvideo_thumbnailer_destroy_image_data(m_image_data);
        m_image_data = nullptr;
    }

    if (m_creat_video_thumbnailer == nullptr
            || m_mvideo_thumbnailer_destroy == nullptr
            || m_mvideo_thumbnailer_create_image_data == nullptr
            || m_mvideo_thumbnailer_destroy_image_data == nullptr
            || m_mvideo_thumbnailer_generate_thumbnail_to_buffer == nullptr
            || m_video_thumbnailer == nullptr) {
        return QImage();
    }

    m_video_thumbnailer->thumbnail_size = static_cast<int>(THUMBNAIL_SIZE);
    //不取第一帧，与文管影院保持一致
//    m_video_thumbnailer->seek_time = const_cast<char *>(SEEK_TIME);
    m_image_data = m_mvideo_thumbnailer_create_image_data();
    QString file = QFileInfo(LibUnionImage_NameSpace::localPath(url)).absoluteFilePath();
    m_mvideo_thumbnailer_generate_thumbnail_to_buffer(m_video_thumbnailer, file.toUtf8().data(), m_image_data);
    QImage img = QImage::fromData(m_image_data->image_data_ptr, static_cast<int>(m_image_data->image_data_size), "png");
    m_mvideo_thumbnailer_destroy_image_data(m_image_data);
    m_image_data = nullptr;
    return img;
}

MovieInfo MovieService::parseFromFile(const QFileInfo &fi)
{
    struct MovieInfo mi;
    mi.valid = false;
    AVFormatContext *av_ctx = nullptr;
    AVCodecParameters *video_dec_ctx = nullptr;
    AVCodecParameters *audio_dec_ctx = nullptr;

    if (!fi.exists()) {
//        if (ok) *ok = false;
        return mi;
    }

    auto ret = g_mvideo_avformat_open_input(&av_ctx, fi.filePath().toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        qWarning() << "avformat: could not open input";
//        if (ok) *ok = false;
        return mi;
    }

    if (g_mvideo_avformat_find_stream_info(av_ctx, nullptr) < 0) {
        qWarning() << "av_find_stream_info failed";
//        if (ok) *ok = false;
        return mi;
    }

    if (av_ctx->nb_streams == 0) {
//        if (ok) *ok = false;
        return mi;
    }

    int videoRet = -1;
    int audioRet = -1;
    AVStream *videoStream = nullptr;
    AVStream *audioStream = nullptr;
    videoRet = g_mvideo_av_find_best_stream(av_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioRet = g_mvideo_av_find_best_stream(av_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (videoRet < 0 && audioRet < 0) {
//        if (ok) *ok = false;
        return mi;
    }

    if (videoRet >= 0) {
        int video_stream_index = -1;
        video_stream_index = videoRet;
        videoStream = av_ctx->streams[video_stream_index];
        video_dec_ctx = videoStream->codecpar;

        mi.width = video_dec_ctx->width;
        mi.height = video_dec_ctx->height;
        mi.vCodecID = videoIndex2str(video_dec_ctx->codec_id);
        mi.vCodeRate = video_dec_ctx->bit_rate;

        if (videoStream->r_frame_rate.den != 0) {
            mi.fps = videoStream->r_frame_rate.num / videoStream->r_frame_rate.den;
        } else {
            mi.fps = 0;
        }
        if (mi.height != 0) {
            mi.proportion = static_cast<float>(mi.width) / static_cast<float>(mi.height);
        } else {
            mi.proportion = 0;
        }
    }
    if (audioRet >= 0) {
        int audio_stream_index = -1;
        audio_stream_index = audioRet;
        audioStream = av_ctx->streams[audio_stream_index];
        audio_dec_ctx = audioStream->codecpar;

        mi.aCodeID = audioIndex2str(audio_dec_ctx->codec_id);
        mi.aCodeRate = audio_dec_ctx->bit_rate;
        mi.aDigit = 8;
        mi.channels = audio_dec_ctx->channels;
        mi.sampling = audio_dec_ctx->sample_rate;
    }

    auto duration = av_ctx->duration == AV_NOPTS_VALUE ? 0 : av_ctx->duration;
    duration = duration + (duration <= INT64_MAX - 5000 ? 5000 : 0);
    mi.duration = Time2str(duration / AV_TIME_BASE);
    mi.resolution = QString("%1x%2").arg(mi.width).arg(mi.height);
    mi.title = fi.fileName(); //FIXME this
    mi.filePath = fi.canonicalFilePath();
    mi.creation = fi.birthTime();
    mi.fileSize = fi.size();
    mi.fileType = fi.suffix();

    AVDictionaryEntry *tag = nullptr;

    //搜索拍摄时间
    tag = g_mvideo_av_dict_get(av_ctx->metadata, "creation_time", tag, AV_DICT_MATCH_CASE);
    if (tag != nullptr) {
        mi.creation = QDateTime::fromString(tag->value, Qt::ISODate);
    }

    g_mvideo_avformat_close_input(&av_ctx);
    mi.valid = true;

//    if (ok) *ok = true;
    return mi;
}

MovieService::MovieService(QObject *parent)
    : QObject(parent)
{
    initFFmpeg();
}

void MovieService::initThumb()
{
    QLibrary library(libPath("libffmpegthumbnailer.so"));
    m_creat_video_thumbnailer = (mvideo_thumbnailer_create) library.resolve("video_thumbnailer_create");
    m_mvideo_thumbnailer_destroy = (mvideo_thumbnailer_destroy) library.resolve("video_thumbnailer_destroy");
    m_mvideo_thumbnailer_create_image_data = (mvideo_thumbnailer_create_image_data) library.resolve("video_thumbnailer_create_image_data");
    m_mvideo_thumbnailer_destroy_image_data = (mvideo_thumbnailer_destroy_image_data) library.resolve("video_thumbnailer_destroy_image_data");
    m_mvideo_thumbnailer_generate_thumbnail_to_buffer = (mvideo_thumbnailer_generate_thumbnail_to_buffer) library.resolve("video_thumbnailer_generate_thumbnail_to_buffer");
    m_video_thumbnailer = m_creat_video_thumbnailer();

    if (m_mvideo_thumbnailer_destroy == nullptr
            || m_mvideo_thumbnailer_create_image_data == nullptr
            || m_mvideo_thumbnailer_destroy_image_data == nullptr
            || m_mvideo_thumbnailer_generate_thumbnail_to_buffer == nullptr
            || m_video_thumbnailer == nullptr) {
        return;
    }

    m_image_data = m_mvideo_thumbnailer_create_image_data();
    m_video_thumbnailer->thumbnail_size = 400 * qApp->devicePixelRatio();
    m_bInitThumb = true;
}

void MovieService::initFFmpeg()
{
    QLibrary avcodecLibrary(libPath("libavcodec.so"));
    QLibrary avformatLibrary(libPath("libavformat.so"));
    QLibrary avutilLibrary(libPath("libavutil.so"));

    g_mvideo_avformat_open_input = (mvideo_avformat_open_input) avformatLibrary.resolve("avformat_open_input");
    g_mvideo_avformat_find_stream_info = (mvideo_avformat_find_stream_info) avformatLibrary.resolve("avformat_find_stream_info");
    g_mvideo_av_find_best_stream = (mvideo_av_find_best_stream) avformatLibrary.resolve("av_find_best_stream");
    g_mvideo_avformat_close_input = (mvideo_avformat_close_input) avformatLibrary.resolve("avformat_close_input");
    g_mvideo_av_dict_get = (mvideo_av_dict_get) avutilLibrary.resolve("av_dict_get");
}

QString MovieService::libPath(const QString &strlib)
{
    QDir  dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);

    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(strlib)) {
        return strlib;
    } else {
        list.sort();
    }

    if(list.isEmpty()) {
        auto libLdPaths = qEnvironmentVariable("LD_LIBRARY_PATH").split(":");
        for(auto &eachPath : libLdPaths) {
            dir.setPath(eachPath);
            QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
            if (list.contains(strlib)) {
                return strlib;
            } else {
                list.sort();
            }
            if(!list.isEmpty()) {
                break;
            }
        }
    }

    Q_ASSERT(list.size() > 0);
    return list.last();
}
