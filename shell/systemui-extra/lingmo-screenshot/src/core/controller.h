/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors

 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include "src/core/capturerequest.h"
#include <QObject>
#include <QPointer>
#include <QPixmap>
#include <QMap>
#include <QTimer>
#include <functional>
#include "src/widgets/capturelauncher.h"
#include "src/widgets/shortcutwidget.h"
#include "src/widgets/infowidget.h"
#include  "src/widgets/delaywidget.h"
#include "windowmanager/windowmanager.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "src/widgets/aboutdialog.h"

using namespace kdk;

class CaptureWidget;
class ConfigWindow;
class QSystemTrayIcon;

using lambda = std::function<void (void)>;

class Controller : public QObject
{
    Q_OBJECT

public:
    static Controller *getInstance();

    Controller(const Controller &) = delete;
    void operator =(const Controller &) = delete;

    void enableExports();
    void disableScreenCut();
    bool getScreenSaveLockStatus();
    void setScreenSaveLockStatus(bool status);
signals:
    void captureTaken(uint id, QPixmap p);
    void captureFailed(uint id);
    void captureExit(uint id);
    void captureCopy(uint id);
public slots:
    void requestCapture(const CaptureRequest &request);

    void openConfigWindow();
    void openShotCutWindow();
    void openInfoWindow();
    void openLauncherWindow();
    void enableTrayIcon();
    void disableTrayIcon();
    void sendTrayNotification(const QString &text,
                              const QString &title = QStringLiteral("lingmo-screenshot Info"),
                              const int timeout = 5000);

    void updateConfigComponents();

    void deletePidfile();

private slots:
    void startFullscreenCapture(const uint id = 0);
    void startVisualCapture(const uint id = 0, const QString &forcedSavePath = QString());
    void startScreenGrab(const uint id = 0, const int screenNumber = -1);

    void handleCaptureTaken(uint id, QPixmap p);
    void handleCaptureFailed(uint id);
public slots:
    void emitSCreenSaveLock();
    void emitSCreenSaveUnLock();

private:
    Controller();

    // replace QTimer::singleShot introduced in Qt 5.4
    // the actual target Qt version is 5.3
    void doLater(int msec, QObject *receiver, lambda func);

    QMap<uint, CaptureRequest> m_requestMap;
    QPointer<CaptureWidget> m_captureWindow;
    QPointer<infoWidget> m_infoWindow;
    QPointer<AboutDialog> m_aboutdialog;
    QPointer<ConfigWindow> m_configWindow;
    QPointer<QSystemTrayIcon> m_trayIcon;
    QPointer<CaptureLauncher> m_launcherWindow;
    QPointer<ShortCutWidget> m_shortCutWindow;
    bool m_onScreenLock = false;
};
