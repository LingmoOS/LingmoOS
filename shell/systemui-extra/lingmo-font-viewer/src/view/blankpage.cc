#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QPainter>

#include "blankpage.h"
#include "globalsizedata.h"

/* 适配组内SDK */
#include <gsettingmonitor.h>
#include <usermanual.h>
#include "windowmanage.hpp"
#include <QPainterPath>

const QSize IMAGESIZE = QSize(128, 128);

BlankPage::BlankPage(QWidget *parent)
    : QWidget(parent)
{
    setWidgetUi();
    initLayout();
    initGsetting();
    changeTheme();
}

BlankPage::~BlankPage()
{
}

void BlankPage::setWidgetUi()
{
    m_blankLabel = new QLabel(this);

    m_blankTextLabel = new QLabel(this);

    m_blankTextLabel->setText(tr("No Font"));
    QString labGrayStyle = "color:#8C8C8C;";
    m_blankTextLabel->setStyleSheet(labGrayStyle);
    m_blankTextLabel->setFixedWidth(128);
    m_blankTextLabel->setAlignment(Qt::AlignHCenter);

    QFont font14;
    font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
    m_blankTextLabel->setFont(font14);

    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();
    return;
}

void BlankPage::initLayout()
{
    this->setMinimumSize(588, 440);
    this->setBackgroundRole(QPalette::Base);

    m_vLayout->addStretch();
    m_vLayout->addWidget(m_blankLabel);
    m_vLayout->addWidget(m_blankTextLabel);
    m_vLayout->addStretch();
    m_vLayout->setSpacing(0);
    m_vLayout->setMargin(0);

    m_hLayout->addStretch();
    m_hLayout->addLayout(m_vLayout);
    m_hLayout->addSpacing(IMAGESIZE.width() / 2);
    m_hLayout->addStretch();
    m_hLayout->setSpacing(0);
    m_hLayout->setMargin(0);

    this->setLayout(m_hLayout);


    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontChange, [=](){
        QFont font14;
        font14.setPointSizeF(GlobalSizeData::getInstance()->g_font14pxToPt);
        m_blankTextLabel->setFont(font14);
    });
    connect(GlobalSizeData::getInstance(), &GlobalSizeData::sigFontNameChange, [=](){
        QFont fontName(GlobalSizeData::getInstance()->m_fontName);
        m_blankTextLabel->setFont(fontName);

    });
    return;
}

void BlankPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    QPainterPath rectPath;
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    return;
}

void BlankPage::initGsetting()
{
    // 主题颜色
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalSizeData::getInstance()->getSystemTheme();
        changeTheme();
    });
}

void BlankPage::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::LINGMOLight) {
        QIcon blankIcon = QIcon(":/data/image/light.svg");
        m_blankLabel->setPixmap(blankIcon.pixmap(blankIcon.actualSize(IMAGESIZE)));
    } else {
        QIcon blankIcon = QIcon(":/data/image/dark.svg");
        m_blankLabel->setPixmap(blankIcon.pixmap(blankIcon.actualSize(IMAGESIZE)));
    }
}

void BlankPage::changeSearchText()
{
    m_blankTextLabel->setText(tr("No Search Results"));
}

void BlankPage::changeBlankText()
{
    m_blankTextLabel->setText(tr("No Font"));
}
