/* 适配组内SDK */
#include "windowmanage.hpp"

#include "popupremove.h"
#include "globalsizedata.h" 
#include "mainview.h"

PopupTips::PopupTips(QWidget *parent) : QWidget(parent)
{
    setWidgetUi();
    changeFontSize();
}

PopupTips::~PopupTips()
{
    if (m_titleBar != nullptr) {
        delete m_titleBar;
        m_titleBar = nullptr;
    }
}

void PopupTips::setWidgetUi()
{
    this->setFixedSize(400, 230);
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
    m_checkBox = new QCheckBox(this);
    m_vMainLayout = new QVBoxLayout();
    m_hMainLayout = new QHBoxLayout();
    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();

    /* 按钮横向布局 */
    m_hLayout->addStretch();
    m_hLayout->addWidget(m_sureBtn);

    /* 按钮和组件纵向布局 */
    m_vLayout->addWidget(m_tipText);
    m_vLayout->addWidget(m_checkBox);
    m_vLayout->addStretch();
    m_vLayout->addLayout(m_hLayout);
    m_vLayout->setSpacing(20);
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

    m_tips = tr("Installing new fonts takes effect after restarting!");
    m_tipText->setFixedWidth(332);
    m_tipText->setText(m_tips);
    m_tipText->setMargin(0);
    m_tipText->setAlignment(Qt::AlignTop);

    m_sureBtn->setFixedHeight(45);
    m_sureBtn->setText(tr("Confirm"));
    m_sureBtn->setProperty("isImportant", true);

    m_checkBox->setText(tr("Do not show again"));

    // 应用内居中
    this->move(MainView::getInstance()->geometry().center() - this->rect().center());

    connect(m_sureBtn, &QPushButton::clicked, this, &PopupTips::slotSureClick);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &PopupTips::changeFontSize);

    changeFontSize();
    return;
}

void PopupTips::changeFontSize()
{
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_tipText->setFont(font14);
    m_sureBtn->setFont(font14);

    QFontMetrics fontmts = QFontMetrics(font14);
    QString text = m_tips;
    if (fontmts.width(m_tips) > m_tipText->width()) {
        m_tipText->setToolTip(m_tips);
        text = fontmts.elidedText(text, Qt::ElideRight, m_tipText->width());
    } 
    m_tipText->setText(text);

    return;
}

void PopupTips::slotSureClick()
{
    this->window()->close();
    return;
}

void PopupTips::closeEvent(QCloseEvent *event)
{
    this->hide();
    emit signalWidgetClose();
    event->ignore(); // 忽略不结束进程
    if (m_checkBox->checkState() == Qt::Checked) {
        Core::getInstance()->settingFontTips(false);
    } else if (m_checkBox->checkState() == Qt::Unchecked) {
        Core::getInstance()->settingFontTips(true);
    }
    return;
}
