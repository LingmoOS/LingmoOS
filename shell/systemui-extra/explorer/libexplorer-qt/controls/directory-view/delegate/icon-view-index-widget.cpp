/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "icon-view-index-widget.h"
#include "icon-view-delegate.h"
#include "icon-view.h"

#include <QPainter>
#include <QPaintEvent>
#include <QLabel>

#include <QApplication>
#include <QStyle>
#include <QTextDocument>
#include <QScrollBar>

#include <QMouseEvent>
#include <QStyleHints>

#include "file-info.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-item.h"
#include "file-utils.h"
#include "emblem-provider.h"

#include <QDebug>
#include <QTextLayout>

using namespace Peony;
using namespace Peony::DirectoryView;

IconViewIndexWidget::IconViewIndexWidget(const IconViewDelegate *delegate, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *parent) : QWidget(parent)
{
    setObjectName("explorer_icon_view_index_widget");
    installEventFilter(parent);

    setMouseTracking(true);

    m_edit_trigger.setInterval(3000);
    m_edit_trigger.setSingleShot(true);

    QTimer::singleShot(750, this, [=]() {
        m_edit_trigger.start();
    });

    //use QTextEdit to show full file name when select
    m_edit = new QTextEdit();

    m_option = option;
    m_index = index;

    m_delegate = delegate;

//    m_delegate->getView()->m_renameTimer->stop();
//    m_delegate->getView()->m_editValid = false;
//    m_delegate->getView()->m_renameTimer->start();

    m_is_dragging = m_delegate->getView()->isDraggingState();

    QSize size = delegate->sizeHint(option, index);
    setMinimumSize(size);

    //extra emblems
    auto proxy_model = static_cast<FileItemProxyFilterSortModel*>(delegate->getView()->model());
    auto item = proxy_model->itemFromIndex(index);
    if (item) {
        m_info = item->info();
    }

    m_delegate->initStyleOption(&m_option, m_index);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    m_option.features.setFlag(QStyleOptionViewItem::WrapText);
#else
    m_option.features |= QStyleOptionViewItem::WrapText;
#endif
    m_option.textElideMode = Qt::ElideNone;

    auto opt = m_option;
    //opt.rect.setHeight(9999);
    opt.rect.moveTo(0, 0);

    //qDebug()<<m_option.rect;
    auto iconExpectedSize = m_delegate->getView()->iconSize();
    QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    auto y_delta = iconExpectedSize.height() - iconRect.height();
    opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);

    m_option = opt;

    adjustPos();

    auto textSize = IconViewTextHelper::getTextSizeForIndex(opt, index, 2);
    int fixedHeight = 5 + iconExpectedSize.height() + 5 + textSize.height() + 5;
    int y_bottom = option.rect.y() + fixedHeight + 20;
    //qDebug() << "Y:" <<option.rect.y() <<fixedHeight <<m_delegate->getView()->height();
    b_elide_text = false;
    if ( y_bottom > m_delegate->getView()->height() && opt.text.length() > ELIDE_TEXT_LENGTH)
    {
        b_elide_text = true;
        int  charWidth = opt.fontMetrics.averageCharWidth();
        opt.text = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
        //recount size
        textSize = IconViewTextHelper::getTextSizeForIndex(opt, index, 2);
        fixedHeight = 5 + iconExpectedSize.height() + 5 + textSize.height() + 5;
    }

    if (fixedHeight >= option.rect.height())
        setFixedHeight(fixedHeight);
    else
        setFixedHeight(option.rect.height());

    m_option.rect.setHeight(fixedHeight - y_delta);

    connect(m_delegate, &IconViewDelegate::updateIndexWidget, this, [=](const QStyleOptionViewItem &option){
        m_option = option;
        m_delegate->initStyleOption(&m_option, m_index);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
        m_option.features.setFlag(QStyleOptionViewItem::WrapText);
#else
        m_option.features |= QStyleOptionViewItem::WrapText;
#endif
        m_option.textElideMode = Qt::ElideNone;

        auto opt = m_option;
        opt.rect.moveTo(0, 0);

        auto iconExpectedSize = m_delegate->getView()->iconSize();
        QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
        auto y_delta = iconExpectedSize.height() - iconRect.height();
        opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);
        m_option = opt;

        m_option.rect.setHeight(fixedHeight - y_delta);
        update();
    });

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    connect(qApp, &QApplication::fontChanged, this, [=]() {
        m_delegate->getView()->setIndexWidget(m_index, nullptr);
    });
