// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/display_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: publish validated value snapshots as one batch and
// centralize logical-display selection and coordinate lookup.

#include "display/display_manager.h"

#include <algorithm>
#include <limits>

#include <set>

#include "base/auto_reset.h"

namespace Lingmo::Display {

namespace {

void setError(DisplayError *target, DisplayErrorCode code, std::string message,
              std::vector<DisplayId> displays = {})
{
    if (target)
        *target = {code, std::move(message), std::move(displays)};
}

bool containsHalfOpen(const base::Rect &bounds, const base::PointF &point)
{
    return point.x() >= bounds.x() && point.y() >= bounds.y()
            && point.x() < bounds.x() + bounds.width() && point.y() < bounds.y() + bounds.height();
}

int64_t intersectionArea(const base::Rect &left, const base::Rect &right)
{
    const base::Rect intersection = left.intersected(right);
    return intersection.isEmpty() ? 0 : int64_t(intersection.width()) * intersection.height();
}

// The centre as it has always been taken: the middle of the first and last
// pixel, rounded towards zero, so an even width leans left.
base::PointF pixelCenter(const base::Rect &rect)
{
    const int64_t right = int64_t(rect.x()) + rect.width() - 1;
    const int64_t bottom = int64_t(rect.y()) + rect.height() - 1;
    return base::PointF(double((rect.x() + right) / 2), double((rect.y() + bottom) / 2));
}

// The largest size of the same aspect ratio as `size` that fits in `bounds`.
base::SizeF fittedInside(const base::SizeF &size, const base::SizeF &bounds)
{
    if (size.width() == 0 || size.height() == 0)
        return bounds;
    const double width = bounds.height() * size.width() / size.height();
    if (width <= bounds.width())
        return {width, bounds.height()};
    return {bounds.width(), bounds.width() * size.height() / size.width()};
}

double squaredDistance(const base::Rect &bounds, const base::PointF &point)
{
    const double x =
            std::clamp(point.x(), double(bounds.x()), double(bounds.x() + bounds.width() - 1));
    const double y =
            std::clamp(point.y(), double(bounds.y()), double(bounds.y() + bounds.height() - 1));
    const double dx = point.x() - x;
    const double dy = point.y() - y;
    return dx * dx + dy * dy;
}

} // namespace

bool DisplayManager::installHardware(HardwareSnapshot snapshot, DisplayError *error)
{
    if (m_publishing) {
        setError(error, DisplayErrorCode::Busy,
                 "Display publication is in progress");
        return false;
    }
    base::AutoReset<bool> publishing(m_publishing, true);
    if (!snapshot.isValid()) {
        setError(error, DisplayErrorCode::Unavailable,
                 "The hardware snapshot is invalid");
        return false;
    }
    if (m_hardware && snapshot.compositorInstanceId == m_hardware->compositorInstanceId
        && snapshot.topologyGeneration <= m_hardware->topologyGeneration) {
        setError(error, DisplayErrorCode::StaleConfiguration,
                 "The hardware snapshot generation did not advance");
        return false;
    }

    displayChangeStarted();
    m_hardware = std::move(snapshot);
    if (m_applied
        && (m_applied->compositorInstanceId != m_hardware->compositorInstanceId
            || m_applied->topologyGeneration != m_hardware->topologyGeneration)) {
        m_applied.reset();
    }
    if (m_publisher)
        m_publisher();
    displaysChanged(m_hardware->topologyGeneration, m_applied ? m_applied->revision : 0);
    return true;
}

bool DisplayManager::installApplied(AppliedConfiguration configuration, DisplayError *error)
{
    if (m_publishing) {
        setError(error, DisplayErrorCode::Busy,
                 "Display publication is in progress");
        return false;
    }
    base::AutoReset<bool> publishing(m_publishing, true);
    if (!m_hardware || configuration.compositorInstanceId != m_hardware->compositorInstanceId
        || configuration.topologyGeneration != m_hardware->topologyGeneration
        || (m_applied && configuration.revision <= m_applied->revision)) {
        setError(error, DisplayErrorCode::StaleConfiguration,
                 "The applied configuration does not match current hardware");
        return false;
    }

    std::set<DisplayId> logicalIds;
    std::set<std::string> waylandNames;
    std::set<DisplayId> destinations;
    int primaryCount = 0;
    for (const LogicalDisplay &logical : configuration.logicalDisplays) {
        if (!logical.id.isValid() || logical.bounds.width() <= 0 || logical.bounds.height() <= 0
            || logical.workArea.isEmpty() || !logical.bounds.contains(logical.workArea)
            || logical.destinations.empty() || logical.waylandName.empty()
            || logicalIds.contains(logical.id) || waylandNames.contains(logical.waylandName)) {
            setError(error, DisplayErrorCode::InvalidLayout,
                     "An applied logical display is invalid", {logical.id});
            return false;
        }
        for (const DisplayId &destination : logical.destinations) {
            if (!physicalDisplay(destination) || destinations.contains(destination)) {
                setError(error, DisplayErrorCode::UnknownOutput,
                         "A physical destination is missing or duplicated",
                         {destination});
                return false;
            }
            destinations.insert(destination);
        }
        logicalIds.insert(logical.id);
        waylandNames.insert(logical.waylandName);
        primaryCount += logical.primary ? 1 : 0;
    }
    const auto &logicalDisplays = configuration.logicalDisplays;
    for (size_t i = 0; i < logicalDisplays.size(); ++i) {
        for (size_t j = i + 1; j < logicalDisplays.size(); ++j) {
            if (logicalDisplays[i].bounds.intersects(logicalDisplays[j].bounds)) {
                setError(error, DisplayErrorCode::InvalidLayout,
                         "Applied logical displays overlap",
                         {logicalDisplays[i].id, logicalDisplays[j].id});
                return false;
            }
        }
    }
    if (!configuration.logicalDisplays.empty() && primaryCount != 1) {
        setError(error, DisplayErrorCode::InvalidLayout,
                 "Exactly one active logical display must be primary");
        return false;
    }

    displayChangeStarted();
    m_applied = std::move(configuration);
    if (m_publisher)
        m_publisher();
    displaysChanged(m_applied->topologyGeneration, m_applied->revision);
    return true;
}

void DisplayManager::clearApplied(uint64_t topologyGeneration)
{
    if (m_publishing)
        return;
    base::AutoReset<bool> publishing(m_publishing, true);
    if (!m_applied && (!m_hardware || m_hardware->topologyGeneration == topologyGeneration))
        return;
    displayChangeStarted();
    m_applied.reset();
    if (m_publisher)
        m_publisher();
    displaysChanged(topologyGeneration, 0);
}

const LogicalDisplay *DisplayManager::display(const DisplayId &id) const
{
    if (!m_applied)
        return nullptr;
    for (const LogicalDisplay &logical : m_applied->logicalDisplays) {
        if (logical.id == id)
            return &logical;
    }
    return nullptr;
}

const LogicalDisplay *DisplayManager::primaryDisplay() const
{
    if (!m_applied)
        return nullptr;
    for (const LogicalDisplay &logical : m_applied->logicalDisplays) {
        if (logical.primary)
            return &logical;
    }
    return nullptr;
}

const LogicalDisplay *DisplayManager::displayAt(const base::PointF &point) const
{
    if (!m_applied)
        return nullptr;
    for (const LogicalDisplay &logical : m_applied->logicalDisplays) {
        if (containsHalfOpen(logical.bounds, point))
            return &logical;
    }
    return nullptr;
}

const LogicalDisplay *DisplayManager::nearestDisplay(const base::PointF &point) const
{
    if (const LogicalDisplay *exact = displayAt(point))
        return exact;
    if (!m_applied)
        return nullptr;

    const LogicalDisplay *nearest = nullptr;
    double best = std::numeric_limits<double>::max();
    for (const LogicalDisplay &logical : m_applied->logicalDisplays) {
        const double distance = squaredDistance(logical.bounds, point);
        if (!nearest || distance < best
            || (distance == best && logical.primary && !nearest->primary)) {
            nearest = &logical;
            best = distance;
        }
    }
    return nearest;
}

const LogicalDisplay *DisplayManager::displayForRect(const base::Rect &rect) const
{
    if (!m_applied || rect.isEmpty())
        return nearestDisplay(pixelCenter(rect));

    const LogicalDisplay *selected = nullptr;
    int64_t best = -1;
    for (const LogicalDisplay &logical : m_applied->logicalDisplays) {
        const int64_t area = intersectionArea(logical.bounds, rect);
        if (!selected || area > best || (area == best && logical.primary && !selected->primary)) {
            selected = &logical;
            best = area;
        }
    }
    return best > 0 ? selected : nearestDisplay(pixelCenter(rect));
}

base::Rect DisplayManager::desktopBounds() const
{
    base::Rect result;
    if (!m_applied)
        return result;
    for (const LogicalDisplay &logical : m_applied->logicalDisplays)
        result = result.united(logical.bounds);
    return result;
}

std::optional<DisplayManager::DestinationMapping>
DisplayManager::mappingFor(const DisplayId &destination) const
{
    if (!m_applied)
        return std::nullopt;
    const LogicalDisplay *logical = nullptr;
    for (const auto &candidate : m_applied->logicalDisplays) {
        if (candidate.hasDestination(destination)) {
            logical = &candidate;
            break;
        }
    }
    const DisplaySnapshot *physical = nullptr;
    for (const auto &candidate : m_applied->physicalDisplays) {
        if (candidate.sessionId == destination) {
            physical = &candidate;
            break;
        }
    }
    if (!logical || !physical || !physical->currentMode)
        return std::nullopt;
    const base::Size pixels = physical->currentMode->pixelSize;
    base::Size oriented = pixels;
    const bool rotated = logical->rotation == DisplayRotation::Rotate90
            || logical->rotation == DisplayRotation::Rotate270;
    if (rotated)
        oriented = oriented.transposed();
    const base::Size extent = logical->bounds.size();
    const double fit = logical->destinations.size() > 1
            ? std::min(double(oriented.width()) / extent.width(),
                       double(oriented.height()) / extent.height())
            : logical->scale.value();
    base::Size content(base::roundToInt(extent.width() * fit),
                       base::roundToInt(extent.height() * fit));
    if (rotated)
        content = content.transposed();
    const base::Rect native = logical->destinations.size() == 1
            ? base::Rect(pixels)
            : base::Rect(base::Point((pixels.width() - content.width()) / 2,
                           (pixels.height() - content.height()) / 2),
                    content);
    return DestinationMapping {logical->bounds, native, fit, logical->rotation};
}

std::optional<DisplayTransform> DisplayManager::transformFor(const DisplayId &id) const
{
    const LogicalDisplay *logical = display(id);
    if (!logical || logical->destinations.empty())
        return std::nullopt;
    const DisplaySnapshot *physical = nullptr;
    for (const auto &candidate : m_applied->physicalDisplays) {
        if (candidate.sessionId == logical->destinations.front()) {
            physical = &candidate;
            break;
        }
    }
    if (!physical || !physical->currentMode)
        return std::nullopt;
    return DisplayTransform(logical->bounds.topLeft(), physical->currentMode->pixelSize,
                            logical->scale, logical->rotation);
}

base::PointF DisplayManager::normalizedToGlobal(const base::PointF &point) const
{
    const base::Rect bounds = desktopBounds();
    return base::PointF(bounds.x() + point.x() * bounds.width(),
                   bounds.y() + point.y() * bounds.height());
}

std::optional<base::PointF>
DisplayManager::touchscreenToGlobal(const base::PointF &normalized) const
{
    if (!m_applied || m_applied->logicalDisplays.empty())
        return std::nullopt;
    // ash's TouchDeviceManager, reduced to its first rules: the touchscreen is
    // the internal panel's when that panel is shown, else the only display's,
    // else the primary's.
    const DisplaySnapshot *panel = nullptr;
    for (const DisplaySnapshot &physical : m_applied->physicalDisplays) {
        if (physical.connectionType == DisplayConnectionType::Internal
            && mappingFor(physical.sessionId)) {
            panel = &physical;
            break;
        }
    }
    if (!panel) {
        const LogicalDisplay *logical = m_applied->logicalDisplays.size() == 1
                ? &m_applied->logicalDisplays.front()
                : primaryDisplay();
        if (!logical || logical->destinations.empty())
            return std::nullopt;
        for (const DisplaySnapshot &physical : m_applied->physicalDisplays) {
            if (physical.sessionId == logical->destinations.front())
                panel = &physical;
        }
    }
    if (!panel || !panel->currentMode)
        return std::nullopt;
    const auto mapping = mappingFor(panel->sessionId);
    if (!mapping)
        return std::nullopt;
    // The glass spans the panel's native mode. A smaller mode on an internal
    // panel goes through the GPU's panel fitter, which keeps the mode's aspect
    // ratio (amdgpu's and i915's default) and centres it between black bars.
    const base::Size pixels = panel->currentMode->pixelSize;
    const base::SizeF glass = panel->connectionType == DisplayConnectionType::Internal
                    && panel->nativeMode
            ? base::SizeF(panel->nativeMode->pixelSize)
            : base::SizeF(pixels);
    base::RectF shown(base::PointF(), fittedInside(base::SizeF(pixels), glass));
    shown.moveCenter(base::RectF(glass).center());
    const base::PointF onShown(
            std::clamp((normalized.x() * glass.width() - shown.x()) / shown.width(), 0.0, 1.0),
            std::clamp((normalized.y() * glass.height() - shown.y()) / shown.height(), 0.0, 1.0));
    // The mode's pixels are where the mapping's transform starts; it takes
    // rotation and mirror letterboxing from there.
    const base::PointF native(onShown.x() * (pixels.width() - 1e-3),
                         onShown.y() * (pixels.height() - 1e-3));
    return mapping->transform().nativeToLogical(native);
}

std::optional<base::PointF> DisplayManager::globalToOutputLocal(const DisplayId &id,
                                                           const base::PointF &point) const
{
    const auto *logical = display(id);
    return logical ? std::optional(point - base::PointF(logical->bounds.topLeft())) : std::nullopt;
}

std::optional<base::PointF> DisplayManager::outputLocalToGlobal(const DisplayId &id,
                                                           const base::PointF &point) const
{
    const auto *logical = display(id);
    return logical ? std::optional(point + base::PointF(logical->bounds.topLeft())) : std::nullopt;
}

const DisplaySnapshot *DisplayManager::physicalDisplay(const DisplayId &id) const
{
    if (!m_hardware)
        return nullptr;
    for (const DisplaySnapshot &physical : m_hardware->displays) {
        if (physical.sessionId == id)
            return &physical;
    }
    return nullptr;
}

} // namespace Lingmo::Display
