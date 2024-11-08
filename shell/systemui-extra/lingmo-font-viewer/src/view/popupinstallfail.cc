#include <QScrollBar>

/* 适配组内SDK */
#include "windowmanage.hpp"

#include "popupinstallfail.h"
#include "globalsizedata.h"
#include "mainview.h"

PopupInstallFail::PopupInstallFail(QStringList fontList, QWidget *parent) : QWidget(parent)
{
    m_uninstallFont = fontList;
    setWidgetUi();
    setWidLayout();
    changeFontSize();
}

PopupInstallFail::~PopupInstallFail()
{
}

void PopupInstallFail::setWidgetUi()
{
    this->setWindowFlag(Qt::Tool);
    this->setWindowModality(Qt::WindowModal);
    this->setWindowModality(Qt::ApplicationModal);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setAttribute(Qt::WA_DeleteOnClose,true);

    /* 适配lingmosdk的窗管 */
    ::kabase::WindowManage::removeHeader(this);

    m_titleBar = new BasePopupTitle();
    m_tipIcon = new QPushButton(this);
    m_tipText = new QLabel(this);
    m_fontListWid = new QListWidget(this);
    m_continueBtn = new QPushButton(this);

    //设置图标
    m_tipIcon->setIcon(QIcon::fromTheme("dialog-error"));
    m_tipIcon->setIconSize(QSize(24, 24));
    m_tipIcon->setProperty("isWindowButton", 0x1);
    m_tipIcon->setProperty("useIconHighlightEffect", 0x2);
    m_tipIcon->setAttribute(Qt::WA_TranslucentBackground, true);
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_tipIcon->setStyleSheet(btnStyle);

    // m_tipText->setFixedSize(300, 55);
    m_tipText->setFixedWidth(300);
    m_tipText->setText(tr("There is a problem with the font file. Installation failed!"));
    m_tipText->adjustSize();
    m_tipText->setWordWrap(true);
    m_tipText->setAlignment(Qt::AlignTop);

    m_continueBtn->setFixedHeight(36);
    m_continueBtn->setText(tr("confirm"));
    m_continueBtn->setProperty("isImportant", true);

    int num = 0;

    for (int i = 0; i < m_uninstallFont.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(328, 36));
        item->setFlags(Qt::ItemIsEnabled);
        item->setTextAlignment(Qt::AlignVCenter);

        QWidget *fontNameWid = new QWidget();
        QLabel *fontNameLab = new QLabel(fontNameWid);
        fontNameLab->setText(m_uninstallFont[i]);

        QHBoxLayout *hLayout;
        hLayout = new QHBoxLayout();
        hLayout->addSpacing(10);
        hLayout->addWidget(fontNameLab);
        hLayout->setMargin(0);
        QVBoxLayout *vLayout;
        vLayout = new QVBoxLayout();
        vLayout->addStretch();
        vLayout->addLayout(hLayout);
        vLayout->addStretch();
        vLayout->setMargin(0);
        fontNameWid->setLayout(vLayout);

        m_fontListWid->addItem(item);
        m_fontListWid->setItemWidget(item, fontNameWid);

        num++;
    }
    if (num > 4) {
        m_fontListWid->setFixedHeight(4 * 40);
    } else {
        m_fontListWid->setFixedHeight(num * 40);
    }
    m_fontListWid->setFixedWidth(328);

    m_fontListWid->setFrameShape(QListView::NoFrame);                              /* 去除边框 */
    m_fontListWid->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           /* 取消横向滚动 */
    m_fontListWid->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);   /* 滚动条禁用右键 */
    m_fontListWid->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_fontListWid->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_fontListWid->setAlternatingRowColors(true);   /* 奇偶行颜色不同 */
    m_fontListWid->setDragEnabled(false);

    this->setFixedWidth(380);
    this->setFixedHeight(150 + m_fontListWid->height() + m_tipText->height());

    connect(m_continueBtn, &QPushButton::clicked, this, &PopupInstallFail::slotContinueClick);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &PopupInstallFail::changeFontSize);

    return;
}

void PopupInstallFail::setWidLayout()
{
    m_vMainLayout = new QVBoxLayout();
    m_hMainLayout = new QHBoxLayout();
    m_hBtnLayout = new QHBoxLayout();
    m_hTipLayout = new QHBoxLayout();
    m_vIconLayout = new QVBoxLayout();
    m_vLayout = new QVBoxLayout();

    // m_vIconLayout->setMargin(0);
    // m_vIconLayout->setSpacing(0);
    m_vIconLayout->addWidget(m_tipIcon);
    m_vIconLayout->addStretch();

    /* 提示横向布局 */
    // m_hTipLayout->addWidget(m_tipIcon);
    m_hTipLayout->addLayout(m_vIconLayout);
    m_hTipLayout->addSpacing(10);
    m_hTipLayout->addWidget(m_tipText);
    m_hTipLayout->addStretch();

    /* 按钮横向布局 */
    m_hBtnLayout->addStretch();
    m_hBtnLayout->addWidget(m_continueBtn);
    m_hBtnLayout->setMargin(0);
    m_hBtnLayout->setSpacing(0);

    /* 按钮和组件纵向布局 */
    m_vLayout->addLayout(m_hTipLayout);
    m_vLayout->addSpacing(10);
    m_vLayout->addWidget(m_fontListWid);
    m_vLayout->addSpacing(24);
    m_vLayout->addLayout(m_hBtnLayout);
    m_vLayout->addStretch();
    m_vLayout->setMargin(0);

    /* 组件居中 */
    m_hMainLayout->addStretch();
    m_hMainLayout->addLayout(m_vLayout);
    m_hMainLayout->addStretch();

    /* 整个窗体布局 */
    m_vMainLayout->setSpacing(0);
    m_vMainLayout->addWidget(m_titleBar);
    m_vMainLayout->addSpacing(10);
    m_vMainLayout->addLayout(m_hMainLayout);
    m_vMainLayout->addStretch();
    m_vMainLayout->setMargin(0);
    this->setLayout(m_vMainLayout);

    // 应用内居中
    this->move(MainView::getInstance()->geometry().center() - this->rect().center());

    return;
}

void PopupInstallFail::changeFontSize()
{
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_tipText->setFont(font14);
    m_continueBtn->setFont(font14);
    this->setFixedHeight(150 + m_fontListWid->height() + m_tipText->height());

    return;
}

void PopupInstallFail::slotContinueClick()
{
    this->window()->close();
    return;
}

void PopupInstallFail::closeEvent(QCloseEvent *event) 
{
    this->hide();
    emit signalWidgetClose();
    event->ignore(); // 忽略不结束进程
    return;
}
