// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layoutwidget.h"

#include <linux/input.h>
#include <xkbcommon/xkbcommon.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QVector>

DWIDGET_USE_NAMESPACE

enum class LayoutStatus {
    NoLayout,
    MultiLayout,
    SingleLayout,
};

struct Item
{
    int key;
    float widthFactor = 1;
};

static QVector<QVector<Item>> keyboardLayout{
    {
        { KEY_ESC, 1.5 },
        { KEY_F1, 1 },
        { KEY_F2, 1 },
        { KEY_F3, 1 },
        { KEY_F4, 1 },
        { KEY_F5, 1 },
        { KEY_F6, 1 },
        { KEY_F7, 1 },
        { KEY_F8, 1 },
        { KEY_F9, 1 },
        { KEY_F10, 1 },
        { KEY_F11, 1 },
        { KEY_F12, 1 },
        { KEY_DELETE, 1.5 },
    },
    {
        { KEY_GRAVE, 1 },
        { KEY_1, 1 },
        { KEY_2, 1 },
        { KEY_3, 1 },
        { KEY_4, 1 },
        { KEY_5, 1 },
        { KEY_6, 1 },
        { KEY_7, 1 },
        { KEY_8, 1 },
        { KEY_9, 1 },
        { KEY_0, 1 },
        { KEY_MINUS, 1 },
        { KEY_EQUAL, 1 },
        { KEY_BACKSPACE, 2 },
    },
    {
        { KEY_TAB, 1.5 },
        { KEY_Q, 1 },
        { KEY_W, 1 },
        { KEY_E, 1 },
        { KEY_R, 1 },
        { KEY_T, 1 },
        { KEY_Y, 1 },
        { KEY_U, 1 },
        { KEY_I, 1 },
        { KEY_O, 1 },
        { KEY_P, 1 },
        { KEY_LEFTBRACE, 1 },
        { KEY_RIGHTBRACE, 1 },
        { KEY_BACKSLASH, 1.5 },
    },
    {
        { KEY_CAPSLOCK, 1.75 },
        { KEY_A, 1 },
        { KEY_S, 1 },
        { KEY_D, 1 },
        { KEY_F, 1 },
        { KEY_G, 1 },
        { KEY_H, 1 },
        { KEY_J, 1 },
        { KEY_K, 1 },
        { KEY_L, 1 },
        { KEY_SEMICOLON, 1 },
        { KEY_APOSTROPHE, 1 },
        { KEY_ENTER, 2.25 },
    },
    {
        { KEY_LEFTSHIFT, 2 },
        { KEY_Z, 1 },
        { KEY_X, 1 },
        { KEY_C, 1 },
        { KEY_V, 1 },
        { KEY_B, 1 },
        { KEY_N, 1 },
        { KEY_M, 1 },
        { KEY_COMMA, 1 },
        { KEY_DOT, 1 },
        { KEY_SLASH, 1 },
        { KEY_RIGHTSHIFT, 3 },
    },
    {
        { KEY_LEFTCTRL, 1 },
        { KEY_FN, 1 },
        { KEY_LEFTMETA, 1 },
        { KEY_LEFTALT, 1 },
        { KEY_SPACE, 6 },
        { KEY_RIGHTALT, 1 },
        { KEY_RIGHTMETA, 1 },
        { KEY_MENU, 1 },
        { KEY_RIGHTCTRL, 1 },
        { KEY_RESERVED, 1 },
    },
};

LayoutWidget::LayoutWidget(QWidget *parent)
    : DFrame(parent)
    , m_ctx(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
    , m_status(LayoutStatus::NoLayout)
    , m_label(new QLabel(this))
{
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hBoxLayout);

    m_label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    m_label->setContentsMargins(0, 0, 0, 0);
    m_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_label->hide();

    hBoxLayout->addWidget(m_label);
}

LayoutWidget::~LayoutWidget()
{
    xkb_context_unref(m_ctx);
}