#else
    //FIXME: handle font change.
#endif
}

IconViewIndexWidget::~IconViewIndexWidget()
{
    delete m_edit;
}

bool IconViewIndexWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        adjustPos();
        update();
    }
    return false;
}

void IconViewIndexWidget::clearModelIndex()
{
    m_delegate->getView()->setIndexWidget(m_index, nullptr);
}

void IconViewIndexWidget::paintEvent(QPaintEvent *e)
{
    bool isUpdateEnable = updatesEnabled();
    if (!isUpdateEnable)
        return;

    if (!m_info.lock()) {
        return;
    }

    IconView *view = m_delegate->getView();
    auto expectedRect = view->visualRect(m_index);
    auto rectPos = this->mapToParent(QPoint(0, 0));
    if (expectedRect.topLeft() != rectPos) {
        adjustPos();
        update();
        return;
    }

    QWidget::paintEvent(e);
    QPainter p(this);
//    p.fillRect(0, 0, 999, 999, qApp->palette().base());

    //adjustPos();
    auto bgColor = QApplication::palette().base().color();
    p.save();
    p.setPen(Qt::transparent);
    p.setBrush(bgColor);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    //qDebug()<<m_option.backgroundBrush;
    //qDebug()<<this->size() << m_delegate->getView()->iconSize();

    auto model = static_cast<FileItemProxyFilterSortModel*>(view->model());
    auto item = model->itemFromIndex(m_index);

#ifdef LINGMO_UDF_BURN
    if (item) {
        /* R类型光盘，所有用于刻录的文件（夹）展示在挂载点时都应该半透明显示，区别于普通文件 ,linkto task#122470 */
        if(item->property("isFileForBurning").toBool()){
            p.setOpacity(0.5);
        }else{
            p.setOpacity(1.0);
        }
    }//end
#endif

    auto opt = m_option;
    auto rawRect = m_option.rect;
    opt.rect = this->rect();

    int horizalMargin = 2;
    auto fontMetrics = opt.fontMetrics;
    int pixelsWide = fontMetrics.width(opt.text);
    int width = opt.rect.width() - 2*horizalMargin;

    if(pixelsWide < width){
       opt.rect = opt.rect.adjusted(0,0,0,-31);
    }

    opt.palette = QApplication::palette();
    //p.fillRect(opt.rect, m_delegate->selectedBrush());
    auto rawDecoSize = opt.decorationSize;
    opt.decorationSize = m_delegate->getView()->iconSize();
    opt.state &= ~QStyle::State_MouseOver;
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem,
                                         &opt,
                                         &p,
                                         nullptr);
    opt.decorationSize = rawDecoSize;

    opt.rect = rawRect;
    auto tmp = opt.text;
    opt.text = nullptr;
    //bug#99340,修改图标选中状态，会变暗
    auto state = opt.state;
    if((opt.state & QStyle::State_Enabled) && (opt.state & QStyle::State_Selected)) {
        opt.state &= ~QStyle::State_Selected;
    }
    p.save();
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, opt.widget);
    p.restore();
    opt.state = state;
    if (b_elide_text)
    {
        int  charWidth = opt.fontMetrics.averageCharWidth();
        tmp = opt.fontMetrics.elidedText(tmp, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }
    opt.text = std::move(tmp);

    //extra emblems
    auto info = m_info.lock();

    // draw color symbols
    if(info->uri().startsWith("favorite://")){/* 快速访问须特殊处理 */
        info = FileInfo::fromUri(FileUtils::getEncodedUri(FileUtils::getTargetUri(info->uri())));
    }
    auto colors = info->getColors();
    auto lineSpacing = opt.fontMetrics.lineSpacing();
    int yoffset = 0;
    int iLine = 0;

    // draw color symbols
    if(info->uri().startsWith("favorite://")){/* 快速访问须特殊处理 */
        //快速访问目录，颜色标记设置后更新不及时问题单独处理,修复bug#118015
        auto matchInfo = FileInfo::fromUri(FileUtils::getEncodedUri(FileUtils::getTargetUri(info->uri())));
        colors = matchInfo->getColors();
    }
    int xoffset = 0;
    if(0 < colors.count())
    {
        const int MAX_LABEL_NUM = 3;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num =  colors.count() - startIndex;

        QString text = opt.text;
        QFont font = opt.font;
        QTextLayout textLayout(text, font);

        QTextOption textOpt;
        textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

        textLayout.setTextOption(textOpt);
        textLayout.beginLayout();


        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            return;
        int width = opt.rect.width() - (num+1)*6 - 2*2 - 4;
        line.setLineWidth(width);

        xoffset = (width - line.naturalTextWidth())/2 ;
        if(xoffset < 0)
        {
            xoffset = 2;
        }
        yoffset = (lineSpacing -12 )/2+2;

        for (int i = startIndex; i < colors.count(); ++i) {
            auto color = colors.at(i);
            p.save();
            //fix bug#147348
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.translate(0, m_delegate->getView()->iconSize().height() + 5);

           // p.translate(2, 2);
            p.setPen(opt.palette.highlightedText().color());
            p.setBrush(color);
            p.drawEllipse(QRectF(xoffset, yoffset, 12, 12));
            p.restore();
            xoffset += 12/2;
        }

        yoffset = 0;
        xoffset += 10;
    }
    QString regFindKeyWords = m_delegate->getRegFindKeyWords();
    p.save();
    p.translate(0, m_delegate->getView()->iconSize().height() + 5 + yoffset);
    p.setPen(opt.palette.highlightedText().color());

    qreal textHeight = IconViewTextHelper::drawText(&p,
                                                     opt,
                                                     9999,
                                                     xoffset,
                                                     regFindKeyWords,
                                                     2,
                                                     4);

    //fix#bug182191 【文件管理器】文件添加标记后选中状态名称显示不全
    int fixedHeight = 5 + m_delegate->getView()->iconSize().height() + 5 + textHeight + 5;
    if (fixedHeight > this->height())
        setFixedHeight(fixedHeight);

    p.restore();

    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left

    //paint symbolic link emblems
    if (info->isSymbolLink()) {
        emblemPoses.removeOne(3);
        QIcon icon = QIcon::fromTheme("emblem-link-symbolic");
        //qDebug()<< "symbolic:" << info->symbolicIconName();
        //icon.paint(&p, this->width() - 30, 10, 20, 20, Qt::AlignCenter);
        //Adjust link emblem to topLeft.link story#8354
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        icon.paint(&p, this->rect().x() + 10, m_delegate->getView()->iconSize().height() - 10, 20, 20, Qt::AlignCenter);
        p.restore();
    }
    if(view->isEnableMultiSelect())
    {
        QIcon icon = QIcon(":/icons/icon-selected.png");
        icon.paint(&p, this->width() - 20, 4, 16, 16, Qt::AlignCenter);
    }

    //paint access emblems
    //NOTE: we can not query the file attribute in smb:///(samba) and network:///.
    if (!info->uri().startsWith("file:")) {
        return;
    }

    auto rect = this->rect();
    if (!info->canRead()) {
        emblemPoses.removeOne(1);
        QIcon icon = QIcon::fromTheme("emblem-unreadable");
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        icon.paint(&p, rect.x() + 10, rect.y() + 10, 20, 20);
        p.restore();
    } else if (!info->canWrite()/* && !info->canExecute()*/) {
        //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
        emblemPoses.removeOne(1);
        QIcon icon = QIcon::fromTheme("emblem-readonly");
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        icon.paint(&p, rect.x() + 10, rect.y() + 10, 20, 20);
        p.restore();
    }

    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
        auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(info->uri());

        for (auto extensionsEmblem : extensionsEmblems) {
            if (emblemPoses.isEmpty()) {
                break;
            }

            QIcon icon = QIcon::fromTheme(extensionsEmblem);
            if (!icon.isNull()) {
                p.save();
                p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                int pos = emblemPoses.takeFirst();
                switch (pos) {
                case 1: {
                    icon.paint(&p, rect.x() + 10, rect.y() + 10, 20, 20, Qt::AlignCenter);
                    break;
                }
                case 2: {
                    icon.paint(&p, rect.x() + rect.width() - 30, rect.y() + 10, 20, 20, Qt::AlignCenter);
                    break;
                }
                case 3: {
                    icon.paint(&p, rect.x() + 10, m_delegate->getView()->iconSize().height() - 10, 20, 20, Qt::AlignCenter);
                    break;
                }
                case 4: {
                    icon.paint(&p, rect.right() - 30, m_delegate->getView()->iconSize().height() - 10, 20, 20, Qt::AlignCenter);
                    break;
                }
                default:
                    break;
                }
                p.restore();
            }
        }

}

