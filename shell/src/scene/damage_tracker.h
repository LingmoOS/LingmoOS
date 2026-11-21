// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "render/damage.h"
#include "scene/node.h"
#include "scene/surface_node.h"
#include "base/region.h"
#include "base/transform.h"
#include "ui/geometry_conversions.h"

namespace Lingmo::Scene {

// Coverage history follows cc/trees/damage_tracker at Chromium 7acd40ddefff.
// Each output owns a history, including contributors outside its last damage.
class DamageTracker
{
public:
    // With `rootIsViewport`, the root's own geometry and revision are what
    // the viewport accounts for, as for a window whose texture spans it: a
    // move or a change of its own is not damage, only its children's are.
    explicit DamageTracker(bool rootIsViewport = false)
        : m_rootIsViewport(rootIsViewport)
    {
    }

    // Tracks only the root's children in [begin, end), for a tree drawn up to
    // a sibling that draws over the rest. Needs a root that is the viewport.
    void setChildRange(size_t begin, size_t end)
    {
        m_childBegin = begin;
        m_childEnd = end;
    }

    // A blur reads what lies behind it. `backdropChanged`, when given, says
    // whether that changed where the tree tracked here does not reach, as for
    // a window drawn offscreen over the output.
    base::Region update(const Node &root, const base::Rect &viewport, double scale = 1,
                   uint32_t transform = 0,
                   const std::function<bool(const base::Rect &)> &backdropChanged = {})
    {
        Frame frame;
        if (m_cached && !backdropChanged && m_cached->matches(root, viewport, scale, transform))
            frame = std::move(m_cached->frame);
        else
            frame = collect(root, viewport, scale, transform, backdropChanged);
        m_cached.reset();
        m_history = std::move(frame.history);
        m_viewport = viewport;
        m_blurs = std::move(frame.blurs);
        return frame.damage;
    }

    // Whether update() would find anything to repaint, leaving the history be.
    // An output asks this before it draws; the collection is kept for the
    // update() that follows, unless a node changes in between.
    bool hasDamage(const Node &root, const base::Rect &viewport, double scale = 1,
                   uint32_t transform = 0) const
    {
        m_cached.emplace(Cached {Node::mutations(), &root, viewport, scale, transform,
                                 collect(root, viewport, scale, transform, {})});
        return !m_cached->frame.damage.isEmpty();
    }

    // What the blurs found in the last update() read, for the repaint to cover
    // whole wherever it reaches one. Left out are those `kept` says the
    // renderer draws again from a result kept from the last pass, as their
    // backdrop has not changed since.
    std::vector<base::Rect> samples(const std::function<bool(uint64_t)> &kept = {}) const
    {
        std::vector<base::Rect> result;
        for (const Blur &blur : m_blurs)
            if (blur.stale || !kept || !kept(blur.owner))
                result.push_back(blur.sampled);
        return result;
    }

    // The surfaces, by damage id, of the blurs samples() leaves out.
    std::vector<uint64_t> freshBackdrops(const std::function<bool(uint64_t)> &kept) const
    {
        std::vector<uint64_t> result;
        for (const Blur &blur : m_blurs)
            if (!blur.stale && kept(blur.owner))
                result.push_back(blur.owner);
        return result;
    }

private:
    struct Entry {
        base::Rect bounds;
        uint64_t revision;
        uint64_t contentRevision;
    };

    struct Blur {
        base::Rect sampled;
        uint64_t owner;
        // Whether what it reads changed, or it moved.
        bool stale;
    };

    struct Frame {
        base::Region damage;
        std::map<uint64_t, Entry> history;
        std::vector<Blur> blurs;
    };

    struct Cached {
        uint64_t mutations;
        const Node *root;
        base::Rect viewport;
        double scale;
        uint32_t transform;
        Frame frame;

        bool matches(const Node &node, const base::Rect &rect, double s, uint32_t t) const
        {
            return mutations == Node::mutations() && root == &node && viewport == rect
                    && scale == s && transform == t;
        }
    };

