// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationstatewidget.h"
#include "backgroundwidget.h"
#include "global_defines.h"
#include "gui/utils/cooperationguihelper.h"

#ifdef linux
#    include <DPalette>
#endif
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
DWIDGET_USE_NAMESPACE
#endif

#include <QVariant>
#include <QVBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QTimer>
#include <QDesktopServices>
#include <QToolButton>
#include <QScrollArea>
#include <QMouseEvent>

#include <common/commonutils.h>

using namespace cooperation_core;

#ifdef __linux__
const char *Kfind_device = "find_device";
const char *Kno_network = "no_network";
const char *Knot_find_device = "not_find_device";
#else
const char *Kfind_device = ":/icons/deepin/builtin/light/icons/find_device_250px.svg";
const char *Kno_network = ":/icons/deepin/builtin/icons/dark@2x.png";
const char *Knot_find_device = ":/icons/deepin/builtin/light/icons/not_find_device_150px.svg";
#endif

LookingForDeviceWidget::LookingForDeviceWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();

    animationTimer = new QTimer(this);
    animationTimer->setInterval(16);
    connect(animationTimer, &QTimer::timeout, this, [this] { update(); });
}

void LookingForDeviceWidget::seAnimationtEnabled(bool enabled)
{
    if (isEnabled == enabled)
        return;

    angle = 0;
    (isEnabled = enabled) ? animationTimer->start() : animationTimer->stop();
}

void LookingForDeviceWidget::initUI()
{
    setFocusPolicy(Qt::ClickFocus);

    iconLabel = new CooperationLabel(this);
    iconLabel->setFixedSize(250, 250);
    QIcon icon = QIcon::fromTheme(Kfind_device);
    iconLabel->setPixmap(icon.pixmap(250, 250));
    connect(CooperationGuiHelper::instance(), &CooperationGuiHelper::themeTypeChanged, this, [icon, this] {
        iconLabel->setPixmap(icon.pixmap(250, 250));
    });

    CooperationLabel *tipsLabel = new CooperationLabel(tr("Looking for devices"), this);
    tipsLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addSpacing(38);
    vLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    vLayout->addWidget(tipsLabel, 0, Qt::AlignVCenter);
    vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    setLayout(vLayout);
}

void LookingForDeviceWidget::paintEvent(QPaintEvent *event)
{
    // 绘制动画效果
    if (isEnabled) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 获取中心点坐标
        int centerX = iconLabel->geometry().center().x() + 1;
        int centerY = iconLabel->geometry().center().y();
        QConicalGradient gradient(centerX, centerY, angle + 180);
        if (CooperationGuiHelper::isDarkTheme()) {
            gradient.setColorAt(0.3, QColor(63, 63, 63));
            gradient.setColorAt(0.7, QColor(63, 63, 63, 0));
        } else {
            gradient.setColorAt(0.3, QColor(208, 228, 245));
            gradient.setColorAt(0.7, QColor(208, 228, 245, 0));
        }

        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);

        // 计算绘制矩形区域，半径111
        int newLeft = centerX - 111;
        int newTop = centerY - 111;
        QRect drawRect(newLeft, newTop, 222, 222);

        painter.drawPie(drawRect, angle * 16, 90 * 16);
        angle -= 2;
    }

    QWidget::paintEvent(event);
}

NoNetworkWidget::NoNetworkWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void NoNetworkWidget::initUI()
{
    setFocusPolicy(Qt::ClickFocus);

    CooperationLabel *iconLabel = new CooperationLabel(this);
    iconLabel->setFixedSize(150, 150);
    QIcon icon = QIcon::fromTheme(Kno_network);
    iconLabel->setPixmap(icon.pixmap(150, 150));
    connect(CooperationGuiHelper::instance(), &CooperationGuiHelper::themeTypeChanged, this, [icon, iconLabel] {
        iconLabel->setPixmap(icon.pixmap(150, 150));
    });

    CooperationLabel *tipsLabel = new CooperationLabel(tr("Please connect to the network"), this);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addSpacing(116);
    vLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    vLayout->addSpacing(14);
    vLayout->addWidget(tipsLabel, 0, Qt::AlignCenter);
    vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    setLayout(vLayout);
}

NoResultTipWidget::NoResultTipWidget(QWidget *parent, bool usetipMode)
    : QWidget(parent), useTipMode(usetipMode)
{
    initUI();
}

void NoResultTipWidget::onLinkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void NoResultTipWidget::setTitleVisible(bool visible)
{
    titleLabel->setVisible(visible);
}

