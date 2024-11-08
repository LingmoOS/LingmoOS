#include <QTimer>

/* 适配组内SDK */
#include "windowmanage.hpp"

#include "popupinstallsuccess.h"
#include "globalsizedata.h"
#include "mainview.h"

PopupInstallSuccess::PopupInstallSuccess(int num ,QWidget *parent) : QWidget(parent)
{
    m_num = num;
    setWidgetUi();
    changeFontSize();
}

PopupInstallSuccess::~PopupInstallSuccess()
{
}

void PopupInstallSuccess::setWidgetUi()
{
    this->setWindowFlag(Qt::Tool);
    this->setWindowModality(Qt::WindowModal);
    this->setWindowModality(Qt::ApplicationModal);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    /* 适配lingmosdk的窗管 */
    ::kabase::WindowManage::removeHeader(this);

    m_tipIcon = new QPushButton(this);
    m_tipText = new QLabel(this);

    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();

    m_hLayout->setSpacing(0);
    m_hLayout->addStretch();
    m_hLayout->addWidget(m_tipIcon);
    m_hLayout->addSpacing(9);
    m_hLayout->addWidget(m_tipText);
    m_hLayout->addStretch();
    m_hLayout->setMargin(0);

    m_vLayout->setContentsMargins(0,0,0,0);
    m_vLayout->setSpacing(0);
    m_vLayout->addStretch();
    m_vLayout->addLayout(m_hLayout);
    m_vLayout->addStretch();
    this->setLayout(m_vLayout);

    //设置图标
    m_tipIcon->setIcon(QIcon::fromTheme("lingmo-dialog-success"));
    m_tipIcon->setIconSize(QSize(24, 24));
    m_tipIcon->setProperty("isWindowButton", 0x1);
    m_tipIcon->setProperty("useIconHighlightEffect", 0x2);
    m_tipIcon->setAttribute(Qt::WA_TranslucentBackground, true);
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_tipIcon->setStyleSheet(btnStyle);

    /* m_tipText为要显示的信息 */
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_tipText->setFont(font14);
    QString tipText = tr("already installed ") + QString::number(m_num) + tr(" fonts!");
    m_tipText->setText(tipText);

    QFontMetrics fontmts = QFontMetrics(font14);
    int width = fontmts.width(tipText) + 55;
    this->setFixedWidth(width);
    this->setFixedHeight(60);

    // 关于弹窗应用内居中
    QRect availableGeometry = this->geometry();
    this->move(availableGeometry.center() - this->rect().center());

    /* 设置定时器，到时自我销毁 */
    QTimer* timer = new QTimer(this);
    timer->start(2000);           /* 时间2秒 */
    timer->setSingleShot(true);   /* 仅触发一次 */

    connect(timer, &QTimer::timeout, this, &PopupInstallSuccess::slotTimeDestroy);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &PopupInstallSuccess::changeFontSize);

    // 应用内居中
    this->move(MainView::getInstance()->geometry().center() - this->rect().center());

    return;
}

void PopupInstallSuccess::changeFontSize()
{
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_tipText->setFont(font14);
    return;
}

void PopupInstallSuccess::slotTimeDestroy()
{
    this->window()->close();
    return;
}