void LayoutWidget::setKeyboardLayout(const QString &layout, const QString &variant)
{
    m_status = LayoutStatus::SingleLayout;
    m_layout = layout.toStdString();
    m_variant = variant.toStdString();
    update();
}

void LayoutWidget::showMulti()
{
    m_status = LayoutStatus::MultiLayout;
    m_layout = "";
    m_variant = "";
    update();
}

void LayoutWidget::showNoLayout()
{
    m_status = LayoutStatus::NoLayout;
    m_layout = "";
    m_variant = "";
    update();
}

void LayoutWidget::paintLayout(QPainter &painter)
{
    xkb_rule_names rnames = {};
    rnames.layout = m_layout.c_str();
    rnames.variant = m_variant.c_str();

    auto *keymap = xkb_keymap_new_from_names(m_ctx, &rnames, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) {
        showNoLayoutLabel();
        return;
    }

    auto *state = xkb_state_new(keymap);
    if (!state) {
        showNoLayoutLabel();
        return;
    }

    m_label->hide();

    const int unitWidth = 22;
    const int unitHheight = 20;
    int padding = 2;
    int radius = 2;
    int painterWidth = unitWidth * 15;
    int painterHeight = unitHheight * 6;

    QPixmap pixmap(unitWidth, unitHheight);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(qRgba(0, 47, 143, 70));

    QFont font;
    font.setPointSize(6);
    painter.setFont(font);

    float widgetRatio = float(rect().width()) / rect().height();
    float painterRatio = float(painterWidth) / painterHeight;
    if (widgetRatio > painterRatio) {
        int scaleFactor = rect().height() / painterHeight;
        painter.scale(scaleFactor, scaleFactor);
        painter.translate(QPoint((rect().width() - painterWidth * scaleFactor) / 2,
                                 (rect().height() - painterHeight * scaleFactor) / 2));
    } else {
        int scaleFactor = rect().width() / painterWidth;
        painter.scale(scaleFactor, scaleFactor);
        painter.translate(QPoint((rect().width() - painterWidth * scaleFactor) / 2,
                                 (rect().height() - painterHeight * scaleFactor) / 2));
    }

    float x = 0, y = 0;
    for (const auto &row : keyboardLayout) {
        for (const auto &item : row) {
            QColor background = "#eaeef4";

            char buff[10] = { 0 };
            int res = xkb_state_key_get_utf8(state, item.key + 8, buff, 9);
            QString text = QString::fromUtf8(buff);
            if (res > 0) {
                if (text.size() == 1 && !text[0].isPrint()) {
                    // 不可见字符
                    text.clear();
                } else {
                    // 可见字符
                    background = "#c2d3ee";
                }
            }

            QPainterPath path;
            path.addRoundedRect(QRect(x + padding,
                                      y + padding,
                                      (item.widthFactor * unitWidth) - padding * 2,
                                      unitHheight - padding * 2),
                                radius,
                                radius);
            painter.fillPath(path, background);
            if (res > 0) {
                painter.drawText(QRect(x, y, item.widthFactor * unitWidth, unitHheight),
                                 Qt::AlignCenter,
                                 text);
            }
            x += item.widthFactor * unitWidth;
        }

        x = 0;
        y += unitHheight;
    }

    painter.end();
}

void LayoutWidget::showNoLayoutLabel()
{
    m_label->setText(QString("- %1 -").arg(tr("The current input method has no keyboard layout")));
    m_label->show();
}

void LayoutWidget::showMultiLayoutLabel()
{
    m_label->setText(QString("- %1 -").arg(tr("Multiple input methods have been selected")));
    m_label->show();
}

void LayoutWidget::paintEvent(QPaintEvent *event)
{
    DFrame::paintEvent(event);
    if (!m_ctx) {
        return;
    }

    switch (m_status) {
    case LayoutStatus::MultiLayout:
        showMultiLayoutLabel();
        break;
    case LayoutStatus::NoLayout:
        showNoLayoutLabel();
        break;
    case LayoutStatus::SingleLayout: {
        QPainter painter;
        painter.begin(this);

        paintLayout(painter);

        painter.end();

        break;
    }
    }
}
