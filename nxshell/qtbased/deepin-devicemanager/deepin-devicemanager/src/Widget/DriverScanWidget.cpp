// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverScanWidget.h"
#include "AnimationLabel.h"
#include "commontools.h"

#include <DFontSizeManager>
#include <DApplication>
#include <DApplicationHelper>

#include <QPropertyAnimation>
#include <QProcess>
#include <QScrollArea>
#include <QScrollBar>
#include <QPainterPath>
#define FONT_WEIGHT 63
#define ICON_LABEL_SIZE 128
#define PROGRESS_HEIGHT 8
#define PROGRESS_WIDTH 422
#define INFO_WIDTH 500
#define BUTTON_WIDTH   310
#define SPACE_15       15

static QPropertyAnimation *animation = nullptr;

DriverScanWidget::DriverScanWidget(DWidget *parent)
    : DFrame(parent)
    , mp_ScanningPicLabel(new AnimationLabel(this))
    , mp_ErrPicLabel(new AnimationLabel(this))
    , mp_ScanningLabel(new DLabel(this))
    , mp_CheckNetworkLabel(new DLabel(this))
    , mp_FeedBackLabel(new DLabel(this))
    , mp_ScanningInfoLabel(new DLabel(this))
    , mp_ScanningProgress(new DProgressBar(this))
    , mp_ReScanButton(new DSuggestButton(this))
    , mp_ScrollWidget(new QWidget(this))
    , mp_ScroBar(nullptr)
    , mp_HLayoutPic(nullptr)
    , mp_HLayoutscLabel(nullptr)
    , mp_HLayoutscProgress(nullptr)
    , mp_HLayoutInfoLabel(nullptr)
    , mp_HLayout(nullptr)
    , mp_VLayout(nullptr)
{
    initUI();
    initConnect();
}

void DriverScanWidget::setScanningUI(const QString &scanInfo, int progressValue)
{
    hideAll();
    // Animation Label
    if (nullptr == animation) {
        animation = new QPropertyAnimation(mp_ScanningPicLabel, "icon");
        animation->setLoopCount(-1);
        animation->setDuration(800);
        animation->setStartValue(1);
        animation->setEndValue(8);
    }
    animation->start();
    mp_ScanningPicLabel->show();

    // 正在扫描Label
    mp_ScanningLabel->setText(QObject::tr("Scanning hardware device drivers, please wait..."));
    mp_ScanningLabel->show();

    // progressBar
    mp_ScanningProgress->setValue(progressValue);
    mp_ScanningProgress->show();

    // 扫描信息Label
    mp_ScanningInfoLabel->setText(QObject::tr("Scanning %1").arg(scanInfo));
    mp_ScanningInfoLabel->show();

    // 水平居中显示
    mp_HLayoutPic->addStretch();
    mp_HLayoutPic->addWidget(mp_ScanningPicLabel);
    mp_HLayoutPic->addStretch();
    mp_HLayoutPic->setContentsMargins(0, 0, 0, 0);
    mp_ScanningPicLabel->setMinimumHeight(mp_ScanningPicLabel->height());

    mp_HLayoutscLabel->addStretch();
    mp_HLayoutscLabel->addWidget(mp_ScanningLabel);
    mp_HLayoutscLabel->addStretch();
    mp_HLayoutscLabel->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutscProgress->addStretch();
    mp_HLayoutscProgress->addWidget(mp_ScanningProgress);
    mp_HLayoutscProgress->addStretch();
    mp_HLayoutscProgress->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutInfoLabel->addStretch();
    mp_HLayoutInfoLabel->addWidget(mp_ScanningInfoLabel);
    mp_HLayoutInfoLabel->addStretch();
    mp_HLayoutInfoLabel->setContentsMargins(0, 0, 0, 0);

    // 整体垂直居中显示
    mp_VLayout->addStretch();
    mp_VLayout->addLayout(mp_HLayoutPic);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutscLabel);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutscProgress);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutInfoLabel);
    mp_VLayout->addSpacing(25);
    mp_VLayout->addStretch();

    mp_VLayout->setContentsMargins(0, 0, 0, 0);

    // 整体水平居中显示
    mp_HLayout->addStretch();
    mp_HLayout->addLayout(mp_VLayout);
    mp_HLayout->addStretch();
    mp_ScrollWidget->setLayout(mp_HLayout);
}

