// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprintersupplyshowdlg.h"
#include "dprintermanager.h"
#include "dprinter.h"

#include <DLabel>
#include <DProgressBar>
#include <DFrame>
#include <DWidget>
#include <DProgressBar>
#include <DFontSizeManager>
#include <DPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QTime>
#include <QSpacerItem>

DPrinterSupplyShowDlg::DPrinterSupplyShowDlg(RefreshSnmpBackendTask *task, QWidget *parent): DDialog(parent)
    , m_pFreshTask(task)
{
    initUI();
    initConnection();
    initColorTrans();
}

DPrinterSupplyShowDlg::~DPrinterSupplyShowDlg()
{
    if (m_pFreshSpinner) {
        m_pFreshSpinner->deleteLater();
    }
}


void DPrinterSupplyShowDlg::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    if (m_pFreshSpinner)
        m_pFreshSpinner->start();

    if (m_pFreshTask) {
        m_pFreshTask->beginTask();
    }
}

void DPrinterSupplyShowDlg::closeEvent(QCloseEvent *event)
{
    if (m_pFreshSpinner)
        m_pFreshSpinner->stop();

    DDialog::closeEvent(event);
}

void DPrinterSupplyShowDlg::initUI()
{
    setIcon(QIcon(":/images/dde-printer.svg"));
    m_pContentWidget = new QWidget;
    QVBoxLayout *pVlayout = new QVBoxLayout;
    pVlayout->setSpacing(0);
    pVlayout->setContentsMargins(10, 0, 10, 0);
    m_pFreshSpinner = new DSpinner;
    m_pFreshSpinner->setFixedSize(36, 36);
    m_pFreshSpinner->setAccessibleName("freshSpinner_contentWidget");
    pVlayout->addWidget(m_pFreshSpinner, 0, Qt::AlignCenter);
    m_pContentWidget->setLayout(pVlayout);
    m_pContentWidget->setAccessibleName("contentWidget_printerSupply");
    addContent(m_pContentWidget);
    /*耗材信息的列表长度无法确定，固定高度窗口会导致内容显示不全*/
    setFixedWidth(380);
    setMinimumHeight(356);
    moveToParentCenter();
    this->setAccessibleName("printerSupply_mainWindow");
}

void DPrinterSupplyShowDlg::initConnection()
{
    if (m_pFreshTask) {
        connect(m_pFreshTask, &RefreshSnmpBackendTask::refreshsnmpfinished,
                this, &DPrinterSupplyShowDlg::supplyFreshed);
    }
}

void DPrinterSupplyShowDlg::moveToParentCenter()
{
    QWidget *pParent = parentWidget();

    if (pParent == nullptr) {
        return;
    } else {
        QRect parentRc = pParent->geometry();
        QPoint parentCen = parentRc.center();
        QRect selfRc = geometry();
        QPoint selfCen = selfRc.center();
        QPoint selfCenInParent = mapToParent(selfCen);
        QPoint mvPhasor = parentCen - selfCenInParent;
        move(mvPhasor.x(), mvPhasor.y());
    }
}

