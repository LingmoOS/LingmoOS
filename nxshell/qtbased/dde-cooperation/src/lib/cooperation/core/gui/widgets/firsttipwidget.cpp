#include "firsttipwidget.h"

#include <QFile>
#include <QToolButton>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QVBoxLayout>

#include "common/commonutils.h"
#include "gui/utils/cooperationguihelper.h"

#ifdef linux
#    include <DPalette>
#endif
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
DWIDGET_USE_NAMESPACE
#endif
using namespace cooperation_core;

FirstTipWidget::FirstTipWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void FirstTipWidget::themeTypeChanged()
{
    if (CooperationGuiHelper::instance()->isDarkTheme()) {
        shadowEffect->setColor(QColor(122, 192, 255, 128));
        bannerLabel->setPixmap(QIcon::fromTheme(":/icons/deepin/builtin/dark/icons/banner_128px.png").pixmap(234, 158));

    } else {
        shadowEffect->setColor(QColor(10, 57, 99, 128));
        bannerLabel->setPixmap(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/banner_128px.png").pixmap(234, 158));
    }
    if (!qApp->property("onlyTransfer").toBool())
        action->setPixmap(QIcon::fromTheme("connect").pixmap(12, 12));
#ifndef __linux__
    action->setPixmap(QIcon::fromTheme(":/icons/deepin/builtin/texts/connect_18px.svg").pixmap(12, 12));
#endif
}

void FirstTipWidget::showEvent(QShowEvent *event)
{
    auto ge = lineBalls.last()->geometry();

    line->setGeometry(32, 26, qApp->devicePixelRatio(), ge.y() - 26);
    QWidget::showEvent(event);
}

void FirstTipWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void FirstTipWidget::initUI()
{
    inittipBtn();
    initbackgroundFrame();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 10, 0, 0);
    mainLayout->addWidget(backgroundFrame);

    line = new LineWidget(backgroundFrame);

    connect(CooperationGuiHelper::instance(), &CooperationGuiHelper::themeTypeChanged, this, &FirstTipWidget::themeTypeChanged);
    themeTypeChanged();
}

void FirstTipWidget::initbackgroundFrame()
{
    backgroundFrame = new QFrame(this);
    QString backlightStyle = ".QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(249, 250, 254), stop:1 rgba(232, 242, 255)); "
                             "border-radius: 10px;"
                             "color: rgba(0, 0, 0, 0.6);"
                             "border: 1px solid rgba(0, 0, 0, 0.05); } ";
    QString backdarkStyle = ".QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(132, 141, 179, 0.24), stop:1 rgba(224, 225, 255, 0.12)); "
                            "border-radius: 10px;"
                            "color: rgba(0, 0, 0, 0.6);"
                            "border: 1px solid rgba(0, 0, 0, 0.05); } ";
    CooperationGuiHelper::initThemeTypeConnect(backgroundFrame, backlightStyle, backdarkStyle);
    backgroundFrame->setFixedWidth(480);

    bannerLabel = new QLabel(this);
    bannerLabel->setPixmap(QIcon::fromTheme(":/icons/deepin/builtin/light/icons/banner_128px.png").pixmap(234, 158));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(26, 7, 0, 7);

    QStringList tips;
    tips << tr("First step")
         << tr("The opposite end opens the application and connects to the same network")
         << tr("Second step")
         << tr("Enter the peer IP in the search box")
         << tr("Third step")
         << tr("Click");
    //最后一句话有图标单独处理

    for (int i = 0; i < tips.size(); i++) {
        QLabel *textLabel = new ElidedLabel(tips[i], 190, this);

        shadowEffect = new QGraphicsDropShadowEffect(this);
        shadowEffect->setBlurRadius(4);
        shadowEffect->setColor(QColor(122, 192, 255, 128));
        shadowEffect->setOffset(0, 2);
        if (i % 2 == 0) {
            QString lightStyle = "color: rgba(0, 0, 0, 0.6);";
            QString darkStyle = "color: rgba(255, 255, 255, 0.6);";
            CooperationGuiHelper::initThemeTypeConnect(textLabel, lightStyle, darkStyle);
            CooperationGuiHelper::setAutoFont(textLabel, 11, QFont::Normal);

            QLabel *lineball = new QLabel(this);
            lineBalls.append(lineball);
            lineball->setFixedSize(12, 12);
            QString balllightStyle = "background-color: rgb(33, 138, 244); "
                                     "border-radius: 6px;"
                                     "border: 1px solid white;";
            QString balldarkStyle = "background-color: rgb(0, 89, 210); "
                                    "border-radius: 6px;"
                                    "border: 1px solid white; ";
            CooperationGuiHelper::initThemeTypeConnect(lineball, balllightStyle, balldarkStyle);
            lineball->setGraphicsEffect(shadowEffect);
            QHBoxLayout *lineLayout = new QHBoxLayout;
            lineLayout->addWidget(lineball);
            lineLayout->addSpacing(13);
            lineLayout->addWidget(textLabel);
            vLayout->addLayout(lineLayout);
            vLayout->addSpacing(4);
        } else {
            QString lightStyle = "color: rgba(0, 0, 0, 0.7);";
            QString darkStyle = "color: rgba(255, 255, 255, 0.7);";
            CooperationGuiHelper::setAutoFont(textLabel, 12, QFont::Medium);
            CooperationGuiHelper::initThemeTypeConnect(textLabel, lightStyle, darkStyle);

            QHBoxLayout *lineLayout = new QHBoxLayout;
            lineLayout->addSpacing(25);
            lineLayout->addWidget(textLabel);
            vLayout->addLayout(lineLayout);

            //最后一句话有图标单独处理
            if (i + 1 == tips.size()) {
                action = new QLabel(this);
                action->setAlignment(Qt::AlignCenter);
                action->setFixedSize(20, 20);

                QString remainTip;
                if (qApp->property("onlyTransfer").toBool()) {
                    action->setPixmap(QIcon::fromTheme(":/icons/deepin/builtin/texts/send_18px.svg").pixmap(12, 12));
                    QString lightStyle = "background-color: rgb(0, 129, 255); "
                                         "border-radius: 10px;";
                    QString darkStyle = "background-color: rgb(0, 129, 255); "
                                        "border-radius: 10px;";
                    CooperationGuiHelper::initThemeTypeConnect(action, lightStyle, darkStyle);
                    remainTip = tr("to send the file");
                } else {
                    QString lightStyle = "background-color: white; "
                                         "border-radius: 10px;";
                    QString darkStyle = "background-color: rgba(255, 255, 255, 0.2); "
                                        "border-radius: 10px;";
                    CooperationGuiHelper::initThemeTypeConnect(action, lightStyle, darkStyle);
                    remainTip = tr("to connect to the peer device");
                }
                QLabel *textLabel2 = new ElidedLabel(remainTip, 100, this);
                CooperationGuiHelper::setAutoFont(textLabel2, 12, QFont::Medium);
                CooperationGuiHelper::initThemeTypeConnect(textLabel2, lightStyle, darkStyle);

                QVBoxLayout *vlineLayout = new QVBoxLayout;
                vlineLayout->setAlignment(Qt::AlignCenter);
                vlineLayout->addWidget(textLabel2);

                lineLayout->addSpacing(6);
                lineLayout->setAlignment(Qt::AlignLeft);
                lineLayout->addWidget(action);
                lineLayout->addSpacing(6);
                lineLayout->addWidget(textLabel2, Qt::AlignBottom);
            } else
                vLayout->addSpacing(8);
        }
    }

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 6, 6, 6);
    hLayout->setAlignment(Qt::AlignRight);
    hLayout->addLayout(vLayout, Qt::AlignLeft);
    hLayout->addWidget(bannerLabel);

    backgroundFrame->setLayout(hLayout);
}

