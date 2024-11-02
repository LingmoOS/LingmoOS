#include <QVariant>
#include <QDebug>

/* SDK gsetting */
#include <gsettingmonitor.h>
#include "buriedpoint.hpp"

#include "globalsizedata.h"

#define THEME_GSETTINGS "org.lingmo.style"
#define APPLY_FONT_KEY "system-font"

// 主题色
int GlobalSizeData::THEME_COLOR = GlobalSizeData::LINGMOLight;
FontType GlobalSizeData::g_fontType = FontType::AllFont;

GlobalSizeData::GlobalSizeData()
{
    getSystemFontSize();
    getSystemTheme();
    getSystemTransparency();
    changeFontSize();
    getSystemFontName();
    changeFontName();
    getSystemDBusInfo();
}

GlobalSizeData *GlobalSizeData::getInstance()
{
    static GlobalSizeData globalData;
    return &globalData;
}

GlobalSizeData::~GlobalSizeData()
{
}

void GlobalSizeData::changeFontSize()
{
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        getSystemFontSize();
        emit this->sigFontChange();
    });
    return;
}

void GlobalSizeData::getSystemFontName()
{
    if (QGSettings::isSchemaInstalled(THEME_GSETTINGS)) {
        /* 主题 */
        m_themeGsettings = new QGSettings(THEME_GSETTINGS, QByteArray(), this);
        m_fontName = m_themeGsettings->get(APPLY_FONT_KEY).toString();
    }
    return;
}

void GlobalSizeData::changeFontName()
{
    connect(m_themeGsettings, &QGSettings::changed, this, [=]() {
        if (m_themeGsettings->get("style-name").toString() != m_fontName) {
            m_fontName = m_themeGsettings->get(APPLY_FONT_KEY).toString();
            emit this->sigFontNameChange();
        }
    });
    return;
}

void GlobalSizeData::getSystemTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    qInfo() << "system item : " << systemTheme;
    if (systemTheme == BLACK_THEME) {
        GlobalSizeData::THEME_COLOR = GlobalSizeData::LINGMODark;
    } else {
        GlobalSizeData::THEME_COLOR = GlobalSizeData::LINGMOLight;
    }
    return;
}

void GlobalSizeData::getSystemFontSize()
{
    /* 获取系统初始字体 */
    g_fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    qInfo() << "system systemFontSize : " << g_fontSize;

    g_font14pxToPt = (g_fontSize * (14.00 / 11)) - 3;
    g_font16pxToPt = (g_fontSize * (16.00 / 11)) - 3;
    g_font18pxToPt = (g_fontSize * (18.00 / 11)) - 3;

    qDebug() << "Info : system init font : 14 ---> " << g_font14pxToPt << "16 ---> " << g_font16pxToPt << "18 ---> " << g_font16pxToPt;

    return;
}

void GlobalSizeData::getSystemTransparency()
{
    double systemTransparency = kdk::GsettingMonitor::getSystemTransparency().toDouble();

    qInfo() << "system systemTransparency : " << systemTransparency;
    GlobalSizeData::g_transparency = systemTransparency * 255;

    return;
}

void GlobalSizeData::addFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewInstallFont)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::removeFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoPrinterManuallyInstallPrinter)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::searchFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewSearchFont)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::applyFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewApplyFont)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::exportFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewExportFont)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::collectionFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewCollectionFont)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::cancelCollectionFontSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewCancelCollectionFont)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::fontInfoSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewFontInformation)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::changecopywritingSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewCopywritingChange)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::changeFontSizeSDKPoint()
{
    ::kabase::BuriedPoint buriedPointTest;

    if (!buriedPointTest.functionBuriedPoint(::kabase::AppName::LingmoFontViewer , ::kabase::BuriedPoint::PT::LingmoFontViewFontSizeChange)) {
        qCritical() << "Error : buried point fail !" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    };

    return;
}

void GlobalSizeData::getSystemDBusInfo()
{
    m_rotateChange = new RotateChangeInfo(this);
    connect(m_rotateChange, &RotateChangeInfo::sigRotationChanged, this, &GlobalSizeData::slotChangeModel);
    getCurrentMode();
    return;
}

void GlobalSizeData::slotChangeModel(bool isPCMode, int width, int height)
{
    m_currentSize.first = width;
    m_currentSize.second = height;
    if (isPCMode) {
        m_currentMode = CurrentMode::PCMode;
        emit sigPCMode();
    } else {
        if (width > height) {
            m_currentMode = CurrentMode::HMode;
            emit sigHFlatMode();
        } else {
            m_currentMode = CurrentMode::VMode;
            emit sigVFlatMode();
        }
    }
    return;
}

CurrentMode GlobalSizeData::getCurrentMode()
{
    bool isPCMode = m_rotateChange->getCurrentMode();
    QPair<int, int> currentSize = m_rotateChange->getCurrentScale();
    m_currentSize = currentSize;
    if (isPCMode) {
        m_currentMode = CurrentMode::PCMode;
    } else {
        if (currentSize.first > currentSize.second) {
            m_currentMode = CurrentMode::HMode;
        } else {
            m_currentMode = CurrentMode::VMode;
        }
    }
    return m_currentMode;
}
