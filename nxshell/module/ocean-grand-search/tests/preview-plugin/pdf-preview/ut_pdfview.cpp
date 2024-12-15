// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdf-preview/pdfview.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>

PDF_PREVIEW_USE_NAMESPACE

class PDFViewTest : public testing::Test
{
public:
    PDFViewTest()
        : testing::Test()
    {
        stub_ext::StubExt st;
        st.set_lamda(&PDFView::initDoc, []() { return; });
        st.set_lamda(&PDFView::initUI, []() { return; });
        st.set_lamda(&PDFView::initConnections, []() { return; });

        view.reset(new PDFView("test.pdf"));
    }

    QSharedPointer<PDFView> view;
};

TEST_F(PDFViewTest, ut_initDoc)
{
    EXPECT_NO_FATAL_FAILURE(view->initDoc("test.pdf"));
}

TEST_F(PDFViewTest, ut_initUI_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&PDFView::syncLoadFirstPage, []() { return; });

    EXPECT_NO_FATAL_FAILURE(view->initUI());
}

TEST_F(PDFViewTest, ut_initUI_1)
{
    view->m_isBadDoc = true;
    EXPECT_NO_FATAL_FAILURE(view->initUI());
}

TEST_F(PDFViewTest, ut_initConnections)
{
    EXPECT_NO_FATAL_FAILURE(view->initConnections());
}

TEST_F(PDFViewTest, ut_onPageUpdated)
{
    view->m_pageLabel = new QLabel(view.data());
    QImage errImg(":/icons/file_damaged.svg");
    EXPECT_NO_FATAL_FAILURE(view->onPageUpdated(errImg));
}

TEST_F(PDFViewTest, ut_syncLoadFirstPage)
{
    stub_ext::StubExt st;
    st.set_lamda(&DPdfDoc::page, []() { return nullptr; });

    view->m_doc.reset(new DPdfDoc(""));
    view->syncLoadFirstPage();
    view->m_future.waitForFinished();
    EXPECT_TRUE(view->m_isLoadFinished);
}
