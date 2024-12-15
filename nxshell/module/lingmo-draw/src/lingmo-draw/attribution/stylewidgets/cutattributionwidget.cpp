// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cutattributionwidget.h"
#include "cspinbox.h"
#include "hboxlayoutwidget.h"
#include <QLabel>
#include <QSpinBox>
#include "drawboard.h"
#include "drawboardtoolmgr.h"
#include "ccuttool.h"
#include <QButtonGroup>
#include "pageview.h"

const QSize ICON_RECT(28, 28);
const QSize BUTTON_RECT(70, 65);
const int BUTTON_SPACING = 5;
CutAttributionWidget::CutAttributionWidget(DrawBoard *drawBoard, QWidget *parent): AttributeWgt(ECutToolAttri, parent)
    , m_drawBoard(drawBoard)
{
    initUi();
    initConnect();
}

void CutAttributionWidget::initUi()
{
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    QHBoxLayout *m_scaleStyle = new QHBoxLayout(this);
    QLabel *title = new QLabel(tr("Ratio"));
    QLabel *scale_w = new QLabel();
    scale_w->setText("W");
    scale_w->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    w_spinbox = new CSpinBox();
    w_spinbox->setEnabledEmbedStyle(true);
    w_spinbox->setSpinRange(10, 10000);
    w_spinbox->setMinimumWidth(90);
    w_spinbox->setValue(1920);

    QLabel *scale_h = new QLabel();
    scale_h->setText("H");
    scale_h->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    h_spinbox = new CSpinBox();
    h_spinbox->setEnabledEmbedStyle(true);
    h_spinbox->setSpinRange(10, 10000);
    h_spinbox->setMinimumWidth(90);
    h_spinbox->setValue(1080);

    mainlayout->addWidget(title);
    m_scaleStyle->addWidget(scale_w);
    m_scaleStyle->addWidget(w_spinbox);
    m_scaleStyle->addSpacing(10);
    m_scaleStyle->addWidget(scale_h);
    m_scaleStyle->addWidget(h_spinbox);

    QGridLayout *m_gridLayout = new QGridLayout();
//    m_gridLayout->setAlignment(Qt::AlignLeft);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setMargin(10);
    m_gridLayout->setVerticalSpacing(10);
    m_gridLayout->setHorizontalSpacing(10);
    QStringList cuttype_name = {tr("Original"), tr("Free"), "1:1", "2:3", "8:5", "16:9"};
    QStringList cuticon = {"original", "free", "1.1", "2.3", "8.5", "16.9"};
    button_group = new QButtonGroup(this);
    for (int i = 0; i < CUTTYPE; ++i) {
        QToolButton *button_type = new QToolButton(this);
        button_type->setIconSize(ICON_RECT);
        button_type->setIcon(QIcon::fromTheme(cuticon[i]));
        button_type->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button_type->setText(cuttype_name[i]);
        button_type->setCheckable(true);
        button_type->setMinimumSize(BUTTON_RECT);
        m_buttonList.push_back(button_type);
        button_group->addButton(button_type);
        m_gridLayout->addWidget(button_type, i / MAXROWNUM, i % MAXROWNUM);
    }
    //确认/取消按钮
    m_confirmbutton = new DSuggestButton(this);
    m_cancelbutton = new DPushButton(this);
    QHBoxLayout *m_buttonlayout = new QHBoxLayout(this);
    m_buttonlayout->setSpacing(10);

    m_cancelbutton->setText(tr("Cancel"));
    m_confirmbutton->setText(tr("Confirm"));
    m_confirmbutton->setFixedWidth(120);
    m_cancelbutton->setFixedWidth(120);

    m_buttonlayout->addWidget(m_cancelbutton);
    m_buttonlayout->addWidget(m_confirmbutton);

    mainlayout->addLayout(m_scaleStyle);
    mainlayout->addLayout(m_gridLayout);
    mainlayout->addStretch();
    mainlayout->addLayout(m_buttonlayout);
    setLayout(mainlayout);
}

