// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include <QtDBus>
#include <QtWidgets>
#include <QPainterPath>

namespace dmr {
namespace utils {
using namespace std;

static bool isWayland = false;

void ShowInFileManager(const QString &path)
{
    if (path.isEmpty() || !QFile::exists(path)) {
        return;
    }

    QUrl url = QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath());
    //Note: The meaning of this code is unknown, use with caution
    /*QUrlQuery query;
    query.addQueryItem("selectUrl", QUrl::fromLocalFile(path).toString());
    url.setQuery(query);*/

    qInfo() << __func__ << url.toString();

    // Try dde-file-manager
    QProcess *fp = new QProcess();
    QObject::connect(fp, SIGNAL(finished(int)), fp, SLOT(deleteLater()));
#ifndef USE_TEST
    fp->start("dde-file-manager", QStringList(url.toString()));
#else
    fp->start("wrong-name-for-test", QStringList(url.toString()));
#endif
    fp->waitForStarted(3000);

    if (fp->error() == QProcess::FailedToStart) {
        // Start dde-file-manager failed, try nautilus
        QDBusInterface iface("org.freedesktop.FileManager1",
                             "/org/freedesktop/FileManager1",
                             "org.freedesktop.FileManager1",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            // Convert filepath to URI first.
            const QStringList uris = { QUrl::fromLocalFile(path).toString() };
            qInfo() << "freedesktop.FileManager";
            // StartupId is empty here.
            QDBusPendingCall call = iface.asyncCall("ShowItems", uris, "");
            Q_UNUSED(call);
        }
        // Try to launch other file manager if nautilus is invalid
        else {
            qInfo() << "desktopService::openUrl";
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath()));
        }
        fp->deleteLater();
    }
}

static int min(int v1, int v2, int v3)
{
    return std::min(v1, std::min(v2, v3));
}

static int stringDistance(const QString &s1, const QString &s2)
{
    int n = s1.size(), m = s2.size();
    if (!n || !m) return max(n, m);

    vector<int> dp(static_cast<vector<int>::size_type>(n + 1));
    for (int i = 0; i < n + 1; i++) dp[static_cast<vector<int>::size_type>(i)] = i;
//    int pred = 0;
    int curr = 0;

    for (int i = 0; i < m; i++) {
        dp[0] = i;
        int pred = i + 1;
        for (int j = 0; j < n; j++) {
            if (s1[j] == s2[i]) {
                curr = dp[static_cast<vector<int>::size_type>(j)];
            } else {
                curr = min(dp[static_cast<vector<int>::size_type>(j)], dp[static_cast<vector<int>::size_type>(j + 1)], pred) + 1;
            }
            dp[static_cast<vector<int>::size_type>(j)] = pred;
            pred = curr;

        }
        dp[static_cast<vector<int>::size_type>(n)] = pred;
    }

    return curr;
}

bool IsNamesSimilar(const QString &s1, const QString &s2)
{
    int dist = stringDistance(s1, s2);
    return (dist >= 0 && dist <= 4); //TODO: check ext.
}

QFileInfoList FindSimilarFiles(const QFileInfo &fi)
{
    QFileInfoList fil;

    QDirIterator it(fi.absolutePath());
    while (it.hasNext()) {
        it.next();
        if (!it.fileInfo().isFile()) {
            continue;
        }

        if (IsNamesSimilar(fi.fileName(), it.fileInfo().fileName())) {
            fil.append(it.fileInfo());
        }

    }
    return fil;
}

bool CompareNames(const QString &fileName1, const QString &fileName2)
{
    static QRegExp rd("\\d+");
    int pos = 0;
    while ((pos = rd.indexIn(fileName1, pos)) != -1) {
        auto inc = rd.matchedLength();
        auto id1 = fileName1.midRef(pos, inc);

        auto pos2 = rd.indexIn(fileName2, pos);
        if (pos == pos2) {
            auto id2 = fileName2.midRef(pos, rd.matchedLength());
            //qInfo() << "id compare " << id1 << id2;
            if (id1 != id2) {
                bool ok1, ok2;
                bool v = id1.toInt(&ok1) < id2.toInt(&ok2);
                if (ok1 && ok2) return v;
                return id1.localeAwareCompare(id2) < 0;
            }
        }

        pos += inc;
    }
    return fileName1.localeAwareCompare(fileName2) < 0;
}

