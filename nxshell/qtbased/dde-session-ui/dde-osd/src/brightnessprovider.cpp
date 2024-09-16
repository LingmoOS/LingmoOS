// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "brightnessprovider.h"

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

BrightnessProvider::BrightnessProvider(QObject *parent)
    : AbstractOSDProvider(parent),
      m_displayInter(new org::deepin::dde::Display1("org.deepin.dde.Display1",
                                                      "/org/deepin/dde/Display1",
                                                      QDBusConnection::sessionBus(), this))
{
    m_suitableParams << "BrightnessUp" << "BrightnessDown" << "BrightnessUpAsh" << "BrightnessDownAsh";

    m_displayInter->setSync(true, false);
}

int BrightnessProvider::rowCount(const QModelIndex &) const
{
    return 1;
}

QVariant BrightnessProvider::data(const QModelIndex &, int role) const
{
    if (role == Qt::DecorationRole) {
        return pixmapName();
    }

    return m_displayInter->brightness().value(m_displayInter->primary(), 0);
}

void BrightnessProvider::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString pixmapName = index.data(Qt::DecorationRole).toString();
    QVariant progressData = index.data(Qt::DisplayRole);

    QColor color;
    bool isLightTheme = false;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        isLightTheme = true;
        color = QColor(Qt::black);
    } else {
        color = QColor(Qt::white);
    }

    DrawHelper::DrawImage(painter, option, pixmapName, isLightTheme);

    if (!m_isVisible) {
        color.setAlpha(0.4 * 255);
    }
    DrawHelper::DrawProgressBar(painter, option, progressData.toDouble(), color);

    int value = progressData.toDouble() * 100;
    DrawHelper::DrawPercentValue(painter, option, value, color);
}

QSize BrightnessProvider::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(ImageTextItemWidth, ImageTextItemHeight);
}

QString BrightnessProvider::pixmapName() const
{
    double brightness = m_displayInter->brightness().value(m_displayInter->primary(), 0);

    QString level = "0";
    if (brightness > 0 && brightness <= 0.33)
        level = "33";
    else if (brightness > 0.33 && brightness <= 0.66)
        level = "66";
    else if (brightness > 0.66 && brightness <= 1)
        level = "100";

    return QString("osd_brightness%1").arg(level);
}
