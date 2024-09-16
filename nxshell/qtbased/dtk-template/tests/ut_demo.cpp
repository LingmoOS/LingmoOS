// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mockdemo/mockdemo.h"
#include "gtest/gtest.h"
#include "3rdparty/cpp-stub/src/stub.h"
#include "demo_p.h"

DDEMO_USE_NAMESPACE

class TestDemo : public testing::Test
{
public:
    void SetUp() override { m_demo = new Demo; }
    void TearDown() override
    {
        delete m_demo;
        m_demo = nullptr;
    }

public:
    Demo *m_demo = nullptr;
};

TEST_F(TestDemo, add)
{
    EXPECT_EQ(3, m_demo->add(1, 2));
}

TEST_F(TestDemo, svg2png)
{
    Stub stub;
    stub.set(rsvg_handle_new_from_file, rsvg_handle_new_from_file_stub);
    stub.set(rsvg_handle_get_dimensions, rsvg_handle_get_dimensions_stub);
    stub.set(cairo_image_surface_create_for_data, cairo_image_surface_create_for_data_stub);
    stub.set(cairo_create, cairo_create_stub);
    stub.set(cairo_scale, cairo_scale_stub);
    stub.set(cairo_translate, cairo_translate_stub);
    stub.set(rsvg_handle_render_cairo, rsvg_handle_render_cairo_stub);
    stub.set(cairo_surface_destroy, cairo_surface_destroy_stub);
    stub.set(cairo_destroy, cairo_destroy_stub);
    stub.set(g_object_unref, g_object_unref_stub);

    EXPECT_TRUE(m_demo->svg2png("/no/exist/svg", "/tmp/test.png"));
}
