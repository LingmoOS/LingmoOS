// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <utility>

#include "ui/view.h"

namespace Lingmo::ui {

// A copy of a view blurred as the FastBlur effect does it (radius in logical
// pixels, transparentBorder off): the content is drawn into a texture the size
// of this view (whole logical pixels, then the device pixel ratio), and the
// blur stays inside it, the edges clamped. Shown at this view's opacity; the
// content itself is not shown: it lives in the tree under an empty clip, so
// the window lays it out. Call contentChanged() after changing it.
class FastBlurredCopy : public View
{
public:
    explicit FastBlurredCopy(double radius = 0);
    ~FastBlurredCopy() override;

    // The content fills this view; give it children placed as the Item's.
    template<typename T, typename... Args>
    T* setContent(Args&&... args)
    {
        auto content = std::make_unique<T>(std::forward<Args>(args)...);
        T* view = content.get();
        setContentView(std::move(content));
        return view;
    }
    void setContentView(std::unique_ptr<View> content);
    View* content() const { return m_content; }
    void contentChanged();

    double radius() const { return m_radius; }
    void setRadius(double radius);

protected:
    void layout() override;
    void paint(Painter& painter) override;

private:
    double m_radius;
    View* m_holder;
    View* m_content = nullptr;
    // The blurred image and what it was made for.
    struct Cache;
    std::unique_ptr<Cache> m_cache;
};

} // namespace Lingmo::ui
