// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>

#include "dmaskeffectnode_p.h"
DQUICK_USE_NAMESPACE

class TextureMaterialItem : public QQuickItem
{
public:
    TextureMaterialItem()
        : QQuickItem()
    {
        setFlag(ItemHasContents, true);
    }

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override
    {
        QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);
        if (!node) {
            node = new QSGGeometryNode();
            auto geo = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
            node->setGeometry(geo);
            node->setFlag(QSGNode::OwnsGeometry);

            auto material = new TextureMaterial();
            material->setMaskTexture(TestUtil::simpleTextureThreePart(255));
            material->setMaskOffset(QVector2D(0, 0));
            material->setMaskScale(QVector2D(1, 1));
            material->setSourceScale(QVector2D(1, 1));

            material->setTexture(TestUtil::simpleTexture(QColor(100, 100, 100, 255)));

            node->setMaterial(material);
            node->setFlag(QSGNode::OwnsMaterial);
        }
        QSGGeometry::updateTexturedRectGeometry(node->geometry(), boundingRect(), QRectF(0, 0, 1, 1));
        node->markDirty(QSGNode::DirtyGeometry);

        return node;
    }
};

TEST(ut_MaskEffectNode, textureMaterial)
{
    TEST_SOFTWARE_SKIP();

    TestUtil::registerType<TextureMaterialItem>("TextureMaterialItem");
    QuickViewHelper<> helper("qrc:/qml/DMaskEffectNode_TextureMaterial.qml");
    ASSERT_TRUE(helper.object);

    auto target = findItem<TextureMaterialItem>(helper.object);
    ASSERT_TRUE(target);

    QImage img = helper.view->grabWindow();

    EXPECT_EQ(img.pixelColor(QPoint(50, 30)), QColor(0, 0, 0, 255));
    EXPECT_EQ(img.pixelColor(QPoint(50, 60)), QColor(0, 100, 0, 255));
    EXPECT_EQ(img.pixelColor(QPoint(50, 90)), QColor(0, 0, 0, 255));
}

class MaskEffectNodeItem : public QQuickItem
{
public:
    MaskEffectNodeItem()
        : QQuickItem()
    {
        setFlag(ItemHasContents, true);
    }

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override
    {
        MaskEffectNode *node = static_cast<MaskEffectNode *>(oldNode);
        if (!node) {
            node = new MaskEffectNode();

            node->setMaskTexture(TestUtil::simpleTextureThreePart(255));
            node->setMaskOffset(QVector2D(0, 0));
            node->setMaskScale(QVector2D(1, 1));
            node->setSourceScale(QVector2D(1, 1));

            node->setFiltering(QSGTexture::Linear);
            node->setMipmapFiltering(QSGTexture::Linear);
            node->setAnisotropyLevel(QSGTexture::Anisotropy2x);
            node->setTextureCoordinatesTransform(QSGImageNode::MirrorHorizontally | QSGImageNode::MirrorVertically);
            node->setOwnsTexture(false);

            node->setTexture(TestUtil::simpleTexture(QColor(100, 100, 100, 255)));
        }
        node->setRect(this->boundingRect());
        node->setSourceRect(node->rect());

        return node;
    }
};

TEST(ut_MaskEffectNode, properties)
{
    TEST_SOFTWARE_SKIP();

    TestUtil::registerType<MaskEffectNodeItem>("MaskEffectNodeItem");
    QuickViewHelper<> helper("qrc:/qml/DMaskEffectNode.qml");
    ASSERT_TRUE(helper.object);

    auto target = findItem<MaskEffectNodeItem>(helper.object);
    ASSERT_TRUE(target);

    QImage img = helper.view->grabWindow();

    EXPECT_EQ(img.pixelColor(QPoint(50, 30)), QColor(0, 0, 0, 255));
    EXPECT_EQ(img.pixelColor(QPoint(50, 60)), QColor(0, 100, 0, 255));
    EXPECT_EQ(img.pixelColor(QPoint(50, 90)), QColor(0, 0, 0, 255));
}
