// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QMimeData>

#include "dropframe.h"

inline bool checkMimeData(const QMimeData *mimeData)
{
    if (mimeData->hasUrls() && mimeData->urls().size() > 0)
    {
        QUrl fileUrl = mimeData->urls()[0];
        if (fileUrl.isLocalFile() && fileUrl.fileName().endsWith("iso"))
        {
            return true;
        }
    }
    return false;
}

DropFrame::DropFrame(QWidget *parent)
    : BackgroundFrame(parent)
{
    setAcceptDrops(true);
}

void DropFrame::dragEnterEvent(QDragEnterEvent *event)
{
    if (checkMimeData(event->mimeData()))
    {
        // magic_t cookie = magic_open(MAGIC_MIME_TYPE);
        // if (cookie == nullptr) {
        //     return;
        // }

        // if (magic_load(cookie, nullptr) != 0) {
        //     printf("cannot load magic database - %s\n", magic_error(cookie));
        //     magic_close(cookie);
        //     return;
        // }
        // const char* mime_type = magic_file(cookie, fileUrl.path().toStdString().c_str());
        // std::cout << mime_type << std::endl;

        // Check extensions only
        event->acceptProposedAction();
        Q_EMIT fileAboutAccept();

        // // Check file contents which requires libmagic
        // if (strcmp(mime_type, "application/x-iso9660-image") == 0) {
        //     event->acceptProposedAction();
        // }
        // magic_close(cookie);
    }
}

void DropFrame::dragLeaveEvent(QDragLeaveEvent *event)
{
    QFrame::dragLeaveEvent(event);
    emit fileCancel();
}

void DropFrame::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (!checkMimeData(mimeData)) {
        return;
    }

    emit fileDrop(mimeData->urls().first().toLocalFile());
    // emit fileCancel();
}
