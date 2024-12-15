// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "video-preview/videopreview_global.h"
#include "video-preview/videopreviewplugin.h"
#include "video-preview/videoview.h"
#include "global/commontools.h"

#include <QTest>

#include <stubext.h>

#include <gtest/gtest.h>

VIDEO_PREVIEW_USE_NAMESPACE

TEST(VideoPreviewPlugin, ut_init)
{
    VideoPreviewPlugin vp;
    ASSERT_EQ(vp.m_view, nullptr);
    ASSERT_EQ(vp.m_proxy, nullptr);
    ASSERT_EQ(vp.m_decode.get(), nullptr);

    stub_ext::StubExt stub;
    bool ui = false;
    stub.set_lamda(&VideoView::initUI,[&ui](){
        ui = true;
    });

    QObject proxy;
    vp.init(&proxy);
    ASSERT_NE(vp.m_view, nullptr);
    ASSERT_EQ(vp.m_proxy, &proxy);
    EXPECT_TRUE(ui);

    EXPECT_EQ(vp.m_decode.get(), nullptr);
}

TEST(VideoPreviewPlugin, ut_previewItem)
{
    VideoPreviewPlugin vp;
    vp.init(nullptr);

    GrandSearch::ItemInfo item;
    stub_ext::StubExt stub;
    bool decode = false;
    stub.set_lamda(&DecodeBridge::decode,[&decode](QSharedPointer<DecodeBridge> self, const QString &file){
        decode = true;
        emit self->sigUpdateInfo(QVariantHash(), true);
        return QVariantHash();
    });

    bool updateInfo = false;
    stub.set_lamda(&VideoPreviewPlugin::updateInfo,[&updateInfo](){
        updateInfo = true;
    });

    EXPECT_FALSE(vp.previewItem(item));
    EXPECT_FALSE(decode);
    EXPECT_FALSE(updateInfo);

    decode = false;
    item.insert(PREVIEW_ITEMINFO_ITEM, "");
    EXPECT_FALSE(vp.previewItem(item));
    EXPECT_FALSE(decode);
    EXPECT_FALSE(updateInfo);

    updateInfo = false;
    decode = false;
    item.insert(PREVIEW_ITEMINFO_ITEM, "/home/user/test.mp4");
    EXPECT_TRUE(vp.previewItem(item));
    QTest::qWaitFor([&updateInfo](){return updateInfo;}, 2000);
    EXPECT_TRUE(decode);
    EXPECT_TRUE(updateInfo);

    EXPECT_FALSE(vp.m_item.isEmpty());
    EXPECT_EQ(vp.m_view->m_title->text(), QString("test.mp4"));
    EXPECT_EQ(vp.m_infos.size(), 6);
}

TEST(VideoPreviewPlugin, ut_item)
{
    VideoPreviewPlugin vp;
    GrandSearch::ItemInfo item;
    item.insert("1","1");
    vp.m_item = item;

    EXPECT_EQ(vp.item(), item);
}

TEST(VideoPreviewPlugin, ut_contentWidget)
{
    VideoView *view = new VideoView;
    VideoPreviewPlugin vp;
    vp.m_view = view;
    EXPECT_EQ(vp.m_view, view);
}

TEST(VideoPreviewPlugin, ut_stopPreview)
{
    VideoPreviewPlugin vp;
    vp.init(nullptr);
    EXPECT_TRUE(vp.stopPreview());

    stub_ext::StubExt stub;
    bool decode = false;
    bool run = false;
    stub.set_lamda(&DecodeBridge::decode,[&decode, &run](QSharedPointer<DecodeBridge> self, const QString &file){
        run = true;
        EXPECT_TRUE(self->decoding);
        QTest::qWaitFor([self](){
            return !self->decoding;
        }, 1000);
        decode = true;
        return QVariantHash();
    });

    GrandSearch::ItemInfo item;
    item.insert(PREVIEW_ITEMINFO_ITEM, "/home/user/test.mp4");
    EXPECT_TRUE(vp.previewItem(item));
    EXPECT_TRUE(vp.m_decode->decoding);

    // waiting for running the decoding thread.
    QTest::qWaitFor([&run](){
        return run;
    }, 1000);

    EXPECT_TRUE(vp.stopPreview());
    QTest::qWaitFor([&decode](){
        return decode;
    }, 1000);

    EXPECT_TRUE(decode);
    EXPECT_FALSE(vp.m_decode->decoding);
}

