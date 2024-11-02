/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "musicsearchlistview.h"
#include "musicsearchlistmodel.h"
#include "musicsearchlistdelegate.h"

#include <QTouchEvent>
#include <QDebug>
#include <QFileInfo>

#define NUM 10000
//DWIDGET_USE_NAMESPACE

MusicSearchListview::MusicSearchListview(QWidget *parent)
    : QListView(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFrameShape(QListView::NoFrame);
    m_model = new MusicSearchListModel(3, 3, this);
    setModel(m_model);
    m_delegate = new MusicSearchListDelegate(this);
    setItemDelegate(m_delegate);
    setUniformItemSizes(true);

    this->setFocusPolicy(Qt::NoFocus);
    setViewMode(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_delegate, &MusicSearchListDelegate::SearchClear, this, &MusicSearchListview::SearchClear);
    connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotOnClicked(const QModelIndex &)));
}


MusicSearchListview::~MusicSearchListview()
{

}

SearchType MusicSearchListview::getSearchType() const
{
    return m_searchType;
}

void MusicSearchListview::setSearchResultWidget(SearchResult *result)
{
    m_SearchResultWidget = result;
}

void MusicSearchListview::setSearchType(SearchType type)
{
    m_searchType = type;
}

void MusicSearchListview::setSearchText(QString text)
{
    m_CurrentIndex = -1;
    m_searchText = text;
    Q_EMIT signalSearchTexts(m_searchText);

    QList<musicDataStruct> musicDataInfos;
    QList<musicDataStruct> singerInfos;
    QList<musicDataStruct> albumInfos;
    g_db->getCurtEstimatedListByKeyword(text,NUM,musicDataInfos,singerInfos,albumInfos);
    if (m_searchType == SearchType::TitleType) {
        m_MusicDataStruct.clear();
        for (int i = 0; i < musicDataInfos.count(); i++) {
            musicDataStruct titleList = musicDataInfos.at(i);
            m_MusicDataStruct.append(titleList);
        }

        m_model->clear();
        for (musicDataStruct musicData : m_MusicDataStruct) {
            QStandardItem *item = new QStandardItem;
            QString Str = musicData.title + "-" + musicData.singer;
            item->setData(Str,Qt::ToolTipRole);
            m_model->appendRow(item);

            auto row = m_model->rowCount() - 1;
            QModelIndex index = m_model->index(row, 0, QModelIndex());

            QVariant mediaMeta;
            mediaMeta.setValue(musicData);
            m_model->setData(index, mediaMeta, Qt::UserRole + SearchType::TitleType);

//            QFont sizeFont;
//            sizeFont.setPixelSize(12);
//            QFontMetrics fm(sizeFont);
//            QString Str = musicData.title + "-" + musicData.singer;
//            QString displayStr = fm.elidedText(Str,Qt::ElideRight,136);
//            item->setData(displayStr, Qt::DisplayRole);
//            item->setData(Str,Qt::ToolTipRole);
//            m_model->appendRow(item);
        }
    } else if (m_searchType == SearchType::SingerType) {
        m_SingerInfos.clear();
        for (int i = 0; i < singerInfos.count(); i++) {
            musicDataStruct singerList = singerInfos.at(i);
            m_SingerInfos.append(singerList);
        }

        m_model->clear();
        for (musicDataStruct singerData : m_SingerInfos) {
            QStandardItem *item = new QStandardItem;
            QString Str = singerData.title + "-" + singerData.singer;
            item->setData(Str,Qt::ToolTipRole);
            m_model->appendRow(item);

            auto row = m_model->rowCount() - 1;
            QModelIndex index = m_model->index(row, 0, QModelIndex());

            QVariant mediaMeta;
            mediaMeta.setValue(singerData);
            m_model->setData(index, mediaMeta, Qt::UserRole + SearchType::SingerType);
//            QFont sizeFont;
//            sizeFont.setPixelSize(12);
//            QFontMetrics fm(sizeFont);
//            QString Str = singerData.title + "-" + singerData.singer;
//            QString displayStr = fm.elidedText(Str,Qt::ElideRight,136);
//            item->setData(displayStr, Qt::DisplayRole);
//            item->setData(Str,Qt::ToolTipRole);
//            m_model->appendRow(item);
        }
    } else if (m_searchType == SearchType::AlbumType) {
        m_AlbumInfos.clear();
        for (int i = 0; i < albumInfos.count(); i++) {
            musicDataStruct albumList = albumInfos.at(i);
            m_AlbumInfos.append(albumList);
        }

        m_model->clear();
        for (musicDataStruct albumData : m_AlbumInfos) {
            QStandardItem *item = new QStandardItem;
            QString Str = albumData.album + "-" + albumData.singer;
            item->setData(Str,Qt::ToolTipRole);
            m_model->appendRow(item);

            auto row = m_model->rowCount() - 1;
            QModelIndex index = m_model->index(row, 0, QModelIndex());

            QVariant mediaMeta;
            mediaMeta.setValue(albumData);
            m_model->setData(index, mediaMeta, Qt::UserRole + SearchType::AlbumType);
//            QFont sizeFont;
//            sizeFont.setPixelSize(12);
//            QFontMetrics fm(sizeFont);
//            QString Str = albumData.album + "-" + albumData.singer;
//            QString displayStr = fm.elidedText(Str,Qt::ElideRight,136);
//            item->setData(displayStr, Qt::DisplayRole);
//            item->setData(Str,Qt::ToolTipRole);
//            m_model->appendRow(item);
        }
    }
}

