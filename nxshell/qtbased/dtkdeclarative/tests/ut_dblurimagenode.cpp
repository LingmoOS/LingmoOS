// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <QQuickItem>
#include <QSGRectangleNode>
#include <QSignalSpy>

#ifndef QT_NO_OPENGL
#include <QOpenGLFramebufferObject>
#endif

#include "dblurimagenode_p.h"
DQUICK_USE_NAMESPACE

class DSGBlurImageNodeItem : public QQuickItem
{
    Q_OBJECT
public:
    DSGBlurImageNodeItem()
        : QQuickItem()
    {
        setFlag(ItemHasContents, true);
    }

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override
    {
        QSGRectangleNode *node = static_cast<QSGRectangleNode *>(oldNode);
        const auto ga = window()->rendererInterface()->graphicsApi();
        if (!node) {
            node = window()->createRectangleNode();

            DSGBlurNode *blurNode = nullptr;
            do {
                if (ga == QSGRendererInterface::Software) {
                    blurNode = new DSoftwareBlurImageNode(this);
                } else if (TestUtil::supportOpengl(ga)) {
    #ifndef QT_NO_OPENGL
                    blurNode = new DOpenGLBlurEffectNode(this);
    #endif
                }
                if (!blurNode)
                    break;

                blurNode->setTexture(TestUtil::imageTexture());
                blurNode->setRenderCallback([](DSGBlurNode *node, void *argu) -> void {
                    auto item = reinterpret_cast<DSGBlurImageNodeItem*>(argu);
                    ASSERT_TRUE(item);
                    Q_EMIT item->callbackCalled();
                    if (auto tmp = dynamic_cast<DSoftwareBlurImageNode *>(node)) {
                        item->nodeTextureImg = tmp->cachedSource;
                    }
                    else {
    #ifndef QT_NO_OPENGL
                        if (auto tmp = dynamic_cast<DOpenGLBlurEffectNode *>(node)) {
                            item->nodeTextureImg = tmp->m_fboVector[0]->toImage();
                        }
    #endif
                    }
                }, this);

                blurNode->setOffscreen(false);
                blurNode->setBlendColor(TestUtil::simpleColor);
                blurNode->setRadius(10);
                blurNode->setWindow(window());
                blurNode->setRect(QRectF(0, 0, 100, 100));
                blurNode->setSourceRect(QRectF(0, 0, 100, 100));

            } while(false);

            if (blurNode)
                node->appendChildNode(blurNode);
        }

        node->setRect(this->boundingRect());

        return node;
    }

signals:
    void callbackCalled();

public:
    QImage nodeTextureImg;
};

TEST(ut_DSGBlurImageNode, properties)
{
    TestUtil::registerType<DSGBlurImageNodeItem>("DSGBlurImageNodeItem");
    QuickViewHelper<DSGBlurImageNodeItem> helper;
    ASSERT_TRUE(helper.load("qrc:/qml/DSGBlurImageNode.qml"));

    auto target = helper.object;
    ASSERT_TRUE(target);
    QSignalSpy spy(target, SIGNAL(callbackCalled()));
    helper.requestExposed();

    EXPECT_GE(spy.count(), 1);

    EXPECT_NE(TestUtil::simpleImage->pixelColor(QPoint(50, 50)), target->nodeTextureImg.pixelColor(QPoint(50, 50)));
}

#include "ut_dblurimagenode.moc"
