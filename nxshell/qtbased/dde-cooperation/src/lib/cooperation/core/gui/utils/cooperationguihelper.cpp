// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationguihelper.h"
#include "global_defines.h"

#ifdef linux
#    include <DGuiApplicationHelper>
#    include <DFontSizeManager>
DGUI_USE_NAMESPACE
#endif

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
DWIDGET_USE_NAMESPACE
#endif

#include <QVariant>

using namespace cooperation_core;

CooperationGuiHelper::CooperationGuiHelper(QObject *parent)
    : QObject(parent)
{
    initConnection();
}

void CooperationGuiHelper::initConnection()
{
#ifdef linux
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CooperationGuiHelper::themeTypeChanged);
#endif
}

CooperationGuiHelper *CooperationGuiHelper::instance()
{
    static CooperationGuiHelper ins;
    return &ins;
}

bool CooperationGuiHelper::autoUpdateTextColor(QWidget *widget, const QList<QColor> &colorList)
{
    if (colorList.size() != 2)
        return false;

    if (isDarkTheme())
        setFontColor(widget, colorList.last());
    else
        setFontColor(widget, colorList.first());

    if (!widget->property("isConnected").toBool()) {
        widget->setProperty("isConnected", true);
        connect(this, &CooperationGuiHelper::themeTypeChanged, widget, [this, widget, colorList] {
            autoUpdateTextColor(widget, colorList);
        });
    }

    return true;
}

bool CooperationGuiHelper::isDarkTheme()
{
#ifdef linux
    return DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
#else
    return false;
#endif
}

void CooperationGuiHelper::setFontColor(QWidget *widget, QColor color)
{
    QPalette palette = widget->palette();
    palette.setColor(QPalette::WindowText, color);
    widget->setPalette(palette);
}

void CooperationGuiHelper::setLabelFont(QLabel *label, int pointSize, int minpointSize, int weight)
{
    QFont font;
    int size = pointSize;
#ifdef DTKWIDGET_CLASS_DSizeMode
    size = DSizeModeHelper::element(minpointSize, pointSize);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, label, [pointSize, minpointSize, label] {
        int size = DSizeModeHelper::element(minpointSize, pointSize);
        QFont font;
        font.setPixelSize(size);
        label->setFont(font);
    });
#endif

    font.setPixelSize(size);
    font.setWeight((QFont::Weight)weight);

    label->setFont(font);
}

void CooperationGuiHelper::setAutoFont(QWidget *widget, int size, int weight)
{
#ifdef linux
    switch (size) {
    case 16:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T5, weight);
        break;
    case 14:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T6, weight);
        break;
    case 12:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T8, weight);
        break;
    case 11:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T9, weight);
        break;
    default:
        DFontSizeManager::instance()->bind(widget, DFontSizeManager::T6, weight);
    }
#else
    QFont font;
    font.setPixelSize(size);
    font.setWeight(weight);
    widget->setFont(font);
#endif
}

void CooperationGuiHelper::initThemeTypeConnect(QWidget *w, const QString &lightstyle, const QString &darkstyle)
{
    if (CooperationGuiHelper::instance()->isDarkTheme())
        w->setStyleSheet(darkstyle);
    else
        w->setStyleSheet(lightstyle);
    connect(CooperationGuiHelper::instance(), &CooperationGuiHelper::themeTypeChanged, w, [w, lightstyle, darkstyle] {
        if (CooperationGuiHelper::instance()->isDarkTheme())
            w->setStyleSheet(darkstyle);
        else
            w->setStyleSheet(lightstyle);
    });
}
