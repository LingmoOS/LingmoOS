// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlrdatacontrolclipboardinterface.h"
#include "wlrdatacontrolofferintegration.h"
#include "dwaylandmimedata.h"
#include <private/qwaylandnativeinterface_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qinternalmimedata_p.h>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <unistd.h>
#include <poll.h>

using namespace Qt::StringLiterals;

const QString PrivateMimeSavedForWayland = u"application/x.lingmo-clipboard-daemon.saved-for-wayland"_s;

// File descriptor close guard
class FdGuard {
public:
    FdGuard(int fd) : m_fd(fd) {}
    ~FdGuard() { close(m_fd); }
private:
    int m_fd;
};

// Extra MIME Type Preprocessing
static QStringList imageMimeFormats(const QList<QByteArray> &imageFormats)
{
    QStringList formats;
    formats.reserve(imageFormats.size());
    for (const auto &format : imageFormats)
        formats.append(QLatin1String("image/") + QLatin1String(format.toLower()));

    // put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);

    return formats;
}

static inline QStringList imageReadMimeFormats()
{
    return imageMimeFormats(QImageReader::supportedImageFormats());
}

// Used when a historical data is replayed (reborn) to retrieve desired data type from q plain QMimeData
static QByteArray getByteArray(QMimeData *mimeData, const QString &mimeType)
{
    QByteArray content;
    if (mimeType == QLatin1String("text/plain")) {
        content = mimeData->text().toUtf8();
    } else if (mimeData->hasImage()
               && (mimeType == QLatin1String("application/x-qt-image")
                   || mimeType.startsWith(QLatin1String("image/")))) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            QBuffer buf;
            buf.open(QIODevice::ReadWrite);
            QByteArray fmt = "PNG";
            if (mimeType.startsWith(QLatin1String("image/"))) {
                QByteArray imgFmt = mimeType.mid(6).toUpper().toLatin1();
                if (QImageWriter::supportedImageFormats().contains(imgFmt))
                    fmt = imgFmt;
            }
            QImageWriter wr(&buf, fmt);
            wr.write(image);
            content = buf.buffer();
        }
    } else if (mimeType == QLatin1String("application/x-color")) {
        content = qvariant_cast<QColor>(mimeData->colorData()).name().toLatin1();
    } else if (mimeType == QLatin1String("text/uri-list")) {
        QList<QUrl> urls = mimeData->urls();
        for (int i = 0; i < urls.count(); ++i) {
            content.append(urls.at(i).toEncoded());
            content.append('\n');
        }
    } else {
        content = mimeData->data(mimeType);
    }
    return content;
}

WlrDataControlClipboardInterface::WlrDataControlClipboardInterface(QObject *parent)
    : QObject{parent}
    , m_readLock(1)
{   
    m_dcManager = std::make_unique<WlrDataControlManagerIntegration>();

    // Wait until manager is ready, obtain device & source by then
    connect(m_dcManager.get(), &QWaylandClientExtension::activeChanged,
            this, &WlrDataControlClipboardInterface::onActiveChanged);

    // Clipboard read / write completion signal
    connect(&m_readTaskWatcher, &QFutureWatcher<std::unique_ptr<QMimeData>>::finished,
            this, &WlrDataControlClipboardInterface::onReadTaskFinished);
}

const QMimeData *WlrDataControlClipboardInterface::mimeData() const
{
    return m_mimeData.get();
}

void WlrDataControlClipboardInterface::setMimeData(QMimeData *mimeData)
{
    // Write clipboard Stage 1: Send MIME Type Offers
    if (!mimeData || mimeData->formats().isEmpty()) {
        delete mimeData;
        return;
    }

    // Replace MIME data
    m_mimeData = std::unique_ptr<QMimeData>(mimeData);
    takeoverClipboardDataSource();
}

void WlrDataControlClipboardInterface::refreshDataControlSourceDevice()
{
    // Create data control device to monitor clipboard content changes
    onDataControlDeviceFinished();
}

void WlrDataControlClipboardInterface::takeoverClipboardDataSource()
{
    // Create a new data control source (and possibly destroy old one)
    m_dcSource = std::make_unique<WlrDataControlSourceIntegration>(m_dcManager->create_data_source());
    if (!m_dcSource) {
        qWarning() << "Cannot create Data Control Source. Clipboard write is aborted.";
        return;
    }
    connect(m_dcSource.get(), &WlrDataControlSourceIntegration::send,
            this, &WlrDataControlClipboardInterface::onSourceSend);

    // Send MIME type offers
    for (const QString &format : m_mimeData->formats()) {
        // 如果是application/x-qt-image类型则需要提供image的全部类型, 比如image/png
        if (u"application/x-qt-image"_s == format) {
            for (auto &i : imageReadMimeFormats()) {
                m_dcSource->offer(i);
            }
        } else {
            m_dcSource->offer(format);
        }
    }

    // Tell the compositor that the clipboard content has changed.
    // Next time someone pastes stuff, we'll receive a signal and enter Stage 2.
    m_dcDevice->set_selection(m_dcSource.get()->object());
}

void WlrDataControlClipboardInterface::abortLastRead()
{
    if (m_readTaskFuture.isRunning()) {
        qWarning() << "An ongoing read was aborted.";
        m_readTaskFuture.cancel();
    }
}