bool first_check_wayland_env()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        isWayland = true;
        return true;
    } else {
        return false;
    }
}

bool check_wayland_env()
{
    return isWayland;
}

#ifdef USE_TEST
void set_wayland(bool _b)
{
    isWayland = _b;
}
#endif

// hash the whole file takes amount of time, so just pick some areas to be hashed
QString FastFileHash(const QFileInfo &fi)
{
    auto sz = fi.size();
    QList<qint64> offsets = {
        4096,
        sz - 8192
    };

    QFile f(fi.absoluteFilePath());
    if (!f.open(QFile::ReadOnly)) {
        return QString();
    }

    if (fi.size() < 8192) {
        auto bytes = f.readAll();
        return QString(QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex());
    }

    QByteArray bytes;
    std::for_each(offsets.begin(), offsets.end(), [&bytes, &f](qint64 v) {
        f.seek(v);
        bytes += f.read(4096);

    });
    f.close();

    return QString(QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex());
}

// hash the entire file (hope file is small)
QString FullFileHash(const QFileInfo &fi)
{
    QFile f(fi.absoluteFilePath());
    if (!f.open(QFile::ReadOnly)) {
        return QString();
    }

    auto bytes = f.readAll();
    f.close();
    return QString(QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex());
}

QPixmap MakeRoundedPixmap(QPixmap pm, qreal rx, qreal ry, qint64 time, int rotation)
{
    QString timeString = utils::Time2str(time);

    QMatrix matrix;
    matrix.rotate(rotation);
    pm = pm.transformed(matrix, Qt::SmoothTransformation);

    auto dpr = pm.devicePixelRatio();
    QPixmap dest(pm.size());
    dest.setDevicePixelRatio(dpr);

    auto scaled_rect = QRectF({0, 0}, QSizeF(dest.size() / dpr));
    dest.fill(Qt::transparent);

    QPainter p(&dest);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(QRect(QPoint(), scaled_rect.size().toSize()), rx, ry);
    p.setClipPath(path);
    p.drawPixmap(scaled_rect.toRect(), pm);

    QPainterPath timePath;
    QRect timeRect(0, scaled_rect.height() - 30, scaled_rect.width(), 30);
    timePath.addRoundedRect(timeRect, rx, ry);
    QLinearGradient linear(QPointF(scaled_rect.width() / 2, scaled_rect.height() - 30), QPointF(scaled_rect.width() / 2, scaled_rect.height()));
    linear.setColorAt(0, Qt::transparent);
    linear.setColorAt(1, QColor(0, 0, 0, 0.3 * 255));
    p.fillPath(timePath, linear);

    p.setPen(QPen(Qt::white));
    p.drawText(timeRect, Qt::AlignCenter, timeString);

    return dest;
}

QPixmap MakeRoundedPixmap(QPixmap pm, qreal rx, qreal ry, int rotation)
{
    QMatrix matrix;
    matrix.rotate(rotation);
    pm = pm.transformed(matrix, Qt::SmoothTransformation);

    auto dpr = pm.devicePixelRatio();
    QPixmap dest(pm.size());
    dest.setDevicePixelRatio(dpr);

    auto scaled_rect = QRectF({0, 0}, QSizeF(dest.size() / dpr));
    dest.fill(Qt::transparent);

    QPainter p(&dest);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(QRect(QPoint(), scaled_rect.size().toSize()), rx, ry);
    p.setClipPath(path);

//    QTransform transform;
//    transform.translate(scaled_rect.width()/2, scaled_rect.height()/2);
//    transform.rotate(rotation);
//    transform.translate(-scaled_rect.width()/2, -scaled_rect.height()/2);
//    p.setTransform(transform);

    p.drawPixmap(scaled_rect.toRect(), pm);

    return dest;
}

