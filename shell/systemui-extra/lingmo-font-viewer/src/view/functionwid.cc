#include <QDebug>
#include "functionwid.h"

FunctionWid::FunctionWid(QWidget *parent) : QWidget(parent)
{
    setWidgetUi();
    setWidgetStyle();
    initLayout();
    setSlotConnet();
}

FunctionWid::~FunctionWid()
{
}

/* 初始化 */
void FunctionWid::setWidgetUi()
{
    m_leftTitle = new BasePopupTitle(false);

    m_funBar = new KNavigationBar(this);

    QStandardItem *itemAll = new QStandardItem(QIcon::fromTheme("font-select-symbolic"),tr("All Font"));
    QStandardItem *itemSys = new QStandardItem(QIcon::fromTheme("system-computer-symbolic"),tr("System Font"));
    QStandardItem *itemUser = new QStandardItem(QIcon::fromTheme("avatar-default-symbolic"),tr("User Font"));
    QStandardItem *itemColl = new QStandardItem(QIcon::fromTheme("lingmo-love-symbolic"),tr("Collection Font"));

    m_funBar->addItem(itemAll);
    m_funBar->addItem(itemSys);
    m_funBar->addItem(itemUser);
    m_funBar->addItem(itemColl);

    return;
}

/* 布局 */
void FunctionWid::initLayout()
{
    this->setFocusPolicy(Qt::ClickFocus);

    m_vLayout = new QVBoxLayout();
    m_vLayout->addWidget(m_funBar);

    m_hLayout = new QHBoxLayout();
    m_hLayout->addSpacing(10);
    m_hLayout->addLayout(m_vLayout);

    m_mainVLayout = new QVBoxLayout();
    m_mainVLayout->addWidget(m_leftTitle);
    m_mainVLayout->addSpacing(8);
    m_mainVLayout->addLayout(m_hLayout);
    m_mainVLayout->addStretch();
    m_mainVLayout->setMargin(0);
    m_mainVLayout->setSpacing(0);

    this->setLayout(m_mainVLayout);

    return;
}

/* 关联信号与槽 */
void FunctionWid::setSlotConnet()
{
    /* 切换列表内容 */
    connect(m_funBar->listview(), &QListView::clicked, this, &FunctionWid::slotNaviClicked);

    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigPCMode, this, &FunctionWid::slotChangePCMode);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigHFlatMode, this, &FunctionWid::slotChangeFaltMode);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigVFlatMode, this, &FunctionWid::slotChangeFaltMode);

    return;
}

/* 切换字体列表 */
 void FunctionWid::slotNaviClicked(const QModelIndex &index)
 {
    QString str;
    QString data = index.data().toString();

    if (data == tr("All Font")) {
        m_fontType = FontType::AllFont;
    } else if(data == tr("Collection Font")) {
        m_fontType = FontType::CollectFont;
    } else if(data == tr("System Font")) {
        m_fontType = FontType::SystemFont;
    } else {
        m_fontType = FontType::MyFont;
    }
    GlobalSizeData::getInstance()->g_fontType = m_fontType;
    emit sigChangeFontType(m_fontType);

    return;
}

void FunctionWid::setWidgetStyle()
{
    switch (GlobalSizeData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:  
        slotChangePCMode();
            break;
        case CurrentMode::HMode:
        slotChangeFaltMode();
            break;
        case CurrentMode::VMode:
        slotChangeFaltMode();
            break;
    }
    return;
}

 void FunctionWid::slotChangePCMode()
{
    m_leftTitle->setFixedHeight(46);
    this->setFixedWidth(190);
    return;
}

void FunctionWid::slotChangeFaltMode()
{
    m_leftTitle->setFixedHeight(64);
    this->setFixedWidth(244);
    return;
}