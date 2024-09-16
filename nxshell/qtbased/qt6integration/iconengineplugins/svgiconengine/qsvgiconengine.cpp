// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsvgiconengine.h"

#ifndef QT_NO_SVGRENDERER

#include "qpainter.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qfileinfo.h"
#include <qmimedatabase.h>
#include <qmimetype.h>
#include <QAtomicInt>
#include "qdebug.h"
#include <private/qguiapplication_p.h>

#include <DSvgRenderer>
#include <QCryptographicHash>
#include <QImageReader>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QDir>

DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(lcDSvg, "dde.dsvg")
#else
Q_LOGGING_CATEGORY(lcDSvg, "dde.dsvg", QtInfoMsg)
#endif

class QSvgIconEnginePrivate : public QSharedData
{
public:
    QSvgIconEnginePrivate()
        : svgBuffers(0), addedPixmaps(0)
        { stepSerialNum(); }

    ~QSvgIconEnginePrivate()
        { delete addedPixmaps; delete svgBuffers; }

    static inline int hashKey(QIcon::Mode mode, QIcon::State state)
        { return (((mode)<<4)|state); }

    inline QString pmcKey(const QSize &size, QIcon::Mode mode, QIcon::State state)
        { return QLatin1String("$qt_svgicon_")
                 + QString::number(serialNum, 16).append(QLatin1Char('_'))
                 + QString::number((((((qint64(size.width()) << 11) | size.height()) << 11) | mode) << 4) | state, 16); }

    inline void stepSerialNum()
        { serialNum = lastSerialNum.fetchAndAddRelaxed(1); }

    void loadDataForModeAndState(DSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state);

    QHash<int, QString> svgFiles;
    QHash<int, QByteArray> *svgBuffers;
    QHash<int, QPixmap> *addedPixmaps;
    int serialNum;
    static QAtomicInt lastSerialNum;
};

QAtomicInt QSvgIconEnginePrivate::lastSerialNum;

QSvgIconEngine::QSvgIconEngine()
    : d(new QSvgIconEnginePrivate)
{
}

QSvgIconEngine::QSvgIconEngine(const QSvgIconEngine &other)
    : QIconEngine(other), d(new QSvgIconEnginePrivate)
{
    d->svgFiles = other.d->svgFiles;
    if (other.d->svgBuffers)
        d->svgBuffers = new QHash<int, QByteArray>(*other.d->svgBuffers);
    if (other.d->addedPixmaps)
        d->addedPixmaps = new QHash<int, QPixmap>(*other.d->addedPixmaps);
}


QSvgIconEngine::~QSvgIconEngine()
{
}


QSize QSvgIconEngine::actualSize(const QSize &size, QIcon::Mode mode,
                                 QIcon::State state)
{
    if (d->addedPixmaps) {
        QPixmap pm = d->addedPixmaps->value(d->hashKey(mode, state));
        if (!pm.isNull() && pm.size() == size)
            return size;
    }

    QPixmap pm = pixmap(size, mode, state);
    if (pm.isNull())
        return QSize();
    return pm.size();
}

void QSvgIconEnginePrivate::loadDataForModeAndState(DSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state)
{
    QByteArray buf;
    const QIcon::State oppositeState = state == QIcon::Off ? QIcon::On : QIcon::Off;
    if (svgBuffers) {
        buf = svgBuffers->value(hashKey(mode, state));
        if (buf.isEmpty())
            buf = svgBuffers->value(hashKey(QIcon::Normal, state));
        if (buf.isEmpty())
            buf = svgBuffers->value(hashKey(QIcon::Normal, oppositeState));
    }
    if (!buf.isEmpty()) {
#ifndef QT_NO_COMPRESS
        if (Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
            buf = qUncompress(buf);
        }
#endif
        renderer->load(buf);
    } else {
        QString svgFile = svgFiles.value(hashKey(mode, state));
        if (svgFile.isEmpty())
            svgFile = svgFiles.value(hashKey(QIcon::Normal, state));
        if (svgFile.isEmpty())
            svgFile = svgFiles.value(hashKey(QIcon::Normal, oppositeState));
        if (!svgFile.isEmpty())
            renderer->load(svgFile);
    }
}

