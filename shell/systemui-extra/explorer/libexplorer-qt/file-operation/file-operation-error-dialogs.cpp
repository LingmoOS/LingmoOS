/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Ding <dingjing@kylinos.cn>
 *
 */
#include "file-operation-error-dialogs.h"

#include <QUrl>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <file-info.h>
#include <QHBoxLayout>
#include <file-info-job.h>
#include <file-utils.h>
#include <QStyleOptionViewItem>
#include "sound-effect.h"
#ifdef LINGMO_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif

#include "file-operation-dialog/kyfiledialogrename.h"

#ifdef LINGMO_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

static QPixmap drawSymbolicColoredPixmap (const QPixmap& source);

static QString formatGerrorString (const Peony::FileOperationError* error);

static const int ELIDE_TEXT_LENGTH = 960;


Peony::FileOperationErrorDialogConflict::FileOperationErrorDialogConflict(FileOperationErrorDialogBase *parent)
    : FileOperationErrorDialogBase(parent)
{
    QPushButton* b = addButton (tr("Replace"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ignore = false;
        m_backup = false;
        m_replace = true;
        done(QDialog::Accepted);
    });

    b = addButton (tr("Ignore"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ignore = true;
        m_backup = false;
        m_replace = false;
        done(QDialog::Accepted);
    });

    b = addButton (tr("Backup"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ignore = false;
        m_backup = true;
        m_replace = false;
        done(QDialog::Accepted);
    });
    b->setDefault(true);

    QCheckBox* c = addCheckBoxLeft (tr("Do the same"));
    connect(c, &QCheckBox::stateChanged, this, [=](int chose) {
        switch (chose) {
        case Qt::Checked:
            m_do_same = true;
            break;
        case Qt::Unchecked:
        default:
            m_do_same = false;
        }
    });
}

Peony::FileOperationErrorDialogConflict::~FileOperationErrorDialogConflict()
{

}

void Peony::FileOperationErrorDialogConflict::setTipFilename(QString name)
{
    if (!name.isNull () && !name.isEmpty()) {
        QStyleOptionViewItem opt;
        QString fileName = QUrl(name).toDisplayString();
        setText(QString(tr("<p>This location already contains the file '%1', Do you want to override it?</p>"))
                    .arg(opt.fontMetrics.elidedText(fileName, Qt::ElideMiddle, 480)));
    } else {
        //fix file name show as chaos code issue, link to bug#116596
        auto src = FileUtils::urlDecode(m_error->srcUri);
        auto destDir = FileUtils::urlDecode(m_error->destDirUri);
        setText(tr("Unexpected error from %1 to %2").arg(src).arg(destDir));
        qCritical()<<QString("Unexpected error from %1 to %2").arg(src).arg(destDir);
    }
}

void Peony::FileOperationErrorDialogConflict::setTipFileicon(QString icon)
{
    if (!icon.isNull () && !icon.isEmpty()) {
        setIcon (icon);
    }
}

void Peony::FileOperationErrorDialogConflict::handle (FileOperationError& error)
{
    m_error = &error;

    if (FileOpRename == m_error->op || FileOpUntrash == m_error->op) {
        FileInfoJob file(error.destDirUri, nullptr);
        file.querySync();
        setTipFileicon(file.getInfo()->iconName());
        setTipFilename(file.getInfo()->displayName());
    } else {
        QString fileName = error.srcUri.split("/").back();
        //fix bug 148806, matches end path name
        QString url = error.destDirUri.split("/").back().contains(fileName) ? error.destDirUri : error.destDirUri + "/" + fileName;
        FileInfoJob file(url, nullptr);
        file.querySync();
        setTipFileicon(file.getInfo()->iconName());
        setTipFilename(file.getInfo()->displayName());
    }

    error.respCode = Retry;
    int ret = exec();
    if (QDialog::Accepted == ret) {
        if (m_do_same) {
            if (m_replace) {
                error.respCode = OverWriteAll;
            } else if (m_backup) {
                error.respCode = BackupAll;
            } else if (m_ignore) {
                error.respCode = IgnoreAll;
            } else {
                error.respCode = Cancel;
            }
        } else {
            if (m_replace) {
                error.respCode = OverWriteOne;
            } else if (m_backup) {
                error.respCode = BackupOne;
            } else if (m_ignore) {
                error.respCode = IgnoreOne;
            } else {
                error.respCode = Cancel;
            }
        }
    } else {
        error.respCode = Cancel;
    }
}


