// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/node.h"

#include <algorithm>
#include <atomic>
#include <unordered_map>
#include "base/region.h"

namespace Lingmo::Scene {

uint64_t Node::s_mutations = 0;

Node::Node(std::string name)
    : m_damageId([] {
        static std::atomic<uint64_t> next {0};
        return ++next;
    }())
    , m_name(std::move(name))
{
}

Node::~Node() = default;

Node *Node::addChild(std::unique_ptr<Node> child)
{
    if (!child)
        return nullptr;

    Node *raw = child.get();
    raw->m_parent = this;
    m_children.push_back(std::move(child));
    m_childOrder.push_back(raw);
    raw->notifyChanged();
    return raw;
}

std::unique_ptr<Node> Node::removeChild(Node *child)
{
    const auto owned = std::find_if(m_children.begin(), m_children.end(),
                                    [child](const std::unique_ptr<Node> &n) {
                                        return n.get() == child;
                                    });
    if (owned == m_children.end())
        return nullptr;

    m_childOrder.erase(std::remove(m_childOrder.begin(), m_childOrder.end(), child),
                       m_childOrder.end());

    std::unique_ptr<Node> result = std::move(*owned);
    m_children.erase(owned);
    result->notifyChanged();
    result->m_parent = nullptr;
    return result;
}

void Node::raise(Node *child)
{
    const auto it = std::find(m_childOrder.begin(), m_childOrder.end(), child);
    if (it == m_childOrder.end() || it + 1 == m_childOrder.end())
        return;
    std::rotate(it, it + 1, m_childOrder.end());
    child->notifyChanged();
}

void Node::lower(Node *child)
{
    const auto it = std::find(m_childOrder.begin(), m_childOrder.end(), child);
    if (it == m_childOrder.end() || it == m_childOrder.begin())
        return;
    std::rotate(m_childOrder.begin(), it, it + 1);
    child->notifyChanged();
}

void Node::reorderChildren(const std::vector<Node *> &order)
{
    if (order.size() != m_childOrder.size())
        return;
    std::unordered_map<const Node *, size_t> position;
    for (size_t i = 0; i < m_childOrder.size(); ++i)
        position.emplace(m_childOrder[i], i);
    std::vector<size_t> sequence;
    std::vector<bool> listed(order.size());
    for (Node *child : order) {
        const auto found = position.find(child);
        if (found == position.end() || listed[found->second])
            return;
        listed[found->second] = true;
        sequence.push_back(found->second);
    }
    // Longest increasing run of current positions, by patience sorting.
    std::vector<size_t> tails;
    std::vector<ptrdiff_t> previous(sequence.size(), -1);
    for (size_t i = 0; i < sequence.size(); ++i) {
        const auto slot = std::lower_bound(tails.begin(), tails.end(), sequence[i],
                                           [&](size_t index, size_t value) {
                                               return sequence[index] < value;
                                           });
        if (slot != tails.begin())
            previous[i] = ptrdiff_t(*(slot - 1));
        if (slot == tails.end())
            tails.push_back(i);
        else
            *slot = i;
    }
    std::vector<bool> kept(sequence.size());
    for (ptrdiff_t i = tails.empty() ? -1 : ptrdiff_t(tails.back()); i >= 0; i = previous[i])
        kept[i] = true;
    m_childOrder = order;
    for (size_t i = 0; i < order.size(); ++i) {
        if (!kept[i])
            order[i]->notifyChanged();
    }
}

void Node::notifyChanged() const
{
    ++m_revision;
    ++s_mutations;
    if (!isEffectivelyVisible())
        return;
    // Callback only queues work; it must not mutate or render the scene here.
    for (const Node *node = this; node; node = node->m_parent)
        if (node->m_changed)
            node->m_changed();
}

void Node::requestFrame() const
{
    if (!isEffectivelyVisible())
        return;
    for (const Node *node = this; node; node = node->m_parent)
        if (node->m_changed)
            node->m_changed();
}

void Node::notifyDamage(const base::Region &damage)
{
    if (damage.isEmpty())
        return;
    ++s_mutations;
    m_damageHistory.emplace_back(++m_contentRevision, damage);
    if (m_damageHistory.size() > 32)
        m_damageHistory.pop_front();
    if (isEffectivelyVisible())
        for (const Node *node = this; node; node = node->m_parent)
            if (node->m_changed)
                node->m_changed();
}

base::Region Node::damageSince(uint64_t revision) const
{
    if (revision == m_contentRevision)
        return {};
    if (m_damageHistory.empty() || revision + 1 < m_damageHistory.front().first)
        return base::Rect(base::Point(), geometry().size());
    base::Region result;
    for (const auto &[serial, damage] : m_damageHistory)
        if (serial > revision)
            result += damage;
    return result;
}

void Node::setGeometry(const base::Rect &geometry)
{
    if (m_geometry == geometry)
        return;
    m_geometry = geometry;
    notifyChanged();
}

void Node::setVisible(bool visible)
{
    if (m_visible == visible)
        return;
    notifyChanged(); // Repaint the old coverage when hiding.
    m_visible = visible;
    if (visible)
        notifyChanged();
}

void Node::setChildrenBelowParent(size_t count)
{
    if (m_childrenBelowParent == count)
        return;
    m_childrenBelowParent = count;
    notifyChanged();
}

base::Point Node::absolutePosition() const
{
    base::Point result = m_geometry.topLeft();
    for (const Node *node = m_parent; node; node = node->m_parent)
        result += node->m_geometry.topLeft();
    return result;
}

base::Rect Node::absoluteGeometry() const
{
    return base::Rect(absolutePosition(), m_geometry.size());
}

bool Node::isEffectivelyVisible() const
{
    for (const Node *node = this; node; node = node->m_parent) {
        if (!node->m_visible)
            return false;
    }
    return true;
}

size_t Node::firstUnoccludedChild(const render::Renderer &renderer) const
{
    for (size_t i = m_childOrder.size(); i-- > 0;) {
        if (m_childOrder[i]->isVisible() && m_childOrder[i]->occludesOutput(renderer))
            return i;
    }
    return 0;
}

bool Node::subtreeOccludesOutput(const render::Renderer &renderer) const
{
    if (!m_visible)
        return false;
    if (occludesOutput(renderer))
        return true;
    return std::any_of(m_childOrder.begin(), m_childOrder.end(), [&renderer](const Node *child) {
        return child->subtreeOccludesOutput(renderer);
    });
}

void Node::render(render::Renderer &renderer) const
{
    if (m_visible)
        renderSubtree(renderer);
}

void Node::renderSubtree(render::Renderer &renderer) const
{
    const size_t split = std::min(m_childrenBelowParent, m_childOrder.size());
    const size_t first = firstUnoccludedChild(renderer);
    for (size_t i = first; i < split; ++i)
        m_childOrder[i]->render(renderer);
    if (first <= split)
        paint(renderer, absoluteGeometry());
    size_t next = std::max(split, first);
    for (size_t i = m_childOrder.size(); i-- > next + 1;) {
        if (m_childOrder[i]->renderSiblingsBelow(renderer, next, i)) {
            next = i;
            break;
        }
    }
    for (size_t i = next; i < m_childOrder.size(); ++i)
        m_childOrder[i]->render(renderer);
}

void Node::renderForCapture(render::Renderer &renderer) const
{
    if (!m_visible)
        return;

    const size_t split = std::min(m_childrenBelowParent, m_childOrder.size());
    const size_t first = firstUnoccludedChild(renderer);
    for (size_t i = first; i < m_childOrder.size(); ++i) {
        if (i == split)
            paint(renderer, absoluteGeometry());
        if (!m_childOrder[i]->isExcludedFromCapture())
            m_childOrder[i]->render(renderer);
    }
    if (split >= m_childOrder.size() && first <= split)
        paint(renderer, absoluteGeometry());
}

void Node::releaseRenderResources()
{
    for (Node *child : m_childOrder)
        child->releaseRenderResources();
}

void Node::finishRetiring()
{
    for (Node *child : m_childOrder)
        child->finishRetiring();
}

void Node::collectRetired()
{
    for (Node *child : std::vector<Node *>(m_childOrder)) {
        child->collectRetired();
        if (child->isRetired())
            removeChild(child);
    }
}

void Node::paint(render::Renderer &, const base::Rect &) const
{
}

ColorNode::ColorNode(const render::Color &color, std::string name)
    : Node(std::move(name))
    , m_color(color)
{
}

void ColorNode::setColor(const render::Color &color)
{
    if (m_color.r == color.r && m_color.g == color.g && m_color.b == color.b
        && m_color.a == color.a)
        return;
    m_color = color;
    notifyChanged();
}

void ColorNode::paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const
{
    if (absoluteGeometry.isEmpty() || !renderer.drawsInto(absoluteGeometry))
        return;
    render::Color color = m_color;
    if (renderer.overwriteOpaque && m_color.a >= 1) {
        // Darkened in place, still opaque: nothing under it is blended in.
        color.r *= renderer.opacityScale;
        color.g *= renderer.opacityScale;
        color.b *= renderer.opacityScale;
    } else {
        color.a *= renderer.opacityScale;
    }
    renderer.fillRect(absoluteGeometry, color);
}

TextureNode::TextureNode(render::Texture *texture, std::string name)
    : Node(std::move(name))
    , m_texture(texture)
{
}

void TextureNode::paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const
{
    if (m_texture && !absoluteGeometry.isEmpty() && renderer.drawsInto(absoluteGeometry)) {
        renderer.drawTexture(m_texture, absoluteGeometry, m_alpha);
    }
}

} // namespace Lingmo::Scene
