// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionpanel.h"
#include <QPainter>
//#include "application.h"
//#include "controller/signalmanager.h"
#include "darrowbutton.h"
#include "accessibility/ac-desktop-define.h"

#include <DFontSizeManager>

using namespace Dtk::Widget;

namespace {

// const int CONTROL_BUTTON_WIDTH = 20;
// const int CONTROL_BUTTON_HEIGHT = 60;
// const int CONTROL_BUTTON_CUBIC_LENGTH = 30;
const int EXTENSION_PANEL_WIDTH = 300 + 20;
// const int EXTENSION_PANEL_MAX_WIDTH = 340;

const QColor DARK_COVERBRUSH = QColor(0, 0, 0, 100);
const QColor LIGHT_COVERBRUSH = QColor(255, 255, 255, 179);
const int ANIMATION_DURATION = 500;
const QEasingCurve ANIMATION_EASING_CURVE = QEasingCurve::InOutCubic;
}  // namespace

ExtensionPanel::ExtensionPanel(QWidget *parent)
//    : DFloatingWidget(parent)
    : DAbstractDialog(parent)
{
    init();
    setObjectName(EXTENSION_PANEL);
#ifdef OPENACCESSIBLE
    setAccessibleName(EXTENSION_PANEL);
    m_titleBar->setObjectName(CONTENT_TITLE_BAR);
    m_titleBar->setAccessibleName(CONTENT_TITLE_BAR);
    m_scrollArea->setObjectName(CONTENT_SCROLL_AREA);
    m_scrollArea->setAccessibleName(CONTENT_SCROLL_AREA);
#endif
    //    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    this->setWindowTitle(tr("Image info"));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6, QFont::Medium);

//    m_contentLayout = new QVBoxLayout(this);
//    m_contentLayout->setContentsMargins(0, 0, 0, 0);
//    m_contentLayout->setSpacing(0);

//    this->setModal(true);

    setFixedWidth(EXTENSION_PANEL_WIDTH);
    setFixedHeight(400);
}

void ExtensionPanel::setContent(QWidget *content)
{
    if (content) {
#if 1
//        QLayoutItem *child;
//        if ((child = m_contentLayout->takeAt(0)) != 0) {
//            if (child->widget())
//                child->widget()->deleteLater();
//            delete child;
//        }
#else
        QLayoutItem *child;
        if ((child = m_contentLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->setParent(nullptr);
        }
        delete child;
#endif
        m_content = content;
//        updateRectWithContent();
#ifdef OPENACCESSIBLE
        if (m_content) {
            m_content->setObjectName(CONTENT_WIDGET);
            m_content->setAccessibleName(CONTENT_WIDGET);
        }
#endif
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_scrollArea->widget()->layout());
        if (nullptr != layout)
            layout->addWidget(content);
//        this->addContent(content);
    }
}

void ExtensionPanel::updateRectWithContent(int height)
{
    setFixedHeight(qMin(540, height)); //tmp for imageinfo
}
//{
//    QPainter painter(this);
//    QPainterPath path;
//    path.moveTo(0, 0);//top left
//    path.lineTo(width() - CONTROL_BUTTON_WIDTH, 0);//top right
//    int cubicStep = 5;
//    //cubic 1
//    QPoint cubic1StartPoint(width() - CONTROL_BUTTON_WIDTH,
//                            (height() - CONTROL_BUTTON_HEIGHT) / 2 - cubicStep);
//    QPoint cubic1EndPoint(width(),
//                          cubic1StartPoint.y() + CONTROL_BUTTON_CUBIC_LENGTH);
//    path.lineTo(cubic1StartPoint); //start point of cubicTo
//    path.cubicTo(QPoint(cubic1StartPoint.x(), cubic1EndPoint.y() - cubicStep),
//        QPoint(width(), cubic1StartPoint.y() + cubicStep), cubic1EndPoint);
//    //cubic 2
//    QPoint cubic2StartPoint(width(), cubic1EndPoint.y() + (CONTROL_BUTTON_HEIGHT
//        - CONTROL_BUTTON_CUBIC_LENGTH) / 2);
//    QPoint cubic2EndPoint(width() - CONTROL_BUTTON_WIDTH,
//                          cubic2StartPoint.y() + CONTROL_BUTTON_CUBIC_LENGTH);
//    path.lineTo(cubic2StartPoint);
//    path.cubicTo(QPoint(cubic2StartPoint.x(), cubic2EndPoint.y() - cubicStep),
//                 QPoint(cubic2EndPoint.x(), cubic2StartPoint.y() + cubicStep),
//                 cubic2EndPoint);
//    path.lineTo(width() - CONTROL_BUTTON_WIDTH, height()); // Right bottom
//    path.lineTo(0, height()); // Left bottom
//    path.lineTo(0, 0); // Back to the start point

//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.setClipPath(path);

//    painter.drawPixmap(0, 0, width(), height(), getResultPixmap());
//    painter.fillRect(0, 0, width(), height(), QBrush(QColor(0, 0, 0, 153)));

//    QPen pen;
//    pen.setColor(QColor(255, 255, 255, 51));
//    pen.setWidth(1);
//    painter.setPen(pen);
//    painter.drawPath(path);
//    painter.end();
//}
//void ExtensionPanel::moveWithAnimation(int x, int y)
//{
//    Q_UNUSED(x);
//    Q_UNUSED(y);
//    //    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
//    //    animation->setDuration(ANIMATION_DURATION);
//    //    animation->setEasingCurve(ANIMATION_EASING_CURVE);
//    //    animation->setStartValue(pos());
//    //    animation->setEndValue(QPoint(x, y));
//    //    animation->start();
//    //    connect(this, &ExtensionPanel::requestStopAnimation, animation,
//    //    &QPropertyAnimation::stop); connect(this, &ExtensionPanel::requestStopAnimation,
//    //    animation,
//    //            &QPropertyAnimation::deleteLater);
//    //    connect(animation, &QPropertyAnimation::finished, animation,
//    //    &QPropertyAnimation::deleteLater);
//}

void ExtensionPanel::init()
{
    m_mainLayout = new QVBoxLayout;

    m_titleBar = new DTitlebar();
    m_titleBar->setMenuVisible(false);
    m_titleBar->setBackgroundTransparent(true);
    m_titleBar->setTitle(this->windowTitle());
    QObject::connect(this, &ExtensionPanel::windowTitleChanged, m_titleBar, &DTitlebar::setTitle);

    m_scrollArea = new QScrollArea;
    m_scrollArea->setMinimumHeight(40);
    QPalette palette = m_scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    m_scrollArea->viewport()->setPalette(palette);
    m_scrollArea->setFrameShape(QFrame::Shape::NoFrame);

    QWdToDWidget *scrollContentWidget = new QWdToDWidget(m_scrollArea);
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 10);
    scrollWidgetLayout->setSpacing(0);
    scrollContentWidget->setLayout(scrollWidgetLayout);
    m_scrollArea->setWidget(scrollContentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addWidget(m_scrollArea);

    this->setLayout(m_mainLayout);

    m_scImageInfo = new QShortcut(this);
    m_scImageInfo->setKey(tr("Ctrl+I"));
    m_scImageInfo->setContext(Qt::ApplicationShortcut);
    m_scImageInfo->setAutoRepeat(false);

    connect(m_scImageInfo, &QShortcut::activated, this, [this] {
//        emit hideExtensionPanel();
        this->hide();
    });
    // Esc
    m_scEsc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_scEsc->setContext(Qt::WindowShortcut);
    connect(m_scEsc, &QShortcut::activated, this, [ = ] {
//        emit hideExtensionPanel();
        this->hide();
    });
}
