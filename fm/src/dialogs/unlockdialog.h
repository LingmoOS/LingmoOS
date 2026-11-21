// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include <functional>
#include <memory>
#include <string>

namespace Lingmo::ui {
class DialogWindow;
class Window;
}

namespace fm {

class PlacesModel;

// The passphrase of an encrypted disk.
class UnlockVolumeDialog
{
public:
    UnlockVolumeDialog(ui::Window* parent, PlacesModel& places, std::string udi, std::string name, bool retry);
    ~UnlockVolumeDialog();

    void show();
    // The passphrase typed stays.
    void retranslate();
    std::function<void()> onDone;

private:
    class Content;

    std::unique_ptr<ui::DialogWindow> m_dialog;
    Content* m_content = nullptr;
    PlacesModel& m_places;
    std::string m_udi;
    std::string m_name;
    bool m_retry;
};

} // namespace fm