static QString getIconCachePath()
{
     QString path = qgetenv("D_ICON_CACHE_PATH");

     if (!qEnvironmentVariableIsSet("D_ICON_CACHE_PATH")) {
         path = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation).append("/deepin/icons");
     }

     if (path.isEmpty()) {
         qCInfo(lcDSvg) << "disable svg icon cache of dsvg plugin";

         return path;
     }

     if (QDir::home().mkpath(path))
         return path;

     qCWarning(lcDSvg) << "can't create a invalid icon cache path:" << path;

     return QString();
}

QPixmap QSvgIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                               QIcon::State state)
{
    QPixmap pm;

    QString pmckey(d->pmcKey(size, mode, state));
    if (Q_LIKELY(QPixmapCache::find(pmckey, &pm)))
        return pm;

    if (d->addedPixmaps) {
        pm = d->addedPixmaps->value(d->hashKey(mode, state));
        if (!pm.isNull() && pm.size() == size)
            return pm;
    }

    const QIcon::State oppositeState = state == QIcon::Off ? QIcon::On : QIcon::Off;
    QString svgFile = d->svgFiles.value(d->hashKey(mode, state));
    if (svgFile.isEmpty())
        svgFile = d->svgFiles.value(d->hashKey(QIcon::Normal, state));
    if (svgFile.isEmpty())
        svgFile = d->svgFiles.value(d->hashKey(QIcon::Normal, oppositeState));

    QString cacheFile;

    if (Q_LIKELY(!svgFile.startsWith(":/") && QFile::exists(svgFile))) {
        static const QString &cachePath = getIconCachePath();

        if (Q_LIKELY(!cachePath.isEmpty())) {
            QByteArray mdst;
            if (d->svgBuffers && !d->svgBuffers->isEmpty())
                mdst = QByteArray::number(d->hashKey(mode, state)).prepend("_");
            // svgFiles[-1] 是为了取到额外的cache key,例如会跟随颜色变化的svg图标，将会添加此特殊的标记
            const QByteArray extraKey = d->svgFiles.value(-1).isEmpty() ? QByteArrayLiteral("") : d->svgFiles.value(-1).toLocal8Bit() + mdst;
            const QByteArray cacheKey = svgFile.toLocal8Bit() + extraKey; // add mode state to cachekey
            const QString &svgFileSha1 = QString::fromLatin1(QCryptographicHash::hash(cacheKey, QCryptographicHash::Sha1).toHex());
            cacheFile = QStringLiteral("%1/%2.png").arg(cachePath).arg(svgFileSha1);
        }
    }

    const QFileInfo cacheFileInfo(cacheFile);
    if (Q_LIKELY(cacheFileInfo.exists())) {
        const QFileInfo svgFileInfo(svgFile);

        if (Q_UNLIKELY(svgFileInfo.lastModified() != cacheFileInfo.lastModified())) {
            // clear invalid cache file
            QFile::remove(cacheFile);
        } else {
            qCDebug(lcDSvg()) << "found cache file:" << cacheFile << ", for:" << svgFile;

            QImageReader ir(cacheFile);
            const QSize &cacheSize = ir.size();

            if (Q_LIKELY(cacheSize.isValid() && cacheSize.width() >= size.width())) {
                pm = QPixmap::fromImage(ir.read().scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else { // 当对同一个图标文件有更大的size要求时，应当继续从源svg文件重新渲染图标并更新缓存文件
                qCDebug(lcDSvg()) << "cache image size less then target size, cache size:" << cacheSize << ", request size:" << size;

                if (Q_UNLIKELY(!ir.canRead())) {
                    qCWarning(lcDSvg()) << "can't read the cache image:" << cacheFile << "file size:" << cacheFileInfo.size() << ", permissions:" << cacheFileInfo.permissions();
                    QFile::remove(cacheFile);
                }
            }
        }
    }

    if (Q_UNLIKELY(pm.isNull())) {
        DSvgRenderer renderer;
        d->loadDataForModeAndState(&renderer, mode, state);
        if (Q_UNLIKELY(!renderer.isValid()))
            return pm;

        QSize actualSize = renderer.defaultSize();
        if (actualSize.width() < size.width())
            actualSize.scale(size, Qt::KeepAspectRatio);

        if (Q_UNLIKELY(actualSize.isEmpty()))
            return pm;

        const QImage image = renderer.toImage(actualSize);

        if (Q_LIKELY(!image.isNull() && !cacheFile.isEmpty())) {
            [[ maybe_unused ]] auto result = QtConcurrent::run(QThreadPool::globalInstance(), [image, cacheFile, svgFile] {
                QSaveFile file(cacheFile);
                // 增加cache文件能被成功保存的概率
                file.setDirectWriteFallback(true);
                if (file.open(QFile::WriteOnly)) {
                    if (image.save(&file, "png", 80) && file.commit()) {
                        QFileInfo svgFileInfo(svgFile);
                        // 不能直接使用QSaveFile改写文件的时间
                        QFile file(cacheFile);
                        if (!file.open(QFile::ReadWrite) || !file.setFileTime(svgFileInfo.lastModified(), QFileDevice::FileModificationTime)) {
                            qCWarning(lcDSvg()) << "set cache file modified date time failed, error message:" << file.errorString()
                                                << ", cache file:" << cacheFile << ", svg file:" << svgFile;
                        }
                    } else {
                        file.cancelWriting();
                        qCWarning(lcDSvg()) << "save cache image failed, cache file:" << cacheFile << ", svg file:" << svgFile;
                    }
                } else {
                    qCWarning(lcDSvg()) << "open cache file failed, error message:" << file.errorString()
                                        << ", cache file:" << cacheFile << ", svg file:" << svgFile;
                }
            });
        }

        // 缩放图标到目标大小
        pm = QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (Q_LIKELY(qobject_cast<QGuiApplication *>(QCoreApplication::instance()))) {
        const QPixmap generated = QGuiApplicationPrivate::instance()->applyQIconStyleHelper(mode, pm);
        if (Q_LIKELY(!generated.isNull()))
            pm = generated;
    }

    if (Q_LIKELY(!pm.isNull()))
        QPixmapCache::insert(pmckey, pm);

    return pm;
}


void QSvgIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                               QIcon::State state)
{
    if (!d->addedPixmaps)
        d->addedPixmaps = new QHash<int, QPixmap>;
    d->stepSerialNum();
    d->addedPixmaps->insert(d->hashKey(mode, state), pixmap);
}

enum FileType { OtherFile, SvgFile, CompressedSvgFile };

static FileType fileType(const QFileInfo &fi)
{
    const QString &abs = fi.absoluteFilePath();
    if (abs.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive))
        return SvgFile;
    if (abs.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)
        || abs.endsWith(QLatin1String(".svg.gz"), Qt::CaseInsensitive)) {
        return CompressedSvgFile;
    }
#ifndef QT_NO_MIMETYPE
    const QString &mimeTypeName = QMimeDatabase().mimeTypeForFile(fi).name();
    if (mimeTypeName == QLatin1String("image/svg+xml"))
        return SvgFile;
    if (mimeTypeName == QLatin1String("image/svg+xml-compressed"))
        return CompressedSvgFile;
#endif // !QT_NO_MIMETYPE
    return OtherFile;
}

