/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
 *              2020 KylinSoft Co., Ltd.
 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include "src/widgets/capture/capturebutton.h"
#include "src/tools/capturetool.h"
#include <QObject>

class CaptureTool;

class ToolFactory : public QObject {
    Q_OBJECT

public:

    explicit ToolFactory(QObject *parent = nullptr);

    ToolFactory(const ToolFactory &) = delete;
    ToolFactory & operator=(const ToolFactory &) = delete;

    CaptureTool* CreateTool(
            CaptureButton::ButtonType t,
            QObject *parent = nullptr);
};