QWidget *DPrinterSupplyShowDlg::initColorSupplyItem(const SUPPLYSDATA &info, bool bColor)
{
    QWidget *pWidget = new QWidget;
    pWidget->setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, QColor(0, 0, 0, int(0.03 * 255)));
    pWidget->setPalette(pal);
    QHBoxLayout *pHlayout = new QHBoxLayout;
    pHlayout->setSpacing(8);

    if ((info.colorant != 0) && bColor) {
        QPixmap colorPix(12, 12);
        colorPix.fill(Qt::transparent);
        QPainter painter(&colorPix);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
        painter.setBrush(QBrush(QColor(info.color)));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRect(0, 0, 12, 12), 4, 4);
        DLabel *pColorLabel = new DLabel;
        pColorLabel->setPixmap(colorPix);
        pColorLabel->setAccessibleName("colorLabel_colorWidget");
        pHlayout->addWidget(pColorLabel);

        if ((info.level < 0) || (info.level > 100)) {
            pColorLabel->hide();
        }
    }

    QString strColorName = getTranslatedColor(info.colorName);

    if (strColorName.trimmed().isEmpty()) {
        if (info.type == 4) {
            strColorName = getTranslatedColor("Waste");
        }
    }

    DLabel *pLabel = new DLabel(strColorName, this);
    pLabel->setAccessibleName("colorName_colorWidget");
    DFontSizeManager::instance()->bind(pLabel, DFontSizeManager::T6, int(QFont::ExtraLight));
    pHlayout->addWidget(pLabel);

    if (info.level <= 100) {
        QLabel *pImageLabel = new DLabel(this);
        QIcon icon = QIcon::fromTheme("dp_warning");
        QPixmap pix = icon.pixmap(QSize(14, 14));
        pImageLabel->setPixmap(pix);
        pImageLabel->setToolTip(tr("Low level"));
        pImageLabel->setAccessibleName("imageLabel_colorWidget");
        DProgressBar *pProcessBar = new DProgressBar;
        pProcessBar->setTextVisible(false);
        pProcessBar->setFixedSize(230, 8);
        pProcessBar->setRange(0, 100);
        pProcessBar->setValue(abs(info.level));
        pal.setColor(QPalette::Background, QColor(0, 0, 0, int(0.1 * 255)));
        pProcessBar->setAutoFillBackground(true);
        pProcessBar->setPalette(pal);
        pProcessBar->setAccessibleName("processBar_colorWidget");
        pHlayout->addStretch(46);
        pHlayout->addWidget(pImageLabel, Qt::AlignRight);
        pHlayout->addWidget(pProcessBar, Qt::AlignRight);

        if ((info.level >= 0) && (info.colorant != 0)) {
            if (info.level < 20) {
                pImageLabel->show();
            } else {
                pImageLabel->hide();
            }
        } else {
            pImageLabel->hide();
        }
    } else {
        QIcon icon = QIcon::fromTheme("dp_caution");
        QPixmap pix = icon.pixmap(QSize(14, 14));
        DLabel *pImageLabel = new DLabel(this);
        pImageLabel->setPixmap(pix);
        pImageLabel->setAccessibleName("imageLabel_colorWidget");
        DLabel *pTextLabel = new DLabel(tr("Unknown"), this);
        pTextLabel->setAccessibleName("textLabel_colorWidget");
        DFontSizeManager::instance()->bind(pTextLabel, DFontSizeManager::T8, int(QFont::ExtraLight));
        pHlayout->addSpacing(13);
        pHlayout->addWidget(pImageLabel, Qt::AlignRight);
        pHlayout->addWidget(pTextLabel, Qt::AlignRight);
        pHlayout->addSpacing(300);
    }

    pWidget->setLayout(pHlayout);
    pWidget->setMinimumHeight(40);
    return pWidget;
}

QString DPrinterSupplyShowDlg::getTranslatedColor(const QString &strColor)
{
    QString strRet;

    if (m_mapColorTrans.contains(strColor)) {
        strRet = m_mapColorTrans.value(strColor);
    } else {
        strRet = strColor;
    }

    return strRet;
}

void DPrinterSupplyShowDlg::initColorTrans()
{
    m_mapColorTrans.clear();
    m_mapColorTrans.insert("Black", QObject::tr("Black"));
    m_mapColorTrans.insert("Blue", QObject::tr("Blue"));
    m_mapColorTrans.insert("Brown", QObject::tr("Brown"));
    m_mapColorTrans.insert("Cyan", QObject::tr("Cyan"));
    m_mapColorTrans.insert("Dark-gray", QObject::tr("Dark-gray"));
    m_mapColorTrans.insert("Dark gray", QObject::tr("Dark gray"));
    m_mapColorTrans.insert("Dark-yellow", QObject::tr("Dark-yellow"));
    m_mapColorTrans.insert("Dark yellow", QObject::tr("Dark yellow"));
    m_mapColorTrans.insert("Gold", QObject::tr("Gold"));
    m_mapColorTrans.insert("Gray", QObject::tr("Gray"));
    m_mapColorTrans.insert("Green", QObject::tr("Green"));
    m_mapColorTrans.insert("Light-black", QObject::tr("Light-black"));
    m_mapColorTrans.insert("Light black", QObject::tr("Light black"));
    m_mapColorTrans.insert("Light-cyan", QObject::tr("Light-cyan"));
    m_mapColorTrans.insert("Light cyan", QObject::tr("Light cyan"));
    m_mapColorTrans.insert("Light-gray", QObject::tr("Light-gray"));
    m_mapColorTrans.insert("Light gray", QObject::tr("Light gray"));
    m_mapColorTrans.insert("Light-magenta", QObject::tr("Light-magenta"));
    m_mapColorTrans.insert("Light magenta", QObject::tr("Light magenta"));
    m_mapColorTrans.insert("Magenta", QObject::tr("Magenta"));
    m_mapColorTrans.insert("Orange", QObject::tr("Orange"));
    m_mapColorTrans.insert("Red", QObject::tr("Red"));
    m_mapColorTrans.insert("Silver", QObject::tr("Silver"));
    m_mapColorTrans.insert("White", QObject::tr("White"));
    m_mapColorTrans.insert("Yellow", QObject::tr("Yellow"));
    m_mapColorTrans.insert("Waste", QObject::tr("Waste"));
}

