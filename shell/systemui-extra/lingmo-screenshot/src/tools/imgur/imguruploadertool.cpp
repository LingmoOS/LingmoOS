/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*
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

#include "imguruploadertool.h"
#include "imguruploader.h"
#include <QPainter>

ImgurUploaderTool::ImgurUploaderTool(QObject *parent) : AbstractActionTool(parent) {

}

bool ImgurUploaderTool::closeOnButtonPressed() const {
    return true;
}

QIcon ImgurUploaderTool::icon(const QColor &background, bool inEditor) const {
   //Q_UNUSED(inEditor);
   // return QIcon(iconPath(background) + "cloud-upload.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
}
#ifdef SUPPORT_LINGMO
QIcon ImgurUploaderTool::icon(const QColor &background, bool inEditor, const CaptureContext &context ) const {
   //Q_UNUSED(inEditor);
   // return QIcon(iconPath(background) + "cloud-upload.svg");
    Q_UNUSED(background);
    return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
                      QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
}
#endif
QString ImgurUploaderTool::name() const {
    return tr("Image Uploader");
}

QString ImgurUploaderTool::nameID() {
    return QLatin1String("");
}

QString ImgurUploaderTool::description() const {
    return tr("Upload the selection to Imgur");
}

QWidget* ImgurUploaderTool::widget() {
    return new ImgurUploader(capture);
}

CaptureTool* ImgurUploaderTool::copy(QObject *parent) {
    return new ImgurUploaderTool(parent);
}

void ImgurUploaderTool::pressed(const CaptureContext &context) {
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
