// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenmirroringwindow.h"
#include "global_defines.h"
#include "vncviewer.h"

#include <QPainter>
#include <QPixmap>
#include <QVBoxLayout>
#include <DTitlebar>

#include <QVBoxLayout>
#include <QStackedLayout>

#include <gui/utils/cooperationguihelper.h>
using namespace cooperation_core;

inline constexpr char KIcon[] { ":/icons/lingmo/builtin/icons/uos_assistant.png" };

ScreenMirroringWindow::ScreenMirroringWindow(const QString &device, QWidget *parent)
    : CooperationMainWindow(parent)
{
    initTitleBar(device);
    initWorkWidget();
    initBottom();

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->addLayout(stackedLayout);
    mainLayout->addWidget(bottomWidget, Qt::AlignBottom);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    connect(this, &ScreenMirroringWindow::buttonClicked, m_vncViewer, &VncViewer::onShortcutAction);
    connect(m_vncViewer, &VncViewer::sizeChanged, this, &ScreenMirroringWindow::handleSizeChange);
}

ScreenMirroringWindow::~ScreenMirroringWindow()
{
    if (m_vncViewer) {
        m_vncViewer->stop();
    }
}

void ScreenMirroringWindow::initWorkWidget()
{
    stackedLayout = new QStackedLayout;

    m_vncViewer = new VncViewer(this);
    stackedLayout->addWidget(m_vncViewer);

    LockScreenWidget *lockWidget = new LockScreenWidget(this);
    stackedLayout->addWidget(lockWidget);
    stackedLayout->setCurrentIndex(0);
}

void ScreenMirroringWindow::initBottom()
{
    bottomWidget = new QWidget(this);
    bottomWidget->setFixedHeight(BOTTOM_HEIGHT);

    QHBoxLayout *buttonLayout = new QHBoxLayout(bottomWidget);
    QStringList buttonIcons = { "phone_back", "home", "multi_task" };   // 按钮icon列表

    for (int i = 0; i < buttonIcons.size(); ++i) {
        CooperationIconButton *btn = new CooperationIconButton();
        btn->setIcon(QIcon::fromTheme(buttonIcons[i]));
        btn->setIconSize(QSize(16, 16));
        btn->setFixedSize(36, 36);

        connect(btn, &CooperationIconButton::clicked, this, [this, i]() {
            emit this->buttonClicked(i);
        });

        buttonLayout->setAlignment(Qt::AlignCenter);
        buttonLayout->setSpacing(20);
        buttonLayout->addWidget(btn);
    }   
}

void ScreenMirroringWindow::initTitleBar(const QString &device)
{
    auto titleBar = titlebar();

    titleBar->setIcon(QIcon::fromTheme(KIcon));
    titleBar->setMenuVisible(false);

    QLabel *title = new QLabel(device);
    titleBar->addWidget(title, Qt::AlignLeft);
}

void ScreenMirroringWindow::initSizebyView(QSize &viewSize)
{
    m_vncViewer->setMobileRealSize(viewSize.width(), viewSize.height());
}

void ScreenMirroringWindow::connectVncServer(const QString &ip, int port, const QString &password)
{
    m_vncViewer->setServes(ip.toStdString(), port, password.toStdString());
    m_vncViewer->start();
}

void ScreenMirroringWindow::handleSizeChange(const QSize &size)
{
    float scale = 1.0;
    QScreen *screen = qApp->primaryScreen();
    if (screen) {
        int height = screen->geometry().height();

        if (height >= 2160) {
            // 4K
            scale = 1.0;
        } else if (height >= 1140) {
            // 2K
            scale = 1.5;
        } else {
            // 1080P
            scale = 2.0;
        }
    }

    auto titleBar = titlebar();
    int w = (size.width() / scale);
    int h = (size.height() / scale) + titleBar->height() + BOTTOM_HEIGHT;

    if (!initShow && screen) {
        int width = screen->geometry().width();
        int height = screen->geometry().height();
        int offw = (size.height() / scale);
        int offh = h;
        if (size.width() > size.height()) {
            //landscape
            offw = w;
            offh = w +  titleBar->height() + BOTTOM_HEIGHT;
        }
        // 计算窗口的X坐标，使其始终以竖屏在右边居中
        int x = width - offw;
        int y = (height - offh) / 2; // 垂直居中

        this->move(x, y);
        initShow = true;
    }

    // only the size is biger current size can be resized.
    if (w > this->width() || h > this->height()) {
        m_expectSize = QSize(0, 0); // has been resize, reset.
        this->resize(w, h);
    } else {
        m_expectSize = size; // record expected size which is not resized
    }
}

void ScreenMirroringWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized() || isMinimized()) {
            m_previousSize = size();
        } else {
            if (m_vncViewer) {
                if (m_expectSize.width() > 0) {
                    // back to previous size and then do expected changed.
                    resize(m_previousSize);
                    handleSizeChange(m_expectSize);
                    return;
                }
            }
        }
    }

    QWidget::changeEvent(event);
}

LockScreenWidget::LockScreenWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme(KIcon).pixmap(64, 64));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(tr("The current device has been locked"), this);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subTitleLabel = new QLabel(tr("You can unlock it on mobile devices"), this);
    subTitleLabel->setStyleSheet("font-weight: 400; font-size: 12px; color:rgba(0, 0, 0, 0.6);");
    subTitleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(subTitleLabel);
    layout->setAlignment(Qt::AlignCenter);
}

void LockScreenWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    //Q_UNUSED(event);
    //    QPainter painter(this);
    //    QPixmap background(":/icons/lingmo/builtin/icons/background_app.png");
    //    painter.drawPixmap(this->rect(), background.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}
