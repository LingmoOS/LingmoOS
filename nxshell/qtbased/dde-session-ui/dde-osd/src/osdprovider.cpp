// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "osdprovider.h"

#include <DGuiApplicationHelper>
#include <QApplication>

DGUI_USE_NAMESPACE

OSDProvider::OSDProvider(QObject *parent)
    : AbstractOSDProvider(parent)
{
    initParamsAndSrc();
}

int OSDProvider::rowCount(const QModelIndex &) const
{
    return 1;
}

QVariant OSDProvider::data(const QModelIndex &, int role) const
{
    if (m_suitableParamsMap.contains(m_param)) {
        QStringList value = m_suitableParamsMap.value(m_param);
        return role == Qt::DecorationRole ? value.at(0) : value.at(1);
    }

    qDebug() << "Unable to recognize osd type!";
    return "Unable to recognize osd type";
}

void OSDProvider::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString iconName = index.data(Qt::DecorationRole).toString();
    QString textData = index.data(Qt::DisplayRole).toString();

    QColor color;
    bool isLightTheme = false;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        color = QColor(Qt::black);
        isLightTheme = true;
    }
    else
        color = QColor(Qt::white);

    DrawHelper::DrawImage(painter, option, iconName, isLightTheme);

    if (!textData.isEmpty()) {
        drawText(painter, option, textData, color);
    }
}

QSize OSDProvider::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    return QSize(option.rect.width(), ImageTextItemHeight);
}

QSize OSDProvider::contentSize() const
{
    const int leftMargin = 68;
    const int rightMargin = 26;

    //根据文本长度自适应窗口宽度
    QString textData = data(QModelIndex(), Qt::DisplayRole).toString();

    // TODO 后续需要更换具体字体
    QFont font;
    font.setPixelSize(22);

    QFontMetrics fontMetrics(font);
    return QSize(fontMetrics.horizontalAdvance(textData) + leftMargin + rightMargin, ImageTextItemHeight);
}

bool OSDProvider::match(const QString &param)
{
    m_param = param;
    return m_suitableParamsMap.contains(param);
}

void OSDProvider::drawText(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, const QColor &color) const
{
    const int leftMargin = 68;

    QRect txtRect(option.rect.left() + leftMargin, 0,
                  option.rect.width() - leftMargin, option.rect.height());

    QTextOption opt;
    opt.setAlignment(Qt::AlignVCenter);

    QFont font;
    font.setPixelSize(22);

    painter->setFont(font);
    painter->setPen(color);
    painter->drawText(txtRect, text, opt);
}

void OSDProvider::initParamsAndSrc()
{
    m_suitableParamsMap["WLANOn"] = QStringList() << "osd_wifi_on" << tr("WLAN on");
    m_suitableParamsMap["WLANOff"] = QStringList() << "osd_wifi_off" << tr("WLAN off");

    m_suitableParamsMap["CapsLockOn"] = QStringList() << "osd_capslock_on" << tr("Caps Lock on"); // 大写
    m_suitableParamsMap["CapsLockOff"] = QStringList() << "osd_capslock_off" << tr("Caps Lock off"); // 小写

    m_suitableParamsMap["NumLockOn"] = QStringList() << "osd_keyboard_on" << tr("Numeric keypad on"); // 数字键盘开启
    m_suitableParamsMap["NumLockOff"] = QStringList() << "osd_keyboard_off" << tr("Numeric keypad off"); // 数字键盘关闭

    m_suitableParamsMap["TouchpadOn"] = QStringList() << "osd_touchpad_on" << tr("Touchpad on"); // 触摸板开启
    m_suitableParamsMap["TouchpadOff"] = QStringList() << "osd_touchpad_off" << tr("Touchpad off"); // 触摸板关闭
    m_suitableParamsMap["TouchpadToggle"] = QStringList() << "osd_touchpad_exchange" << tr("Touchpad toggle"); // 触控板切换

    m_suitableParamsMap["FnToggle"] = QStringList() << "osd_fn" << tr("Fn toggle"); // Fn切换

    m_suitableParamsMap["AirplaneModeOn"] = QStringList() << "osd_airplane_mode_on" << tr("Airplane mode on");
    m_suitableParamsMap["AirplaneModeOff"] = QStringList() << "osd_airplane_mode_off" << tr("Airplane mode off");

    m_suitableParamsMap["AudioMicMuteOn"] = QStringList() << "osd_mic_on" << tr("Mute on");
    m_suitableParamsMap["AudioMicMuteOff"] = QStringList() << "osd_mic_off" << tr("Mute off");

    m_suitableParamsMap["balance"] = QStringList() << "osd_power_balance" << tr("Balanced power");
    m_suitableParamsMap["powersave"] = QStringList() << "osd_power_save" << tr("Power saver");
    m_suitableParamsMap["performance"] = QStringList() << "osd_power_performance" << tr("High performance");

    m_suitableParamsMap["SwitchWM3D"] = QStringList() << "osd_wm_3d" << tr("Window effect enabled");
    m_suitableParamsMap["SwitchWM2D"] = QStringList() << "osd_wm_2d" << tr("Window effect disabled");
    m_suitableParamsMap["SwitchWMError"] = QStringList() << "osd_wm_failed" << tr("Failed to enable window effects");
}