QPixmap MakeRoundedPixmap(QSize sz, QPixmap pm, qreal rx, qreal ry, qint64 time)
{
    int nX = (sz.width() - pm.width()) / 2;
    int nY = (sz.height() - pm.height()) / 2;
    auto dpr = pm.devicePixelRatio();
    QPixmap dest(sz);
    dest.setDevicePixelRatio(dpr);
    dest.fill(Qt::transparent);

    auto scaled_rect = QRectF(QPoint(nX, nY), QSizeF(pm.size() / dpr));

    QPainter p(&dest);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

//    p.setPen(QColor(0, 0, 0, 255 / 10));
//    p.drawRoundedRect(scaled_rect, rx, ry);

    QPainterPath path;
    auto r = scaled_rect.marginsRemoved({1, 1, 1, 1});
    path.addRoundedRect(r, rx, ry);
    p.setClipPath(path);
    p.drawPixmap(nX, nY, pm);


#if 0
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QFont ft;
    ft.setPixelSize(12);
    ft.setWeight(QFont::Medium);
    p.setFont(ft);

    auto tm_str = QTime(0, 0, 0).addSecs(static_cast<int>(time)).toString("hh:mm:ss");
    QRect bounding = QFontMetrics(ft).boundingRect(tm_str);
    bounding.moveTopLeft({(static_cast<int>(dest.width() / dpr)) - 5 - bounding.width(),
                          (static_cast<int>(dest.height() / dpr)) - 5 - bounding.height()});

    {
        QPainterPath pp;
        pp.addText(bounding.bottomLeft() + QPoint{0, 1}, ft, tm_str);
        QPen pen(QColor(0, 0, 0, 50));
        pen.setWidth(2);
        p.setBrush(QColor(0, 0, 0, 50));
        p.setPen(pen);
        p.drawPath(pp);
    }

    {
        QPainterPath pp;
        pp.addText(bounding.bottomLeft(), ft, tm_str);
        p.fillPath(pp, QColor(Qt::white));
    }
#endif

    return dest;
}

uint32_t InhibitStandby()
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
                         "/org/freedesktop/ScreenSaver",
                         "org.freedesktop.ScreenSaver");
    QDBusReply<uint32_t> reply = iface.call("Inhibit", "deepin-movie", "playing in fullscreen");

    if (reply.isValid()) {
        return reply.value();
    }

    qInfo() << reply.error().message();
    return 0;
}

void UnInhibitStandby(uint32_t cookie)
{
    QDBusInterface iface("org.freedesktop.ScreenSaver",
                         "/org/freedesktop/ScreenSaver",
                         "org.freedesktop.ScreenSaver");
    iface.call("UnInhibit", cookie);
}

uint32_t InhibitPower()
{
    QDBusInterface iface("org.freedesktop.PowerManagement",
                         "/org/freedesktop/PowerManagement",
                         "org.freedesktop.PowerManagement");
    QDBusReply<uint32_t> reply = iface.call("Inhibit", "deepin-movie", "playing in fullscreen");

    if (reply.isValid()) {
        return reply.value();
    }

    qInfo() << reply.error().message();
    return 0;
}

void UnInhibitPower(uint32_t cookie)
{
    QDBusInterface iface("org.freedesktop.PowerManagement",
                         "/org/freedesktop/PowerManagement",
                         "org.freedesktop.PowerManagement");
    iface.call("UnInhibit", cookie);
}

void MoveToCenter(QWidget *w)
{
    QDesktopWidget *dw = QApplication::desktop();
    QRect r = dw->availableGeometry(w);

    w->move(r.center() - w->rect().center());
}

QString Time2str(qint64 seconds)
{
    QTime d(0, 0, 0);
    if (seconds < DAYSECONDS) {
        d = d.addSecs(static_cast<int>(seconds));
        return d.toString("hh:mm:ss");
    } else {
        d = d.addSecs(static_cast<int>(seconds));
        int add = static_cast<int>(seconds / DAYSECONDS) * 24;
        QString dayOut =  d.toString("hh:mm:ss");
        dayOut.replace(0, 2, QString::number(add + dayOut.left(2).toInt()));
        return dayOut;
    }
}

