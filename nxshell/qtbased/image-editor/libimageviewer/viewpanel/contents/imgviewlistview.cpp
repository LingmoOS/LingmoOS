// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imgviewlistview.h"

#include <math.h>

#include <QDebug>
#include <QDrag>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeData>
#include <QScrollBar>
#include <QMutex>
#include <QScroller>

#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "imageengine.h"
#include "service/commonservice.h"
#include "accessibility/ac-desktop-define.h"

LibImgViewListView::LibImgViewListView(QWidget *parent)
    :  DListView(parent)
{
    m_model = new QStandardItemModel(this);
    m_delegate = new LibImgViewDelegate(this);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSpacing(ITEM_SPACING);
    setDragEnabled(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    QListView::setFlow(QListView::LeftToRight);
    QListView::setWrapping(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->verticalScrollBar()->setDisabled(true); // 禁用滚动
//    setAttribute(Qt::WA_TransparentForMouseEvents, true);
//    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);

//    setUniformItemSizes(true);

    setItemDelegate(m_delegate);
    setModel(m_model);
//    installEventFilter(viewport());

//    setMouseTracking(true);
//    this->viewport()->setMouseTracking(true);
    connect(ImageEngine::instance(), &ImageEngine::sigOneImgReady, this, &LibImgViewListView::slotOneImgReady, Qt::QueuedConnection);
}

LibImgViewListView::~LibImgViewListView()
{
    qDebug() << "~-------------------ImgViewListView";
}

void LibImgViewListView::setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path)
{
    qDebug() << "---" << __FUNCTION__ << "---path = " << path;
    m_model->clear();
    m_currentPath = path;
    int count = itemInfos.size();
    for (int i = 0; i < count; i++) {
        imageViewerSpace::ItemInfo info = itemInfos.at(i);
        if (info.path == path) {
            info.imgWidth = ITEM_CURRENT_WH;
            info.imgHeight = ITEM_CURRENT_WH;
            m_currentRow = i;
        } else {
            info.imgWidth = ITEM_NORMAL_WIDTH;
            info.imgHeight = ITEM_NORMAL_HEIGHT;
        }
        QStandardItem *item = new QStandardItem;
        QVariant infoVariant;
        infoVariant.setValue(info);
        item->setData(infoVariant, Qt::DisplayRole);

        item->setData(QVariant(QSize(info.imgWidth, info.imgHeight)), Qt::SizeHintRole);
        m_model->appendRow(item);
    }

    doItemsLayout();

    this->setFixedSize((2 * (count + 1) + ITEM_NORMAL_WIDTH * count + ITEM_CURRENT_WH - ITEM_NORMAL_WIDTH), 80);
}

int LibImgViewListView::getSelectIndexByPath(QString path)
{
    int index = -1;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex itemIndex = m_model->index(i, 0);
        imageViewerSpace::ItemInfo info = itemIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (info.path == path) {
            return i;
        }
    }
    return index;
}

void LibImgViewListView::setSelectCenter()
{
    QModelIndex itemIndex = m_model->index(m_currentRow, 0);
    QRect rect = this->visualRect(itemIndex);
    this->horizontalScrollBar()->setValue(rect.x());
}

void LibImgViewListView::openNext()
{
    if (m_currentRow == (m_model->rowCount() - 1)) {
        return;
    }

    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    QModelIndex nextIndex = m_model->index((m_currentRow + 1), 0);
    if (!nextIndex.isValid()) {
        return;
    }

    imageViewerSpace::ItemInfo info = nextIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    if (info.path.isEmpty()) {
        return;
    }

    if (currentIndex.isValid()) {
        //重置前一个选中项的宽高
        m_model->setData(currentIndex,
                         QVariant(QSize(LibImgViewListView::ITEM_NORMAL_WIDTH, LibImgViewListView::ITEM_NORMAL_HEIGHT)), Qt::SizeHintRole);
    }

    if (nextIndex.isValid()) {
        //重置新选中项的宽高
        m_model->setData(nextIndex,
                         QVariant(QSize(LibImgViewListView::ITEM_CURRENT_WH, LibImgViewListView::ITEM_CURRENT_WH)), Qt::SizeHintRole);

    }
    doItemsLayout();

    m_currentRow = m_currentRow + 1;
    m_currentPath = info.path;

    loadFiftyRight();

    startMoveToLeftAnimation();

    emit openImg(m_currentRow, m_currentPath);
}