Peony::FileOperationErrorHandler *Peony::FileOperationErrorDialogFactory::getDialog(Peony::FileOperationError &errInfo)
{
    FileOperationErrorHandler* dlg = nullptr;

    switch (errInfo.dlgType) {
    case ED_CONFLICT:
        dlg = new FileOperationErrorDialogConflict();
        break;
    case ED_WARNING:
        dlg = new FileOperationErrorDialogWarning();
        break;
    case ED_NOT_SUPPORTED: {
        dlg = new FileOperationErrorDialogNotSupported();
        break;
    }
#ifdef LINGMO_FILE_DIALOG
    case ED_RENAME: {
        dlg = new KyFileDialogRename();
        break;
    }
#endif
    default:
        dlg = new FileOperationErrorDialogWarning();
        break;
    }

    return dlg;
}

Peony::FileOperationErrorDialogWarning::FileOperationErrorDialogWarning(Peony::FileOperationErrorDialogBase *parent)
    : FileOperationErrorDialogBase(parent)
{
    QPushButton* b = addButton (tr("OK"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ok = true;
        m_cancel = false;
        done(QDialog::Accepted);
    });
    b->setDefault(true);

    m_cancel_btn = b = addButton (tr("Cancel"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ok = false;
        m_cancel = true;
        done(QDialog::Rejected);
    });

}

Peony::FileOperationErrorDialogWarning::~FileOperationErrorDialogWarning()
{

}

void Peony::FileOperationErrorDialogWarning::handle(Peony::FileOperationError &error)
{
    m_error = &error;
    //SoundEffect::getInstance()->copyOrMoveFailedMusic();
    //Task#152997, use sdk play sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
    kdk::KSoundEffects::playSound(SoundType::OPERATION_UNSUPPORTED);
#endif
    QStyleOptionViewItem opt;
    if (nullptr != m_error->errorStr) {
        auto errorText = m_error->errorStr;
        errorText.replace("\n", "<br>");
        QString htmlString = QString("<p>%1</p>")
                                 .arg(opt.fontMetrics.elidedText(m_error->errorStr/*.toHtmlEscaped()*/, Qt::ElideMiddle, ELIDE_TEXT_LENGTH).toHtmlEscaped());
        setText(htmlString);
    } else {
        QString htmlString = QString("<p>%1</p>")
                                 .arg(opt.fontMetrics.elidedText(tr("Make sure the disk is not full or write protected and that the file is not protected"), Qt::ElideMiddle, ELIDE_TEXT_LENGTH).toHtmlEscaped());
        setText(htmlString);
    }

    //fix bug#161394, support cancel rename operation
//    if (m_error->op && FileOpRenameToHideFile == m_error->op) {
//        if (m_cancel_btn) {
//            delete m_cancel_btn;
//        }
//    }

    int ret = exec();

    switch (m_error->errorCode) {
    case G_IO_ERROR_BUSY:
    case G_IO_ERROR_PENDING:
    case G_IO_ERROR_NO_SPACE:
    case G_IO_ERROR_CANCELLED:
    case G_IO_ERROR_INVALID_DATA:
    case G_IO_ERROR_NOT_SUPPORTED:
    case G_IO_ERROR_PERMISSION_DENIED:
    case G_IO_ERROR_CANT_CREATE_BACKUP:
    case G_IO_ERROR_TOO_MANY_OPEN_FILES:
        error.respCode = Cancel;
        break;
    case G_IO_ERROR_FAILED:
        error.respCode = IgnoreAll;
        break;
    default:
        error.respCode = IgnoreOne;
        break;
    }

    // Delete file to the Recycle Bin error, prompt whether to force deletion
    if (QDialog::Accepted == ret && m_error->op == FileOpTrash && m_error->errorCode == G_IO_ERROR_FILENAME_TOO_LONG) {
        error.respCode = Force;
    }

    if (QDialog::Rejected == ret) {
        error.respCode = Cancel;
    }
}

static QPixmap drawSymbolicColoredPixmap (const QPixmap& source)
{
    // 18, 32, 69
    QPushButton      m_btn;
    QColor baseColor = m_btn.palette().color(QPalette::Text).light(150);
    QImage img = source.toImage();

    for (int x = 0; x < img.width(); ++x) {
        for (int y = 0; y < img.height(); ++y) {
            auto color = img.pixelColor(x, y);
            color.setRed(baseColor.red());
            color.setGreen(baseColor.green());
            color.setBlue(baseColor.blue());
            img.setPixelColor(x, y, color);
        }
    }

    return QPixmap::fromImage(img);
}

