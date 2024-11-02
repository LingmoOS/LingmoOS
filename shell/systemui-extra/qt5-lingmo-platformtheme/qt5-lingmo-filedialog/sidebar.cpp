/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#include "sidebar.h"
#include <QTimer>
#include <PeonySideBarModel>
#include <QScrollBar>
#include <QHeaderView>
#include <QPainterPath>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolTip>
#include <QScreen>
#include <QDBusInterface>
#include "kyfiledialog.h"
#include "explor-qt/controls/menu/side-bar-menu/side-bar-menu.h"
#include "explor-qt/side-bar-abstract-item.h"
#include "explor-qt/volume-manager.h"
#include "explor-qt/file-enumerator.h"
#include "explor-qt/file-utils.h"
#include "explor-qt/file-info.h"
#include "settings/lingmo-style-settings.h"
#include "debug.h"
using namespace LINGMOFileDialog;

FileDialogComboBox::FileDialogComboBox(QWidget *parent):QComboBox(parent)
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        auto settings = LINGMOStyleSettings::globalInstance();
        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            pDebug << "key changed:" << key << this->size();
                m_styleChanged = true;
        });
    }

    connect(this, &FileDialogComboBox::setStyleChanged, this, [=](bool change){
        pDebug << "setStyleChanged m_styleChanged:" << change;
        m_styleChanged = change;
    });
    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
    if(interFace->isValid()){
        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
    }
    m_minSize = this->minimumSize();
    pDebug << "combobox minsize...." << m_minSize;
}

void FileDialogComboBox::tableModeChanged(bool isTableMode)
{
    m_styleChanged = true;
}

QSize FileDialogComboBox::minimumSizeHint() const
{
    pDebug << "minimumSizeHint...." << m_styleChanged << this->size() << m_minSize;
    if(m_styleChanged || (qApp->screenAt(QCursor::pos())->availableSize().width() - 300) < QComboBox::minimumSizeHint().width())
    {
        Q_EMIT setStyleChanged(false);
        return m_minSize;//this->size();
    }
    QSize size = QComboBox::minimumSizeHint();


    return size;
}


FileDialogSideBar::FileDialogSideBar(QWidget *parent) : QTreeView(parent)
{
//    setStyle(SideBarStyle::getStyle());

    setIconSize(QSize(16, 16));

//    header()->setSectionResizeMode(QHeaderView::Stretch );
    header()->setStretchLastSection(false);
    header()->hide();

    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setFrameStyle(QFrame::NoFrame);
    setSortingEnabled(true);

    setProperty("useIconHighlightEffect", true);
    setProperty("highlightMode", true);

    setDragDropMode(QTreeView::NoDragDrop);
    setProperty("doNotBlur", true);
    viewport()->setProperty("doNotBlur", true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setExpandsOnDoubleClick(false);
    setMouseTracking(true);//追踪鼠标
    setAutoScrollMargin(0);


    m_delegate = new SideBarItemDelegate(this);
    setItemDelegate(m_delegate);

    m_model = new Peony::SideBarModel(this);
    m_proxyModel = new Peony::SideBarProxyFilterSortModel(this);
    m_proxyModel->setSourceModel(m_model);
    setModel(m_proxyModel);

    Peony::VolumeManager *volumeManager = Peony::VolumeManager::getInstance();
    connect(volumeManager,&Peony::VolumeManager::volumeAdded,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxyModel->invalidate();//display DVD device in real time.
    });
    connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxyModel->invalidate();//The drive does not display when the DVD device is removed.
        //pDebug << "volumeRemoved:" <<QToolTip::text();
        //fix abnormal pull out usb device tips not hide issue, link to bug#81190
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    });
    connect(volumeManager,&Peony::VolumeManager::driveDisconnected,this,[=](const std::shared_ptr<Peony::Drive> &drive){
        m_proxyModel->invalidate();//Multiple udisk eject display problem
    });
        connect(volumeManager,&Peony::VolumeManager::mountAdded,this,[=](const std::shared_ptr<Peony::Mount> &mount){
        m_proxyModel->invalidate();//display udisk in real time after format it.
    });


    connect(this, &FileDialogSideBar::clicked, this, [=](const QModelIndex &index){
        switch (index.column()) {
        case 0: {
            auto item = m_proxyModel->itemFromIndex(index);

            if (! item)
                break;
            //continue to fix crash issue, related to bug#116201,116589
            if(item->isMountable()&&!item->isMounted())
                item->mount();
            else{
                auto uri = item->uri();

                if (!item->uri().isEmpty()){
                    if (uri == "computer:///lingmo-data-volume") {
                        uri = "file:///data";
                    }
                    Q_EMIT goToUriRequest(uri);
                }
            }
            break;
        }
        case 1: {
            auto item = m_proxyModel->itemFromIndex(index);
            if (item->isMounted() || item->isEjectable()||item->isStopable()) {
                auto leftIndex = m_proxyModel->index(index.row(), 0, index.parent());
                this->collapse(leftIndex);
                item->ejectOrUnmount();
            } else {
                // if item is not unmountable, just be same with first column.
                // fix #39716
                auto uri = item->uri();

                if (!item->uri().isNull()){
                    if (uri == "computer:///lingmo-data-volume") {
                        uri = "file:///data";
                    }
                    Q_EMIT goToUriRequest(uri);
                }
            }
            break;
        }
        default:
            break;
        }
    });

    connect(this, &FileDialogSideBar::expanded, this, [=](const QModelIndex &index){
        auto item = m_proxyModel->itemFromIndex(index);
        item->findChildrenAsync();
    });

    connect(this, &FileDialogSideBar::collapsed, this, [=](const QModelIndex &index){
        auto item = m_proxyModel->itemFromIndex(index);
        item->clearChildren();
    });

    connect(this, &QTreeView::customContextMenuRequested, this, [=](const QPoint &pos){
        auto index = indexAt(pos);
        auto item = m_proxyModel->itemFromIndex(index);
        if (item) {
            if (item->type() != Peony::SideBarAbstractItem::SeparatorItem) {
                Peony::SideBarMenu menu(item, nullptr);
                QList<QAction *> actionList;
                if (item->type() == Peony::SideBarAbstractItem::FileSystemItem) {
                    if ((0 != QString::compare(item->uri(), "computer:///")) &&
                        (0 != QString::compare(item->uri(), "filesafe:///"))) {
                        for (const auto &actionItem : actionList) {
                            if(item->isVolume())/* 分区才去需要判断是否已挂载 */
                                actionItem->setEnabled(item->isMounted());
                        }
                    }
                }

                menu.exec(QCursor::pos());
            }
        }
    });

    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
    if(interFace->isValid()){
        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
    }

