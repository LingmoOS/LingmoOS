// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopreview_global.h"
#include "audiopreviewplugin.h"
#include "audiofileinfo.h"
#include "audioview.h"
#include "global/commontools.h"

#include <QFileInfo>
#include <QRect>
#include <QDateTime>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::audio_preview;

AudioPreviewPlugin::AudioPreviewPlugin(QObject *parent)
    : QObject (parent)
    , PreviewPlugin()
{

}

AudioPreviewPlugin::~AudioPreviewPlugin()
{
    if (m_audioView)
        m_audioView->deleteLater();
}

void AudioPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
    if (!m_audioView)
        m_audioView = new AudioView();
}

bool AudioPreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    m_audioView->setItemInfo(item);

    AudioFileInfo afi;
    AudioFileInfo::AudioMetaData amd = afi.openAudioFile(path);

    // 歌手 尾部截断
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Artist:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(amd.artist.isEmpty() ? "--" : amd.artist));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 专辑 尾部截断
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Album:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(amd.album.isEmpty() ? "--" : amd.album));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 时长
    if (amd.duration.isEmpty()) {
        AVFormatContext *avFmormat = avformat_alloc_context();
        avformat_open_input(&avFmormat, path.toStdString().c_str(), nullptr, nullptr);
        if (avFmormat) {
            avformat_find_stream_info(avFmormat, nullptr);
            qint64 dura = avFmormat->duration / (qint64)AV_TIME_BASE;
            if (dura >= 0)
                amd.duration = CommonTools::durationString(dura);
        }
        avformat_close_input(&avFmormat);
        avformat_free_context(avFmormat);
    }

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Duration:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(amd.duration.isEmpty() ? "--" : amd.duration));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 类型
    QFileInfo fileInfo(path);
    auto suffix = fileInfo.suffix();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Type:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(suffix.isEmpty() ? "--" : suffix));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 位置
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Location:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.absoluteFilePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 修改时间
    auto time = fileInfo.lastModified();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Time modified:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(time.toString(CommonTools::dateTimeFormat())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    m_item = item;
    return true;
}

ItemInfo AudioPreviewPlugin::item() const
{
    return m_item;
}

bool AudioPreviewPlugin::stopPreview()
{
    return true;
}

QWidget *AudioPreviewPlugin::contentWidget() const
{
    return m_audioView;
}

DetailInfoList AudioPreviewPlugin::getAttributeDetailInfo() const
{
    return m_detailInfos;
}

QWidget *AudioPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool AudioPreviewPlugin::showToolBar() const
{
    return true;
}
