// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "ui/view.h"

#include <functional>
#include <string>

namespace fm {

class RenameField;

// The name edited in place over an item's label, in a
// highlighted rounded frame; Return or Tab keeps it, Escape drops it, and a
// name that cannot be used says why.
class RenameEditor : public ui::View
{
public:
    // selectionEnd is the byte offset the first selection ends at: before
    // the extension.
    RenameEditor(std::string name, int selectionEnd, bool centered);
    ~RenameEditor() override;

    // The label's pixel size (0 for the theme's) and weight.
    void setFont(double pixelSize, int weight);
    // The label's rect in the parent's coordinates and the parent's width.
    void setTarget(const ui::RectF& label, double viewWidth);
    void begin();
    // False, with the reason shown, when the name cannot be used.
    bool commit();
    void cancel();

    std::function<void(const std::string& name)> onSubmitted;
    std::function<void()> onFinished;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    friend class RenameField;

    void finish();
    void place();
    void setError(std::string message);

    std::string m_original;
    int m_selectionEnd;
    bool m_centered;
    bool m_finishing = false;
    std::string m_error;
    ui::RectF m_label;
    double m_viewWidth = 0;
    RenameField* m_field;
};

} // namespace fm
