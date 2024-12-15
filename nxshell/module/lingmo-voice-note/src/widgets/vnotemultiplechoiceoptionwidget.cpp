// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnotemultiplechoiceoptionwidget.h"
#include "globaldef.h"
#include "common/utils.h"

#include <DApplication>

#include <QDebug>
#include <QImageReader>
#include <QKeyEvent>

//多选操作页面
const int m_iconWidth = 24;
VnoteMultipleChoiceOptionWidget::VnoteMultipleChoiceOptionWidget(QWidget *parent)
    : DWidget(parent)
{
    initUi();
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::initUi
 * @param
 */
//初始化ui
void VnoteMultipleChoiceOptionWidget::initUi()
{
    QVBoxLayout *vlayout = new QVBoxLayout;

    DLabel *iconLabel = new DLabel(this);
    QImageReader reader;
    QPixmap pixmap;
    QSize size(162, 156);
    reader.setFileName(QString(STAND_ICON_PAHT).append("detail_icon/detail_icon_note.svg"));
    const qreal ratio = qGuiApp->devicePixelRatio();
    if (reader.canRead()) {
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(QString(STAND_ICON_PAHT).append("detail_icon/detail_icon_note.svg"));
    }
    iconLabel->setPixmap(pixmap);

    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->addStretch();
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch();

    m_tipsLabel = new DLabel(this);
    m_tipsLabel->setFixedHeight(29);

    DFontSizeManager::instance()->bind(m_tipsLabel, DFontSizeManager::T4);
    QHBoxLayout *tipsLayout = new QHBoxLayout;
    tipsLayout->addStretch();
    tipsLayout->addWidget(m_tipsLabel);
    tipsLayout->addStretch();

    m_moveButton = new DToolButton(this);
    m_saveTextButton = new DToolButton(this);
    m_saveVoiceButton = new DToolButton(this);
    m_deleteButton = new DToolButton(this);

    m_moveButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_saveTextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_saveVoiceButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_deleteButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_moveButton->installEventFilter(this);
    m_saveTextButton->installEventFilter(this);
    m_saveVoiceButton->installEventFilter(this);
    m_deleteButton->installEventFilter(this);
    onFontChanged();
    //设置主题
    changeFromThemeType();
    //初始化链接
    initConnections();

    vlayout->addStretch(4);
    vlayout->addLayout(iconLayout);
    vlayout->addSpacing(10);
    vlayout->addLayout(tipsLayout);
    vlayout->addSpacing(20);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(m_moveButton);
    hlayout->addWidget(m_saveTextButton);
    hlayout->addWidget(m_saveVoiceButton);
    hlayout->addWidget(m_deleteButton);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);
    vlayout->addStretch(7);
    hlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(vlayout);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::initConnections
 * @param number
 */
//初始化链接
void VnoteMultipleChoiceOptionWidget::initConnections()
{
    //移动
    connect(m_moveButton, &DToolButton::pressed, this, [=] {
        buttonPressed(ButtonValue::Move);
    });
    connect(m_moveButton, &DToolButton::clicked, this, [=] {
        trigger(ButtonValue::Move);
    });
    //保存文本
    connect(m_saveTextButton, &DToolButton::pressed, this, [=] {
        buttonPressed(ButtonValue::SaveAsTxT);
    });
    connect(m_saveTextButton, &DToolButton::clicked, this, [=] {
        trigger(ButtonValue::SaveAsTxT);
    });
    //保存语音
    connect(m_saveVoiceButton, &DToolButton::pressed, this, [=] {
        buttonPressed(ButtonValue::SaveAsVoice);
    });
    connect(m_saveVoiceButton, &DToolButton::clicked, this, [=] {
        trigger(ButtonValue::SaveAsVoice);
    });
    //删除
    connect(m_deleteButton, &DToolButton::pressed, this, [=] {
        buttonPressed(ButtonValue::Delete);
    });
    connect(m_deleteButton, &DToolButton::clicked, this, [=] {
        trigger(ButtonValue::Delete);
    });
    //恢复图标
    connect(m_saveVoiceButton, &DToolButton::released, this, [=] {
        changeFromThemeType();
    });
    connect(m_saveTextButton, &DToolButton::released, this, [=] {
        changeFromThemeType();
    });
    connect(m_moveButton, &DToolButton::released, this, [=] {
        changeFromThemeType();
    });
    connect(m_deleteButton, &DToolButton::released, this, [=] {
        changeFromThemeType();
    });
    //主题切换更换按钮和文本颜色
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &VnoteMultipleChoiceOptionWidget::changeFromThemeType);

    //字体切换长度适应
    connect(qGuiApp, &QGuiApplication::fontChanged, this, &VnoteMultipleChoiceOptionWidget::onFontChanged);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::buttonPressed
 * @param value
 * press更新svg
 */
void VnoteMultipleChoiceOptionWidget::buttonPressed(ButtonValue value)
{
    DPalette dpalette = DApplicationHelper::instance()->palette(m_deleteButton);
    QColor textColor = dpalette.color(DPalette::Highlight);
    QString color = textColor.name(QColor::HexRgb);
    QString iconPath = QString(STAND_ICON_PAHT).append("light/");
    QPixmap pixmap;
    switch (value) {
    case ButtonValue::Move:
        pixmap = setSvgColor(iconPath.append("detail_notes_move.svg"), color);
        m_moveButton->setIcon(pixmap);
        break;
    case ButtonValue::SaveAsTxT:
        pixmap = setSvgColor(iconPath.append("detail_notes_saveText.svg"), color);
        m_saveTextButton->setIcon(pixmap);
        break;
    case ButtonValue::SaveAsVoice:
        pixmap = setSvgColor(iconPath.append("detail_notes_saveVoice.svg"), color);
        m_saveVoiceButton->setIcon(pixmap);
        break;
    case ButtonValue::Delete:
        pixmap = setSvgColor(iconPath.append("detail_notes_delete.svg"), color);
        m_deleteButton->setIcon(pixmap);
        break;
    }
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::setNoteNumber
 * @param number
 */
//设置笔记数量
void VnoteMultipleChoiceOptionWidget::setNoteNumber(int number)
{
    if (number != m_noteNumber) {
        m_noteNumber = number;
        QString str = QString(DApplication::translate("DetailPage", "%1 notes selected").arg(number));
        m_tipsLabel->setText(str);
    }
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::enableButtons
 * @param saveAsTxtButtonStatus,saveAsVoiceButtonStatus
 */
//设置按钮是否置灰
void VnoteMultipleChoiceOptionWidget::enableButtons(bool saveAsTxtButtonStatus, bool saveAsVoiceButtonStatus, bool moveButtonStatus)
{
    m_saveTextButton->setEnabled(saveAsTxtButtonStatus);
    m_saveVoiceButton->setEnabled(saveAsVoiceButtonStatus);
    m_moveButton->setEnabled(moveButtonStatus);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::setSvgColor
 * @param svgPath
 * @param color
 */
//获得svg
QPixmap VnoteMultipleChoiceOptionWidget::setSvgColor(const QString &svgPath, const QString &color)
{
    //设置图标颜色
    QString path = svgPath;
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    QDomDocument doc;
    doc.setContent(data);
    file.close();
    QDomElement elem = doc.documentElement();
    setSVGBackColor(elem, "fill", color);

    //    int scaled =qApp->devicePixelRatio() == 1.25 ? 2 : 1;
    double scaled = qGuiApp->devicePixelRatio();
    QSvgRenderer svg_render(doc.toByteArray());

    QPixmap pixmap(QSize(24, 24) * scaled);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(scaled);

    QPainter painter(&pixmap);
    svg_render.render(&painter, QRect(0, 0, 24, 24));
    return pixmap;
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::setSVGBackColor
 * @param ielem
 * @param attr
 * @param val
 */
//设置svg颜色属性
void VnoteMultipleChoiceOptionWidget::setSVGBackColor(QDomElement &elem, QString attr, QString val)
{
    if (elem.tagName().compare("g") == 0 && elem.attribute("id").compare("color") == 0) {
        QString before_color = elem.attribute(attr);
        elem.setAttribute(attr, val);
    }
    for (int i = 0; i < elem.childNodes().count(); i++) {
        if (!elem.childNodes().at(i).isElement())
            continue;
        QDomElement element = elem.childNodes().at(i).toElement();
        setSVGBackColor(element, attr, val);
    }
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::trigger
 * @param id
 * 触发多选操作
 */
void VnoteMultipleChoiceOptionWidget::trigger(int id)
{
    //dtk控件偶现不能清空hover状态问题，手动清空hover状态
    m_moveButton->setAttribute(Qt::WA_UnderMouse, false);
    m_saveTextButton->setAttribute(Qt::WA_UnderMouse, false);
    m_saveVoiceButton->setAttribute(Qt::WA_UnderMouse, false);
    m_deleteButton->setAttribute(Qt::WA_UnderMouse, false);
    emit requestMultipleOption(id);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::changeFromThemeType
 * @param
 * 根据主题设置图标与删除按钮文本颜色
 */
void VnoteMultipleChoiceOptionWidget::changeFromThemeType()
{
    bool isDark = (DApplicationHelper::DarkType == DApplicationHelper::instance()->themeType()) ? true : false;
    QString iconPath = QString(STAND_ICON_PAHT);
    if (isDark) {
        iconPath.append("dark/");
        //设置字体颜色（特殊颜色与UI沟通可以不根据DTK色板单独设置）
        DPalette deletePalette = DApplicationHelper::instance()->palette(m_deleteButton);
        deletePalette.setBrush(DPalette::ButtonText, QColor("#9A2F2F"));
        DApplicationHelper::instance()->setPalette(m_deleteButton, deletePalette);
    } else {
        iconPath.append("light/");
        //设置字体颜色（特殊颜色与UI沟通可以不根据DTK色板单独设置）
        DPalette deletePalette = DApplicationHelper::instance()->palette(m_deleteButton);
        deletePalette.setBrush(DPalette::ButtonText, QColor("#FF5736"));
        DApplicationHelper::instance()->setPalette(m_deleteButton, deletePalette);
    }
    //根据主题设置图标
    m_moveButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_move.svg")));
    m_saveTextButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_saveText.svg")));
    m_saveVoiceButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_saveVoice.svg")));
    m_deleteButton->setIcon(QIcon(QString("%1%2").arg(iconPath).arg("detail_notes_delete.svg")));
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::onFontChanged
 * @param
 */
//根据字体大小调整按钮UI
void VnoteMultipleChoiceOptionWidget::onFontChanged()
{
    QFontMetrics fontMetrics(m_deleteButton->font());
    //设置move和delete按钮的高度和文本内容
    m_moveButton->setText(DApplication::translate("NotesContextMenu", "Move"));
    m_moveButton->setFixedHeight(m_iconWidth + 2);
    m_deleteButton->setText(DApplication::translate("NotesContextMenu", "Delete"));
    m_deleteButton->setFixedHeight(m_iconWidth + 2);
    //    //计算参数
    int iconWidth = qCeil(qGuiApp->devicePixelRatio() * m_iconWidth) + 30;
    int saveTextWidth = fontMetrics.width(DApplication::translate("NotesContextMenu", "Save note")) + iconWidth;
    int saveVoiceWidth = fontMetrics.width(DApplication::translate("NotesContextMenu", "Save voice recording")) + iconWidth;
    int deleteButtonWidth = fontMetrics.width(DApplication::translate("NotesContextMenu", "Delete")) + iconWidth;
    int moveButtonWidth = fontMetrics.width(DApplication::translate("NotesContextMenu", "Move")) + iconWidth;

    m_moveButton->setFixedWidth(moveButtonWidth);
    m_deleteButton->setFixedWidth(deleteButtonWidth);
    m_saveTextButton->setFixedSize(saveTextWidth, m_iconWidth + 2);
    m_saveVoiceButton->setFixedSize(saveVoiceWidth, m_iconWidth + 2);

    if (m_saveTextButton->width() + m_saveVoiceButton->width() + m_deleteButton->width() + m_moveButton->width() <= width()) {
        m_saveTextButton->setText(DApplication::translate("NotesContextMenu", "Save note"));
        m_saveVoiceButton->setText(DApplication::translate("NotesContextMenu", "Save voice recording"));
    } else {
        int midWidth = (width() - m_deleteButton->width() - m_moveButton->width()) / 2;
        if (midWidth > 0 && m_saveTextButton->width() > midWidth) {
            m_saveTextButton->setFixedWidth(midWidth);
            m_saveTextButton->setText(fontMetrics.elidedText(DApplication::translate("NotesContextMenu", "Save note"), Qt::ElideRight, midWidth - iconWidth));
        } else {
            m_saveTextButton->setText(DApplication::translate("NotesContextMenu", "Save note"));
        }
        saveVoiceWidth = width() - m_deleteButton->width() - m_moveButton->width() - m_saveTextButton->width() - 30;
        if (saveVoiceWidth > 0 && saveVoiceWidth < m_saveVoiceButton->width()) {
            m_saveVoiceButton->setFixedWidth(saveVoiceWidth);
            m_saveVoiceButton->setText(fontMetrics.elidedText(DApplication::translate("NotesContextMenu", "Save voice recording"), Qt::ElideRight, saveVoiceWidth - iconWidth));
        } else {
            m_saveVoiceButton->setText(DApplication::translate("NotesContextMenu", "Save voice recording"));
        }
    }

    QSize iconSize(m_iconWidth, m_iconWidth);
    m_moveButton->setIconSize(iconSize);
    m_saveTextButton->setIconSize(iconSize);
    m_saveVoiceButton->setIconSize(iconSize);
    m_deleteButton->setIconSize(iconSize);
}

/**
 * @brief VnoteMultipleChoiceOptionWidget::resizeEvent
 * @param
 */
void VnoteMultipleChoiceOptionWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    onFontChanged();
}

bool VnoteMultipleChoiceOptionWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            DToolButton *btn = dynamic_cast<DToolButton *>(o);
            btn->click();
            return true;
        }
        if (o == m_deleteButton && keyEvent->key() == Qt::Key_Tab) {
            Utils::setTitleBarTabFocus(keyEvent); //将焦点转移至标题栏下一个控件
            return true;
        }
    }
    return false;
}