void LibImgViewListView::openPre()
{
    if (m_currentRow <= 0) {
        return;
    }

    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    QModelIndex preIndex = m_model->index((m_currentRow - 1), 0);
    if (!preIndex.isValid()) {
        return;
    }

    imageViewerSpace::ItemInfo info = preIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    if (info.path.isEmpty()) {
        return;
    }

    if (currentIndex.isValid()) {
        //重置前一个选中项的宽高
        m_model->setData(currentIndex,
                         QVariant(QSize(LibImgViewListView::ITEM_NORMAL_WIDTH, LibImgViewListView::ITEM_NORMAL_HEIGHT)), Qt::SizeHintRole);
    }

    if (preIndex.isValid()) {
        //重置新选中项的宽高
        m_model->setData(preIndex,
                         QVariant(QSize(LibImgViewListView::ITEM_CURRENT_WH, LibImgViewListView::ITEM_CURRENT_WH)), Qt::SizeHintRole);

    }
    doItemsLayout();

    m_currentRow = m_currentRow - 1;
    m_currentPath = info.path;

    emit openImg(m_currentRow, m_currentPath);
}

void LibImgViewListView::removeCurrent()
{
    //当前显示数量大于3
    if (m_model->rowCount() > 1) {
        //删除最后一个,继续显示第一张
        qDebug() << "---" << __FUNCTION__ << "---m_currentRow = " << m_currentRow;
        qDebug() << "---" << __FUNCTION__ << "---m_model->rowCount() = " << m_model->rowCount();
        if (m_currentRow == (m_model->rowCount() - 1)) {
            QModelIndex index;
            if (m_currentRow >= 1) {
                index = m_model->index(m_currentRow - 1, 0);
            } else {
                index = m_model->index(0, 0);
            }

            onClicked(index);
            m_model->removeRow(m_model->rowCount() - 1);
            if (m_model->rowCount() == 1) {
                this->horizontalScrollBar()->setValue(0);
            }
        } else {
            //显示下一张
            QModelIndex index = m_model->index((m_currentRow + 1), 0);
            onClicked(index);
            m_currentRow = m_currentRow - 1;
            //m_currentRow在onClicked中已经被修改了
            m_model->removeRow((m_currentRow));
        }
    } else if (m_model->rowCount() == 1) {
        //数量只有一张
        m_model->clear();
        m_currentRow = -1;
        m_currentPath = "";
    }
}

void LibImgViewListView::setCurrentPath(const QString &path)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        imageViewerSpace::ItemInfo data = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (data.path == path) {
            onClicked(index);
        }
    }
}

QStringList LibImgViewListView::getAllPath()
{
    QStringList list;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        imageViewerSpace::ItemInfo data = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        list << data.path;
    }
    return list;
}

int LibImgViewListView::getCurrentItemX()
{
    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
//    qDebug() << this->visualRect(currentIndex).x();
//    qDebug() << this->visualRect(currentIndex).x();
//    qDebug() << 2 * (count + 1) + ITEM_NORMAL_WIDTH *count + ITEM_CURRENT_WH - ITEM_NORMAL_WIDTH;
//    qDebug() << (2 * (m_currentRow + 1) + ITEM_NORMAL_WIDTH * m_currentRow + ITEM_CURRENT_WH - ITEM_NORMAL_WIDTH);
//    qDebug() << this->visualRect(currentIndex).x();
    return this->visualRect(currentIndex).x();
}

int LibImgViewListView::getRowWidth()
{
    int count = m_model->rowCount();
    return 2 * (count + 1) + ITEM_NORMAL_WIDTH * count + ITEM_CURRENT_WH - ITEM_NORMAL_WIDTH;
}



//void ImgViewListView::mousePressEvent(QMouseEvent *event)
//{
//    qDebug() << "mousePressEvent";
//    return DListView::mousePressEvent(event);
//}

//void ImgViewListView::mouseMoveEvent(QMouseEvent *event)
//{
//    qDebug() << "mouseMoveEvent";
//    return DListView::mouseMoveEvent(event);
//}

//void ImgViewListView::mouseReleaseEvent(QMouseEvent *event)
//{
//    qDebug() << "mouseReleaseEvent";
//    return DListView::mouseReleaseEvent(event);
//}

void LibImgViewListView::slotOneImgReady(QString path, imageViewerSpace::ItemInfo pix)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        imageViewerSpace::ItemInfo data = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (data.path == path) {
            pix.imgWidth = data.imgWidth;
            pix.imgHeight = data.imgHeight;

            // 更新文件信息
            QVariant meta;
            meta.setValue(pix);
            m_model->setData(index, meta, Qt::DisplayRole);

            // 判断当前的文件路径是否变更(重命名时)，若变更则更新
            if (path == m_currentPath && pix.path != m_currentPath) {
                m_currentPath = pix.path;
            }

            this->update(index);
            this->viewport()->update();
            break;
        }
    }
}

