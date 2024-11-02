/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FCITXVIRTUALKEYBOARDSERVICEPROXY_H
#define FCITXVIRTUALKEYBOARDSERVICEPROXY_H

#include <memory>

#include <QDBusInterface>
#include <QString>

#include "virtualkeyboardentry/fcitxvirtualkeyboardservice.h"

class FcitxVirtualKeyboardServiceProxy : public FcitxVirtualKeyboardService {
public:
    FcitxVirtualKeyboardServiceProxy();
    ~FcitxVirtualKeyboardServiceProxy() override = default;

    void showVirtualKeyboard() const override;
    void hideVirtualKeyboard() const override;

private:
    std::unique_ptr<QDBusInterface> virtualKeyboardService = nullptr;

    static const QString serviceName_;
    static const QString servicePath_;
    static const QString serviceInterface_;
};

#endif // FCITXVIRTUALKEYBOARDSERVICEPROXY_H
