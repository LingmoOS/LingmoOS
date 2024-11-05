/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "navigation-tab-bar.h"
#include "x11-window-manager.h"

#include "file-utils.h"
#include "search-vfs-uri-parser.h"

#include <QToolButton>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include <QMimeData>
#include <QDrag>
#include <QGraphicsOpacityEffect>

#include <KWindowSystem>

#include <QApplication>
#include <QWindow>

#include <QPainter>
#include <QPainterPath>

#include <QStyleOption>

#include "FMWindowIface.h"
#include "main-window.h"
#include "file-info.h"
#include "file-info-job.h"

static TabBarStyle *global_instance = nullptr;

NavigationTabBar::NavigationTabBar(QWidget *parent) : QTabBar(parent)
{
    setProperty("isWindowButton", 0x1);
    setProperty("useIconHighlightEffect", 0x2);

    setFocusPolicy(Qt::StrongFocus);

    setAcceptDrops(true);
    m_drag_timer.setInterval(750);
    m_drag_timer.setSingleShot(true);

    setStyle(TabBarStyle::getStyle());

    setContentsMargins(0, 0, 0, 0);
    //setUsesScrollButtons(false);
    //setFixedHeight(36);

    setProperty("useStyleWindowManager", false);
    setMovable(true);
    setExpanding(false);
    setTabsClosable(true);
    setUsesScrollButtons(true);
    X11WindowManager::getInstance()->registerWidget(this);

    connect(this, &QTabBar::currentChanged, this, [=](int index) {
        //qDebug()<<"current changed"<<index;
    });

    connect(this, &QTabBar::tabMoved, this, [=](int from, int to) {
        //qDebug()<<"move"<<from<<"to"<<to;
    });

    connect(this, &QTabBar::tabBarClicked, this, [=](int index) {
        //qDebug()<<"tab bar clicked"<<index;
    });

    connect(this, &QTabBar::tabBarDoubleClicked, this, [=](int index) {
        //qDebug()<<"tab bar double clicked"<<index;
    });

//    connect(this, &QTabBar::tabCloseRequested, this, [=](int index){
//        removeTab(index);
//    });

    setDrawBase(false);
}

void NavigationTabBar::addPages(const QStringList &uri)
{

}

void NavigationTabBar::updateLocation(int index, const QString &uri)
{
    auto info = Peony::FileInfo::fromUri(uri);
    auto infoJob = new Peony::FileInfoJob(info);
    //infoJob->setAutoDelete();
    setTabData(index, uri);

    connect(infoJob, &Peony::FileInfoJob::queryAsyncFinished, this, [=](){
        infoJob->deleteLater();
        if (uri != tabData(index).toString())
            return;
        auto iconName = Peony::FileUtils::getFileIconName(uri);
        auto displayName = Peony::FileUtils::getFileDisplayName(uri);
        //qDebug() << "updateLocation text:" <<displayName <<uri;
        if (uri.startsWith("search:///"))
        {
            QString nameRegexp = Peony::SearchVFSUriParser::getSearchUriNameRegexp(uri);
            QString targetDirectory = Peony::SearchVFSUriParser::getSearchUriTargetDirectory(uri);
            displayName = tr("Search \"%1\" in \"%2\"").arg(nameRegexp).arg(targetDirectory);
        }

        //elide text if it is too long
//        if (displayName.length() > ELIDE_TEXT_LENGTH)
//        {
//            int  charWidth = fontMetrics().averageCharWidth();
//            displayName = fontMetrics().elidedText(displayName, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
//        }
        if (displayName.contains("&")) {
            displayName = Peony::FileUtils::handleSpecialSymbols(displayName);
        }
        setElideMode(Qt::ElideRight);
        setTabText(index, displayName);
        setTabIcon(index, QIcon::fromTheme(iconName));
        setTabData(index, uri);
        setTabToolTip(index, displayName);

        Q_EMIT this->locationUpdated(uri);
    });

    infoJob->queryAsync();
}

void NavigationTabBar::addPage(const QString &uri, bool jumpToNewTab)
{
    setFocus();
    if (uri.isEmpty())
        return;
    m_info = Peony::FileInfo::fromUri(uri);
    if (!uri.isNull()) {
        auto iconName = Peony::FileUtils::getFileIconName(uri);
        auto displayName = Peony::FileUtils::getFileDisplayName(uri);
        if (displayName.contains("&")) {
            displayName = Peony::FileUtils::handleSpecialSymbols(displayName);
        }
        addTab(QIcon::fromTheme(iconName), displayName);
        setTabData(count() - 1, uri);
        if (jumpToNewTab)
            setCurrentIndex(count() - 1);
        Q_EMIT this->pageAdded(uri);
    } else {
        if (currentIndex() == -1) {
            addPage("file:///", true);
        } else {
            QString uri = tabData(currentIndex()).toString();
            addPage(uri, jumpToNewTab);
        }
    }
}

void NavigationTabBar::tabRemoved(int index)
{
    //qDebug()<<"tab removed"<<index;
    QString uri = tabData(index).toString();

    QTabBar::tabRemoved(index);

    Q_EMIT pageRemoved(uri);

    if (count() == 0) {
        Q_EMIT closeWindowRequest();
    }
}

void NavigationTabBar::tabInserted(int index)
{
    //qDebug()<<"tab inserted"<<index;
    QTabBar::tabInserted(index);
}

void NavigationTabBar::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->source() != this) {
        QPoint pos = e->pos();
        int index = tabAt(pos);
        if (index >= 0) {
            setCurrentIndex(index);
        }
    }
    e->accept();
    return;
}

void NavigationTabBar::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->source() == this) {
        m_should_trigger_drop = false;
        m_drag->cancel();
        grabMouse();
    }
    e->accept();
    return;
}

