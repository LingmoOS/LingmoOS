// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "fake_renderer.h"
#include "render/renderer.h"
#include "scene/scene.h"
#include "base/geometry.h"

using namespace Lingmo;
using namespace Lingmo::Scene;

namespace {

using RecordingRenderer = Lingmo::testsupport::FakeRenderer;

class NamedNode : public Node
{
public:
    using Node::Node;
    mutable std::vector<std::string>* log = nullptr;

protected:
    void paint(render::Renderer&, const base::Rect&) const override
    {
        if (log)
            log->push_back(name());
    }
};

NamedNode* addNamed(Node* parent, const std::string& name, std::vector<std::string>* log)
{
    auto node = std::make_unique<NamedNode>(name);
    node->log = log;
    return static_cast<NamedNode*>(parent->addChild(std::move(node)));
}

} // namespace

TEST(SceneNode, TracksParentAndChildren)
{
    Node root;
    Node* child = root.addChild(std::make_unique<Node>("child"));

    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->parent(), &root);
    EXPECT_EQ(root.children().size(), 1u);
    EXPECT_EQ(root.children().front(), child);
    EXPECT_EQ(child->name(), "child");
}

TEST(SceneNode, RemoveChildTransfersOwnership)
{
    Node root;
    Node* child = root.addChild(std::make_unique<Node>());

    std::unique_ptr<Node> detached = root.removeChild(child);
    ASSERT_EQ(detached.get(), child);
    EXPECT_EQ(child->parent(), nullptr);
    EXPECT_TRUE(root.children().empty());
    EXPECT_EQ(root.removeChild(child), nullptr);
}

TEST(SceneNode, AbsoluteGeometryAccumulatesAncestorOffsets)
{
    Node root;
    root.setGeometry(base::Rect(0, 0, 1920, 1080));

    Node* parent = root.addChild(std::make_unique<Node>());
    parent->setGeometry(base::Rect(100, 50, 800, 600));

    Node* child = parent->addChild(std::make_unique<Node>());
    child->setGeometry(base::Rect(10, 20, 100, 200));

    EXPECT_EQ(child->absolutePosition(), base::Point(110, 70));
    EXPECT_EQ(child->absoluteGeometry(), base::Rect(110, 70, 100, 200));
    EXPECT_EQ(parent->absoluteGeometry(), base::Rect(100, 50, 800, 600));
}

TEST(SceneNode, SetPositionKeepsSize)
{
    Node node;
    node.setGeometry(base::Rect(0, 0, 640, 480));
    node.setPosition(base::Point(5, 7));
    EXPECT_EQ(node.geometry(), base::Rect(5, 7, 640, 480));
}

TEST(SceneNode, EffectiveVisibilityFollowsAncestors)
{
    Node root;
    Node* parent = root.addChild(std::make_unique<Node>());
    Node* child = parent->addChild(std::make_unique<Node>());

    EXPECT_TRUE(child->isEffectivelyVisible());
    parent->setVisible(false);
    EXPECT_FALSE(child->isEffectivelyVisible());
    EXPECT_TRUE(child->isVisible());
}

TEST(SceneNode, PaintsChildrenBackToFrontAfterTheParent)
{
    std::vector<std::string> log;
    Node root;
    NamedNode* first = addNamed(&root, "first", &log);
    addNamed(&root, "second", &log);
    addNamed(first, "first-child", &log);

    RecordingRenderer renderer;
    root.render(renderer);

    EXPECT_EQ(log, std::vector<std::string>({"first", "first-child",
                                "second"}));
}

TEST(SceneNode, RaiseAndLowerReorderSiblings)
{
    std::vector<std::string> log;
    Node root;
    NamedNode* a = addNamed(&root, "a", &log);
    NamedNode* b = addNamed(&root, "b", &log);
    NamedNode* c = addNamed(&root, "c", &log);

    root.raise(a);
    EXPECT_EQ(root.children(), std::vector<Node*>({b, c, a}));

    root.lower(c);
    EXPECT_EQ(root.children(), std::vector<Node*>({c, b, a}));

    RecordingRenderer renderer;
    root.render(renderer);
    EXPECT_EQ(log, std::vector<std::string>({"c", "b", "a"}));
}

TEST(SceneNode, ReorderingMovesOnlyWhatIsOutOfPlace)
{
    Node root;
    std::vector<Node*> nodes;
    for (int i = 0; i < 5; ++i)
        nodes.push_back(root.addChild(std::make_unique<Node>()));
    const auto revisions = [&] {
        std::vector<uint64_t> result;
        for (const Node* node : nodes)
            result.push_back(node->revision());
        return result;
    };
    auto before = revisions();
    root.reorderChildren({nodes[1], nodes[2], nodes[3], nodes[4], nodes[0]});
    EXPECT_EQ(root.children(), std::vector<Node*>({nodes[1], nodes[2], nodes[3], nodes[4], nodes[0]}));
    auto after = revisions();
    EXPECT_NE(after[0], before[0]);
    for (int i = 1; i < 5; ++i)
        EXPECT_EQ(after[i], before[i]);
    before = after;
    root.reorderChildren(root.children());
    EXPECT_EQ(revisions(), before);
    // Of two neighbours trading places, one moves.
    root.reorderChildren({nodes[2], nodes[1], nodes[3], nodes[4], nodes[0]});
    after = revisions();
    int moved = 0;
    for (int i = 0; i < 5; ++i)
        moved += after[i] != before[i];
    EXPECT_EQ(moved, 1);
    const auto children = root.children();
    root.reorderChildren({nodes[0], nodes[0], nodes[1], nodes[2], nodes[3]});
    root.reorderChildren({nodes[0]});
    EXPECT_EQ(root.children(), children);
}

