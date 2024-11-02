#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QPalette>

#include "basepopuptitle.h"
#include "globalsizedata.h"

const QSize ICON_SIZE_PC = QSize(24, 24);
const QSize ICON_SIZE_PAD = QSize(26, 26);
const QSize CLOSE_SIZE_PC = QSize(30, 30);
const QSize CLOSE_SIZE_PAD = QSize(48, 48);
const int WIDGET_WIDTH_PC = 180;
const int WIDGET_WIDTH_PAD = 254;
const int WIDGET_HEIGHT_PC = 40;
const int WIDGET_HEIGHT_PAD = 64;

BasePopupTitle::BasePopupTitle(bool isClose, QString value)
{
    m_value = value;
    m_iconAppPath = "lingmo-font-viewer";
    m_iconClosePath = "window-close-symbolic";
    m_space = 4;
    m_isClose = isClose;
    this->init();
}

BasePopupTitle::~BasePopupTitle()
{
}

void BasePopupTitle::init(void)
{
    m_iconBtn = new QPushButton(this);
    m_iconBtn->setIcon(QIcon::fromTheme(m_iconAppPath));
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_iconBtn->setStyleSheet(btnStyle);

    m_valueLab = new QLabel(this);
    m_valueLab->setText(m_value);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setIcon(QIcon::fromTheme(m_iconClosePath));
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("Close"));
    if (!m_isClose) {
        m_closeBtn->hide();
    }

    m_hlayout = new QHBoxLayout();
    m_hlayout->setContentsMargins(4, 4, 4, 4);
    m_hlayout->setSpacing(0);
    m_hlayout->addSpacing(m_space);
    m_hlayout->addWidget(m_iconBtn);
    m_hlayout->addSpacing(m_space);
    m_hlayout->addWidget(m_valueLab);
    m_hlayout->addStretch();
    m_hlayout->addWidget(m_closeBtn);

    this->setLayout(m_hlayout);

    setWidgetStyle();

    connect(m_closeBtn , &QPushButton::clicked , this , &BasePopupTitle::slotCloseWid);
    
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigPCMode, this, &BasePopupTitle::slotChangePCModeSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigHFlatMode, this, &BasePopupTitle::slotChangePadModeSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigVFlatMode, this, &BasePopupTitle::slotChangePadModeSize);

    return;
}

void BasePopupTitle::setWidgetStyle()
{
    switch (GlobalSizeData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCModeSize();        
            break;
        case CurrentMode::HMode:
        slotChangePadModeSize();
            break;
        case CurrentMode::VMode:
        slotChangePadModeSize();
            break;
    }
    return;
}

void BasePopupTitle::slotCloseWid()
{
    this->window()->close();
    return;
}

void BasePopupTitle::slotChangePCModeSize()
{
    this->setMinimumWidth(WIDGET_WIDTH_PC);
    this->setFixedHeight(WIDGET_HEIGHT_PC);
    m_iconBtn->setIconSize(ICON_SIZE_PC);
    m_closeBtn->setFixedSize(CLOSE_SIZE_PC);
    return;
}

void BasePopupTitle::slotChangePadModeSize()
{
    this->setMinimumWidth(WIDGET_WIDTH_PAD);
    this->setFixedHeight(WIDGET_HEIGHT_PAD);
    m_iconBtn->setIconSize(ICON_SIZE_PAD);
    m_closeBtn->setFixedSize(CLOSE_SIZE_PAD);
    return;
}