// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QTest>
#include <QQuickItem>
#include <QSGSimpleTextureNode>
#include <QSignalSpy>

#include "dblitframebuffernode_p.h"
DQUICK_USE_NAMESPACE

class DBlitFramebufferNodeItem : public QQuickItem
{
    Q_OBJECT
public:
    DBlitFramebufferNodeItem()
        : QQuickItem()
    {
        setFlag(ItemHasContents, true);
    }

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override
    {
        DBlitFramebufferNode *node = static_cast<DBlitFramebufferNode *>(oldNode);
        const auto ga = window()->rendererInterface()->graphicsApi();
        if (!node) {
            if (ga == QSGRendererInterface::Software) {
                node = DBlitFramebufferNode::createSoftwareNode(this, false, true);
            } else if (TestUtil::supportOpengl(ga)) {
#ifndef QT_NO_OPENGL
                node = DBlitFramebufferNode::createOpenGLNode(this, true, true);
#endif
            }
            if (!node)
                return nullptr;

            auto subNode = new QSGSimpleTextureNode();
            subNode->setTexture(TestUtil::simpleTexture(Qt::blue));
            node->setRenderCallback([](DBlitFramebufferNode *node, void *argu) -> void {
                auto item = reinterpret_cast<DBlitFramebufferNodeItem*>(argu);
                ASSERT_TRUE(item);
                Q_EMIT item->callbackCalled();
                item->nodeTextureImg = node->toImage();
            }, this);
            node->appendChildNode(subNode);
        }
        QSGSimpleTextureNode *subNode = static_cast<QSGSimpleTextureNode*>(node->firstChild());

        subNode->setRect(QRectF(0, 0, 50, 50));
        subNode->setSourceRect(QRectF(0, 0, 100, 100));
        node->resize(QSizeF(100, 100));
        node->setMargins(QMarginsF(10, 10, 10, 10));
        return node;
    }

signals:
    void callbackCalled();

public:
    QImage nodeTextureImg;
};

TEST(ut_DBlitFramebufferNode, properties)
{
    TEST_QRHI_SKIP();

    TestUtil::registerType<DBlitFramebufferNodeItem>("DBlitFramebufferNodeItem");
    QuickViewHelper<> helper;
    ASSERT_TRUE(helper.load("qrc:/qml/DBlitFramebufferNodeItem.qml"));

    auto target = findItem<DBlitFramebufferNodeItem>(helper.object);
    ASSERT_TRUE(target);
    QSignalSpy spy(target, SIGNAL(callbackCalled()));
    helper.requestExposed();

    ASSERT_GE(spy.count(), 1);

    QImage img = helper.view->grabWindow();

    // in Corner
    ASSERT_EQ(img.pixelColor(QPoint(50, 40)), Qt::red);
    // in RoundedNode
    ASSERT_EQ(img.pixelColor(QPoint(60, 60)), Qt::blue);

    // node texture
    ASSERT_EQ(target->nodeTextureImg.pixelColor(QPoint(60, 60)), Qt::red);
}

#include "ut_dblitframebuffernode.moc"