//    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
//        auto settings = LINGMOStyleSettings::globalInstance();
//        connect(settings, &QGSettings::changed, this, [=](const QString &key){
//            pDebug << "key changed:" << key << width();
//            if (key == "styleName") {
//                m_siderWidth = this->width();
//            }
//        });
//    }
    expandToDepth(1);
    setViewportMargins(4, 4, 0, 0);

    pDebug << "columndffffffffffffffff";
//    setColumnHidden(1, true);
}

FileDialogSideBar::~FileDialogSideBar()
{
    if(m_proxyModel){
        m_proxyModel->deleteLater();
        m_proxyModel = nullptr;
    }
    if(m_model){
        m_model->deleteLater();
        m_model = nullptr;
    }
    if(m_delegate){
        m_delegate->deleteLater();
        m_delegate = nullptr;
    }
}

void FileDialogSideBar::tableModeChanged(bool isTableMode)
{
    pDebug << "tableModeChanged........";
//    setStyle(SideBarStyle::getStyle());
}

void FileDialogSideBar::resizeEvent(QResizeEvent *e)
{
    setViewportMargins(4, 4, 0, 0);

//    this->setGeometry(this->x(), this->y(), m_siderWidth, this->height());
    QTreeView::resizeEvent(e);
    pDebug << "FileDialogSideBar resizeEvent header count......:" << header()->count() << this->viewport()->width() << this->columnWidth(0) << this->columnWidth(1);
    if (header()->count() > 0) {
        this->setColumnWidth(1, 20);
        header()->resizeSection(0, this->viewport()->width() - this->columnWidth(1));
    }
}

void FileDialogSideBar::menuRequest(const QPoint &pos)
{
//    auto index = indexAt(pos);
//    auto item = proxyModel->itemFromIndex(index);
//    if (item) {
//        pDebug << "sider bar menu00000" << item->type() << index;
//        if (item->type() != Peony::SideBarAbstractItem::SeparatorItem) {
//            pDebug << "sider bar menu11111";
////            Peony::SideBarMenu *menu = new Peony::SideBarMenu(item, this);
//            pDebug << "sider bar menu22222222";
////            menu->exec(QCursor::pos());
//            pDebug << "sider bar menu3333333";
//        }
//    }
}

void FileDialogSideBar::paintEvent(QPaintEvent *event)
{
//    pDebug << "sidebar paintEvent.......:" << this->size();
    QTreeView::paintEvent(event);
}

//void FileDialogSideBar::paintEvent(QPaintEvent *e)
//{
//    QColor color = this->palette().window().color();
//    QColor colorBase = QColor(Qt::red); //this->palette().base().color();

//    int R1 = color.red();
//    int G1 = color.green();
//    int B1 = color.blue();
//    qreal a1 = 0.3;

//    int R2 = colorBase.red();
//    int G2 = colorBase.green();
//    int B2 = colorBase.blue();
//    qreal a2 = 1;

//    qreal a = 1 - (1 - a1)*(1 - a2);

