/* 适配组内SDK */
#include"windowmanage.hpp"

#include "popupfontinfo.h"
#include "globalsizedata.h"
#include "mainview.h"

PopupFontInfo::PopupFontInfo(QString fontPath , QWidget *parent) : QWidget(parent)
{
    m_font = Core::getInstance()->getFontDetails(fontPath);
    setWidegtUi();
    setWidLayout();
    changeFontSize();
}

PopupFontInfo::~PopupFontInfo()
{
}

void PopupFontInfo::setWidegtUi()
{
    int valueWidth = 300;

    this->setFixedSize(420, 470);
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
    m_fontName = new QLabel(this);
    m_nameValue = new QLabel(this);
    m_fontSeries = new QLabel(this);
    m_seriesValue = new QLabel(this);
    m_fontStyle = new QLabel(this);
    m_styleValue = new QLabel(this);
    m_fontType = new QLabel(this);
    m_typeValue = new QLabel(this);
    m_fontVersion = new QLabel(this);
    m_versionValue = new QLabel(this);
    m_fontPath = new QLabel(this);
    m_pathValue = new QLabel(this);
    m_fontCopyright = new QLabel(this);
    m_copyrightValue = new QLabel(this);
    m_fontTrademark = new QLabel(this);
    m_trademarkValue = new QLabel(this);

    m_tipText->setFixedHeight(35);
    m_tipText->setText(m_font.family);
    m_tipText->adjustSize();
    m_tipText->setWordWrap(true);
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        m_tipText->setAlignment(Qt::AlignTop|Qt::AlignRight);
    }else{
        m_tipText->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    }
    m_fontName->setFixedHeight(30);
    m_fontName->setText(tr("FontName:"));
    m_fontName->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_nameValue->setFixedHeight(30);
    m_nameValue->setFixedWidth(valueWidth);
    m_nameValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_nameValue->setText(m_font.name);
    m_nameValue->adjustSize();
    m_nameValue->setWordWrap(true);
    m_nameValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_nameValue->setCursor(Qt::IBeamCursor);

    if (m_font.name == "") {
        m_fontName->hide();
        m_nameValue->hide();
    }

    m_fontSeries->setFixedHeight(30);
    m_fontSeries->setText(tr("FontSeries:"));
    m_fontSeries->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_seriesValue->setFixedHeight(30);
    m_seriesValue->setFixedWidth(valueWidth);
    m_seriesValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_seriesValue->setText(m_font.family);
    m_seriesValue->adjustSize();
    m_seriesValue->setWordWrap(true);
    m_seriesValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_seriesValue->setCursor(Qt::IBeamCursor);

    if (m_font.family == "") {
        m_fontSeries->hide();
        m_seriesValue->hide();
    }

    m_fontStyle->setFixedHeight(30);
    m_fontStyle->setText(tr("FontStyle:"));
    m_fontStyle->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_styleValue->setFixedHeight(30);
    m_styleValue->setFixedWidth(valueWidth);
    m_styleValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_styleValue->setText(m_font.style);
    m_styleValue->adjustSize();
    m_styleValue->setWordWrap(true);
    m_styleValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_styleValue->setCursor(Qt::IBeamCursor);

    if (m_font.style == "") {
        m_fontStyle->hide();
        m_styleValue->hide();
    }

    m_fontType->setFixedHeight(30);
    m_fontType->setText(tr("FontType:"));
    m_fontType->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_typeValue->setFixedHeight(30);
    m_typeValue->setFixedWidth(valueWidth);
    m_typeValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_typeValue->setText(m_font.type);
    m_typeValue->adjustSize();
    m_typeValue->setWordWrap(true);
    m_typeValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_typeValue->setCursor(Qt::IBeamCursor);

    if (m_font.type == "") {
        m_fontType->hide();
        m_typeValue->hide();
    }

    m_fontVersion->setFixedHeight(30);
    m_fontVersion->setText(tr("FontVersion:"));
    m_fontVersion->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_versionValue->setText(m_font.version);
    m_versionValue->setFixedWidth(valueWidth);
    m_versionValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_versionValue->adjustSize();
    m_versionValue->setWordWrap(true);
    m_versionValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_versionValue->setCursor(Qt::IBeamCursor);

    if (m_font.version == "") {
        m_fontVersion->hide();
        m_versionValue->hide();
    }

    m_fontPath->setFixedHeight(30);
    m_fontPath->setText(tr("FontPath:"));
    m_fontPath->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_pathValue->setText(m_font.path);
    m_pathValue->setFixedWidth(valueWidth);
    m_pathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_pathValue->adjustSize();
    m_pathValue->setWordWrap(true);
    m_pathValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_pathValue->setCursor(Qt::IBeamCursor);

    if (m_font.path == "") {
        m_pathValue->hide();
        m_pathValue->hide();
    }

    m_fontCopyright->setFixedHeight(30);
    m_fontCopyright->setText(tr("FontCopyright:"));
    m_fontCopyright->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_copyrightValue->setText(m_font.copyright);
    m_copyrightValue->setFixedWidth(valueWidth);
    m_copyrightValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_copyrightValue->adjustSize();
    m_copyrightValue->setWordWrap(true);
    m_copyrightValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_copyrightValue->setCursor(Qt::IBeamCursor);

    if (m_font.copyright == "") {
        m_fontCopyright->hide();
        m_copyrightValue->hide();
    }

    m_fontTrademark->setFixedHeight(30);
    m_fontTrademark->setText(tr("FontTrademark:"));
    m_fontTrademark->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    m_trademarkValue->setText(m_font.manufacturer);
    m_trademarkValue->setFixedWidth(valueWidth);
    m_trademarkValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_trademarkValue->adjustSize();
    m_trademarkValue->setWordWrap(true);
    m_trademarkValue->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_trademarkValue->setCursor(Qt::IBeamCursor);

    if (m_font.manufacturer == "") {
        m_fontTrademark->hide();
        m_trademarkValue->hide();
    }

    m_continueBtn = new QPushButton();
    m_continueBtn->setFixedHeight(36);
    m_continueBtn->setText(tr("Confirm"));
    m_continueBtn->setProperty("isImportant", true);

    m_cancelBtn = new QPushButton();
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setText(tr("Cancel"));

    initWidgetStyle();

    connect(m_cancelBtn, &QPushButton::clicked, this, &PopupFontInfo::slotCancelClick);
    connect(m_continueBtn, &QPushButton::clicked, this, &PopupFontInfo::slotContinueClick);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, this, &PopupFontInfo::changeFontSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontNameChange, this, &PopupFontInfo::slotChangeFontName);

    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigPCMode, this, &PopupFontInfo::slotChangePCSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigHFlatMode, this, &PopupFontInfo::slotChangeFaltSize);
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigVFlatMode, this, &PopupFontInfo::slotChangeFaltSize);

    return;
}

