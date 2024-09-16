// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwidget.h"
#include "shortcutview.h"

#include <DPlatformWindowHandle>
#include <DApplication>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QProcessEnvironment>
#include <QTimer>
#include <QPainter>
#include <QPalette>
#include <QKeyEvent>
#include <QScreen>

DWIDGET_USE_NAMESPACE

MainWidget::MainWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
{
    initUI();
}

void MainWidget::setJsonData(const QString &data)
{
    if (m_mainView) {
        m_mainLayout->removeWidget(m_mainView);
        m_mainView->deleteLater();
    }

    m_mainView = new ShortcutView(this);
    m_mainView->setObjectName("MainView");
    m_mainLayout->addWidget(m_mainView);
    m_mainView->setData(data);
    adjustSize();
}

void MainWidget::initUI()
{
    if (qApp->platformName() == "wayland") {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog);
        setWindowFlag(Qt::FramelessWindowHint);
    } else {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Popup);
    }

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);

    setLayout(m_mainLayout);
    initMargins();
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    if (DApplication::isDXcbPlatform()) {
        DPlatformWindowHandle handle(this);

        handle.setBorderWidth(2);
        handle.setBorderColor(QColor(255, 255, 255, static_cast<int>(255 * 0.15)));
    }
}

void MainWidget::initMargins()
{

    QLocale systemLocale = QLocale::system();
    bool isNormal { systemLocale.language() == QLocale::Chinese };
    if (isNormal && qApp->devicePixelRatio() > 1.2)
        isNormal = false;
    // 高分屏不减少margin
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        QSize screenSize = primaryScreen->size();
        if (screenSize.width() > 2000)
            isNormal = true;
    }
    int margin { CONTENT_MARGINS };
    if (!isNormal)
        margin = 10;

    setContentsMargins(margin, margin, margin, margin);
}

void MainWidget::mousePressEvent(QMouseEvent *e)
{
    hide();
    DBlurEffectWidget::mousePressEvent(e);
}

void MainWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift) {
        releaseKeyboard();
        DBlurEffectWidget::keyReleaseEvent(e);
        hide();
    }
}

void MainWidget::focusInEvent(QFocusEvent *e)
{
    grabKeyboard();
    DBlurEffectWidget::focusInEvent(e);
}

void MainWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Back)
        hide();
}

void MainWidget::showEvent(QShowEvent *e)
{
    DBlurEffectWidget::showEvent(e);

    setFocus(Qt::MouseFocusReason);
    grabKeyboard();

    QTimer::singleShot(500, this, [this]() {
        if (DApplication::queryKeyboardModifiers() != (Qt::ShiftModifier | Qt::ControlModifier))
            hide();
    });
}

void MainWidget::hideEvent(QHideEvent *e)
{
    DBlurEffectWidget::hideEvent(e);
}

void MainWidget::paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);
}