void DriverScanWidget::setScanFailedUI()
{
    hideAll();
    // Scan Failed Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/fail.svg"));
    QPixmap pix = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_ErrPicLabel->setPixmap(pix);
    mp_ErrPicLabel->show();

    // 正在扫描Label
    mp_ScanningLabel->setText(QObject::tr("Scan failed"));
    mp_ScanningLabel->show();

    mp_FeedBackLabel->show();
    mp_ReScanButton->show();

    // 控件水平居中显示
    mp_HLayoutPic->addStretch();
    mp_HLayoutPic->addWidget(mp_ErrPicLabel);
    mp_HLayoutPic->addStretch();
    mp_HLayoutPic->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutscLabel->addStretch();
    mp_HLayoutscLabel->addWidget(mp_ScanningLabel);
    mp_HLayoutscLabel->addStretch();
    mp_HLayoutscLabel->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutscProgress->addStretch();
    mp_HLayoutscProgress->addWidget(mp_FeedBackLabel);
    mp_HLayoutscProgress->addStretch();
    mp_HLayoutscProgress->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutInfoLabel->addStretch();
    mp_HLayoutInfoLabel->addWidget(mp_ReScanButton);
    mp_HLayoutInfoLabel->addStretch();
    mp_HLayoutInfoLabel->setContentsMargins(0, 0, 0, 0);

    // 整体垂直居中显示
    mp_VLayout->setSpacing(0);
    mp_VLayout->addStretch();
    mp_VLayout->addLayout(mp_HLayoutPic);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutscLabel);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutscProgress);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutInfoLabel);
    mp_VLayout->addSpacing(20);
    mp_VLayout->addStretch();
    mp_VLayout->setContentsMargins(0, 0, 0, 0);

    // 整体水平居中显示
    mp_HLayout->addStretch();
    mp_HLayout->addLayout(mp_VLayout);
    mp_HLayout->addStretch();
    mp_ScrollWidget->setLayout(mp_HLayout);
}

void DriverScanWidget::setNetworkErr()
{
    hideAll();
    // Scan Failed Label
    QIcon icon(QIcon::fromTheme(":/icons/deepin/builtin/icons/erro-128.svg"));
    QPixmap pix = icon.pixmap(ICON_LABEL_SIZE, ICON_LABEL_SIZE);
    mp_ErrPicLabel->setPixmap(pix);
    mp_ErrPicLabel->show();

    // 正在扫描Label
    mp_ScanningLabel->setText(QObject::tr("Network unavailable"));
    mp_ScanningLabel->show();

    // 请检查网络连接
    mp_CheckNetworkLabel->setText(QObject::tr("Please check your network connection"));
    mp_CheckNetworkLabel->show();
    mp_ReScanButton->show();

    // 控件水平居中显示
    mp_HLayoutPic->addStretch();
    mp_HLayoutPic->addWidget(mp_ErrPicLabel);
    mp_HLayoutPic->addStretch();
    mp_HLayoutPic->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutscLabel->addStretch();
    mp_HLayoutscLabel->addWidget(mp_ScanningLabel);
    mp_HLayoutscLabel->addStretch();
    mp_HLayoutscLabel->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutscProgress->addStretch();
    mp_HLayoutscProgress->addWidget(mp_CheckNetworkLabel);
    mp_HLayoutscProgress->addStretch();
    mp_HLayoutscProgress->setContentsMargins(0, 0, 0, 0);

    mp_HLayoutInfoLabel->addStretch();
    mp_HLayoutInfoLabel->addWidget(mp_ReScanButton);
    mp_HLayoutInfoLabel->addStretch();
    mp_HLayoutInfoLabel->setContentsMargins(0, 0, 0, 0);

    // 整体垂直居中显示
    mp_VLayout->setSpacing(0);
    mp_VLayout->addStretch();
    mp_VLayout->addLayout(mp_HLayoutPic);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutscLabel);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutscProgress);
    mp_VLayout->addSpacing(SPACE_15);
    mp_VLayout->addLayout(mp_HLayoutInfoLabel);
    mp_VLayout->addSpacing(20);
    mp_VLayout->addStretch();
    mp_VLayout->setContentsMargins(0, 0, 0, 0);

    // 整体水平居中显示
    mp_HLayout->addStretch();
    mp_HLayout->addLayout(mp_VLayout);
    mp_HLayout->addStretch();
    mp_ScrollWidget->setLayout(mp_HLayout);
}

void DriverScanWidget::slotFeedBack()
{
    CommonTools::feedback();
}

void DriverScanWidget::slotReDetected()
{
    setScanningUI("", 0);
    emit redetected();
}

