/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#ifndef PEONYDESKTOPAPPLICATION_H
#define PEONYDESKTOPAPPLICATION_H

#include "singleapplication.h"
#include "qtsingleapplication.h"
#include "volume-manager.h"
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <QScreen>
#include <QWindow>

class DesktopBackgroundWindow;

namespace Peony {
class DesktopIconView;
class DesktopItemModel;
}

using namespace Peony;

class QTimeLine;

class PeonyDesktopApplication : public QtSingleApplication
{
    Q_OBJECT
public:
    explicit PeonyDesktopApplication(int &argc, char *argv[], const QString &applicationName = "explor-qt-desktop");

    static bool userGuideDaemonRunning();
    static void showGuide(const QString &appName = "");
    static void gotoSetBackground();
    static void gotoSetResolution();

    static qint64 explor_desktop_start_time;
    static Peony::DesktopItemModel* getModel();
    Peony::DesktopIconView *getIconView(QPoint pos);
    Peony::DesktopIconView *getIconView(int id);
    Peony::DesktopIconView *getIconView(const KScreen::OutputPtr &output);
    DesktopBackgroundWindow *getWindow(int id);
    Peony::DesktopIconView * removeUri(const QString& uri);
    int checkScreenMode(const QRect &geometry);
    Peony::DesktopIconView *getNotFullView();
    void singleScreenMode();
    void multiscreenMode();

    // only used in model refresh.
    void clearViewCache();

Q_SIGNALS:
    void requestSetLINGMOOutputEnable(bool enable);
    void emitFinish();

protected Q_SLOTS:
    void parseCmd(QString msg, bool isPrimary);
    bool isPrimaryScreen(QScreen *screen);

public Q_SLOTS:
    void layoutDirectionChangedProcess(Qt::LayoutDirection direction);
    void primaryScreenChangedProcess(QScreen *screen);
    void screenAddedProcess(QScreen *screen);
    void screenRemovedProcess(QScreen *screen);
    void volumeRemovedProcess(const std::shared_ptr<Peony::Volume> &volume);

    void addWindow(QScreen *screen, bool checkPrimay = true);
    void changeBgProcess(const QString& bgPath);
    void checkWindowProcess();
    void updateVirtualDesktopGeometryByWindows();

    void addBgWindow(const KScreen::OutputPtr &output);
    void relocateIconView(const KScreen::OutputPtr &output);

    void outputAdded(const KScreen::OutputPtr &output);
    void outputRemoved(int outputId);
    void changeMode(int mode);

private:
    void setupDesktop();
    void setupBgAndDesktop();
    void setConfig(KScreen::ConfigOperation *op);
    void clearIcons(const QStringList &args);
    int getDesktopWindowId();

    bool m_first_parse = true;

    QList<DesktopBackgroundWindow *> m_bg_windows;

    QTimeLine *m_primaryScreenSettingsTimeLine = nullptr;

    KScreen::ConfigPtr m_config     = nullptr;
    int m_mode = 0;
};

#endif // PEONYDESKTOPAPPLICATION_H
