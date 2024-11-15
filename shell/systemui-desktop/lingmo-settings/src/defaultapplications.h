/*
 * Copyright (C) 2021 - 2022 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
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

#ifndef DEFAULTAPPLICATIONS_H
#define DEFAULTAPPLICATIONS_H

#include <QObject>

class AppItem {
public:
    QString path;
    QString fileName;
    QString name;
    QString icon;
    QString mimeType;
    QString categories;
};

class DefaultApplications : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList browserList READ browserList NOTIFY loadFinished)
    Q_PROPERTY(QVariantList fileManagerList READ fileManagerList NOTIFY loadFinished)
    Q_PROPERTY(QVariantList emailList READ emailList NOTIFY loadFinished)
    Q_PROPERTY(QVariantList terminalList READ terminalList NOTIFY loadFinished)
    Q_PROPERTY(int browserIndex READ browserIndex NOTIFY loadFinished)
    Q_PROPERTY(int fileManagerIndex READ fileManagerIndex NOTIFY loadFinished)
    Q_PROPERTY(int emailIndex READ emailIndex NOTIFY loadFinished)
    Q_PROPERTY(int terminalIndex READ terminalIndex NOTIFY loadFinished)

public:
    explicit DefaultApplications(QObject *parent = nullptr);

    void loadApps();

    QVariantList browserList();
    QVariantList fileManagerList();
    QVariantList emailList();
    QVariantList terminalList();

    int browserIndex();
    int fileManagerIndex();
    int emailIndex();
    int terminalIndex();

    Q_INVOKABLE void setDefaultBrowser(int index);
    Q_INVOKABLE void setDefaultFileManager(int index);
    Q_INVOKABLE void setDefaultEMail(int index);
    Q_INVOKABLE void setDefaultTerminal(int index);

private:
    void setDefaultApp(const QString &mimeType, const QString &path);
    QString mimeAppsListFilePath() const;

signals:
    void loadFinished();

private:
    QList<AppItem> m_browserList;
    QList<AppItem> m_fileManagerList;
    QList<AppItem> m_emailList;
    QList<AppItem> m_terminalList;

    int m_browserIndex = -1;
    int m_fileManagerIndex = -1;
    int m_emailIndex = -1;
    int m_terminalIndex = -1;
};

#endif // DEFAULTAPPLICATIONS_H