void NavigationTabBar::dragLeaveEvent(QDragLeaveEvent *e)
{
    QTabBar::dragLeaveEvent(e);
}

void NavigationTabBar::dropEvent(QDropEvent *e)
{
    m_start_drag = false;
    if (e->source() != this) {
        if (e->mimeData()->hasUrls()) {
            for (auto url : e->mimeData()->urls()) {
                if (Peony::FileUtils::isFileDirectory(url.url())) {
                    addPageRequest(url.url(), true);
                }
            }
        } else if (e->mimeData()->hasFormat("explorer/tab-index")) {
            auto uri = e->mimeData()->data("explorer/tab-index");
            if (Peony::FileUtils::isFileDirectory(uri)) {
                addPageRequest(uri, true);
            }
        }

        //finish the drag, remove old tab page from old tab.
        if (auto oldTab = qobject_cast<NavigationTabBar *>(e->source())) {
            oldTab->removeTab(oldTab->currentIndex());
        }
    }
    releaseMouse();
}

void NavigationTabBar::mousePressEvent(QMouseEvent *e)
{
    QTabBar::mousePressEvent(e);
    m_press_pos = e->pos();
    if (tabAt(e->pos()) >= 0)
        m_start_drag = true;
    else
        m_start_drag = false;
}

void NavigationTabBar::mouseMoveEvent(QMouseEvent *e)
{
    QTabBar::mouseMoveEvent(e);

    if (e->source() != Qt::MouseEventNotSynthesized) {
        return;
    }

    auto offset = e->pos() - m_press_pos;
    auto offsetX = qAbs(offset.x());
    auto offsetY = qAbs(offset.y());

    if (e->pos().y() >= 0 && e->pos().y() <= this->height()) {
        return;
    }

    if (!m_start_drag)
        return;

    //start a drag
    //note that we should remove this tab from the window
    //at other tab's drop event.

    auto pixmap = this->topLevelWidget()->grab().scaledToWidth(this->topLevelWidget()->width()/2, Qt::SmoothTransformation);

    auto thisWindow = this->topLevelWidget();
    //KWindowSystem::lowerWindow(this->topLevelWidget()->winId());
    for (auto win : qApp->allWidgets()) {
        if (auto mainWin = qobject_cast<MainWindow *>(win)) {
            if (thisWindow != mainWin)
                KWindowSystem::raiseWindow(win->winId());
        }
    }

    QDrag *d = new QDrag(this);
    m_drag = d;
    QMimeData *data = new QMimeData();
    auto uri = tabData(currentIndex()).toString();
    //data->setText(uri);
    data->setData("explorer/tab-index", uri.toUtf8());
    d->setMimeData(data);

    d->setPixmap(pixmap);
    d->setHotSpot(pixmap.rect().center());
    m_should_trigger_drop = true;
    d->exec();
    qApp->restoreOverrideCursor();
    m_drag = nullptr;

    if (m_should_trigger_drop) {
        if (auto tab = qobject_cast<NavigationTabBar *>(d->target())) {
            //do nothing for target tab bar helped us handling yet.
        } else {
            auto window = dynamic_cast<Peony::FMWindowIface *>(this->topLevelWidget());
            auto newWindow = dynamic_cast<QWidget *>(window->create(this->tabData(currentIndex()).toString()));
            newWindow->show();
            KWindowSystem::raiseWindow(newWindow->winId());
            newWindow->move(QCursor::pos() - newWindow->rect().center());
            removeTab(currentIndex());
        }
        delete d;
        releaseMouse();
        m_start_drag = false;
    } else {
        d->deleteLater();
    }
}

void NavigationTabBar::mouseReleaseEvent(QMouseEvent *e)
{
    QTabBar::mouseReleaseEvent(e);
    m_start_drag = false;
    releaseMouse();
}

void NavigationTabBar::resizeEvent(QResizeEvent *e)
{
    QTabBar::resizeEvent(e);
}

TabBarStyle *TabBarStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new TabBarStyle;
    }
    return global_instance;
}

void TabBarStyle::polish(QWidget *widget)
{
    QProxyStyle::polish(widget);
    widget->setProperty("isWindowButton", 0x1);
    //bug#167146 useIconHighlightEffect=2 导致打开预览框的图标不反白
    //widget->setProperty("useIconHighlightEffect", 0x2);
}

int TabBarStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QRect TabBarStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_TabBarScrollLeftButton:
    case SE_TabBarTearIndicatorLeft: {
        QRect tabRect = option->rect;
        tabRect.setRight(tabRect.left() + 8);
        return tabRect;
    }
    case SE_TabBarScrollRightButton:
    case SE_TabBarTearIndicatorRight: {
        QRect tabRect = option->rect;
        tabRect.setLeft(tabRect.right() - 8);
        return tabRect;
    }
    default:
        return QProxyStyle::subElementRect(element, option, widget);
    }
}

void TabBarStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && (widget->objectName() == "addPageButton" || widget->objectName() == "toolButton")) {
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addEllipse(widget->rect().adjusted(2, 2, -2, -2));
        painter->setClipPath(path);
        qApp->style()->drawComplexControl(control, option, painter, widget);
        painter->restore();
    } else {
        qApp->style()->drawComplexControl(control, option, painter, widget);
    }
}

void TabBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->objectName() == "previewButtons") {
        return;
    }
    qApp->style()->drawControl(element, option, painter, widget);
}

void TabBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight: {
        QStyleOption tmp = *option;
        tmp.palette.setColor(QPalette::HighlightedText, qApp->palette().buttonText().color());
        return qApp->style()->drawPrimitive(element, &tmp, painter, widget);
    }
    default:
        return qApp->style()->drawPrimitive(element, option, painter, widget);
    }
}