void PopupFontInfo::setWidLayout()
{
    /* 字体信息网格布局 */
    m_gridLayout = new QGridLayout();

    m_gridLayout->setHorizontalSpacing(1);         // 设置水平间距
    m_gridLayout->setVerticalSpacing(4);            // 设置垂直间距
    m_gridLayout->setContentsMargins(5, 0, 0, 0);   // 设置外间距

    /* 字体名称 */
    m_gridLayout->addWidget(m_fontName, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_nameValue, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 系列 */
    m_gridLayout->addWidget(m_fontSeries, 1, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_seriesValue, 1, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 样式 */
    m_gridLayout->addWidget(m_fontStyle, 2, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_styleValue, 2, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 种类 */
    m_gridLayout->addWidget(m_fontType, 3, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_typeValue, 3, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 版本 */
    m_gridLayout->addWidget(m_fontVersion, 4, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_versionValue, 4, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 位置 */
    m_gridLayout->addWidget(m_fontPath, 5, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_pathValue, 5, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 版权 */
    m_gridLayout->addWidget(m_fontCopyright, 6, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_copyrightValue, 6, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    /* 商标 */
    m_gridLayout->addWidget(m_fontTrademark, 7, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->addWidget(m_trademarkValue, 7, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);

    m_vGridLayout = new QVBoxLayout();
    m_vGridLayout->addLayout(m_gridLayout);
    m_vGridLayout->addStretch();
    m_vGridLayout->setMargin(0);

    m_widget = new QWidget(this);
    m_widget->setFixedWidth(370);
    m_widget->setLayout(m_vGridLayout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFixedHeight( 300);
    // m_scrollArea->setFixedSize(370, 300);
    m_scrollArea->setWidget(m_widget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea {background-color:transparent;}");
    m_scrollArea->viewport()->setStyleSheet("background-color:transparent;");
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    /* 按钮横向布局 */
    m_hBtnLayout = new QHBoxLayout();
    m_hBtnLayout->addStretch();
    m_hBtnLayout->addWidget(m_cancelBtn);
    m_hBtnLayout->addSpacing(10);
    m_hBtnLayout->addWidget(m_continueBtn);
    m_hBtnLayout->setMargin(0);
    m_hBtnLayout->setContentsMargins(0, 0, 24, 0);

    /* 按钮和组件纵向布局 */
    m_vLayout = new QVBoxLayout();
    m_vLayout->addWidget(m_tipText);
    m_vLayout->addSpacing(10);
    m_vLayout->addWidget(m_scrollArea);
    m_vLayout->addSpacing(10);
    m_vLayout->addLayout(m_hBtnLayout);
    m_vLayout->addStretch();
    m_vLayout->setMargin(0);

    /* 组件居中 */
    m_hMainLayout = new QHBoxLayout();
    m_hMainLayout->addSpacing(24);
    m_hMainLayout->addLayout(m_vLayout);

    /* 整个窗体布局 */
    m_vMainLayout = new QVBoxLayout();
    m_vMainLayout->setSpacing(0);
    m_vMainLayout->addWidget(m_titleBar);
    m_vMainLayout->addSpacing(10);
    m_vMainLayout->addLayout(m_hMainLayout);
    m_vMainLayout->addStretch();
    m_vMainLayout->setMargin(0);
    m_vMainLayout->setContentsMargins(0, 0, 0, 20);   // 设置外间距

    this->setLayout(m_vMainLayout);

    /* 应用内居中 */
    this->move(MainView::getInstance()->geometry().center() - this->rect().center());

    return;
}

void PopupFontInfo::changeFontSize()
{
    QFont font16;
    font16.setPointSizeF(GlobalSizeData::getInstance()->g_font16pxToPt);
    font16.setBold(true);
    font16.setWeight(60);
    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);

    m_tipText->setFont(font16);

    m_fontName->setFont(font14);
    m_nameValue->setFont(font14);
    m_fontSeries->setFont(font14);
    m_seriesValue->setFont(font14);
    m_fontStyle->setFont(font14);
    m_styleValue->setFont(font14);
    m_fontType->setFont(font14);
    m_typeValue->setFont(font14);
    m_fontVersion->setFont(font14);
    m_versionValue->setFont(font14);
    m_fontPath->setFont(font14);
    m_pathValue->setFont(font14);
    m_fontCopyright->setFont(font14);
    m_copyrightValue->setFont(font14);
    m_fontTrademark->setFont(font14);
    m_trademarkValue->setFont(font14);

    m_continueBtn->setFont(font14);
    m_cancelBtn->setFont(font14);

    return;
}

void PopupFontInfo::slotContinueClick()
{
    this->window()->close();
    return;
}

void PopupFontInfo::slotCancelClick()
{
    this->window()->close();
    return;
}

void PopupFontInfo::slotChangeFontName()
{
    this->update();
    return;
}

void PopupFontInfo::initWidgetStyle()
{
    switch (GlobalSizeData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCSize();        
            break;
        case CurrentMode::HMode:
        slotChangeFaltSize();
            break;
        case CurrentMode::VMode:
        slotChangeFaltSize();
            break;
    }
    return;
}

void PopupFontInfo::slotChangeFaltSize()
{
    this->setFixedSize(420, 500);
    return;
}

void PopupFontInfo::slotChangePCSize()
{
    this->setFixedSize(420, 470);
    return;
}