TEST(VideoPreviewPlugin, ut_toolBarWidget)
{
    VideoPreviewPlugin vp;
    vp.init(nullptr);
    EXPECT_EQ(vp.toolBarWidget(), nullptr);
    EXPECT_TRUE(vp.showToolBar());
}

TEST(VideoPreviewPlugin, ut_getAttributeDetailInfo)
{
    VideoPreviewPlugin vp;
    vp.init(nullptr);

    using namespace GrandSearch;

    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant("test"));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(QString("test")));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    vp.m_infos.append(detailInfo);
    EXPECT_EQ(vp.m_infos, vp.getAttributeDetailInfo());
}

TEST(VideoPreviewPlugin, ut_updateInfo)
{
    VideoPreviewPlugin vp;
    vp.init(nullptr);

    stub_ext::StubExt stub;
    stub.set_lamda(&DecodeBridge::decode,[](QSharedPointer<DecodeBridge> self, const QString &file){
        return QVariantHash();
    });

    bool updated;
    stub.set_lamda(&GrandSearch::requestUpdateDetailInfo,[&updated](){
        updated = true;
        return true;
    });

    using namespace GrandSearch;

    GrandSearch::ItemInfo item;
    item.insert(PREVIEW_ITEMINFO_ITEM, "/home/user/test.mp4");
    EXPECT_TRUE(vp.previewItem(item));
    ASSERT_EQ(vp.m_infos.size(), 6);
    EXPECT_EQ(vp.m_infos.at(0).first.value(DetailInfoProperty::Text).toString(), QString("Dimensions:"));
    EXPECT_EQ(vp.m_infos.at(1).first.value(DetailInfoProperty::Text).toString(), QString("Type:"));
    EXPECT_EQ(vp.m_infos.at(2).first.value(DetailInfoProperty::Text).toString(), QString("Size:"));
    EXPECT_EQ(vp.m_infos.at(3).first.value(DetailInfoProperty::Text).toString(), QString("Duration:"));
    EXPECT_EQ(vp.m_infos.at(4).first.value(DetailInfoProperty::Text).toString(), QString("Location:"));
    EXPECT_EQ(vp.m_infos.at(5).first.value(DetailInfoProperty::Text).toString(), QString("Time modified:"));

    QVariantHash info;
    info.insert("Dimensions:",QSize(10,10));
    info.insert("Duration:", 7777);
    QPixmap pix(QSize(50,50));
    pix.fill();
    info.insert("Thumbnailer", pix);

    vp.updateInfo(info, true);
    EXPECT_FALSE(updated);
    {
        auto org = vp.m_infos.at(0);
        EXPECT_EQ(org.second.value(DetailInfoProperty::Text).toString(), QString("10*10"));
    }
    {
        auto org = vp.m_infos.at(3);
        EXPECT_EQ(org.second.value(DetailInfoProperty::Text).toString(), GrandSearch::CommonTools::durationString(7777));
    }


    updated = false;
    vp.updateInfo(info, false);
    EXPECT_FALSE(updated);

    QObject proxy;
    vp.m_proxy = &proxy;

    updated = false;
    vp.updateInfo(info, true);
    EXPECT_TRUE(updated);
}

TEST(DecodeBridge, ut_decode)
{
    QSharedPointer<DecodeBridge> decode(new DecodeBridge);
    decode->decoding = true;
    bool ok = false;
    QObject::connect(decode.get(),&DecodeBridge::sigUpdateInfo,[&ok](){
        ok = true;
    });
    DecodeBridge::decode(decode, QString("/home/user/test.mp4"));
    EXPECT_TRUE(ok);
}

TEST(DecodeBridge, ut_scaleAndRound)
{
    QPixmap pix(QSize(500, 500));
    pix.fill();

    QSize size(32, 32);
    pix = DecodeBridge::scaleAndRound(pix.toImage(), size);
    EXPECT_EQ(pix.size(), size);
}