void FirstTipWidget::inittipBtn()
{
    tipBtn = new QToolButton(dynamic_cast<QWidget *>(parent()));
    tipBtn->setIcon(QIcon::fromTheme(":/icons/deepin/builtin/icons/close_white.svg"));
    tipBtn->setIconSize(QSize(8, 8));
    connect(tipBtn, &QToolButton::clicked, this, [this]() {
        QFile flag(deepin_cross::CommonUitls::tipConfPath());
        if (flag.open(QIODevice::WriteOnly))
            flag.close();
        setVisible(false);
    });
    tipBtn->setStyleSheet("QToolButton { background-color: rgba(0, 0, 0, 0.1); border-radius: 9px; }"
                          "QToolButton::hover { background-color: rgba(0, 0, 0, 0.2); border-radius: 9px; }");
}

void FirstTipWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    tipBtn->setVisible(visible);
#ifdef linux
    tipBtn->setGeometry(467, 68, 18, 18);
#    ifdef DTKWIDGET_CLASS_DSizeMode
    tipBtn->setGeometry(467, DSizeModeHelper::element(53, 68), 18, 18);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this] {
        tipBtn->setGeometry(467, DSizeModeHelper::element(53, 68), 18, 18);
    });
#    endif
#else
    tipBtn->setGeometry(467, 115, 18, 18);
#endif
}

void LineWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QColor color;
    if (CooperationGuiHelper::instance()->isDarkTheme())
        color.setRgb(189, 222, 255);
    else
        color.setRgb(33, 138, 244);
    color.setAlphaF(0.17);

    QPen pen;
    pen.setWidth(2);
    pen.setColor(color);
    painter.setPen(pen);

    int x = width() / 2;
    int y = 0;
    int dashLength = 4;   // 每段虚线的长度
    while (y < height()) {
        painter.drawLine(x, y, x, qMin(y + dashLength, height()));
        y += 2 * dashLength;   // 虚线间距
    }
}

ElidedLabel::ElidedLabel(const QString &text, int maxwidth, QWidget *parent)
    : QLabel(parent), maxWidth(maxwidth)
{
    setText(text);
    setToolTip(text);
}

void ElidedLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QFontMetrics metrics(font());

    int textWidth = metrics.horizontalAdvance(text());
    QString elidedtext = text();
    if (textWidth > maxWidth)
        elidedtext = metrics.elidedText(text(), Qt::ElideRight, maxWidth);

    painter.drawText(rect(), Qt::AlignLeft, elidedtext);
}
