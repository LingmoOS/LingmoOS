/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef __keditfiletype_h
#define __keditfiletype_h

#include <QDialog>

class QDialogButtonBox;

class MimeTypeData;
class FileTypeDetails;

// A dialog for ONE file type to be edited.
class FileTypeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FileTypeDialog(MimeTypeData *mime);
    ~FileTypeDialog() override;

    void setApplyButtonEnabled(bool);

public Q_SLOTS:
    void accept() override;

protected Q_SLOTS:
    void clientChanged(bool state);
    void slotDatabaseChanged();

private Q_SLOTS:
    void save();

private:
    void init();
    FileTypeDetails *m_details;
    MimeTypeData *m_mimeTypeData;
    QDialogButtonBox *m_buttonBox;
};

#endif
