// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appdrawerwidget.h"
#include "calculate_util.h"
#include "appsmanager.h"
#include "constants.h"
#include "editlabel.h"

#include <QHBoxLayout>

#include <DAnchors>

AppDrawerWidget::AppDrawerWidget(QWidget *parent)
    : QWidget (parent)
    , m_maskWidget(new QWidget(this))
    , m_appDelegate(new AppItemDelegate(this))
    , m_multipageView(new MultiPagesView(AppsListModel::Dir, this))
    , m_blurBackground(new DBlurEffectWidget(m_multipageView))
    , m_clickIndex(QModelIndex())
{
    initUi();
    initConnection();
    initAccessible();
}

AppDrawerWidget::~AppDrawerWidget()
{
}

void AppDrawerWidget::initUi()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    m_maskWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_maskWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    DAnchors<QWidget> parentAnchor(this->parentWidget());
    DAnchors<AppDrawerWidget> currentAnchor(this);
    DAnchors<QWidget> maskAnchor(m_maskWidget);
    // not sure why setFill not works properly here...
    // currentAnchor.setFill(parentAnchor);
    currentAnchor.setTop(parentAnchor.top());
    currentAnchor.setLeft(parentAnchor.left());
    currentAnchor.setBottom(parentAnchor.bottom());
    currentAnchor.setRight(parentAnchor.right());
    maskAnchor.setFill(currentAnchor);

    qApp->installEventFilter(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_maskWidget);
    mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_multipageView);
    m_maskWidget->setLayout(mainLayout);

    constexpr quint8 alpha = std::numeric_limits<quint8>::max() * 0.8;
    m_blurBackground->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurBackground->setMaskAlpha(alpha);
    m_blurBackground->setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->lower();
    m_multipageView->raise();
    m_multipageView->setDataDelegate(m_appDelegate);
}

void AppDrawerWidget::initAccessible()
{
    m_multipageView->setAccessibleName("drawerWidget");
    m_blurBackground->setAccessibleName("blurBackground");
    m_maskWidget->setAccessibleName("appDrawerMaskWidget");
}

void AppDrawerWidget::initConnection()
{
    connect(m_multipageView, &MultiPagesView::titleChanged, this, &AppDrawerWidget::onTitleChanged);
    connect(m_multipageView, &MultiPagesView::connectViewEvent, this, &AppDrawerWidget::drawerClicked);
}

void AppDrawerWidget::updateBackgroundImage(const QPixmap &img)
{
    m_pix = img;
    update();
}

void AppDrawerWidget::refreshDrawerTitle(const QString &title)
{
    m_multipageView->refreshTitle(title, rect().width());
}

void AppDrawerWidget::setCurrentIndex(const QModelIndex &index)
{
    m_clickIndex = index;
}

void AppDrawerWidget::onTitleChanged()
{
    m_multipageView->updateAppDrawerTitle(m_clickIndex);
}

void AppDrawerWidget::showEvent(QShowEvent *event)
{
    QSize itemSize = CalculateUtil::instance()->appItemSize() * 5 / 4;
    QSize widgetSize = QSize(itemSize.width() * 4, itemSize.height() * 3 + DLauncher::DRAW_TITLE_HEIGHT);

    m_multipageView->setFixedSize(widgetSize);
    m_blurBackground->setFixedSize(widgetSize);

    m_multipageView->updatePageCount(AppsListModel::Dir);
    m_multipageView->setModel(AppsListModel::Dir);

    return QWidget::showEvent(event);
}

bool AppDrawerWidget::eventFilter(QObject *object, QEvent *event)
{
    if ((event->type() == QEvent::DragMove) && isVisible()) {
        const QRect widgetRect = m_multipageView->geometry().marginsRemoved(QMargins(10, 10, 10, 10));
        // 左，上，右，下, 等于或者超越边界时，从文件夹移出应用
        if (!widgetRect.contains(QCursor::pos())) {
            hide();
            AppsManager::instance()->setDragMode(AppsManager::DirOut);
            AppsManager::instance()->removeDragItem();
        }
    }

    return QWidget::eventFilter(object, event);
}

void AppDrawerWidget::mousePressEvent(QMouseEvent *event)
{
    m_multipageView->getEditLabel()->cancelEditState();

    // 多页视图范围之外，蒙板之内点击，向上传递点击事件
    if (isVisible() && !m_multipageView->geometry().contains(event->pos()))
        return QWidget::mousePressEvent(event);
}

void AppDrawerWidget::hideEvent(QHideEvent *event)
{
    m_multipageView->getEditLabel()->cancelEditState();

    return QWidget::hideEvent(event);
}
