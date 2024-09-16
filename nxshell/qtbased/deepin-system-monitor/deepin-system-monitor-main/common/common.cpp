// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "stack_trace.h"
#include "hash.h"
#include "helper.hpp"

#include <QPainter>
#include <QString>
#include <QtDBus>
#include <QDesktopServices>
#include <QApplication>

namespace common {

void displayShortcutHelpDialog(const QRect &rect)
{
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    // system section
    QJsonObject sysObj;
    sysObj.insert("groupName", QApplication::translate("Help.Shortcut.System", "System"));
    QJsonArray sysObjArr;

    // display shortcut shortcut help
    QJsonObject shortcutItem;
    shortcutItem.insert("name",
                        QApplication::translate("Help.Shortcut.System", "Display shortcuts"));
    shortcutItem.insert("value", "Ctrl+Shift+?");
    sysObjArr.append(shortcutItem);

    // display search shortcut help
    QJsonObject searchItem;
    searchItem.insert("name", QApplication::translate("Title.Bar.Search", "Search"));
    searchItem.insert("value", "Ctrl+F");
    sysObjArr.append(searchItem);

    sysObj.insert("groupItems", sysObjArr);
    jsonGroups.append(sysObj);

    // processes section
    QJsonObject procObj;
    procObj.insert("groupName", QApplication::translate("Title.Bar.Switch", "Processes"));
    QJsonArray procObjArr;

    // force end application shortcut help
    QJsonObject killAppItem;
    killAppItem.insert("name",
                       QApplication::translate("Title.Bar.Context.Menu", "Force end application"));
    killAppItem.insert("value", "Ctrl+Alt+K");
    procObjArr.append(killAppItem);

    // end process shortcut help
    QJsonObject endProcItem;
    endProcItem.insert("name",
                       QApplication::translate("Process.Table.Context.Menu", "End process"));
    endProcItem.insert("value", "Alt+E");
    procObjArr.append(endProcItem);
    // suspend process shortcut help
    QJsonObject pauseProcItem;
    pauseProcItem.insert("name",
                         QApplication::translate("Process.Table.Context.Menu", "Suspend process"));
    pauseProcItem.insert("value", "Alt+P");
    procObjArr.append(pauseProcItem);
    // resume process shortcut help
    QJsonObject resumeProcItem;
    resumeProcItem.insert("name",
                          QApplication::translate("Process.Table.Context.Menu", "Resume process"));
    resumeProcItem.insert("value", "Alt+C");
    procObjArr.append(resumeProcItem);
    // properties shortcut help
    QJsonObject propItem;
    propItem.insert("name", QApplication::translate("Process.Table.Context.Menu", "Properties"));
    propItem.insert("value", "Alt+Enter");
    procObjArr.append(propItem);
    // kill process shortcut help
    QJsonObject killProcItem;
    killProcItem.insert("name",
                        QApplication::translate("Process.Table.Context.Menu", "Kill process"));
    killProcItem.insert("value", "Alt+K");
    procObjArr.append(killProcItem);

    procObj.insert("groupItems", procObjArr);
    jsonGroups.append(procObj);

    // services section
    QJsonObject svcObj;
    svcObj.insert("groupName", QApplication::translate("Title.Bar.Switch", "Services"));
    QJsonArray svcObjArr;

    // start service shortcut help
    QJsonObject startSvcItem;
    startSvcItem.insert("name", QApplication::translate("Service.Table.Context.Menu", "Start"));
    startSvcItem.insert("value", "Alt+S");
    svcObjArr.append(startSvcItem);
    // stop service shortcut help
    QJsonObject stopSvcItem;
    stopSvcItem.insert("name", QApplication::translate("Service.Table.Context.Menu", "Stop"));
    stopSvcItem.insert("value", "Alt+T");
    svcObjArr.append(stopSvcItem);
    // restart service shortcut help
    QJsonObject restartSvcItem;
    restartSvcItem.insert("name", QApplication::translate("Service.Table.Context.Menu", "Restart"));
    restartSvcItem.insert("value", "Alt+R");
    svcObjArr.append(restartSvcItem);
    // refresh service shortcut help
    QJsonObject refreshSvcItem;
    refreshSvcItem.insert("name", QApplication::translate("Service.Table.Context.Menu", "Refresh"));
    refreshSvcItem.insert("value", "F5");
    svcObjArr.append(refreshSvcItem);

    svcObj.insert("groupItems", svcObjArr);
    jsonGroups.append(svcObj);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    // fork new shortcut display process
    QProcess shortcutViewProcess;
    QStringList shortcutString;
    // shortcut help json formatted text param
    QString param1 = "-j=" + QString(doc.toJson().data());
    // display position param
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    // detach this process
    shortcutViewProcess.startDetached("deepin-shortcut-viewer", shortcutString);
}

int getStatusBarMaxWidth()
{
    // TODO: use more elegent way to calc bar width
    return 280;
}

void drawLoadingRing(QPainter &painter, int centerX, int centerY, int radius, int penWidth,
                     int loadingAngle, int rotationAngle, QColor foregroundColor,
                     double foregroundOpacity, QColor backgroundColor, double backgroundOpacity,
                     double percent)
{
    drawRing(painter, centerX, centerY, radius, penWidth, loadingAngle, rotationAngle,
             backgroundColor, backgroundOpacity);
    drawRing(painter, centerX, centerY, radius, penWidth, int(loadingAngle * percent), rotationAngle,
             foregroundColor, foregroundOpacity);
}

void drawRing(QPainter &painter, int centerX, int centerY, int radius, int penWidth,
              int loadingAngle, int rotationAngle, QColor color, double opacity)
{
    QRect drawingRect;

    drawingRect.setX(centerX - radius + penWidth);
    drawingRect.setY(centerY - radius + penWidth);
    drawingRect.setWidth(radius * 2 - penWidth * 2);
    drawingRect.setHeight(radius * 2 - penWidth * 2);

    painter.setOpacity(opacity);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(QBrush(QColor(color)), penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    int arcLengthApproximation = penWidth + penWidth / 3;
    painter.drawArc(drawingRect, 90 * 16 - arcLengthApproximation + rotationAngle * 16,
                    -loadingAngle * 16);
}

bool startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() == QChar::Script_Han;
}

void openFilePathItem(const QString &path)
{
    bool result = QProcess::startDetached(QString("dde-file-manager --show-item \"%1\"").arg(path));
    if (!result) {
        QDesktopServices::openUrl(QUrl(path));
    }
}

namespace init {
QList<QString> shellList;
QList<QString> scriptList;
QList<QString> pathList;
//默认为低性能，防止在获取CPU性能之前就卡死
CPUMaxFreq CPUPerformance = CPUMaxFreq::High;
bool WaylandCentered;
int specialComType = -1;

void WaylandSearchCentered()
{
#ifdef USE_DEEPIN_WAYLAND
    auto e = QProcessEnvironment::systemEnvironment();

    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        WaylandCentered = true;
        if (!common::systemInfo().isOldVersion() && !common::systemInfo().isTreeLand())
            qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    } else {
        WaylandCentered = false;
    }
#else
    WaylandCentered = false;
#endif //USE_DEEPIN_WAYLAND

}

static void init_shell_list()
{
    FILE *fp;
    fp = fopen("/etc/shells", "r");
    if (fp) {
        char buf[128] {};
        char *s;
        while ((s = fgets(buf, 128, fp))) {
            if (s[0] == '/') {
                auto sh = QLatin1String(basename(s));
                if (sh.endsWith('\n'))
                    sh.chop(1);
                if (!shellList.contains(sh)) {
                    shellList << sh;
                }
            }
        }
        fclose(fp);
    }
}

static void init_script_list()
{
    // fill scripting lang list (!!far from complete)
    scriptList << "/usr/bin/python";
    scriptList << "/usr/bin/perl";
    scriptList << "/usr/bin/php";
    scriptList << "/usr/bin/ruby";
}

static void init_path_list()
{
    // fill environment path
    auto paths = qgetenv("PATH");
    auto list = paths.split(':');
    if (list.size() > 0)
        for (auto path : list)
            pathList << path;
    else {
        // use default path
        pathList << "/bin";
        pathList << "/usr/bin";
    }
}

unsigned int kb_shift;
unsigned long HZ;

static void get_HZ()
{
    long ticks;

    if ((ticks = sysconf(_SC_CLK_TCK)) == -1) {
        perror("sysconf");
    }

    HZ = ulong(ticks);
}

static void get_kb_shift()
{
    int shift = 0;
    long size;

    /* One can also use getpagesize() to get the size of a page */
    if ((size = sysconf(_SC_PAGESIZE)) == -1) {
        perror("sysconf");
    }

    size >>= 10; /* Assume that a page has a minimum size of 1 kB */

    while (size > 1) {
        shift++;
        size >>= 1;
    }

    kb_shift = uint(shift);
}

void global_init()
{
    util::installCrashHandler();
    util::common::init_seed();

    init_shell_list();
    init_script_list();
    init_path_list();

    get_HZ();
    get_kb_shift();
}
} // namespace init

