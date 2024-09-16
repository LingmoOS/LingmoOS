// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calculate_util.h"
#include "util.h"
#include "constants.h"
#include "appslistmodel.h"
#include "amdbuslauncherinterface.h"
#include "amdbusdockinterface.h"
#include "aminterface.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

QPointer<CalculateUtil> CalculateUtil::INSTANCE = nullptr;

CalculateUtil *CalculateUtil::instance()
{
    if (INSTANCE.isNull())
        INSTANCE = new CalculateUtil(nullptr);

    return INSTANCE;
}

qreal CalculateUtil::getCurRatio()
{
    return m_launcherGsettings ? m_launcherGsettings->get("apps-icon-ratio").toDouble() : 0.5;
}

QSize CalculateUtil::appIconSize(int modelMode) const
{
    if (modelMode == AppsListModel::TitleMode || modelMode == AppsListModel::LetterMode)
        return QSize(DLauncher::APP_ITEM_ICON_SIZE, DLauncher::APP_ITEM_ICON_SIZE);

    // 相应模式的应用大小在计算时确定
    return QSize(m_appItemSize, m_appItemSize) * DLauncher::DEFAULT_RATIO;
}

/**
 * @brief CalculateUtil::getScreenScaleX
 * 获取屏幕宽度为1920的倍数
 * @return 获取屏幕宽度为1920的倍数
 */
double CalculateUtil::getScreenScaleX()
{
    int width  = currentScreen()->geometry().width();
    return double(width) / 1920;
}

/**
 * @brief CalculateUtil::getScreenScaleY
 * 取屏幕高度为1080的倍数
 * @return 取屏幕高度为1080的倍数
 */
double CalculateUtil::getScreenScaleY()
{
    int width = currentScreen()->geometry().height();
    return double(width) / 1080;
}

QSize CalculateUtil::getScreenSize() const
{
    return currentScreen()->geometry().size();
}

QSize CalculateUtil::getAppBoxSize()
{
    int height = int(currentScreen()->geometry().height() * 0.69);
    int width = int(currentScreen()->geometry().width() * 0.51);
    return  QSize(width, height);
}

/**
 * @brief CalculateUtil::calendarSelectIcon
 * 根据系统时间设置日历app的月、周、日样式
 * @return 返回日历app的月、周、日资源路径list
 */
QStringList CalculateUtil::calendarSelectIcon() const
{
    QStringList iconList;
    iconList.clear();
    iconList.append(QString(":/icons/skin/icons/calendar_bg.svg"));
    int month_num = QDate::currentDate().month();
    switch(month_num)
    {
    case 1 ... 12:
        iconList.append(QString(":/icons/skin/icons/calendar_month/month%1.svg").arg(month_num));
        break;
    default:
        //default , if month is invalid
        iconList.append(QString(":/icons/skin/icons/calendar_month/month4.svg"));
        break;
    }
    int day_num = QDate::currentDate().day();
    switch(day_num)
    {
    case 1 ... 31:
        iconList.append(QString(":/icons/skin/icons/calendar_day/day%1.svg").arg(day_num));
        break;
    default:
        //default , if day is invalid
        iconList.append(QString(":/icons/skin/icons/calendar_day/day23.svg"));
        break;
    }
    int week_num = QDate::currentDate().dayOfWeek();
    switch(week_num)
    {
    case 1 ... 7:
        iconList.append(QString(":/icons/skin/icons/calendar_week/week%1.svg").arg(week_num));
        break;
    default:
        //default , if week is invalid
        iconList.append(QString(":/icons/skin/icons/calendar_week/week4.svg"));
        break;
    }
    return iconList;
}

