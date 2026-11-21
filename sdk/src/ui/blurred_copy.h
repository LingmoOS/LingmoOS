// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <utility>

#include "ui/view.h"

namespace Lingmo::ui {

// A blurred copy of a view, such as a text or icon shadow: the content view is
// drawn into a texture the size of this view (whole logical pixels, then the
// device pixel ratio), blurred by the six-level mip blur of multi_effect_blur.h
// (blurMax, blur, padded automatically), and shown at this view's opacity,
// reaching blurMax past each edge. The content itself is not shown. It lives
// outside the window's tree: call contentChanged() after changing it. The blur
// is computed on the CPU once per change, size or scale.
class BlurredCopy : public View
{
public:
    explicit BlurredCopy(double blurMax = 6, double blur = 0.5);
    ~BlurredCopy() override;

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
    View* content() const { return m_content.get(); }
    void contentChanged();

    RectF visualRect() const override;

protected:
    void layout() override;
    void paint(Painter& painter) override;

private:
    bool adoptTextLayoutMode();

    double m_blurMax;
    double m_blur;
    std::unique_ptr<View> m_content;
    // The blurred image and what it was made for.
    struct Cache;
    std::unique_ptr<Cache> m_cache;
};

} // namespace Lingmo::ui
