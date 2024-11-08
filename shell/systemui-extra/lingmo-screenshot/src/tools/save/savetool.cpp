/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*               2020 KylinSoft Co., Ltd.
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

#include "savetool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#include <QStandardPaths>
#include <src/core/controller.h>
SaveTool::SaveTool(QObject *parent) : AbstractActionTool(parent)
{
}

bool SaveTool::closeOnButtonPressed() const
{
    return true;
}

QIcon SaveTool::icon(const QColor &background, bool inEditor) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "content-save.svg");
    Q_UNUSED(background);
    /*return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-save.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "content-save.svg");*/
    return QIcon();
}

#ifdef SUPPORT_LINGMO
QIcon SaveTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    // Q_UNUSED(inEditor);
    // return QIcon(iconPath(background) + "content-save.svg");
    Q_UNUSED(background);
    /*return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-save.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "content-save.svg");*/
    return QIcon();
}

#endif
QString SaveTool::name() const
{
    return tr("save");
}

QString SaveTool::nameID()
{
    return QLatin1String("");
}

QString SaveTool::description() const
{
    return tr("Save the capture");
}

CaptureTool *SaveTool::copy(QObject *parent)
{
    return new SaveTool(parent);
}

void SaveTool::pressed(const CaptureContext &context)
{
#ifndef SUPPORT_NEWUI
    QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (context.savePath.isEmpty()) {
        emit requestAction(REQ_HIDE_GUI);
        if (context.saveType.isEmpty()) {
            bool ok = ScreenshotSaver().saveToFilesystem(
                context.selectedScreenshotArea(), a.at(0), ".png");
            if (ok) {
                emit requestAction(REQ_CAPTURE_DONE_OK);
            }
        } else {
            bool ok = ScreenshotSaver().saveToFilesystem(
                context.selectedScreenshotArea(), a.at(0), context.saveType);
            if (ok) {
                emit requestAction(REQ_CAPTURE_DONE_OK);
            }
        }
    } else {
        bool ok = ScreenshotSaver().saveToFilesystem(
            context.selectedScreenshotArea(), context.savePath, context.saveType);
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        }
    }
#else
    QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (context.savePath.isEmpty()) {
        emit requestAction(REQ_HIDE_GUI);
        bool ok;
        if(Controller::getInstance()->getScreenSaveLockStatus()) {
            ok = ScreenshotSaver().saveToFilesystem(
                context.selectedScreenshotArea(), a.at(0));
        } else {
            ok = ScreenshotSaver().saveToFilesystemGUI(
                context.selectedScreenshotArea());
        }
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        } else {
            emit captureExit(0);
        }
    } else {
        bool ok = ScreenshotSaver().saveToFilesystem(
            context.selectedScreenshotArea(), context.savePath);
        if (ok) {
            emit requestAction(REQ_CAPTURE_DONE_OK);
        } else {
            emit captureExit(0);
        }
    }
#endif
}
