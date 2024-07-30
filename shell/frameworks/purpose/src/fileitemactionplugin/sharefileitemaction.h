/*
    SPDX-FileCopyrightText: 2011 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHAREFILEITEMACTION_H
#define SHAREFILEITEMACTION_H

#include <KAbstractFileItemActionPlugin>
#include <KFileItemListProperties>
#include <QLoggingCategory>

class QAction;
class KFileItemListProperties;
class QWidget;

namespace Purpose
{
class Menu;
}

Q_DECLARE_LOGGING_CATEGORY(PURPOSE_FILEITEMACTION)
class ShareFileItemAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT
public:
    ShareFileItemAction(QObject *parent);
    ~ShareFileItemAction() override;
    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget) override;

private:
    Purpose::Menu *m_menu;
    bool m_isFinished = false;
};

#endif // SHAREFILEITEMACTION_H