QString videoIndex2str(int index)
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

QString audioIndex2str(int index)
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

///not used///
/*QString subtitleIndex2str(int index)
{
    QStringList subtitleList1 = {"dvd_subtitle", "dvb_subtitle", "text", "xsub", "ssa",
                                 "mov_text", "hdmv_pgs_subtitle", "dvb_teletext", "srt"
                                };
    QStringList subtitleList2 = {"microdvd", "eia_608", "jacosub", "sami", "realtext", "stl", "subviewer1", "subviewer",
                                 "subrip", "webvtt", "mpl2", "vplayer", "pjs", "ass", "hdmv_text_subtitle", "ttml"
                                };
    QMap<int, QString> codecMap;
    for (int i = 0; i < subtitleList1.size(); i++) {
        codecMap.insert(i + 94208, subtitleList1[i]);
    }
    for (int i = 0; i < subtitleList2.size(); i++) {
        codecMap.insert(i + 96256, subtitleList2[i]);
    }
    return codecMap[index];
}*/

//cppcheck 单元测试在用
bool ValidateScreenshotPath(const QString &path)
{
    auto name = path.trimmed();
    if (name.isEmpty()) return false;

    if (name.size() && name[0] == '~') {
        name.replace(0, 1, QDir::homePath());
    }

    QFileInfo fi(name);
    if (fi.exists()) {
        if (!fi.isDir()) {
            return false;
        }

        if (!fi.isReadable() || !fi.isWritable()) {
            return false;
        }
    }

    return true;
}

QImage LoadHiDPIImage(const QString &filename)
{
    QImageReader reader(filename);
    reader.setScaledSize(reader.size() * qApp->devicePixelRatio());
    auto img =  reader.read();
    img.setDevicePixelRatio(qApp->devicePixelRatio());
    return img;
}

QPixmap LoadHiDPIPixmap(const QString &filename)
{
    return QPixmap::fromImage(LoadHiDPIImage(filename));
}

QString ElideText(const QString &text, const QSize &size,
                  QTextOption::WrapMode wordWrap, const QFont &font,
                  Qt::TextElideMode mode, int lineHeight, int lastLineWidth)
{
    int height = 0;

    QTextLayout textLayout(text);
    QString str;
    QFontMetrics fontMetrics(font);

    textLayout.setFont(font);
    const_cast<QTextOption *>(&textLayout.textOption())->setWrapMode(wordWrap);

    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight >= size.height()) {
            str += fontMetrics.elidedText(text.mid(line.textStart() + line.textLength() + 1),
                                          mode, lastLineWidth);

            break;
        }

        line.setLineWidth(size.width());

        const QString &tmp_str = text.mid(line.textStart(), line.textLength());

        if (tmp_str.indexOf('\n'))
            height += lineHeight;

        str += tmp_str;

        line = textLayout.createLine();

        if (line.isValid())
            str.append("\n");
    }

    textLayout.endLayout();

    if (textLayout.lineCount() == 1) {
        str = fontMetrics.elidedText(str, mode, lastLineWidth);
    }

    return str;
}

void getPlayProperty(const char *path, QMap<QString, QString> *&proMap)
{
    QFileInfo fi(path);
    if ((fi.exists() && fi.isFile()) && fi.isReadable()) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly) | QIODevice::Text) {
            QByteArray t;
            int index = 0;
            while (!file.atEnd()) {
                t = file.readLine();
                QList<QByteArray> list = t.split('=');
                index++;
                if (2 == list.size()) {
                    QString temp = list.back();
                    temp = temp.mid(0, temp.length() - 1);//去除/n
                    proMap->insert(list.first(), temp);
                } else {
                    qWarning() << __func__ << QString("config line:%1 has error ").arg(index);
                    continue;
                }
            }
        }
        file.close();
    } else {
        qWarning() << __func__ << "file path error!!!!!";
    }
}
}
}
