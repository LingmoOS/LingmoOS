// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/launcher.h"
#include "dialogs/framelessdialog.h"

#include <functional>
#include <string>
#include <vector>

namespace Lingmo::ui {
class CheckBox;
class ListView;
}

namespace fm {

// The applications for a file, the default
// first, to open it with once or from now on.
class OpenWithDialog : public FramelessDialog
{
public:
    OpenWithDialog(std::string path, std::string mimeType);

    // The application chosen and Set as default stay.
    void retranslate() override;

    std::function<void()> onDone;

private:
    std::unique_ptr<ui::View> createContent();
    void openApplication();
    void done();

    std::string m_path;
    std::string m_mimeType;
    std::vector<launcher::Application> m_applications;
    int m_current = 0;
    ui::ListView* m_list = nullptr;
    ui::CheckBox* m_default = nullptr;
};

} // namespace fm
