// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "screenshotportal.h"
#include "protocols/common.h"
#include "protocols/treelandcapture.h"

#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QDir>
#include <QRegion>
#include <QStandardPaths>

#include <private/qwaylandscreen_p.h>

Q_LOGGING_CATEGORY(portalWayland, "ocean.portal.wayland");
struct ScreenCaptureInfo {
    QtWaylandClient::QWaylandScreen *screen {nullptr};
    QPointer<ScreenCopyFrame> capturedFrame {nullptr};
    QImage capturedImage {};
};

ScreenshotPortalWayland::ScreenshotPortalWayland(PortalWaylandContext *context)
    : AbstractWaylandPortal(context)
{

}

uint ScreenshotPortalWayland::PickColor(const QDBusObjectPath &handle,
                                 const QString &app_id,
                                 const QString &parent_window, // might just ignore this argument now
                                 const QVariantMap &options,
                                 QVariantMap &results)
{
    // TODO Implement PickColor
    return 0;
}

QString ScreenshotPortalWayland::fullScreenShot()
{
    std::list<std::shared_ptr<ScreenCaptureInfo>> captureList;
    int pendingCapture = 0;
    auto screenCopyManager = context()->screenCopyManager();
    QEventLoop eventLoop;
    QRegion outputRegion;
    QImage::Format formatLast;
    // Capture each output
    for (auto screen : waylandDisplay()->screens()) {
        auto info = std::make_shared<ScreenCaptureInfo>();
        outputRegion += screen->geometry();
        auto output = screen->output();
        info->capturedFrame = screenCopyManager->captureOutput(false, output);
        info->screen = screen;
        ++pendingCapture;
        captureList.push_back(info);
        connect(info->capturedFrame, &ScreenCopyFrame::ready, this, [&formatLast, info, &pendingCapture, &eventLoop, this](QImage image) {
            info->capturedImage = image;
            formatLast = info->capturedImage.format();
            if (--pendingCapture == 0) {
                eventLoop.quit();
            }
        });
        connect(info->capturedFrame, &ScreenCopyFrame::failed, this, [&pendingCapture, &eventLoop]{
            if (--pendingCapture == 0) {
                eventLoop.quit();
            }
        });
    }
    eventLoop.exec();
    // Cat them according to layout
    QImage image(outputRegion.boundingRect().size(), formatLast);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    for (const auto &info : std::as_const(captureList)) {
        if (!info->capturedImage.isNull()) {
            QRect targetRect = info->screen->geometry();
            // Convert to screen image local coordinates
            auto sourceRect = targetRect;
            sourceRect.moveTo(sourceRect.topLeft() - info->screen->geometry().topLeft());
            p.drawImage(targetRect, info->capturedImage, sourceRect);
        } else {
            qCWarning(portalWayland) << "image is null!!!";
        }
    }
    static const char *SaveFormat = "PNG";
    auto saveBasePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir saveBaseDir(saveBasePath);
    if (!saveBaseDir.exists()) return "";
    QString picName = "portal screenshot - " + QDateTime::currentDateTime().toString() + ".png";
    if (image.save(saveBaseDir.absoluteFilePath(picName), SaveFormat)) {
        return saveBaseDir.absoluteFilePath(picName);
    } else {
        return "";
    }
}
QString ScreenshotPortalWayland::captureInteractively()
{
    auto captureManager = context()->treelandCaptureManager();
    auto captureContext = captureManager->getContext();
    if (!captureContext) {
        return "";
    }
    captureContext->selectSource(QtWayland::treeland_capture_context_v1::source_type_output
                                         | QtWayland::treeland_capture_context_v1::source_type_window
                                         | QtWayland::treeland_capture_context_v1::source_type_region
                                 ,true
                                 , false
                                 ,nullptr);
    QEventLoop loop;
    connect(captureContext, &TreeLandCaptureContext::sourceReady, &loop, &QEventLoop::quit);
    loop.exec();
    auto frame = captureContext->frame();
    QImage result;
    connect(frame, &TreeLandCaptureFrame::ready, this, [this, &result, &loop](QImage image) {
        result = image;
        loop.quit();
    });
    connect(frame, &TreeLandCaptureFrame::failed, &loop, &QEventLoop::quit);
    loop.exec();
    if (result.isNull()) return "";
    auto saveBasePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir saveBaseDir(saveBasePath);
    if (!saveBaseDir.exists()) return "";
    QString picName = "portal screenshot - " + QDateTime::currentDateTime().toString() + ".png";
    if (result.save(saveBaseDir.absoluteFilePath(picName), "PNG")) {
        return saveBaseDir.absoluteFilePath(picName);
    } else {
        return "";
    }
}

uint ScreenshotPortalWayland::Screenshot(const QDBusObjectPath &handle,
                                  const QString &app_id,
                                  const QString &parent_window,
                                  const QVariantMap &options,
                                  QVariantMap &results)
{
    if (options["modal"].toBool()) {
        // TODO if modal, we should block parent_window
    }
    QString filePath;
    if (options["interactive"].toBool()) {
        filePath = captureInteractively();
    } else {
        filePath = fullScreenShot();
    }
    if (filePath.isEmpty()) {
        return 1;
    }
    results.insert(QStringLiteral("uri"), QUrl::fromLocalFile(filePath).toString(QUrl::FullyEncoded));
    return 0;
}
