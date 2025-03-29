/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CREATEFOLDERDIALOG_H
#define CREATEFOLDERDIALOG_H

#include <QObject>

class CreateFolderDialog : public QObject
{
    Q_OBJECT

public:
    explicit CreateFolderDialog(QObject *parent = nullptr);

    void setPath(const QString &path);
    void show();

    Q_INVOKABLE void newFolder(const QString &folderName);

private:
    QString m_path;
};

#endif // CREATEFOLDERDIALOG_H