//    qreal R = (a1*R1 + (1 - a1)*a2*R2) / a;
//    qreal G = (a1*G1 + (1 - a1)*a2*G2) / a;
//    qreal B = (a1*B1 + (1 - a1)*a2*B2) / a;

//    colorBase.setRed(R);
//    colorBase.setGreen(G);
//    colorBase.setBlue(B);
//    colorBase.setAlphaF(1);

////    QPainter p(this);
////    QPainterPath sidebarPath;
////    sidebarPath.addRoundedRect(this->geometry(), 6, 6);
////    p.fillPath(sidebarPath, colorBase);
////    viewport()->setObjectName("viewport");
////    viewport()->setStyleSheet("QWidget#viewport{background-color:rgba(255,255,255,0.5)");////" + QString::number(colorBase.red()) + "," + QString::number(colorBase.green()) + "," + QString::number(colorBase.blue()) + "," + QString::number(colorBase.alpha()) + "}");
//    QTreeView::paintEvent(e);
//}


SideBarItemDelegate::SideBarItemDelegate(QObject *parent)
{
//    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
//        auto settings = LINGMOStyleSettings::globalInstance();
//        connect(settings, &QGSettings::changed, this, [=](const QString &key){
//            pDebug << "key changed:" << key;
//            if (key == "styleName") {
//                pDebug << "key changed....";
//                this->paint();
//                this->update();
//            }
//        });
//    }
}

//QSize SideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    auto size = QStyledItemDelegate::sizeHint(option, index);
//    size.setHeight(36);
//    return size;
//}


void SideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    if (index.column() == 1) {
        QPainterPath rightRoundedRegion;
        rightRoundedRegion.setFillRule(Qt::WindingFill);
        auto rect = option.rect;
        auto view = qobject_cast<const QAbstractItemView *>(option.widget);
        if (view) {
            rect.setRight(view->viewport()->rect().right());
        }
        int radius = 4;
        if(view && view->property("minRadius").isValid())
            radius = view->property("minRadius").toInt();

        rightRoundedRegion.addRoundedRect(rect, radius, radius);
        rightRoundedRegion.addRect(rect.adjusted(0, 0, -1 * radius, 0));
        painter->setClipPath(rightRoundedRegion);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();
}

static SideBarStyle *global_instance = nullptr;
SideBarStyle::SideBarStyle()
{

}

SideBarStyle* SideBarStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new SideBarStyle;
    }
    return global_instance;
}

void SideBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    switch (element) {
    case QStyle::PE_IndicatorItemViewItemDrop: {
        painter->setRenderHint(QPainter::Antialiasing, true);/* 反锯齿 */
        /* 按设计要求，边框颜色为调色板highlight值，圆角为6px */
        QColor color = option->palette.color(QPalette::Highlight);
        painter->setPen(color);
        int radius = 6;
        if(widget && widget->property("normalRadius").isValid())
            radius = widget->property("normalRadius").toInt();

        painter->drawRoundedRect(option->rect, radius, radius);
        painter->restore();
        return;
    }

    case QStyle::PE_IndicatorBranch: {
        painter->setRenderHint(QPainter::Antialiasing, true);
        if (option->rect.x() == 0) {
            QPainterPath leftRoundedRegion;
            leftRoundedRegion.setFillRule(Qt::WindingFill);
            int radius = 4;
            if(widget && widget->property("minRadius").isValid())
                radius = widget->property("minRadius").toInt();

            leftRoundedRegion.addRoundedRect(option->rect, radius, radius);
            leftRoundedRegion.addRect(option->rect.adjusted(radius, 0, 0, 0));
            painter->setClipPath(leftRoundedRegion);
        }
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
//                opt.palette.setColor(QPalette::Highlight, Qt::red);
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
//                opt.palette.setColor(QPalette::Highlight, Qt::green);
            }
        }
        qApp->style()->drawPrimitive(element, &opt, painter, widget);
        painter->restore();
        return;
    }
    case QStyle::PE_PanelItemViewRow: {
        painter->restore();
        return;
        break;
    }
    case QStyle::PE_PanelItemViewItem: {
        /*
        const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *vi;
        painter->save();
        painter->setPen(Qt::NoPen);
        QPainterPath path;
        if (qobject_cast<const QTreeView*>(widget) || qobject_cast<const QTreeWidget*>(widget)) {
            path.addRoundedRect(vi->rect, 6, 6);
        }
        else {
            path.addRect(vi->rect);
        }
//                pDebug << "vi->rect:" << vi->rect;

        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
            }
        }
        painter->drawPath(path);
        */
        break;
    }

    default:
        break;
    }

    qApp->style()->drawPrimitive(element, option, painter, widget);
    painter->restore();
}

void SideBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::CE_ItemViewItem) {
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
            }
        }
        return qApp->style()->drawControl(element, &opt, painter, widget);
    }
}