int MusicSearchListview::rowCount()
{
    return m_model->rowCount();
}

int MusicSearchListview::getIndexInt()const
{
    return m_CurrentIndex;
}

QString MusicSearchListview::getSearchText()const
{
    return m_searchText;
}

void MusicSearchListview::setCurrentIndexInt(int row)
{
    m_CurrentIndex = row;
}

void MusicSearchListview::switchToSearchResultTab(const QModelIndex &index)
{
//    if (m_SearchType == SearchType::SearchMusic) {
//        MediaMeta mediaMeta = index.data(Qt::UserRole + SearchType::SearchMusic).value<MediaMeta>();
//        Q_EMIT CommonService::getInstance()->signalSwitchToView(SearchMusicResultType, mediaMeta.title);
//        if (m_SearchResultWidget) {
//            m_SearchResultWidget->setLineEditSearchString(mediaMeta.title);
//        }
//    } else if (m_SearchType == SearchType::SearchSinger) {
//        SingerInfo singerInfo = index.data(Qt::UserRole + SearchType::SearchSinger).value<SingerInfo>();
//        Q_EMIT CommonService::getInstance()->signalSwitchToView(SearchSingerResultType, singerInfo.singerName);
//        if (m_SearchResultWidget) {
//            m_SearchResultWidget->setLineEditSearchString(singerInfo.singerName);
//        }
//    } else if (m_SearchType == SearchType::SearchAlbum) {
//        AlbumInfo albumInfo = index.data(Qt::UserRole + SearchType::SearchAlbum).value<AlbumInfo>();
//        Q_EMIT CommonService::getInstance()->signalSwitchToView(SearchAlbumResultType, albumInfo.albumName);
//        if (m_SearchResultWidget) {
//            m_SearchResultWidget->setLineEditSearchString(albumInfo.albumName);
//        }
//    }
}

void MusicSearchListview::SearchClear()
{
    Q_EMIT this->sigSearchClear();
}

void MusicSearchListview::slotOnClicked(const QModelIndex &index)
{
    switchToSearchResultTab(index);
}

void MusicSearchListview::onReturnPressed()
{
    qDebug() << "------MusicSearchListview::onReturnPressed";
    slotOnClicked(m_model->index(m_CurrentIndex, 0));
}

void MusicSearchListview::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "MusicSearchListview::mouseMoveEvent";
}

bool MusicSearchListview::event(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        QTouchEvent *touch = static_cast<QTouchEvent *>(event);
        if (touch && touch->touchPoints().size() > 0) {
            QModelIndex index = this->indexAt(touch->touchPoints().at(0).pos().toPoint());
            switchToSearchResultTab(index);
        }
    }
    return QListView::event(event);
}
