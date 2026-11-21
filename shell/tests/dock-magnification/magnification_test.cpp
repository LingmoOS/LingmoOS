// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "shell/dock/dock_magnification.h"
#include "shell/dock/dock_presentation.h"

namespace {

using Magnification = Lingmo::Shell::DockMagnification;

void check(bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void near(double actual, double expected, const std::string &message)
{
    check(std::abs(actual - expected) < 1e-6,
          message + ": actual=" + std::to_string(actual) + ", expected=" + std::to_string(expected));
}

class MagnificationFrames : public Magnification
{
public:
    MagnificationFrames() : Magnification(nullptr) {}
    using Magnification::tick;
};

class PresentationFrames : public Lingmo::Shell::DockPresentation
{
public:
    PresentationFrames() : DockPresentation(nullptr) {}
    using DockPresentation::tick;
};

std::vector<Magnification::Item> itemsFor(const std::vector<double> &centres)
{
    std::vector<Magnification::Item> items;
    for (double centre : centres)
        items.push_back({centre, 54});
    return items;
}

void checkSweep(const std::vector<double> &centres)
{
    auto items = itemsFor(centres);
    auto ordered = items;
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) {
        return a.centre < b.centre;
    });
    for (double factor : {1.25, 1.5, 2.0}) {
        for (double pointer = ordered.front().centre - 70; pointer <= ordered.back().centre + 70; pointer += 7) {
            MagnificationFrames actual;
            MagnificationFrames reference;
            actual.setItems(items, 70);
            reference.setItems(ordered, 70);
            actual.setFactor(factor);
            reference.setFactor(factor);
            actual.setPointer(pointer);
            reference.setPointer(pointer);
            const auto result = actual.targets();
            const auto expected = reference.targets();
            for (size_t i = 0; i < items.size(); ++i) {
                const auto it = std::find_if(ordered.begin(), ordered.end(), [&](const auto &item) {
                    return item.centre == items[i].centre;
                });
                const size_t index = size_t(it - ordered.begin());
                near(result[i].scale, expected[index].scale, "storage order changes scale");
                near(result[i].shift, expected[index].shift, "storage order changes shift");
                check(std::abs(result[i].shift) < 4 * 70, "magnification spreads across the output");
            }
            actual.setPointer(std::nullopt);
            for (const auto &placement : actual.targets()) {
                near(placement.scale, 1, "pointer leave scale");
                near(placement.shift, 0, "pointer leave shift");
            }
        }
    }
}

void testNormalLayout()
{
    MagnificationFrames magnification;
    magnification.setItems(itemsFor({27, 97, 167}), 70);
    magnification.setPointer(97);
    const auto result = magnification.targets();
    // Existing ordered-row behaviour: the centre icon stays under the
    // pointer; its equal neighbours grow and spread by equal distances.
    near(result[0].scale, 1.3528, "normal left scale");
    near(result[1].scale, 1.5, "normal centre scale");
    near(result[2].scale, 1.3528, "normal right scale");
    near(result[0].shift, -23.0256, "normal left shift");
    near(result[1].shift, 0, "normal centre shift");
    near(result[2].shift, 23.0256, "normal right shift");
    magnification.setEnabled(false);
    for (const auto &placement : magnification.targets()) {
        near(placement.scale, 1, "disabled scale");
        near(placement.shift, 0, "disabled shift");
    }
    magnification.setEnabled(true);
    magnification.setItems(itemsFor({27}), 70);
    magnification.setPointer(27);
    near(magnification.targets()[0].scale, 1.5, "single icon scale");
    near(magnification.targets()[0].shift, 0, "single icon shift");
    magnification.setItems({}, 70);
    check(magnification.targets().empty(), "empty layout");
}

void testStorageOrder()
{
    // The icon inserted at slot 1 is appended before the trash in storage.
    checkSweep({27, 167, 237, 307, 377, 447, 517, 587, 657, 727, 97, 797});
    checkSweep({27, 237, 447, 307, 97, 377, 167, 517});
    checkSweep({307, 237, 167, 97, 27});
}