void QSvgIconEngine::addFile(const QString &fileName, const QSize &,
                             QIcon::Mode mode, QIcon::State state)
{
    if (!fileName.isEmpty()) {
         const QFileInfo fi(fileName);
         const QString abs = fi.absoluteFilePath();
         const FileType type = fileType(fi);
#ifndef QT_NO_COMPRESS
         if ((type == SvgFile || type == CompressedSvgFile) && Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
#else
         if (type == SvgFile) {
#endif
             DSvgRenderer renderer(abs);
             if (renderer.isValid()) {
                 d->stepSerialNum();
                 d->svgFiles.insert(d->hashKey(mode, state), abs);
             }
         } else if (type == OtherFile) {
             QPixmap pm(abs);
             if (!pm.isNull())
                 addPixmap(pm, mode, state);
         }
    }
}

void QSvgIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state)
{
    qreal ratio = 1.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (qApp->testAttribute(Qt::AA_UseHighDpiPixmaps))
        ratio = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
#else
    ratio = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
#endif

    QSize pixmapSize = rect.size() * ratio;
    QPixmap pix = pixmap(pixmapSize, mode, state);

    pix.setDevicePixelRatio(ratio);
    painter->drawPixmap(rect, pix);
}

QString QSvgIconEngine::key() const
{
    return QLatin1String("svg");
}

