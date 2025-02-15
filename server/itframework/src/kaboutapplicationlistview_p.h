/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KABOUT_APPLICATION_LIST_VIEW_H
#define KABOUT_APPLICATION_LIST_VIEW_H

#include <QListView>

namespace KDEPrivate
{
class KAboutApplicationListView : public QListView
{
    Q_OBJECT
public:
    explicit KAboutApplicationListView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *e) override;
};

} // namespace KDEPrivate

#endif // KABOUT_APPLICATION_LIST_VIEW_H