void NoResultTipWidget::initUI()
{
    CooperationGuiHelper::setAutoFont(this, 12, QFont::Normal);
    QString leadintText =
            tr("1. Enable cross-end collaborative applications. Applications on the UOS "
               "can be downloaded from the App Store, and applications on the Windows "
               "side can be downloaded from: ");
    QString hyperlink = "https://www.chinauos.com/resource/assistant";

    QString websiteLinkTemplate =
            "<br/><a href='%1' style='text-decoration: none; color: #0081FF;word-wrap: break-word;'>%2</a>";
    QString content1 = leadintText + websiteLinkTemplate.arg(hyperlink, hyperlink);
    CooperationLabel *contentLable1 = new CooperationLabel(this);
    contentLable1->setWordWrap(true);
    contentLable1->setText(content1);
    connect(contentLable1, &QLabel::linkActivated, this, &NoResultTipWidget::onLinkActivated);

    CooperationLabel *contentLable2 = new CooperationLabel(tr("2. On the same LAN as the device"), this);
    contentLable2->setWordWrap(true);

    QString settingTip;
    if (qApp->property("onlyTransfer").toBool()) {
        settingTip = tr("3. File Manager-Settings-File Drop-Allow the following users to drop files to me -\"Everyone on the same LAN\"");
    } else {
        settingTip = tr("3. Settings-Basic Settings-Discovery Mode-\"Allow everyone in the same LAN\"");
    }

    CooperationLabel *contentLable3 = new CooperationLabel(settingTip, this);
    contentLable3->setWordWrap(true);
    CooperationLabel *contentLable4 = new CooperationLabel(
            tr("4. Try entering the target device IP in the top search box"),
            this);
    contentLable4->setWordWrap(true);

    titleLabel = new CooperationLabel(tr("Unable to find collaborative device？"));
    titleLabel->setAlignment(Qt::AlignLeft);
    CooperationGuiHelper::setAutoFont(titleLabel, 14, QFont::Medium);
    titleLabel->setWordWrap(true);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(5);
    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(contentLable1);
    contentLayout->addWidget(contentLable2);
    contentLayout->addWidget(contentLable3);
    contentLayout->addWidget(contentLable4);
    contentLayout->setContentsMargins(5, 3, 5, 5);
    setLayout(contentLayout);

    if (useTipMode) {
        titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        contentLable1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        contentLable2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        contentLable3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        contentLable4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

#ifdef linux
    contentLable1->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
    contentLable2->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
    contentLable3->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
    contentLable4->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTips);
    titleLabel->setForegroundRole(DTK_GUI_NAMESPACE::DPalette::TextTitle);
#else
    QList<QColor> colorList { QColor(0, 0, 0, qRound(255 * 0.6)),
                              QColor(192, 192, 192) };
    CooperationGuiHelper::instance()->autoUpdateTextColor(contentLable1, colorList);
    CooperationGuiHelper::instance()->autoUpdateTextColor(contentLable2, colorList);
    CooperationGuiHelper::instance()->autoUpdateTextColor(contentLable3, colorList);
    CooperationGuiHelper::instance()->autoUpdateTextColor(contentLable4, colorList);
    CooperationGuiHelper::instance()->autoUpdateTextColor(titleLabel, colorList);
    contentLayout->setSpacing(15);
#endif
}

