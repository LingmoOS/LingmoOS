/*
 * Peony-Qt
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "icon-view-delegate.h"
#include "desktop-index-widget.h"

#include "desktop-icon-view-delegate.h"
#include "desktop-icon-view.h"
#include "file-info.h"
#include "emblem-provider.h"

#include <QPainter>
#include <QStyle>
#include <QApplication>

#include <QTextEdit>
#include <QTextOption>

#include <QMouseEvent>
#include <QStyleHints>

#include <QDebug>

//qt's global function
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace Peony;

DesktopIndexWidget::DesktopIndexWidget(DesktopIconViewDelegate *delegate,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index,
                                       QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    m_option = option;

    m_index = index;
    m_delegate = delegate;

    updateItem();

    //FIXME: how to handle it in old version?
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    connect(qApp, &QApplication::fontChanged, this, [=]() {
        m_delegate->getView()->setIndexWidget(m_index, nullptr);
    });
#endif

    auto view = m_delegate->getView();
    view->m_real_do_edit = false;
    view->m_edit_trigger_timer.stop();
    view->m_edit_trigger_timer.start();

}

DesktopIndexWidget::~DesktopIndexWidget()
{

}

bool DesktopIndexWidget::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::ApplicationFontChange:
    case QEvent::FontChange: {
        updateItem();
        break;
    }
    default:
        break;
    }
    return false;
}

void DesktopIndexWidget::paintEvent(QPaintEvent *e)
{
    auto view = m_delegate->getView();
    //qDebug()<<"paint";
    auto visualRect = m_delegate->getView()->visualRect(m_index);
    if (this->pos() != visualRect.topLeft()) {
        move(visualRect.topLeft());
        return;
    }

    if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
        view->m_real_do_edit = false;
        view->m_edit_trigger_timer.stop();
        this->close();
        return;
    }

    Q_UNUSED(e)
    QPainter p(this);
//    auto bgColor = m_option.palette.highlight().color();
    int radius = view->radius();
    auto bgColor = QApplication::palette().highlight().color();
    p.save();
    p.setPen(Qt::transparent);
    bgColor.setAlpha(255*0.7);
    p.setBrush(bgColor);
    p.drawRoundedRect(this->rect(), radius, radius);
    p.restore();

    //auto font = view->getViewItemFont(&m_option);

    auto opt = m_option;
    auto iconSizeExcepted = m_delegate->getView()->iconSize();
    auto iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    int y_delta = iconSizeExcepted.height() - iconRect.height();
    opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);

    //int maxTextHight = this->height() - iconSizeExcepted.height() - 10;
    int maxTextHight = view->viewport()->height() - this->geometry().y() - iconSizeExcepted.height() - 10;

    //setFixedHeight(opt.rect.height() + y_delta);

    // draw icon
    opt.text = nullptr;

    p.save();
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, m_delegate->getView());
    p.restore();

    p.save();
    p.translate(0, 5 + m_delegate->getView()->iconSize().height() + 5);

    if (b_elide_text)
    {
        int  charWidth = opt.fontMetrics.averageCharWidth();
        m_option.text = opt.fontMetrics.elidedText(m_option.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }
    // draw text shadow
    p.save();

    auto expectedSize = Peony::DirectoryView::IconViewTextHelper::getTextSizeForIndex(m_option, m_index, 0, 0);
    QPixmap pixmap(expectedSize);
    pixmap.fill(Qt::transparent);
    QPainter shadowPainter(&pixmap);
    shadowPainter.setPen(Qt::black);
    Peony::DirectoryView::IconViewTextHelper::paintText(&shadowPainter, m_option, m_index, maxTextHight, 0, 9999, false, Qt::black);
    shadowPainter.end();

    QImage shadowImage(expectedSize + QSize(4, 4), QImage::Format_ARGB32_Premultiplied);
    shadowImage.fill(Qt::transparent);
    shadowPainter.begin(&shadowImage);
    shadowPainter.drawPixmap(2, 2, pixmap);
    qt_blurImage(shadowImage, 8, false, false);

    for (int x = 0; x < shadowImage.width(); x++) {
        for (int y = 0; y < shadowImage.height(); y++) {
            auto color = shadowImage.pixelColor(x, y);
            if (color.alpha() > 0) {
                color.setAlphaF(qMin(color.alphaF() * 1.5, 1.0));
                shadowImage.setPixelColor(x, y, color);
            }
        }
    }

    shadowPainter.end();

    p.translate(-3, -1);
    p.drawImage(0, 0, shadowImage);

    p.restore();

    // draw text
    p.setPen(m_option.palette.highlightedText().color());
    p.setFont(qApp->font());
    Peony::DirectoryView::IconViewTextHelper::paintText(&p,
            m_option,
            m_index,
            maxTextHight,
            0,
            9999,
            false);

    p.restore();

    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left

    //paint link icon and locker icon
    FileInfo *file = FileInfo::fromUri(m_index.data(Qt::UserRole).toString()).get();
    if ((m_index.data(Qt::UserRole).toString() != "computer:///") && (m_index.data(Qt::UserRole).toString() != "trash:///")) {
        QSize lockerIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case DesktopIconView::Small: {
            lockerIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case DesktopIconView::Normal: {
            break;
        }
        case DesktopIconView::Large: {
            offset = 4;
            lockerIconSize = QSize(24, 24);
            break;
        }
        case DesktopIconView::Huge: {
            offset = 2;
            lockerIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }
        auto topRight = opt.rect.topRight();
        topRight.setX(topRight.x() - opt.rect.width() + 10);
        topRight.setY(topRight.y() + 10);
        auto linkRect = QRect(topRight, lockerIconSize);

        if (! file->canRead())
        {
            emblemPoses.removeOne(1);
            QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-unreadable");
            p.save();
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            symbolicLinkIcon.paint(&p, linkRect, Qt::AlignCenter);
            p.restore();
        }
        else if(! file->canWrite())
        {
            //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
            emblemPoses.removeOne(1);
            QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-readonly");
            p.save();
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            symbolicLinkIcon.paint(&p, linkRect, Qt::AlignCenter);
            p.restore();
        }
    }

    if (m_index.data(Qt::UserRole + 1).toBool()) {
        emblemPoses.removeOne(3);
        QSize symbolicIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case DesktopIconView::Small: {
            symbolicIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case DesktopIconView::Normal: {
            break;
        }
        case DesktopIconView::Large: {
            offset = 4;
            symbolicIconSize = QSize(24, 24);
            break;
        }
        case DesktopIconView::Huge: {
            offset = 2;
            symbolicIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }

        //Adjust link emblem to topLeft.link story#8354
        auto topLeft = opt.rect.topLeft();
        topLeft.setX(opt.rect.topLeft().x() + 10);
        topLeft.setY(opt.rect.topLeft().y() + offset + iconRect.height() - symbolicIconSize.height());
        auto linkRect = QRect(topLeft, symbolicIconSize);
        QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-link-symbolic");
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        symbolicLinkIcon.paint(&p, linkRect, Qt::AlignCenter);
        p.restore();
    }

    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
    auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(file->uri());
    for (auto extensionsEmblem : extensionsEmblems) {
        if (emblemPoses.isEmpty()) {
            break;
        }

        QIcon icon = QIcon::fromTheme(extensionsEmblem);

        QSize emblemsIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case DesktopIconView::Small: {
            emblemsIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case DesktopIconView::Normal: {
            break;
        }
        case DesktopIconView::Large: {
            offset = 4;
            emblemsIconSize = QSize(24, 24);
            break;
        }
        case DesktopIconView::Huge: {
            offset = 2;
            emblemsIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }

        if (!icon.isNull()) {
            int pos = emblemPoses.takeFirst();
            p.save();
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            switch (pos) {
            case 1: {
                icon.paint(&p,
                           opt.rect.topLeft().x() + 10,
                           opt.rect.topLeft().y() + 10,
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            case 2: {
                icon.paint(&p,
                           opt.rect.topRight().x() - offset - emblemsIconSize.width(),
                           opt.rect.topRight().y() + 10,
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            case 3: {
                icon.paint(&p,
                           opt.rect.topLeft().x() + 10,
                           opt.rect.topLeft().y() + offset + iconRect.height() - emblemsIconSize.height(),
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            case 4: {
                icon.paint(&p,
                           opt.rect.topRight().x() - offset - emblemsIconSize.width(),
                           opt.rect.topRight().y() + offset + iconRect.height() - emblemsIconSize.height(),
                           emblemsIconSize.width(),
                           emblemsIconSize.height(),
                           Qt::AlignCenter);
                break;
            }
            default:
                break;
            }
            p.restore();
        }
    }

    bgColor.setAlpha(255*0.8);
    p.setPen(bgColor);
    p.drawRoundedRect(this->rect().adjusted(0, 0, -1, -1), radius, radius);
}

void DesktopIndexWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        auto view = m_delegate->getView();
        view->m_real_do_edit = true;
        if (view->m_edit_trigger_timer.isActive()) {
            if (view->m_edit_trigger_timer.remainingTime() < 3000 - qApp->styleHints()->mouseDoubleClickInterval() && view->m_edit_trigger_timer.remainingTime() > 0) {
                QTimer::singleShot(300, this, [=]() {
                    if (view->m_real_do_edit) {
                        //not allow to edit special items:computer,trash and personal home path folder name
                        bool special_index = false;
                        QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                        special_index = (m_index.data(Qt::UserRole).toString() == "computer:///" ||
                                         m_index.data(Qt::UserRole).toString() == "trash:///" ||
                                         m_index.data(Qt::UserRole).toString() == homeUri);
                        if (special_index)
                            return ;

                        if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
                            view->m_real_do_edit = false;
                            view->m_edit_trigger_timer.stop();
                            this->close();
                            return;
                        }

                        view->setIndexWidget(m_index, nullptr);
                        view->edit(m_index);
                    }
                });
            } else if(view->m_edit_trigger_timer.remainingTime() >= 3000 - qApp->styleHints()->mouseDoubleClickInterval())
            {
                //优化文件点击策略，提升用户体验，关联bug#125368
                //在双击时间间隔内，如果未触发双击事件，但是点击的是同一个有效图标，触发双击事件
                //系统默认双击间隔为400ms, 策略为[0,400]，触发双击，(400,3000)触发重命名
                mouseDoubleClickEvent(event);
            }
            else {
                view->m_real_do_edit = false;
                view->m_edit_trigger_timer.stop();
            }
        } else {
            view->m_real_do_edit = false;
            view->m_edit_trigger_timer.start();
        }
        event->accept();
        return;
//        if (m_edit_trigger.isActive()) {
//            qDebug()<<"IconViewIndexWidget::mousePressEvent: edit"<<e->type();
//            m_delegate->getView()->setIndexWidget(m_index, nullptr);
//            m_delegate->getView()->edit(m_index);
//            return;
//        }
    }
    if(event->button() == Qt::RightButton){
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void DesktopIndexWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    auto view = m_delegate->getView();
    if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
        view->m_real_do_edit = false;
        view->m_edit_trigger_timer.stop();
        this->close();
        return;
    }

    m_delegate->getView()->activated(m_index);
    m_delegate->getView()->setIndexWidget(m_index, nullptr);
    return;
}

void DesktopIndexWidget::updateItem()
{
    auto view = m_delegate->getView();
    m_option = view->viewOptions();
    m_option.font = qApp->font();
    m_option.fontMetrics = qApp->fontMetrics();
    m_delegate->initStyleOption(&m_option, m_index);
    QSize size = m_delegate->sizeHint(m_option, m_index);
    auto visualRect = m_delegate->getView()->visualRect(m_index);
    auto rectCopy = visualRect;
    move(visualRect.topLeft());
    setFixedWidth(visualRect.width());

    m_option.rect.setWidth(visualRect.width());

    int rawHeight = size.height();
    auto textSize = Peony::DirectoryView::IconViewTextHelper::getTextSizeForIndex(m_option, m_index, 0, 0);
    int fixedHeight = 5 + m_delegate->getView()->iconSize().height() + 5 + textSize.height() + 10;

//    int y_bottom = rectCopy.y() + fixedHeight;
//    qDebug() << "Y:" <<rectCopy.y() <<fixedHeight <<m_delegate->getView()->height();
//    b_elide_text = false;
//    if ( y_bottom > m_delegate->getView()->height() && m_option.text.length() > ELIDE_TEXT_LENGTH)
//    {
//        b_elide_text = true;
//        int  charWidth = m_option.fontMetrics.averageCharWidth();
//        m_option.text = m_option.fontMetrics.elidedText(m_option.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
//        //recount size
//        textSize = Peony::DirectoryView::IconViewTextHelper::getTextSizeForIndex(m_option, m_index, 0, 0);
//        fixedHeight = 5 + m_delegate->getView()->iconSize().height() + 5 + textSize.height() + 10;
//    }

//    qDebug() << "updateItem fixedHeight:" <<fixedHeight <<rawHeight <<m_option.text;
    if (fixedHeight < rawHeight)
        fixedHeight = rawHeight;

    m_option.text = m_index.data().toString();
    //qDebug()<<m_option.text;
    //m_option.features.setFlag(QStyleOptionViewItem::WrapText);
    m_option.features |= QStyleOptionViewItem::WrapText;
    m_option.textElideMode = Qt::ElideNone;

    //m_option.rect.setHeight(9999);
    m_option.rect.moveTo(0, 0);

    //qDebug()<<m_option.rect;
    //auto font = view->getViewItemFont(&m_option);

    auto rawTextRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &m_option, m_option.widget);
    auto iconSizeExcepted = m_delegate->getView()->iconSize();

    auto iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &m_option, m_option.widget);

    auto y_delta = iconSizeExcepted.height() - iconRect.height();

    rawTextRect.setTop(iconRect.bottom() + y_delta + 5);
    rawTextRect.setHeight(9999);

    fixedHeight = qMin(view->viewport()->height() - this->geometry().y(), fixedHeight);

    setFixedHeight(fixedHeight);
}