QString format::formatHz(quint32 freq, format::HzUnit base, int prec)
{
    int u = base;
    qreal v = freq;

    while (v > 1000. && u <= ZHz) {
        v /= 1000;
        u++;
    }

    return QString("%1%2").arg(v, 0, 'f', prec).arg(HzUnitSuffix[u]);
} // ::format::formatHz

//内存、磁盘统一单位
QString format::formatUnit_memory_disk(QVariant size, format::SizeUnit base, int prec, bool isSpeed)
{
    int u = base;
    if (!size.canConvert(QMetaType::Double))
        return {};
    qreal v = size.toReal();

    while (v > 1024. && u <= EB) {
        v /= 1024;
        u++;
    }

    if (isSpeed) {
        return QString("%1 %2%3").arg(v, 0, 'f', prec).arg(UnitSuffixother[u]).arg("/s");
    }
    return QString("%1 %2").arg(v, 0, 'f', prec).arg(UnitSuffixother[u]);
}

//网络统一单位
QString format::formatUnit_net(QVariant size, format::SizeUnit base, int prec, bool isSpeed)
{
    int u = base;
    if (!size.canConvert(QMetaType::Double))
        return {};
    qreal v = size.toReal();

    while (v > 1024. && u <= EB) {
        v /= 1024;
        u++;
    }

    if (isSpeed) {
        return QString("%1 %2%3").arg(v, 0, 'f', prec).arg(UnitSuffixnet[u]).arg("/s");
    }
    //统一单位
    return QString("%1 %2").arg(v, 0, 'f', prec).arg(UnitSuffixnet[u]);
}

// ::format::formatUnit
} // namespace common
