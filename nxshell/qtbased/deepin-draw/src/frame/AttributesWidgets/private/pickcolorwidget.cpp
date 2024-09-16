// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pickcolorwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QDBusInterface>
#include <QDebug>

#include "utils/global.h"
#include "service/colorpickerinterface.h"
#include "ciconbutton.h"
#include "cviewmanagement.h"
#include "cgraphicsview.h"

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

const QSize PICKCOLOR_WIDGET_SIZE = QSize(294, 215);

PickColorWidget::PickColorWidget(DWidget *parent)
    : DWidget(parent)
{
    setFixedSize(PICKCOLOR_WIDGET_SIZE);
    DLabel *titleLabel = new DLabel(this);
    QFont titleLabelFont = titleLabel->font();
    titleLabelFont.setPixelSize(13);
    titleLabel->setText("RGB");
    titleLabel->setFixedWidth(27);
    titleLabel->setFont(titleLabelFont);

    m_cp = new ColorPickerInterface("com.deepin.Picker",
                                    "/com/deepin/Picker", QDBusConnection::sessionBus(), this);

    connect(m_cp, &ColorPickerInterface::colorPicked, this, [ = ](QString uuid, QString colorName) {
        if (uuid == QString("%1").arg(qApp->applicationPid())) {
            this->setColor(QColor(colorName));
            QTimer::singleShot(200, [ = ] {//取色器是另外的应用,当取色器返回时,应重置视图的焦点
                if (nullptr != CURRENTVIEW)
                    CURRENTVIEW->captureFocus();
            });
        }
        m_picker->setChecked(false);
    });

    m_redEditLabel = new EditLabel(this);

    m_greenEditLabel = new EditLabel(this);

    m_blueEditLabel = new EditLabel(this);

    QMap<int, QMap<CIconButton::EIconButtonSattus, QString>> pictureMap;

    //取色器使用系统托管icon方式设置图标
    m_picker = new CIconButton(pictureMap, QSize(55, 36), this, false);
    m_picker->setIconMode();
    m_picker->setIconSize(QSize(36, 36));
    m_picker->setIcon(QIcon::fromTheme("dorpper_normal"));

    QHBoxLayout *rgbLayout = new QHBoxLayout;
    rgbLayout->setMargin(0);
    rgbLayout->setSpacing(0);
    rgbLayout->addWidget(titleLabel);
    rgbLayout->addSpacing(22);
    rgbLayout->addWidget(m_redEditLabel);
    rgbLayout->addSpacing(7);
    rgbLayout->addWidget(m_greenEditLabel);
    rgbLayout->addSpacing(8);
    rgbLayout->addWidget(m_blueEditLabel);
    rgbLayout->addSpacing(10);
    rgbLayout->addWidget(m_picker);
    m_colorSlider = new ColorSlider(this);
    m_colorLabel = new ColorLabel(this);
    m_colorLabel->setFixedSize(PICKCOLOR_WIDGET_SIZE.width(), 136);

    connect(m_colorSlider, &ColorSlider::valueChanged, m_colorLabel, [ = ](int val) {
        m_colorLabel->setHue(val);
    });
    connect(m_colorLabel, &ColorLabel::pickedColor, this, [ = ](QColor color) {
        this->setColor(color);
    });

    connect(m_colorLabel, &ColorLabel::signalPreViewColor, this, [ = ](QColor color) {
        this->updateColor(color);
    });

    connect(m_picker, &CIconButton::mouseRelease, this, [ = ] {
        m_cp->StartPick(QString("%1").arg(qApp->applicationPid()));
    });

    QVBoxLayout *mLayout = new QVBoxLayout;
    mLayout->setMargin(0);
    mLayout->setSpacing(0);
    mLayout->addLayout(rgbLayout);
    mLayout->addSpacing(10);
    mLayout->addWidget(m_colorLabel, 0, Qt::AlignCenter);
    mLayout->addSpacing(11);
    mLayout->addWidget(m_colorSlider, 0, Qt::AlignCenter);
    setLayout(mLayout);
}

void PickColorWidget::updateColor(const QColor &color)
{
    QColor c = color.isValid() ? color : curColor;
    m_redEditLabel->setText(QString("%1").arg(c.red()));
    m_greenEditLabel->setText(QString("%1").arg(c.green()));
    m_blueEditLabel->setText(QString("%1").arg(c.blue()));

    if (color.isValid()) {
        //证明是预览发出信号通知外界
        emit previewedColorChanged(color);
    }
}

void PickColorWidget::setTheme(int theme)
{
    m_picker->setTheme(theme);
}

PickColorWidget::~PickColorWidget()
{
}

QColor PickColorWidget::color()
{
    return curColor;
}

void PickColorWidget::setColor(const QColor &c, bool internalChanged)
{
    bool changed = (c != curColor);

    if (changed) {
        curColor = c;

        updateColor();

        if (internalChanged) {
            emit colorChanged(curColor);
        }
    }
}
