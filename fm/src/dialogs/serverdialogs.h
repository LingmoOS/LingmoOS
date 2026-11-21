// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class DialogWindow;
class TextField;
class Window;
}

namespace fm {

class PlacesModel;

// An address, or one of the recent servers.
class ConnectServerDialog
{
public:
    ConnectServerDialog(ui::Window* parent, PlacesModel& places);
    ~ConnectServerDialog();

    void show();
    // The address typed stays.
    void retranslate();
    std::function<void()> onDone;

private:
    class Content;

    std::unique_ptr<ui::DialogWindow> m_dialog;
    PlacesModel& m_places;
    ui::TextField* m_field = nullptr;
};

// A name, domain and password, or a guest.
class ServerPasswordDialog
{
public:
    struct Request
    {
        std::string address;
        std::string user;
        std::string domain;
        bool needsDomain = false;
        bool anonymousAllowed = false;
        bool retry = false;
    };

    ServerPasswordDialog(ui::Window* parent, PlacesModel& places, Request request);
    ~ServerPasswordDialog();

    void show();
    // What was typed and chosen stays.
    void retranslate();
    std::function<void()> onDone;

private:
    class Content;

    std::unique_ptr<ui::DialogWindow> m_dialog;
    Content* m_content = nullptr;
    PlacesModel& m_places;
    Request m_request;
    bool m_answered = false;
};

} // namespace fm