void testDropAndReorder()
{
    PresentationFrames presentation;
    presentation.setPitch(70, false);
    presentation.setItemSize(54);
    presentation.setKeys({"launcher", "files", "terminal"});
    presentation.setAnimationsEnabled(true);
    presentation.beginDrag(-1, false);
    presentation.setDropSlot(2, true);
    presentation.tick(1'000'000'000);
    presentation.tick(2'000'000'000);
    presentation.setKeys({"launcher", "files", "editor", "terminal"});
    presentation.endDrag();
    presentation.tick(3'000'000'000);
    presentation.tick(4'000'000'000);
    check(!presentation.isDragging() && !presentation.isRunning(), "drop did not settle");
    check(presentation.entries().back().key == "editor", "test must reproduce appended storage");

    const auto sweepPresentation = [&] {
        std::vector<double> centres;
        for (const auto &entry : presentation.entries())
            centres.push_back(presentation.position(entry) + 27);
        centres.push_back(presentation.length() + 27); // Trash.
        checkSweep(centres);
    };
    sweepPresentation();
    presentation.setKeys({"launcher", "editor", "terminal", "files"});
    presentation.tick(5'000'000'000);
    presentation.tick(6'000'000'000);
    sweepPresentation();

    // Another launcher drop, this time before the first pinned icon.
    presentation.beginDrag(-1, false);
    presentation.setDropSlot(1, true);
    presentation.setKeys({"launcher", "calendar", "editor", "terminal", "files"});
    presentation.endDrag();
    presentation.tick(7'000'000'000);
    presentation.tick(8'000'000'000);
    sweepPresentation();
}

void testOverlappingRestPositions()
{
    // Icons can share or cross a position during insertion/reordering. With
    // almost no magnification the displacement must approach zero too.
    MagnificationFrames magnification;
    magnification.setItems(itemsFor({27, 167, 97, 97, 114}), 70);
    magnification.setFactor(1 + 1e-9);
    magnification.setPointer(100);
    for (const auto &placement : magnification.targets())
        check(std::abs(placement.shift) < 1e-5, "overlap creates a permanent layout gap");
}

void testAnimationReturnsToRest()
{
    MagnificationFrames magnification;
    magnification.setItems(itemsFor({27, 167, 237, 97, 307}), 70);
    magnification.setPointer(97);
    uint64_t time = 1'000'000'000;
    for (int frame = 0; frame < 180; ++frame) {
        magnification.tick(time);
        time += 16'666'667;
    }
    const auto expected = magnification.targets();
    for (size_t i = 0; i < expected.size(); ++i) {
        near(magnification.placements()[i].scale, expected[i].scale, "animated scale");
        near(magnification.placements()[i].shift, expected[i].shift, "animated shift");
    }
    magnification.setPointer(std::nullopt);
    for (int frame = 0; frame < 180; ++frame) {
        magnification.tick(time);
        time += 16'666'667;
    }
    check(!magnification.isRunning(), "return animation did not stop");
    for (const auto &placement : magnification.placements()) {
        near(placement.scale, 1, "returned scale");
        near(placement.shift, 0, "returned shift");
    }
}

} // namespace

int main(int argc, char **argv)
{
    try {
        const std::string selected = argc > 1 ? argv[1] : "all";
        check(selected == "all" || selected == "normal-layout" || selected == "storage-order" || selected == "drop-and-reorder"
                      || selected == "overlaps" || selected == "return-to-rest", "unknown test case");
        if (selected == "all" || selected == "normal-layout")
            testNormalLayout();
        if (selected == "all" || selected == "storage-order")
            testStorageOrder();
        if (selected == "all" || selected == "drop-and-reorder")
            testDropAndReorder();
        if (selected == "all" || selected == "overlaps")
            testOverlappingRestPositions();
        if (selected == "all" || selected == "return-to-rest")
            testAnimationReturnsToRest();
        std::puts("Dock magnification tests passed");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