void CutAttributionWidget::initConnect()
{
    auto tool_manager = m_drawBoard->toolManager();
    connect(m_drawBoard, &DrawBoard::cutSizeChange, this, [ = ](QSizeF rect, bool setattr) {
        setCutSize(rect.toSize(), setattr);
    });

    for (int i = 0; i < m_buttonList.size(); ++i) {
        connect(m_buttonList[i], &DToolButton::clicked, this, [ = ]() {
            CCutTool *current_tool =  dynamic_cast<CCutTool *>(tool_manager->tool(tool_manager->currentTool()));
            QSizeF newSize = current_tool->changeCutType(cutstyle, m_drawBoard->currentPage()->scene());;
            if (cutstyle > ECutType::cut_free) {
                qreal rd = Radio[cutstyle];
                newSize.setWidth(qRound(newSize.height() * rd));
            }
            setCutSize(newSize.toSize(), true);
        });
    }
    connect(button_group, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, [ = ](QAbstractButton * button, bool ischeckd) {
        int current_style =  m_buttonList.indexOf(static_cast<DToolButton *>(button));
        if (current_style >= 0 && current_style < m_buttonList.size())
            cutstyle = current_style;
    });

    connect(h_spinbox, &CSpinBox::valueChanged, this, [ = ]() {
        QSize newSize = m_cutCutSize;
        newSize.setHeight(h_spinbox->value());
        //切换模式为自由
        setCutType(cut_free);
        setCutSize(newSize, true);
    });
    connect(h_spinbox, &CSpinBox::editingFinished, this, [ = ]() {
        QSignalBlocker block(h_spinbox);
        h_spinbox->setFocus(Qt::NoFocusReason);
        m_drawBoard->currentPage()->view()->setFocus();
    }, Qt::QueuedConnection);

    connect(w_spinbox, &CSpinBox::editingFinished, this, [ = ]() {
        QSignalBlocker block(w_spinbox);
        w_spinbox->setFocus(Qt::NoFocusReason);
        m_drawBoard->currentPage()->view()->setFocus();
    }, Qt::QueuedConnection);

    connect(w_spinbox, &CSpinBox::valueChanged, this, [ = ]() {
        QSize newSize = m_cutCutSize;
        newSize.setWidth(w_spinbox->value());
        //切换模式为自由
        setCutType(cut_free);
        setCutSize(newSize, true);
    }, Qt::QueuedConnection);
    connect(m_confirmbutton, &DToolButton::clicked, this, [ = ]() {
        CCutTool *current_tool =  dynamic_cast<CCutTool *>(tool_manager->tool(tool_manager->currentTool()));
        current_tool->doFinished(true, true);

    });
    connect(m_cancelbutton, &DToolButton::clicked, this, [ = ]() {
        CCutTool *current_tool =  dynamic_cast<CCutTool *>(tool_manager->tool(tool_manager->currentTool()));
        current_tool->doFinished(false, true);
    });

}

void CutAttributionWidget::setCutSize(const QSize &sz, bool setattr)
{
    if (sz != m_cutCutSize) {
        m_cutCutSize = sz;
        w_spinbox->setSpinValue(sz.width());
        h_spinbox->setSpinValue(sz.height());
        if (setattr) {
            QList<QVariant> vars;
            vars << cutstyle << sz;
            m_drawBoard->setDrawAttribution(ECutToolAttri, vars);
        }
    }
}

void CutAttributionWidget::setCutType(const int type)
{
    if (cutstyle == type)
        return;
    cutstyle = type;
    m_buttonList[cutstyle]->toggle();
}

void CutAttributionWidget::resetCutAttribution()
{
    QRectF sceneRect =  m_drawBoard->currentPage()->context()->pageRect();
    //切换模式为自由
    setCutType(cut_free);
    setCutSize(QSize(qRound(sceneRect.width()), qRound(sceneRect.height())), true);
}

QSize CutAttributionWidget::getCutSzie()
{
    return QSize(w_spinbox->value(), h_spinbox->value());
}