NoResultWidget::NoResultWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void NoResultWidget::initUI()
{
    setFocusPolicy(Qt::ClickFocus);

    CooperationLabel *iconLabel = new CooperationLabel(this);
    iconLabel->setFixedSize(150, 150);
    QIcon icon = QIcon::fromTheme(Knot_find_device);
    iconLabel->setPixmap(icon.pixmap(150, 150));
    connect(CooperationGuiHelper::instance(), &CooperationGuiHelper::themeTypeChanged, this, [icon, iconLabel] {
        iconLabel->setPixmap(icon.pixmap(150, 150));
    });

    CooperationLabel *tipsLabel = new CooperationLabel(tr("No device found"), this);
    auto font = tipsLabel->font();
    font.setWeight(QFont::Medium);
    tipsLabel->setFont(font);

    BackgroundWidget *contentBackgroundWidget = new BackgroundWidget(this);
    contentBackgroundWidget->setBackground(17, BackgroundWidget::ItemBackground,
                                           BackgroundWidget::TopAndBottom);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    NoResultTipWidget *noResultTipWidget = new NoResultTipWidget();
    noResultTipWidget->setTitleVisible(false);
    contentLayout->addWidget(noResultTipWidget);
    contentBackgroundWidget->setLayout(contentLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);

    QSpacerItem *sp_1 = new QSpacerItem(20, 88, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSpacerItem *sp_2 = new QSpacerItem(20, 14, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSpacerItem *sp_3 = new QSpacerItem(20, 22, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vLayout->addItem(sp_1);
    vLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    vLayout->addItem(sp_2);
    vLayout->addWidget(tipsLabel, 0, Qt::AlignCenter);
    vLayout->addItem(sp_3);
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentBackgroundWidget);
    scrollArea->show();
    scrollArea->setFrameStyle(QFrame::NoFrame);
    vLayout->addWidget(scrollArea);

    vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    setLayout(vLayout);
}

BottomLabel::BottomLabel(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    setFixedSize(500, 33);
    dialog->installEventFilter(this);
}

void BottomLabel::setIp(const QString &ip)
{
    QString iptext = QString(tr("Local IP: %1").arg(ip));
    ipLabel->setText(iptext);
}

void BottomLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor color(0, 0, 0, 12);
    painter.setPen(QPen(color, 3));
    painter.drawLine(0, 0, width(), 0);
}

bool BottomLabel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == tipLabel) {
        if (event->type() == QEvent::Enter)
            showDialog();
        else if (event->type() == QEvent::Leave)
            timer->start();
    } else if (obj == dialog) {
        if (event->type() == QEvent::Enter) {
            showDialog();
        } else if (event->type() == QEvent::Leave) {
            timer->start();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void BottomLabel::initUI()
{
    QString ip = QString(tr("Local IP: %1").arg("---"));
    ipLabel = new QLabel(ip);
    ipLabel->setAlignment(Qt::AlignHCenter);
    ipLabel->setFixedHeight(30);
    CooperationGuiHelper::setAutoFont(ipLabel, 12, QFont::Normal);

    dialog = new CooperationAbstractDialog(this);
    QScrollArea *scrollArea = new QScrollArea(dialog);
    tipLabel = new QLabel(qobject_cast<QWidget *>(this->parent()));
    tipLabel->installEventFilter(this);

#ifdef linux
    updateSizeMode();
    connect(CooperationGuiHelper::instance(), &CooperationGuiHelper::themeTypeChanged, this, &BottomLabel::updateSizeMode);
#    ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &BottomLabel::updateSizeMode);
#    endif
#else
    tipLabel->setGeometry(455, 600, 24, 24);
    tipLabel->setPixmap(QIcon(":/icons/deepin/builtin/light/icons/icon_tips_128px.svg").pixmap(24, 24));
    dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog->setStyleSheet("background-color: white;"
                          "border-radius: 10px;}"
                          "QScrollBar:vertical {"
                          "width: 0px;"
                          "}");
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
#endif

    dialog->setFixedSize(260, 208);
    scrollArea->setWidgetResizable(true);
    QWidget *contentWidget = new QWidget;

    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(5, 6, 5, 0);
    NoResultTipWidget *tipWidgt = new NoResultTipWidget(scrollArea, true);
    layout->addWidget(tipWidgt);
    scrollArea->setWidget(contentWidget);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(scrollArea);
    contentLayout->setAlignment(Qt::AlignCenter);

    dialog->setLayout(contentLayout);
    dialog->setWindowFlags(Qt::ToolTip);

    CooperationGuiHelper::setAutoFont(tipWidgt, 14, QFont::Normal);
    CooperationGuiHelper::setAutoFont(tipWidgt, 12, QFont::Normal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ipLabel);
    mainLayout->setAlignment(Qt::AlignHCenter);
    setLayout(mainLayout);

    timer = new QTimer(this);
    timer->setInterval(200);
    connect(timer, &QTimer::timeout, dialog, &QDialog::hide);
}

void BottomLabel::showDialog() const
{
    timer->stop();
    if (dialog->isVisible())
        return;
    QMainWindow *activeMainWindow = qobject_cast<QMainWindow *>(qApp->topLevelAt(QCursor::pos()));
    dialog->move(activeMainWindow->pos() + QPoint(228, 398));
    dialog->show();
}

void BottomLabel::updateSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    tipLabel->setGeometry(460, DSizeModeHelper::element(562, 552), 24, 24);
    int size = DSizeModeHelper::element(18, 24);
    ipLabel->setFixedHeight(DSizeModeHelper::element(15, 30));
    tipLabel->setPixmap(QIcon::fromTheme("icon_tips").pixmap(size, size));
#else
    tipLabel->setGeometry(480, 552, 24, 24);
    ipLabel->setFixedHeight(30);
    tipLabel->setPixmap(QIcon(":/icons/deepin/builtin/light/icons/icon_tips.svg").pixmap(24, 24));
#endif
}