void DPrinterSupplyShowDlg::supplyFreshed(const QString &strName, bool bRet)
{
    m_pFreshSpinner->stop();
    m_pFreshSpinner->hide();
    QVBoxLayout *pVlayout = static_cast<QVBoxLayout *>(m_pContentWidget->layout());
    QLabel *pLabel = new QLabel;
    DFontSizeManager::instance()->bind(pLabel, DFontSizeManager::T6, int(QFont::Medium));
    pLabel->setText(tr("Ink/Toner Status"));
    pLabel->setAccessibleName("label_contentWidget");
    pVlayout->addWidget(pLabel, 0, Qt::AlignHCenter | Qt::AlignTop);

    if (!bRet) {
        pVlayout->addStretch(500);
        QLabel *pLabel2 = new QLabel;
        pLabel2->setText(tr("Unknown amount"));
        pLabel2->setAccessibleName("label2_contentWidget");
        DFontSizeManager::instance()->bind(pLabel2, DFontSizeManager::T4, int(QFont::Medium));
        QPalette pal = pLabel2->palette();
        QColor color = pal.color(QPalette::WindowText);
        pal.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), 160));
        pLabel2->setPalette(pal);
        QLabel *pLabel3 = new QLabel;
        pLabel3->setText(tr("Unable to get the remaining amount"));
        pVlayout->addWidget(pLabel2, 0, Qt::AlignHCenter);
        pVlayout->addSpacing(10);
        pVlayout->addWidget(pLabel3, 0, Qt::AlignHCenter);
        pVlayout->addSpacing(20);
        DFontSizeManager::instance()->bind(pLabel3, DFontSizeManager::T6, int(QFont::Light));
        pal = pLabel3->palette();
        color = pal.color(QPalette::WindowText);
        pal.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), 120));
        pLabel3->setPalette(pal);
        pLabel3->setAccessibleName("label3_contentWidget");
    } else {
        pVlayout->addSpacing(20);
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(strName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                QVector<SUPPLYSDATA> supplyInfos = m_pFreshTask->getSupplyData(strName);
                bool bColor = pPrinter->canSetColorModel();

                for (int i = 0; i < supplyInfos.size(); i++) {
                    QWidget *pColorWidget = initColorSupplyItem(supplyInfos[i], bColor);
                    pColorWidget->setAccessibleName("colorWidget_contentWidget");
                    pVlayout->addWidget(pColorWidget);
                    pVlayout->addSpacing(1);
                }

                pVlayout->addSpacing(10);
                DLabel *pTimelLabel = new DLabel;
                pTimelLabel->setWordWrap(true);
                pTimelLabel->setAlignment(Qt::AlignHCenter);
                DFontSizeManager::instance()->bind(pTimelLabel, DFontSizeManager::T8, int(QFont::ExtraLight));
                QTime time = QTime::currentTime();
                QString strTime = QString("%1:%2").arg(time.hour(), 2, 10, QLatin1Char('0')).arg(time.minute(), 2, 10, QLatin1Char('0'));
                pTimelLabel->setText(tr("The amounts are estimated, last updated at %1").arg(strTime));
                pTimelLabel->setMinimumHeight(60);
                pTimelLabel->setAccessibleName("timeLabel_contentWidget");
                pVlayout->addWidget(pTimelLabel);
                pVlayout->addSpacing(20);
            }
        }
    }

    pVlayout->addStretch(500);
    m_pConfirmBtn = new DPushButton();
    m_pConfirmBtn->setFixedWidth(230);
    DFontSizeManager::instance()->bind(m_pConfirmBtn, DFontSizeManager::T6, int(QFont::ExtraLight));
    m_pConfirmBtn->setText(tr("OK", "button"));
    m_pConfirmBtn->setAccessibleName("confirmBtn_contentWidget");
    pVlayout->addWidget(m_pConfirmBtn, 0, Qt::AlignCenter | Qt::AlignBottom);
    m_pContentWidget->setLayout(pVlayout);
    m_pContentWidget->setAccessibleName("contentWidget_printerSupply");
    connect(m_pConfirmBtn, &DPushButton::clicked, this, &QDialog::accept);
}