TEST(SceneNode, ReorderingAnUnrelatedNodeIsANoOp)
{
    Node root;
    Node* a = root.addChild(std::make_unique<Node>());
    Node other;

    root.raise(&other);
    root.lower(&other);
    EXPECT_EQ(root.children(), std::vector<Node*>({a}));
}

TEST(SceneNode, InvisibleSubtreesAreSkipped)
{
    std::vector<std::string> log;
    Node root;
    NamedNode* hidden = addNamed(&root, "hidden", &log);
    addNamed(hidden, "hidden-child", &log);
    addNamed(&root, "shown", &log);
    hidden->setVisible(false);

    RecordingRenderer renderer;
    root.render(renderer);

    EXPECT_EQ(log, std::vector<std::string>({"shown"}));
}

TEST(SceneNode, ACaptureLeavesOutChildrenExcludedFromCapture)
{
    std::vector<std::string> log;
    NamedNode root("root");
    root.log = &log;
    root.setChildrenBelowParent(1);
    addNamed(&root, "below", &log);
    NamedNode* overlay = addNamed(&root, "overlay", &log);
    addNamed(overlay, "overlay-child", &log);
    addNamed(&root, "above", &log);
    overlay->setExcludedFromCapture(true);

    RecordingRenderer renderer;
    root.render(renderer);
    EXPECT_EQ(log, std::vector<std::string>({"below", "root",
                                "overlay", "overlay-child",
                                "above"}));

    log.clear();
    root.renderForCapture(renderer);
    EXPECT_EQ(log, std::vector<std::string>({"below", "root",
                                "above"}));
}

TEST(SceneNode, ColorNodeFillsItsAbsoluteGeometry)
{
    Node root;
    root.setGeometry(base::Rect(0, 0, 800, 600));
    Node* parent = root.addChild(std::make_unique<Node>());
    parent->setGeometry(base::Rect(30, 40, 400, 300));

    auto* color = static_cast<ColorNode*>(parent->addChild(
        std::make_unique<ColorNode>(render::Color{1.0f, 0.5f, 0.25f, 1.0f})));
    color->setGeometry(base::Rect(5, 5, 100, 100));

    RecordingRenderer renderer;
    root.render(renderer);

    ASSERT_EQ(renderer.calls.size(), 1);
    EXPECT_EQ(renderer.calls.at(0).kind, "fill");
    EXPECT_EQ(renderer.calls.at(0).rect, base::Rect(35, 45, 100, 100));
    EXPECT_FLOAT_EQ(renderer.calls.at(0).color.g, 0.5f);
}

TEST(SceneNode, EmptyGeometryDrawsNothing)
{
    Node root;
    auto* color = static_cast<ColorNode*>(
        root.addChild(std::make_unique<ColorNode>(render::Color{})));
    color->setGeometry(base::Rect(10, 10, 0, 50));

    RecordingRenderer renderer;
    root.render(renderer);
    EXPECT_TRUE(renderer.calls.empty());
}

TEST(SceneNode, TextureNodeDrawsWithAlpha)
{
    // A stand-in address; the recording renderer never dereferences it.
    auto* handle = reinterpret_cast<render::Texture*>(0x1);

    Node root;
    auto* node = static_cast<TextureNode*>(
        root.addChild(std::make_unique<TextureNode>(handle)));
    node->setGeometry(base::Rect(4, 8, 64, 64));
    node->setAlpha(0.5f);

    RecordingRenderer renderer;
    root.render(renderer);

    ASSERT_EQ(renderer.calls.size(), 1);
    EXPECT_EQ(renderer.calls.at(0).kind, "texture");
    EXPECT_EQ(renderer.calls.at(0).texture, handle);
    EXPECT_EQ(renderer.calls.at(0).rect, base::Rect(4, 8, 64, 64));
    EXPECT_FLOAT_EQ(renderer.calls.at(0).alpha, 0.5f);
}

TEST(SceneNode, TextureNodeWithoutATextureDrawsNothing)
{
    Node root;
    auto* node = static_cast<TextureNode*>(
        root.addChild(std::make_unique<TextureNode>(nullptr)));
    node->setGeometry(base::Rect(0, 0, 10, 10));

    RecordingRenderer renderer;
    root.render(renderer);
    EXPECT_TRUE(renderer.calls.empty());
}

TEST(Scene, SizeIsTheRootGeometry)
{
    Scene scene;
    scene.setSize(base::Size(1920, 1080));
    EXPECT_EQ(scene.size(), base::Size(1920, 1080));
    EXPECT_EQ(scene.root()->geometry(), base::Rect(0, 0, 1920, 1080));
}

TEST(Scene, ClearsBeforePaintingTheTree)
{
    Scene scene;
    scene.setSize(base::Size(640, 480));
    scene.setBackground(render::Color{0.1f, 0.2f, 0.3f, 1.0f});

    auto* color = static_cast<ColorNode*>(scene.root()->addChild(
        std::make_unique<ColorNode>(render::Color{1.0f, 1.0f, 1.0f, 1.0f})));
    color->setGeometry(base::Rect(0, 0, 640, 480));

    RecordingRenderer renderer;
    scene.render(renderer);

    const auto kinds = renderer.kinds();
    ASSERT_EQ(2, kinds.size());
    EXPECT_EQ(kinds.at(0), "clear");
    EXPECT_EQ(kinds.at(1), "fill");
    EXPECT_FLOAT_EQ(renderer.calls.at(0).color.b, 0.3f);
}