void LibImgViewListView::slotCurrentImgFlush(QPixmap pix, const QSize &originalSize)
{
    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    imageViewerSpace::ItemInfo data = currentIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();

    data.imgOriginalWidth = originalSize.width();
    data.imgOriginalHeight = originalSize.height();
    data.image = pix.toImage();

    // FIX:暂时修复，用于解决某些场景下初始化，图片全数据还未处理完成（多线程处理）
    // data.imageType 还未设置的情况
    if (data.imageType == imageViewerSpace::ImageTypeBlank) {
        data.imageType = LibUnionImage_NameSpace::getImageType(data.path);
    }

    QVariant meta;
    meta.setValue(data);
    m_model->setData(currentIndex, meta, Qt::DisplayRole);

    LibCommonService::instance()->slotSetImgInfoByPath(data.path, data);
    this->update(currentIndex);
    this->viewport()->update();
}

void LibImgViewListView::onClicked(const QModelIndex &index)
{
    //重新点击,m_currentPath需要重新赋值
    imageViewerSpace::ItemInfo info = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    m_currentPath = info.path;
    if (index.row() == m_currentRow) {
        return;
    }

    if (info.path.isEmpty()) {
        return;
    }

    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    if (currentIndex.isValid()) {
        //重置前一个选中项的宽高
        m_model->setData(currentIndex,
                         QVariant(QSize(LibImgViewListView::ITEM_NORMAL_WIDTH, LibImgViewListView::ITEM_NORMAL_HEIGHT)), Qt::SizeHintRole);
    }
    //重置新选中项的宽高
    m_model->setData(index,
                     QVariant(QSize(LibImgViewListView::ITEM_CURRENT_WH, LibImgViewListView::ITEM_CURRENT_WH)), Qt::SizeHintRole);

    m_currentRow = index.row();
    qDebug() << "---" << __FUNCTION__ << "---m_currentRow = " << m_currentRow;
    qDebug() << "---" << __FUNCTION__ << "---info.path = " << info.path;
    //刷新界面
    doItemsLayout();
    //如果点击的是最后一个则向前移动
    startMoveToLeftAnimation();
    //提前加载后面图片缩略图
    loadFiftyRight();

    emit openImg(m_currentRow, m_currentPath);
}

void LibImgViewListView::cutPixmap(imageViewerSpace::ItemInfo &iteminfo)
{
    int width = iteminfo.image.width();
    if (width == 0)
        width = 180;
    int height = iteminfo.image.height();
    if (abs((width - height) * 10 / width) >= 1) {
        QRect rect = iteminfo.image.rect();
        int x = rect.x() + width / 2;
        int y = rect.y() + height / 2;
        if (width > height) {
            x = x - height / 2;
            y = 0;
            iteminfo.image = iteminfo.image.copy(x, y, height, height);
        } else {
            y = y - width / 2;
            x = 0;
            iteminfo.image = iteminfo.image.copy(x, y, width, width);
        }
    }
}
//加载后50张
void LibImgViewListView::loadFiftyRight()
{
    int count = 0;
    for (int i = m_currentRow; i < m_model->rowCount(); i++) {
        count++;
        QModelIndex indexImg = m_model->index(i, 0);
        imageViewerSpace::ItemInfo infoImg = indexImg.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (infoImg.image.isNull()) {
//            if (!ImageEngineApi::instance()->m_imgLoaded.contains(infoImg.path)) {
//                emit ImageEngineApi::instance()->sigLoadThumbnailIMG(infoImg.path);
//                ImageEngineApi::instance()->m_imgLoaded.append(infoImg.path);
//            }
        }
        if (count == 50) {
            break;
        }
    }
}

void LibImgViewListView::startMoveToLeftAnimation()
{
    if (m_moveAnimation == nullptr) {
        m_moveAnimation = new QPropertyAnimation(this->horizontalScrollBar(), "value", this);
    }

    m_moveAnimation->setDuration(100);
    m_moveAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_moveAnimation->setStartValue(this->horizontalScrollBar()->value());
    m_moveAnimation->setEndValue((this->horizontalScrollBar()->value() + 32));
    //如果点击的是最后一个则向左滑动
    QRect rect = this->visualRect(m_model->index(m_currentRow, 0));
    if ((rect.x() + 52) >= (this->width() - 32)) {
        if (m_moveAnimation->state() == QPropertyAnimation::State::Running) {
            m_moveAnimation->stop();
        }
        m_moveAnimation->start();
    }
}

const QString LibImgViewListView::getPathByRow(int row)
{
    QString result;
    if (row <= (m_model->rowCount() - 1)) {
        QModelIndex index = m_model->index(row, 0);
        if (index.isValid()) {
            imageViewerSpace::ItemInfo info = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
            result = info.path;
        }
    }
    return result;
}