void CalculateUtil::calculateAppLayout(const QSize &containerSize, const int currentmode)
{
#ifdef QT_DEBUG
    qInfo() << " currentmode : " << currentmode;
#endif

    if (fullscreen())
        calculateTextSize();

    int rows = 1;
    int cols = 7;
    int containerW = containerSize.width();
    int containerH = containerSize.height();

    if (!fullscreen()) {
        cols = 4;
        rows = 2;
    } else if (currentmode == AppsListModel::FullscreenAll) {
        cols = 7;
        rows = 4;
    } else if (currentmode == AppsListModel::Search) {
        cols = 7;
        rows = 1;
    } else if (currentmode == AppsListModel::PluginSearch) {
        cols = 4;
        rows = 1;
    }

    // 默认边距保留最小5像素
    m_appMarginLeft = 5;
    m_appMarginTop = 5;

    // 去掉默认边距后，计算每个Item区域的宽高
    int perItemWidth  = (containerW - m_appMarginLeft * 2) / cols;
    int perItemHeight = (containerH - m_appMarginTop) / rows;

    // 因为每个Item是一个正方形的，所以取宽高中最小的值
    int perItemSize = qMin(perItemHeight, perItemWidth);

    // 图标大小取区域的4 / 5
    m_appItemSize = perItemSize * 4 / 5;
    m_appItemSpacing = (perItemSize - m_appItemSize) / 2;

    if (!fullscreen()) {
        /* 分类模式图标固定，收藏列表，所有应用列表图标大小一致*/
#ifdef QT_DEBUG
        qInfo() << __LINE__ << ", window app size: " << perItemSize;
#endif
        m_appMarginLeft = (containerW - m_appItemSize * cols - m_appItemSpacing * (cols - 1)) / 2;
        m_appMarginTop =  (containerH - m_appItemSize * rows - m_appItemSpacing * (rows - 1)) / 2;
    } else if (currentmode == AppsListModel::FullscreenAll) {
#ifdef QT_DEBUG
        qInfo() << __LINE__ << ", fullscreen app size: " << perItemSize;
#endif
        // 重新计算左右上边距
        m_appMarginLeft = (containerW - m_appItemSize * cols - m_appItemSpacing * cols * 2) / 2 - 1;
        m_appMarginTop =  (containerH - m_appItemSize * rows - m_appItemSpacing * rows * 2) / 2;
    } else if (currentmode == AppsListModel::Search) {
#ifdef QT_DEBUG
        qInfo() << __LINE__ << ", fullscreen search mode app size: " << perItemSize;
#endif
        m_appMarginLeft = (containerW - m_appItemSize * cols - m_appItemSpacing * cols * 2) / 2 - 1;
        m_appMarginTop =  (containerH - m_appItemSize * rows - m_appItemSpacing * rows * 2) / 2;
    }
    // 计算字体大小
    m_appItemFontSize = m_appItemSize <= 80 ? 8 : qApp->font().pointSize() + 3;

    emit layoutChanged();
}

/**
 * @brief CalculateUtil::CalculateUtil
 * 计算屏幕、应用列表布局、日历app等大小、间隔等
 * @param parent
 */
CalculateUtil::CalculateUtil(QObject *parent)
    : QObject(parent)
    , m_amDbusLauncher(new AMDBusLauncherInter(this))
    , m_amDbusDockInter(new AMDBusDockInter(this))
    , m_isFullScreen(AMInter::isAMReborn() ? AMInter::instance()->fullScreen() : m_amDbusLauncher->fullscreen())
    , m_launcherGsettings(SettingsPtr("com.deepin.dde.launcher", "/com/deepin/dde/launcher/", this))
    , m_appItemFontSize(12)
    , m_appItemSpacing(10)
    , m_appMarginLeft(0)
    , m_appMarginTop(0)
    , m_appItemSize(0)
    , m_navgationTextSize(14)
    , m_appPageItemCount(28)
    , m_titleTextSize(40)
    , m_categoryAppPageItemCount(12)
    , m_categoryCount(11)
    , m_currentCategory(4)
{
}

void CalculateUtil::calculateTextSize()
{
    if (currentScreen()->geometry().width() > 1366) {
        m_navgationTextSize = 14;
        m_titleTextSize = 40;
    } else {
        m_navgationTextSize = 11;
        m_titleTextSize = 38;
    }
}

QScreen *CalculateUtil::currentScreen() const
{
    QScreen *primaryScreen = qApp->primaryScreen();

    const QRect dockRect = m_amDbusDockInter->frontendWindowRect();
    const auto ratio = qApp->devicePixelRatio();

    for (auto *screen : qApp->screens()) {
        const QRect &sg = screen->geometry();
        const QRect &rg = QRect(sg.topLeft(), sg.size() * ratio);
        if (rg.contains(dockRect.topLeft())) {
            primaryScreen = screen;
            break;
        }
    }

    return primaryScreen;
}