    // Bounds are kept relative to the viewport's origin: an output's viewport
    // stays put, while a window texture's moves with the window, whose subtree
    // then reads as unchanged, the way a cc render surface's damage is tracked
    // in the surface's own space and not the target's.
    Frame collect(const Node &root, const base::Rect &viewport, double scale, uint32_t transform,
                  const std::function<bool(const base::Rect &)> &backdropChanged) const
    {
        Frame frame;
        bool filtered = false;
        const base::Point origin = viewport.topLeft();
        // Damage in drawing order, with each blur where it is drawn: only what
        // is drawn before a blur changes what it reads, as in cc's tracker.
        struct Step {
            base::Region damage;
            base::Rect blurred;
            size_t blur = SIZE_MAX;
        };
        std::vector<Step> steps;
        const auto visit = [&](const auto &self, const Node &node, bool ancestorChanged,
                               bool redrawn) -> void {
            if (!node.isVisible() && !node.paintsWhenHidden())
                return;
            const auto &children = node.children();
            if (m_rootIsViewport && &node == &root) {
                for (size_t i = m_childBegin; i < std::min(m_childEnd, children.size()); ++i)
                    self(self, *children[i], false, false);
                return;
            }
            const base::Rect absolute = node.absolutePaintBounds();
            const Entry current {absolute.translated(-origin), node.revision(),
                                 node.contentRevision()};
            const auto previous = m_history.find(node.damageId());
            const bool changed = ancestorChanged || previous == m_history.end()
                    || previous->second.bounds != current.bounds
                    || previous->second.revision != current.revision;
            base::Region damage;
            if (changed) {
                damage += current.bounds;
                if (previous != m_history.end())
                    damage += previous->second.bounds;
            } else {
                damage += expandedContentDamage(node,
                                                node.damageSince(previous->second.contentRevision))
                                  .translated(node.absolutePosition() - origin);
            }
            frame.history.emplace(node.damageId(), current);
            // Filter inputs and transformed subtrees need complete source pixels,
            // but only on the outputs they draw on.
            filtered |= node.requiresFullRepaint() && absolute.intersects(viewport);
            // A blur drawn again from scratch reads everything behind it again.
            const bool redrawsChildren =
                    redrawn || (&node != &root && node.redrawsSubtree(viewport, scale));
            const size_t split = std::min(node.childrenBelowParent(), children.size());
            for (size_t i = 0; i < split; ++i)
                self(self, *children[i], changed, redrawsChildren);
            const auto *surface = dynamic_cast<const SurfaceNode *>(&node);
            if (surface && surface->hasBackgroundBlur()) {
                if (transform != 0 || scale <= 0) {
                    filtered |= absolute.intersects(viewport);
                } else {
                    // Every blur input, including the outward-rounded sample halo.
                    const int halo = render::kBackdropBlurSupport + int(std::ceil(2 / scale));
                    const base::Rect sampled = absolute.adjusted(-halo, -halo, halo, halo);
                    const base::Point position = node.absolutePosition();
                    const base::Rect blurred = surface->backgroundBlurRect().translated(position).united(
                            ui::toBase(surface->backgroundBlurShape().boundingRect()).translated(position)
                                    .toEnclosingRect());
                    steps.push_back({{}, blurred.adjusted(-1, -1, 1, 1), frame.blurs.size()});
                    frame.blurs.push_back(
                            {sampled, node.damageId(),
                             changed || redrawn || (backdropChanged && backdropChanged(sampled))});
                }
            }
            steps.push_back({damage});
            for (size_t i = split; i < children.size(); ++i)
                self(self, *children[i], changed, redrawsChildren);
        };
        visit(visit, root, false, false);
        base::Region &damage = frame.damage;
        // A node gone from the tree is taken as drawn below everything.
        for (const auto &[id, previous] : m_history)
            if (!frame.history.contains(id))
                damage += previous.bounds;
        const bool full = m_history.empty() || viewport.size() != m_viewport.size() || filtered;
        for (const Step &step : steps) {
            if (step.blur == SIZE_MAX) {
                damage += step.damage;
                continue;
            }
            Blur &blur = frame.blurs[step.blur];
            blur.stale = full || blur.stale || damage.intersects(blur.sampled.translated(-origin));
            if (blur.stale)
                damage += step.blurred.translated(-origin);
        }
        if (full)
            damage = base::Rect(base::Point(), viewport.size());
        damage = damage.translated(origin).intersected(viewport);
        return frame;
    }

    static base::Region expandedContentDamage(const Node &node, const base::Region &damage)
    {
        const auto *surface = dynamic_cast<const SurfaceNode *>(&node);
        if (!surface || !surface->texture() || node.geometry().isEmpty() || damage.isEmpty())
            return damage;
        const base::Size size = node.geometry().size(), pixels = surface->texture()->size();
        const base::AffineTransform toBuffer = base::AffineTransform::fromScale(1.0 / size.width(), 1.0 / size.height())
                * surface->textureMapping()
                * base::AffineTransform::fromScale(pixels.width(), pixels.height());
        bool invertible = false;
        const base::AffineTransform toSurface = toBuffer.inverted(&invertible);
        const base::Rect bounds(base::Point(), size);
        if (!invertible)
            return bounds;
        base::Region result;
        // Linear sampling can change neighboring pixels beyond the damaged texel.
        for (const base::Rect &rect : damage.intersected(bounds)) {
            const base::RectF source = toBuffer.mapRect(base::RectF(rect)).adjusted(-1, -1, 1, 1);
            result += toSurface.mapRect(source).toEnclosingRect().intersected(bounds);
        }
        return result;
    }

    bool m_rootIsViewport = false;
    size_t m_childBegin = 0;
    size_t m_childEnd = SIZE_MAX;
    std::map<uint64_t, Entry> m_history;
    base::Rect m_viewport;
    std::vector<Blur> m_blurs;
    mutable std::optional<Cached> m_cached;
};

} // namespace Lingmo::Scene
