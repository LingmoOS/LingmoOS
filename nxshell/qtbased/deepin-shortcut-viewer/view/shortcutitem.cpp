// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutitem.h"

#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QPainter>

DWIDGET_USE_NAMESPACE

ShortcutItem::ShortcutItem(bool isGroup, QWidget *parent)
    : QWidget(parent),
      m_isGroup(isGroup)
{
    QColor textColor = QColor(65, 77, 104);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(192, 198, 212);
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, textColor);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setPalette(labelPalette);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_valueLabel->setWordWrap(true);
    m_valueLabel->setPalette(labelPalette);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_nameLabel);
    mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    mainLayout->addWidget(m_valueLabel);

    if (isGroup) {
        QFont font;
        font.setPixelSize(17);
        font.setWeight(70);

        m_nameLabel->setFont(font);
        m_valueLabel->setFont(font);

        mainLayout->setContentsMargins(10, 5, 10, 6);
    } else {
        if (qApp->devicePixelRatio() > 1.2) {
            int fontSize { DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6) };
            QFont font { m_nameLabel->font() };
            static const QMap<int, int> fontMap {
                { 15, 14 },
                { 16, 16 },
                { 17, 16 },
                { 18, 16 },
                { 20, 18 }
            };
            int size { fontMap.value(fontSize, 14) };
            font.setPixelSize(size);
            m_nameLabel->setFont(font);
            m_valueLabel->setFont(font);
        }
        mainLayout->setContentsMargins(10, 0, 10, 0);
    }

    // 系统字号太大时，Text会超出label显示范围
    int fontSize { m_nameLabel->fontInfo().pixelSize() };
    static const QMap<int, int> sizeMap {
        { 16, 310 },
        { 17, 320 },
        { 18, 330 },
        { 19, 340 },
        { 20, 350 }
    };
    int width { sizeMap.value(fontSize, 300) };
    if (fontSize > 20)
        width = 360;

    setLayout(mainLayout);
    setFixedWidth(width);
}

void ShortcutItem::setText(const QString &name, const QString &value)
{
    m_nameLabel->setText(name);
    m_nameLabel->adjustSize();

    m_valueLabel->setText(value);
    m_valueLabel->adjustSize();
}

QString ShortcutItem::name() const
{
    return m_nameLabel->text();
}

QString ShortcutItem::value() const
{
    return m_valueLabel->text();
}

void ShortcutItem::setEnableBackground(bool enable)
{
    if (enable == m_hasBackground)
        return;

    m_hasBackground = enable;
    update();
}

bool ShortcutItem::enableBackground() const
{
    return m_hasBackground;
}

void ShortcutItem::paintEvent(QPaintEvent *event)
{
    if (m_hasBackground) {
        const DPalette &dp = DPaletteHelper::instance()->palette(this);
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(dp.brush(DPalette::ItemBackground));
        p.drawRoundedRect(rect(), 8, 8);
    }

    return QWidget::paintEvent(event);
}