void DriverScanWidget::initUI()
{
    this->setLineWidth(0);
    // 设置动画关键帧
    mp_ScanningPicLabel->setImageList(QStringList() << ":/icons/deepin/builtin/icons/animation/0.png"
                                      << ":/icons/deepin/builtin/icons/animation/1.png"
                                      << ":/icons/deepin/builtin/icons/animation/2.png"
                                      << ":/icons/deepin/builtin/icons/animation/3.png"
                                      << ":/icons/deepin/builtin/icons/animation/4.png"
                                      << ":/icons/deepin/builtin/icons/animation/5.png"
                                      << ":/icons/deepin/builtin/icons/animation/6.png"
                                      << ":/icons/deepin/builtin/icons/animation/7.png");

    // 正在扫描Label
    QFont font = mp_ScanningLabel->font();
    font.setWeight(FONT_WEIGHT);
    mp_ScanningLabel->setFont(font);
    DFontSizeManager::instance()->bind(mp_ScanningLabel, DFontSizeManager::T5);

    // bug134487
    DFontSizeManager::instance()->bind(mp_ScanningInfoLabel, DFontSizeManager::T8);
    DPalette pa = DApplicationHelper::instance()->palette(mp_ScanningInfoLabel);
    pa.setColor(DPalette::Text, pa.color(DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(mp_ScanningInfoLabel, pa);
    mp_ScanningInfoLabel->setElideMode(Qt::ElideRight);

    // 切换主题
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
        plt.setColor(DPalette::Text, plt.color(DPalette::TextTips));
        mp_ScanningInfoLabel->setPalette(plt);
    });

    // 反馈Label
    QString feedbackStr = QString("<a style=\"text-decoration:none\" href=\"submit feedback\">") + QObject::tr("submit feedback") + "</a>";
    mp_FeedBackLabel->setText(QObject::tr("Please scan again or %1 to us").arg(feedbackStr));

    // 重新扫描Button
    mp_ReScanButton->setText(QObject::tr("Scan Again"));
    mp_ReScanButton->setFixedWidth(BUTTON_WIDTH);

    // progressbar
    mp_ScanningProgress->setFixedHeight(PROGRESS_HEIGHT);
    mp_ScanningProgress->setMinimumWidth(PROGRESS_WIDTH);
    mp_ScanningInfoLabel->setFixedWidth(INFO_WIDTH);
    mp_ScanningInfoLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QScrollArea *area = new QScrollArea(this);
    mp_ScrollWidget->setMinimumHeight(290);

    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setWidget(mp_ScrollWidget);
    mp_ScroBar = area->verticalScrollBar();

    mainLayout->addWidget(area);
    this->setLayout(mainLayout);
}

void DriverScanWidget::initConnect()
{
    // 反馈
    connect(mp_FeedBackLabel, &QLabel::linkActivated, this, &DriverScanWidget::slotFeedBack);

    // 重新检测
    connect(mp_ReScanButton, &DSuggestButton::clicked, this, &DriverScanWidget::slotReDetected);

}

void DriverScanWidget::hideAll()
{
    // 隐藏控件
    mp_ScanningPicLabel->hide();
    mp_ErrPicLabel->hide();
    mp_ScanningLabel->hide();
    mp_CheckNetworkLabel->hide();
    mp_FeedBackLabel->hide();
    mp_ScanningInfoLabel->hide();
    mp_ScanningProgress->hide();
    mp_ReScanButton->hide();

    // 清除布局
    delete mp_ScrollWidget->layout();
    mp_HLayoutPic = new QHBoxLayout();
    mp_HLayoutscLabel = new QHBoxLayout();
    mp_HLayoutscProgress = new QHBoxLayout();
    mp_HLayoutInfoLabel = new QHBoxLayout();
    mp_HLayout = new QHBoxLayout();
    mp_VLayout = new QVBoxLayout();

    if (nullptr != animation)
        animation->stop();
}

void DriverScanWidget::setProgressFinish()
{
    if (nullptr != animation)
        animation->stop();
    mp_ScanningProgress->setValue(100);
}

void DriverScanWidget::refreshProgress(QString info, int progress)
{
    mp_ScanningInfoLabel->setText(QObject::tr("Scanning %1").arg(info));
    mp_ScanningInfoLabel->show();

    int old = mp_ScanningProgress->value();
    mp_ScanningProgress->setValue(old + progress);
}

void DriverScanWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();
    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/)
        cg = DPalette::Active;
    else
        cg = DPalette::Inactive;

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillPath(path, bgBrush);

    painter.restore();

    DFrame::paintEvent(event);
}

void DriverScanWidget::resizeEvent(QResizeEvent *event)
{
    mp_ScroBar->setValue(mp_ScroBar->maximum());
    mp_ScroBar->setVisible(false);
    return DFrame::resizeEvent(event);
}
