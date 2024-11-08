/* 适配组内SDK */
#include "windowmanage.hpp"

#include "popupremove.h"
#include "globalsizedata.h" 
#include "mainview.h"

PopupRemove::PopupRemove(QString path,QWidget *parent) : QWidget(parent)
{
    m_path = path;
    setWidgetUi();
    changeFontSize();
}

PopupRemove::~PopupRemove()
{
}

void PopupRemove::setWidgetUi()
{
    this->setFixedSize(400, 200);
    this->setWindowFlag(Qt::Tool);
    this->setWindowModality(Qt::WindowModal);
    this->setWindowModality(Qt::ApplicationModal);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setAttribute(Qt::WA_DeleteOnClose,true);

    /* 适配lingmosdk的窗管 */
    ::kabase::WindowManage::removeHeader(this);

    m_titleBar = new BasePopupTitle();
    m_tipText = new QLabel(this);
    m_sureBtn = new QPushButton(this);
    m_cancelBtn = new QPushButton(this);
    m_vMainLayout = new QVBoxLayout();
    m_hMainLayout = new QHBoxLayout();
    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();

    /* 按钮横向布局 */
    m_hLayout->addStretch();
    m_hLayout->addWidget(m_cancelBtn);
    m_hLayout->setSpacing(10);
    m_hLayout->addWidget(m_sureBtn);

    /* 按钮和组件纵向布局 */
    m_vLayout->addWidget(m_tipText);
    m_vLayout->setSpacing(5);
    m_vLayout->addLayout(m_hLayout);
    m_vLayout->addStretch();
    m_vLayout->setMargin(0);

    /* 组件居中 */
    m_hMainLayout->addStretch();
    m_hMainLayout->addLayout(m_vLayout);
    m_hMainLayout->addStretch();

    /* 整个窗体布局 */
    m_vMainLayout->setSpacing(0);
    m_vMainLayout->addWidget(m_titleBar);
    m_vMainLayout->addSpacing(16);
    m_vMainLayout->addLayout(m_hMainLayout);
    m_vMainLayout->addStretch();
    m_vMainLayout->setMargin(0);
    this->setLayout(m_vMainLayout);

    m_tipText->setFixedSize(332, 65);
    m_tipText->setText(tr("Are you sure you want to remove this font?"));

    m_tipText->adjustSize();   /* 当内容过长自动换行 */
    m_tipText->setWordWrap(true);
    m_tipText->setAlignment(Qt::AlignTop);

    m_sureBtn->setFixedHeight(36);
    m_sureBtn->setText(tr("Confirm"));
    m_sureBtn->setProperty("isImportant", true);

    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setText(tr("Cancel"));

    // 应用内居中
    this->move(MainView::getInstance()->geometry().center() - this->rect().center());

    connect(m_sureBtn, &QPushButton::clicked, this, &PopupRemove::slotSureClick);
    connect(m_cancelBtn, &QPushButton::clicked, this, &PopupRemove::slotCancelClick);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &PopupRemove::changeFontSize);

    return;
}

void PopupRemove::changeFontSize()
{
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_tipText->setFont(font14);
    m_sureBtn->setFont(font14);
    m_cancelBtn->setFont(font14);
    return;
}

void PopupRemove::slotSureClick()
{
    emit sigSureClick(m_path);
    return;
}

void PopupRemove::slotCancelClick()
{
    this->window()->close();
    return;
}
