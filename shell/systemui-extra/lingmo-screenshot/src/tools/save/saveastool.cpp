/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
 *
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
#include "saveastool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#include <QStandardPaths>
SaveAsTool::SaveAsTool(QObject *parent) : AbstractActionToolWithWidget(parent) {

}

bool SaveAsTool::closeOnButtonPressed() const {
    return false;
}

QIcon SaveAsTool::icon(const QColor &background, bool inEditor) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "content-save.svg");
    Q_UNUSED(background);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-save.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-save.svg");
    return  QIcon(QStringLiteral(":/img/material/white/down_1.png"));
}
#ifdef SUPPORT_LINGMO
QIcon SaveAsTool::icon(const QColor &background, bool inEditor , const CaptureContext &context) const {
    //Q_UNUSED(inEditor);
    //return QIcon(iconPath(background) + "content-save.svg");
    Q_UNUSED(background);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-save.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-save.svg");
    return  QIcon(QStringLiteral(":/img/material/white/down_1.png"));
}
#endif
QString SaveAsTool::name() const {
    return tr("saveas");
}

QString SaveAsTool::nameID() {
    return QLatin1String("");
}

QString SaveAsTool::description() const {
    return tr("Save the capture.....");
}

CaptureTool* SaveAsTool::copy(QObject *parent) {
    return new SaveAsTool(parent);
}

void SaveAsTool::pressed(const CaptureContext &context) {
   /* if (context.savePath.isEmpty()) {
            emit requestAction(REQ_HIDE_GUI);
            bool ok = ScreenshotSaver().saveToFilesystemGUI(
                        context.selectedScreenshotArea());
            if (ok) {
                emit requestAction(REQ_CAPTURE_DONE_OK);
            }
        } else {
            bool ok = ScreenshotSaver().saveToFilesystem(
                        context.selectedScreenshotArea(), context.savePath);
            if (ok) {
                emit requestAction(REQ_CAPTURE_DONE_OK);
            }
        }*/
    emit requestAction(REQ_CAPTURE_DONE_OK);
}