QIconEngine *QSvgIconEngine::clone() const
{
    return new QSvgIconEngine(*this);
}


bool QSvgIconEngine::read(QDataStream &in)
{
    d = new QSvgIconEnginePrivate;
    d->svgBuffers = new QHash<int, QByteArray>;

    if (in.version() >= QDataStream::Qt_4_4) {
        int isCompressed;
        QHash<int, QString> fileNames;  // For memoryoptimization later
        in >> fileNames >> isCompressed >> *d->svgBuffers;
#ifndef QT_NO_COMPRESS

        if (!isCompressed && Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
            for (auto it = d->svgBuffers->begin(), end = d->svgBuffers->end(); it != end; ++it)
                it.value() = qCompress(it.value());
        }
#else
        if (isCompressed) {
            qWarning("QSvgIconEngine: Can not decompress SVG data");
            d->svgBuffers->clear();
        }
#endif
        int hasAddedPixmaps;
        in >> hasAddedPixmaps;
        if (hasAddedPixmaps) {
            d->addedPixmaps = new QHash<int, QPixmap>;
            in >> *d->addedPixmaps;
        }

        d->svgFiles = fileNames;
    }
    else {
        QPixmap pixmap;
        QByteArray data;
        uint mode;
        uint state;
        int num_entries;

        in >> data;
        if (!data.isEmpty()) {
#ifndef QT_NO_COMPRESS
            if (Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
                data = qUncompress(data);
            }
#endif
            if (!data.isEmpty())
                d->svgBuffers->insert(d->hashKey(QIcon::Normal, QIcon::Off), data);
        }
        in >> num_entries;
        for (int i=0; i<num_entries; ++i) {
            if (in.atEnd())
                return false;
            in >> pixmap;
            in >> mode;
            in >> state;
            // The pm list written by 4.3 is buggy and/or useless, so ignore.
            //addPixmap(pixmap, QIcon::Mode(mode), QIcon::State(state));
        }
    }

    return true;
}


bool QSvgIconEngine::write(QDataStream &out) const
{
    if (out.version() >= QDataStream::Qt_4_4) {
        int isCompressed = 0;
#ifndef QT_NO_COMPRESS
        if (Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
            isCompressed = 1;
        }
#endif
        QHash<int, QByteArray> svgBuffers;
        if (d->svgBuffers)
            svgBuffers = *d->svgBuffers;
        for (auto it = d->svgFiles.cbegin(), end = d->svgFiles.cend(); it != end; ++it) {
            QByteArray buf;
            QFile f(it.value());
            if (f.open(QIODevice::ReadOnly))
                buf = f.readAll();
#ifndef QT_NO_COMPRESS
            if (Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
                buf = qCompress(buf);
            }
#endif
            svgBuffers.insert(it.key(), buf);
        }
        out << d->svgFiles << isCompressed << svgBuffers;
        if (d->addedPixmaps)
            out << (int)1 << *d->addedPixmaps;
        else
            out << (int)0;
    }
    else {
        QByteArray buf;
        if (d->svgBuffers)
            buf = d->svgBuffers->value(d->hashKey(QIcon::Normal, QIcon::Off));
        if (buf.isEmpty()) {
            QString svgFile = d->svgFiles.value(d->hashKey(QIcon::Normal, QIcon::Off));
            if (!svgFile.isEmpty()) {
                QFile f(svgFile);
                if (f.open(QIODevice::ReadOnly))
                    buf = f.readAll();
            }
        }
#ifndef QT_NO_COMPRESS
        if (Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
            buf = qCompress(buf);
        }
#endif
        out << buf;
        // 4.3 has buggy handling of added pixmaps, so don't write any
        out << (int)0;
    }
    return true;
}

QT_END_NAMESPACE

#endif // QT_NO_SVGRENDERER
