#include "filesysteminfoitem.h"
#include "../macro.h"

#include <QPainterPath>
#include <QPainter>
#include <QIcon>
#include <QDebug>
#include <QApplication>

FileSystemInfoItem::FileSystemInfoItem(QWidget *parent)
    : QPushButton(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    installEventFilter(this);
    setAttribute(Qt::WA_TranslucentBackground);
    initUI();
    initConnections();
}

FileSystemInfoItem::~FileSystemInfoItem()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void FileSystemInfoItem::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
}

void FileSystemInfoItem::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    adjustContentSize();
}

void FileSystemInfoItem::initUI()
{
    // 初始化布局
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setContentsMargins(20,8,20,8);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setAlignment(Qt::AlignVCenter);
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(0,0,0,0);
    m_contentLayout->setSpacing(6);
    m_contentLayout->setAlignment(Qt::AlignVCenter);
    m_contentMidLayout = new QHBoxLayout();
    m_contentMidLayout->setContentsMargins(0,0,0,0);
    m_contentMidLayout->setSpacing(8);

    m_iconDev = new QLabel();
    m_iconDev->setFixedSize(60,60);
    m_mainLayout->addWidget(m_iconDev);
    m_labelDevName = new KLabel();
    m_labelDevName->setAlignment(Qt::AlignLeft);
    m_labelDevName->setFixedWidth(250);
    m_contentLayout->addWidget(m_labelDevName, 0, Qt::AlignLeft|Qt::AlignVCenter);
    m_contentLayout->addLayout(m_contentMidLayout);
    m_labelUsedCapacity = new KLabel();
    m_labelUsedCapacity->setFixedWidth(120);
    m_labelUsedCapacity->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_labelUsedCapacity->setStyleSheet("color:palette(highlight)");
    m_contentMidLayout->addWidget(m_labelUsedCapacity,0,Qt::AlignLeft|Qt::AlignVCenter);
    m_labelOther = new KLabel();
    m_labelOther->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_contentMidLayout->addWidget(m_labelOther,1,Qt::AlignLeft|Qt::AlignVCenter);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setTextVisible(false);
    m_progressBar->setMaximum(100);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setMinimumWidth(623);
    m_contentLayout->addWidget(m_progressBar);

    m_mainLayout->addLayout(m_contentLayout);
    this->setLayout(m_mainLayout);
    initStyleTheme();
}

void FileSystemInfoItem::initConnections()
{
    connect(this, &FileSystemInfoItem::clicked, this, [&,this](){
        Q_EMIT this->dblClicked(this->m_fsData.mountDir()); });
}

void FileSystemInfoItem::onUpdateInfo(FileSystemData& fsData)
{
    m_fsData = fsData;
    m_iconDev->setPixmap(QIcon::fromTheme("drive-harddisk").pixmap(60,60));
    m_labelDevName->setText(fsData.deviceName());
    m_progressBar->setValue(fsData.usedPercentage());
    QString strUsedCapcity = QString(tr("%1 Used")).arg(fsData.usedCapactiy());
    m_labelUsedCapacity->setText(strUsedCapcity);
    QString strOther = QString(tr("(%1 Available/%2 Free)  Total %3  %4  Directory:%5")).arg(fsData.availCapacity())
            .arg(fsData.freeCapacity()).arg(fsData.totalCapacity()).arg(fsData.diskType()).arg(fsData.mountDir());
    m_labelOther->setText(strOther);
    adjustContentSize();
}

void FileSystemInfoItem::initStyleTheme()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        //监听主题改变
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == "styleName") {
                auto variant = m_styleSettings->get("styleName");
                if (variant.isValid()) {
                    onThemeStyleChange(variant.toString());
                }
            } else if ("iconThemeName" == key) {
                if (m_iconDev) {
                    m_iconDev->setPixmap(QIcon::fromTheme("drive-harddisk").pixmap(60,60));
                }
            } else if("systemFont" == key || "systemFontSize" == key) {
                auto variant = m_styleSettings->get(FONT_SIZE);
                if (variant.isValid()) {
                    m_curFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
                    onThemeFontChange(m_curFontSize);
                }
            }
        });
        auto variant = m_styleSettings->get(FONT_SIZE);
        if (variant.isValid()) {
            m_curFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
            onThemeFontChange(m_curFontSize);
        }
        variant = m_styleSettings->get("styleName");
        if (variant.isValid()) {
            onThemeStyleChange(variant.toString());
        }
    }
}

void FileSystemInfoItem::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    QFont titleFont = this->font();
    if (m_labelUsedCapacity) {
        m_labelUsedCapacity->setFont(titleFont);
    }
    if (m_labelOther) {
        m_labelOther->setFont(titleFont);
    }
    if (m_labelDevName) {
        m_labelDevName->setFont(titleFont);
    }
}

void FileSystemInfoItem::onThemeStyleChange(QString strStyleName)
{
    if (m_styleSettings->keys().contains("widget-theme-name") || m_styleSettings->keys().contains("widgetThemeName")) {
        auto variant = m_styleSettings->get("widget-theme-name");
        if (variant.isValid()) {
            if (variant.toString().compare("classical", Qt::CaseInsensitive) == 0) {
                mBoardRadius = 0;
            } else {
                mBoardRadius = 6;
            }
        }
    }

    if(strStyleName == "lingmo-dark" || strStyleName == "lingmo-black") {
        this->setStyleSheet(QString("QPushButton{text-align:center;border: none;border-radius: %1px;outline: none;"
                            "background-color: rgba(26, 26, 26, 100%);}"
                            "QPushButton::hover{background-color: rgba(65, 65, 65, 100%);}"
                            "QPushButton::pressed {background-color: rgba(55, 55, 55, 100%);}"
                            "QPushButton::checked {background-color: rgba(55, 55, 55, 100%);}").arg(mBoardRadius));
    } else {
        this->setStyleSheet(QString("QPushButton{text-align:center;border: none;border-radius: %1px;outline: none;"
                            "background-color: rgba(245, 245, 245, 100%);}"
                            "QPushButton::hover{background-color: rgba(230, 230, 230, 100%);}"
                            "QPushButton::pressed {background-color: rgba(218, 218, 218, 100%);}"
                            "QPushButton::checked {background-color: rgba(218, 218, 218, 100%);}").arg(mBoardRadius));
    }
    repaint();
}

void FileSystemInfoItem::adjustContentSize()
{
    if (m_labelOther) {
        m_labelOther->setFixedWidth(width()-250);
    }
}

bool FileSystemInfoItem::eventFilter(QObject *watched, QEvent *event)
{
    return QPushButton::eventFilter(watched, event);
}