void IconViewIndexWidget::mousePressEvent(QMouseEvent *e)
{

    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    if (singleClicked) {
        IconView *view = m_delegate->getView();
        if (!view->m_touch_active_timer->isActive()) {
            view->m_touch_active_timer->start(1100);
        }
    }

    if (e->button() == Qt::LeftButton) {
        IconView *view = m_delegate->getView();

        if (view->isDraggingState() || m_is_dragging || view->m_ctrl_key_pressed) {
            view->m_renameTimer->stop();
            view->m_editValid = false;
            return QWidget::mousePressEvent(e);
        }
        if (view->isEnableMultiSelect()) {
            //选中后再次点击会调用widget的mousePressEvent而不走iconview的mousePressEvent，导致m_mouse_release_unselect是false
            view->releaseUnselect(true);
            return;
        }

        //FIXME: Modify the icon style, only click on the text to respond, click on the icon to not respond
        QRect rect =  m_option.rect;
        QSize iconExpectedSize = m_delegate->getView()->iconSize();
        rect.setY(iconExpectedSize.height());
        rect.setHeight(m_option.rect.height()-iconExpectedSize.height());
        if(!rect.contains(e->pos()))
        {
            view->m_editValid = false;
            view->m_renameTimer->start();
            return ;
        }

        view->m_editValid = true;
        if (view->m_renameTimer->isActive()) {
            if (view->m_renameTimer->remainingTime() < 3000 - qApp->styleHints()->mouseDoubleClickInterval() && view->m_renameTimer->remainingTime() > 0) {
                view->slotRename();
            } else if(view->m_renameTimer->remainingTime() >= 3000 - qApp->styleHints()->mouseDoubleClickInterval()){
                //优化文件点击策略，提升用户体验，关联bug#125368
                //在双击时间间隔内，如果未触发双击事件，但是点击的是同一个有效图标，触发双击事件
                //系统默认双击间隔为400ms, 策略为[0,400]，触发双击，(400,3000)触发重命名
                mouseDoubleClickEvent(e);
            }
            else {
                view->m_editValid = false;
                view->m_renameTimer->stop();
            }
        } else {
            view->m_editValid = false;
            view->m_renameTimer->start();
        }
        e->ignore();
    }
    if(e->button() == Qt::RightButton){
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void IconViewIndexWidget::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
    //comment to fix click selected file name unselect file, when file has long name
    //m_is_dragging = true;
}

void IconViewIndexWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    //m_is_dragging = false;
}

void IconViewIndexWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    bool isPreviewMode = m_delegate->getView()->topLevelWidget()->property("isPreviewMode").toBool();
    if (!singleClicked || isPreviewMode) {
        m_delegate->getView()->activated(m_index);
    }
    return;
}

void IconViewIndexWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    hide();
}

void IconViewIndexWidget::adjustPos()
{
    IconView *view = m_delegate->getView();
    if (m_index.model() != view->model())
        return;

    if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
        this->close();
        return;
    }

    auto visualRect = view->visualRect(m_index);
    if (this->mapToParent(QPoint()) != visualRect.topLeft())
        this->move(visualRect.topLeft());
}
