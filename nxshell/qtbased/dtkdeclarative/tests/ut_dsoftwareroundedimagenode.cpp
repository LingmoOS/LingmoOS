// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>

#include "dsoftwareroundedimagenode_p.h"

DQUICK_USE_NAMESPACE

class DSoftwareRoundedImageNodeItem : public QQuickItem
{
public:
    DSoftwareRoundedImageNodeItem()
        : texture(TestUtil::simpleTexture(Qt::blue))
    {
        setFlag(ItemHasContents, true);
    }

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override
    {
        DSoftwareRoundedImageNode *node = static_cast<DSoftwareRoundedImageNode *>(oldNode);
        if (!node)
            node = new DSoftwareRoundedImageNode(this);

        node->setTexture(texture);
        node->setSourceRect(QRectF(0, 0, 10.0, 10.0));
        node->setRadius(16);
        node->setRect(QRectF(QPointF(20, 20), QSize(150, 150)));
        node->setSmooth(true);
        return node;
    }
    QSGTexture *texture = nullptr;
};

TEST(ut_DSoftwareRoundedImageNode, properties)
{
    TEST_NOTSOFTWARE_SKIP();

    TestUtil::registerType<DSoftwareRoundedImageNodeItem>("DSoftwareRoundedImageNodeItem");
    QuickViewHelper<> helper("qrc:/qml/DSoftwareRoundedImageNodeItem.qml");
    ASSERT_TRUE(helper.object);

    QImage img = helper.view->grabWindow();

    // in Corner
    ASSERT_EQ(img.pixelColor(QPoint(20, 22)), Qt::red);
    // in RoundedNode
    ASSERT_EQ(img.pixelColor(QPoint(100, 100)), Qt::blue);
}
