/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#include "screenshotdialog.h"
#include "screenshotdialog_debug.h"

#include <QPushButton>

#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QFutureWatcher>
#include <QStandardItemModel>
#include <QTimer>
#include <QWindow>
#include <QtConcurrentRun>
#include <qplatformdefs.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

static int readData(int fd, QByteArray &data)
{
    char buffer[4096];
    pollfd pfds[1];
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;

    while (true) {
        // give user 30 sec to click a window, afterwards considered as error
        const int ready = poll(pfds, 1, 30000);
        if (ready < 0) {
            if (errno != EINTR) {
                qWarning() << "poll() failed:" << strerror(errno);
                return -1;
            }
        } else if (ready == 0) {
            qDebug() << "failed to read screenshot: timeout";
            return -1;
        } else if (pfds[0].revents & POLLIN) {
            const int n = QT_READ(fd, buffer, sizeof(buffer));

            if (n < 0) {
                qWarning() << "read() failed:" << strerror(errno);
                return -1;
            } else if (n == 0) {
                return 0;
            } else if (n > 0) {
                data.append(buffer, n);
            }
        } else if (pfds[0].revents & POLLHUP) {
            return 0;
        } else {
            qWarning() << "failed to read screenshot: pipe is broken";
            return -1;
        }
    }

    Q_UNREACHABLE();
}

static QImage allocateImage(const QVariantMap &metadata)
{
    bool ok;

    const uint width = metadata.value(QStringLiteral("width")).toUInt(&ok);
    if (!ok) {
        return QImage();
    }

    const uint height = metadata.value(QStringLiteral("height")).toUInt(&ok);
    if (!ok) {
        return QImage();
    }

    const uint format = metadata.value(QStringLiteral("format")).toUInt(&ok);
    if (!ok) {
        return QImage();
    }

    return QImage(width, height, QImage::Format(format));
}

static QImage readImage(int pipeFd, const QVariantMap &metadata)
{
    QByteArray content;
    if (readData(pipeFd, content) != 0) {
        close(pipeFd);
        return QImage();
    }
    close(pipeFd);

    QImage result = allocateImage(metadata);
    if (result.isNull()) {
        return QImage();
    }

    QDataStream ds(content);
    ds.readRawData(reinterpret_cast<char *>(result.bits()), result.sizeInBytes());
    return result;
}

ScreenshotDialog::ScreenshotDialog(QObject *parent)
    : QuickDialog(parent)
{
    QStandardItemModel *model = new QStandardItemModel(this);
    model->appendRow(new QStandardItem(i18n("Full Screen")));
    model->appendRow(new QStandardItem(i18n("Current Screen")));
    model->appendRow(new QStandardItem(i18n("Active Window")));
    create(QStringLiteral("qrc:/ScreenshotDialog.qml"),
           {
               {"app", QVariant::fromValue<QObject *>(this)},
               {"screenshotTypesModel", QVariant::fromValue<QObject *>(model)},
           });
}

QImage ScreenshotDialog::image() const
{
    return m_image;
}

void ScreenshotDialog::takeScreenshotNonInteractive()
{
    QFuture<QImage> future = takeScreenshot();
    if (!future.isStarted()) {
        return;
    }

    future.waitForFinished();
    m_image = future.result();
}

void ScreenshotDialog::takeScreenshotInteractive()
{
    const QFuture<QImage> future = takeScreenshot();
    if (!future.isStarted()) {
        return;
    }

    QFutureWatcher<QImage> *watcher = new QFutureWatcher<QImage>(this);
    QObject::connect(watcher, &QFutureWatcher<QImage>::finished, this, [watcher, this] {
        watcher->deleteLater();
        m_image = watcher->result();
        m_theDialog->setProperty("screenshotImage", m_image);
    });

    watcher->setFuture(future);
}

QFuture<QImage> ScreenshotDialog::takeScreenshot()
{
    int pipeFds[2];
    if (pipe2(pipeFds, O_CLOEXEC | O_NONBLOCK) != 0) {
        Q_EMIT failed();
        return {};
    }

    QVariantMap options;
    options.insert(QStringLiteral("native-resolution"), true); // match xdg-desktop-portal-gnome, and fixes flameshot!3164
    if (m_theDialog->property("withCursor").toBool()) {
        options.insert(QStringLiteral("include-cursor"), true);
    }
    if (m_theDialog->property("withBorders").toBool()) {
        options.insert(QStringLiteral("include-decoration"), true);
    }

    QString method;
    switch (ScreenshotType(m_theDialog->property("screenshotType").toInt())) {
    case FullScreen:
        method = QStringLiteral("CaptureWorkspace");
        break;
    case CurrentScreen:
        method = QStringLiteral("CaptureActiveScreen");
        break;
    case ActiveWindow:
        method = QStringLiteral("CaptureActiveWindow");
        break;
    }

    QVariantList arguments;
    arguments.append(options);
    arguments.append(QVariant::fromValue(QDBusUnixFileDescriptor(pipeFds[1])));

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin.ScreenShot2"),
                                                          QStringLiteral("/org/kde/KWin/ScreenShot2"),
                                                          QStringLiteral("org.kde.KWin.ScreenShot2"),
                                                          method);
    message.setArguments(arguments);

    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(message);
    ::close(pipeFds[1]);
    if (!reply.isValid()) {
        qCWarning(XdgDesktopPortalKdeScreenshotDialog) << method << "failed:" << reply.error();
        close(pipeFds[0]);
        return QFuture<QImage>();
    }

    return QtConcurrent::run(readImage, pipeFds[0], reply);
}