void WlrDataControlClipboardInterface::onDataControlDeviceFinished()
{
    // Abort an ongoing read, if any
    abortLastRead();

    auto waylandIface = static_cast<QtWaylandClient::QWaylandNativeInterface *>(qGuiApp->platformNativeInterface());
    m_dcDevice = std::make_unique<WlrDataControlDeviceIntegration>(m_dcManager->get_data_device(waylandIface->seat()));

    connect(m_dcDevice.get(), &WlrDataControlDeviceIntegration::finished,
            this, &WlrDataControlClipboardInterface::onDataControlDeviceFinished);
    connect(m_dcDevice.get(), &WlrDataControlDeviceIntegration::newSelection,
            this, &WlrDataControlClipboardInterface::onNewSelection);
}

void WlrDataControlClipboardInterface::onNewSelection(WlrDataControlOfferIntegration *offer)
{
    // Read clipboard procedures.
    // Stage 1 (1.1, 1.2) are in DataControlDevice.
    // Stage 2: DataControlDevice sends us an offer.
    if (!offer) {
        qWarning() << "Offer not valid";
        return;
    }

    // Filter MIME types.
    auto mimeTypes = QStringList(offer->availableMimeTypes());

    // Detect recursion caused by saving clipboard content (see onReadTaskFinished for explanation)
    if (mimeTypes.contains(PrivateMimeSavedForWayland)) {
        return;
    }

    for (auto it = mimeTypes.begin(); it != mimeTypes.end(); ) {
        // 根据窗管的要求，不读取纯大写、和不含'/'的字段，因为源窗口可能没有写入这些字段的数据，导致获取数据的线程一直等待。
        if ((it->contains("/") || it->toUpper() != *it)
            || *it == "FROM_LINGMO_CLIPBOARD_MANAGER"
            || *it == "TIMESTAMP") {
            ++it;
        } else {
            // Remove such entries
            it = mimeTypes.erase(it);
        }
    }
    if (mimeTypes.isEmpty()) {
        qWarning() << "No acceptable MIME types found, exiting.";
        return;
    }

    // Abort ongoing read if any, wait until it fully stops
    if (!m_readLock.tryAcquire()) {
        abortLastRead();
        m_readLock.acquire();
    }

    // Stage 3: Start a read.
    // QMimeData is "not move constructible" so we had to wrap it with a unique_ptr
    auto result = std::make_unique<DWaylandMimeData>();

    // Delete offer object automatically
    auto offerGuard = std::unique_ptr<WlrDataControlOfferIntegration>(offer);

    // Auto release semaphore
    QSemaphoreReleaser semReleaser(m_readLock);

    // Used to force roundtrip
    auto display = QtWaylandClient::QWaylandIntegration::instance()->display();

    // Read each requested MIME type
    for (auto i : mimeTypes) {
        // Open communication pipe
        int pipefd[2];
        if (pipe(pipefd) != 0) {
            // Fail
            qCritical() << "Failed to create pipe, errno =" << errno;
            return;
        }

        // Close pipe read end automatically in case of error
        FdGuard fdGuard(pipefd[0]);

        // Tell it what MIME type we want, and close our copy of pipe write end
        offer->receive(i, pipefd[1]);
        close(pipefd[1]);
        display->forceRoundTrip(); // FIXME: Sometimes it will deadlock

        // Begin reading from pipe
        QFile pipeFile;
        if (!pipeFile.open(pipefd[0], QFile::ReadOnly)) {
            qWarning() << "Cannot open pipe; error " << pipeFile.errorString();
            continue;
        }
        if (!pipeFile.isReadable()) {
            qWarning() << "Pipe is not readable; failing.";
            return;
        }

        QByteArray data;
        constexpr int readLimit = 4096;
        QByteArray buffer(readLimit, 0);
        int readCount = 0;
        do {
            auto buf = pipeFile.read(readLimit);
            readCount = buf.size();
            data.append(buf);
        } while (readCount == readLimit);

        result->setData(i, data);
    }
    m_mimeData = std::move(result);

    Q_EMIT dataChanged();

    // FIXME: Because clipboard will discard any data other than its known type (if a rich text copied from
    // word processor can be identified as an image, then all it knows about the content is an image, the
    // rich text is discarded), we should not use the takeover behavior as it might completely destroy the
    // user's clipboard content.
#if 0
    // Take over the Wayland clipboard data source. This is specific to Wayland: on Wayland the clipboard
    // content is kept by copy-source application, and when source application is closed, the clipboard
    // content is lost. We emulate X11 behavior (as it's the most sensible behavior for users) by taking
    // over clipboard data source.

    // Set a private MIME type to break the possible recursion.
    m_mimeData->setData(PrivateMimeSavedForWayland, QByteArray());
    // Take over clipboard.
    takeoverClipboardDataSource();
#endif
}

void WlrDataControlClipboardInterface::onReadTaskFinished()
{
}

void WlrDataControlClipboardInterface::onSourceSend(QString mimeType, int fd)
{
    // Write clipboard Stage 3: dispatch write task.
    // This should be put into a thread because daemon itself will also reply on reading
    // the clipboard (design burden)
    auto _ = QtConcurrent::run([](QByteArray data, int fd){
        FdGuard fdGuard(fd);
        QFile fdFile;
        if (!fdFile.open(fd, QFile::WriteOnly)) {
            qWarning() << "Cannot open pipe; error:" << fdFile.errorString();
            return;
        }
        if (!fdFile.isWritable()) {
            qWarning() << "Pipe file is not writable.";
            return;
        }

        fdFile.write(data);
    }, getByteArray(m_mimeData.get(), mimeType), fd);
}

void WlrDataControlClipboardInterface::onSourceCancelled()
{
    // Destroy the data source.
    m_dcSource = nullptr;
}
