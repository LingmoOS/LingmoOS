// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdf-preview/pdfpreviewplugin.h"
#include "pdf-preview/pdfview.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

PDF_PREVIEW_USE_NAMESPACE

TEST(PDFPreviewPluginTest, ut_init)
{
    PDFPreviewPlugin pdf;
    EXPECT_NO_FATAL_FAILURE(pdf.init(nullptr));
}

TEST(PDFPreviewPluginTest, ut_previewItem)
{
    PDFPreviewPlugin pdf;
    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ITEM] = "";
    EXPECT_FALSE(pdf.previewItem(info));

    stub_ext::StubExt st;
    st.set_lamda(&QFileInfo::isReadable, [](){ return false; });
    info[PREVIEW_ITEMINFO_ITEM] = "/test.pdf";
    EXPECT_FALSE(pdf.previewItem(info));

    st.reset(&QFileInfo::isReadable);
    st.set_lamda(&QFileInfo::isReadable, [](){ return true; });
    st.set_lamda(&PDFView::initUI, [](){ return; });
    st.set_lamda(&PDFView::initDoc, [](){ return; });
    st.set_lamda(&PDFView::initConnections, [](){ return; });
    EXPECT_TRUE(pdf.previewItem(info));
}

TEST(PDFPreviewPluginTest, ut_item)
{
    PDFPreviewPlugin pdf;
    EXPECT_TRUE(pdf.item().isEmpty());
}

TEST(PDFPreviewPluginTest, ut_contentWidget)
{
    PDFPreviewPlugin pdf;
    EXPECT_FALSE(pdf.contentWidget());
}

TEST(PDFPreviewPluginTest, ut_stopPreview)
{
    PDFPreviewPlugin pdf;
    EXPECT_TRUE(pdf.stopPreview());
}

TEST(PDFPreviewPluginTest, ut_getAttributeDetailInfo)
{
    PDFPreviewPlugin pdf;
    EXPECT_TRUE(pdf.getAttributeDetailInfo().isEmpty());
}

TEST(PDFPreviewPluginTest, ut_toolBarWidget)
{
    PDFPreviewPlugin pdf;
    EXPECT_FALSE(pdf.toolBarWidget());
}

TEST(PDFPreviewPluginTest, ut_showToolBar)
{
    PDFPreviewPlugin pdf;
    EXPECT_TRUE(pdf.showToolBar());
}