Peony::FileOperationErrorDialogNotSupported::FileOperationErrorDialogNotSupported(Peony::FileOperationErrorDialogBase *parent) : FileOperationErrorDialogBase(parent)
{
    setIcon ("dialog-warning");

    QPushButton* b = addButton (tr("No"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ok = false;
        m_cancel = true;
        done(QDialog::Rejected);
    });

    b = addButton (tr("Yes"));
    b->setBackgroundRole(QPalette::Button);
    connect(b, &QPushButton::pressed, this, [=] () {
        m_ok = true;
        m_cancel = false;
        done(QDialog::Accepted);
    });
    b->setDefault(true);
//    QCheckBox* c = addCheckBoxLeft (tr("Do the same"));
//    connect(c, &QCheckBox::stateChanged, this, [=](int chose) {
//        switch (chose) {
//        case Qt::Checked:
//            m_do_same = true;
//            break;
//        case Qt::Unchecked:
//        default:
//            m_do_same = false;
//        }
//    });
}

Peony::FileOperationErrorDialogNotSupported::~FileOperationErrorDialogNotSupported()
{

}

void Peony::FileOperationErrorDialogNotSupported::handle(Peony::FileOperationError &error)
{
    m_error = &error;

    QStyleOptionViewItem opt;
    if (nullptr != m_error->errorStr) {
        QString htmlString = QString("<p>%1</p>")
                                 .arg(opt.fontMetrics.elidedText(m_error->errorStr.toHtmlEscaped(), Qt::ElideMiddle, ELIDE_TEXT_LENGTH).toHtmlEscaped());
        setText(htmlString);
    } else {
        QString htmlString = QString("<p>%1</p>")
                                 .arg(opt.fontMetrics.elidedText(tr("Make sure the disk is not full or write protected and that the file is not protected"), Qt::ElideMiddle, ELIDE_TEXT_LENGTH).toHtmlEscaped());
        setText(htmlString);
    }

    int ret = exec();

    switch (m_error->errorCode) {
    case G_IO_ERROR_NOT_SUPPORTED: {
        if (!m_do_same)
            error.respCode = OverWriteOne;
        else
            error.respCode = OverWriteAll;
        break;
    }
    case G_IO_ERROR_BUSY:
    case G_IO_ERROR_PENDING:
    case G_IO_ERROR_NO_SPACE:
    case G_IO_ERROR_CANCELLED:
    case G_IO_ERROR_INVALID_DATA:
    case G_IO_ERROR_PERMISSION_DENIED:
    case G_IO_ERROR_CANT_CREATE_BACKUP:
    case G_IO_ERROR_TOO_MANY_OPEN_FILES:
        error.respCode = Cancel;
        break;
    default:
        error.respCode = IgnoreOne;
        break;
    }

    if (QDialog::Accepted == ret && m_error->op == FileOpTrash && G_IO_ERROR_NOT_SUPPORTED == m_error->errorCode) {
        error.respCode = m_do_same ? error.respCode = ForceAll : error.respCode = Force;
    } else if (QDialog::Rejected == ret) {
        error.respCode = Cancel;
    }
}


// @note 希望错误提示定制化暂时先在这里统一处理，后续需要优化时候可以统一从这里迁移。
static QString formatGerrorString (const Peony::FileOperationError* error)
{
    QStyleOptionViewItem opt;
    using namespace Peony;

    qDebug() << "error code:" << error->errorCode << "  error msg:" << error->errorStr;

    QString errorStr = error->errorStr;

    switch (error->op) {
    case FileOpCreateTemp:
        if (ET_GIO == error->errorType) {
            switch (error->errorCode) {
            case G_IO_ERROR_PERMISSION_DENIED: {
                QString fileName = error->srcUri.split ("/").last ();
                QString filePath = error->destDirUri + (fileName.isEmpty () ? "" : ("/" + error->srcUri.split ("/").last ()));
                errorStr = QString(QObject::tr("Failed to open file \"%1\": insufficient permissions.")).arg (opt.fontMetrics.elidedText(FileUtils::urlDecode (filePath), Qt::ElideMiddle, 380));
                break;
            }
            }
        }
        break;
    case FileOpCopy:
        if (ET_GIO == error->errorType) {
            switch (error->errorCode) {
            case G_IO_ERROR_NOT_FOUND: {
                QString filePath = QUrl(FileUtils::urlDecode (error->srcUri)).path ();
                errorStr = QString(QObject::tr("File “%1” does not exist. Please check whether the file has been deleted.")).arg (opt.fontMetrics.elidedText(filePath, Qt::ElideMiddle, 380));
                break;
            }
            }
        }
        break;
    default:
        break;
    }

    return errorStr;
}
