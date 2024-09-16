/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SPLITMANAGER_H
#define SPLITMANAGER_H

#include "../utils/common.h"
#include "splitbar.h"
#include <QMutex>

namespace KWin {

class Window;

class SplitGroup
{
public:
    SplitGroup(int);
    ~SplitGroup(){};

    int desktop() { return m_desktop; };
    void setDesktop(int);
    void storeSplitWindow(Window *);
    void deleteSplitWindow(Window *);
    void getSplitWindow(QVector<Window *> &);
    int getNumSplitWin() {return m_splitGroup.size();}

private:
    int m_desktop;
    QVector<Window *> m_splitGroup;
};

class SplitManage : public QObject
{
    Q_OBJECT

public:
    explicit SplitManage();
    ~SplitManage(){};

    void add(Window *window);
    void remove(Window *window);
    void getSplitWindows(QHash<QString, QVector<Window *>> &);
    void getSplitBarWindow(QHash<QString, Window *> &);

    void inhibit();
    void uninhibit();
    void removeInternal(Window *window);

    void resetNumSplitWin(Window *window);
    int getNumSplitWin() {return m_numSplitWin;}

Q_SIGNALS:
    void signalSplitWindow(QString &, Window *w);

public Q_SLOTS:
    void updateSplitWindowGeometry(QString, QPointF, Window *, bool);

private:
    struct WindowData
    {
        QRectF geometry;
        QuickTileMode quickTile;
        QString screenName;
        int desktop;
    };

private:
    void handleQuickTile();
    void windowScreenChange();
    void windowDesktopChange();
    void windowFrameSizeChange();
    void windowKeepAboveChange(bool);
    void removeQuickTile(Window *window);
    void addQuickTile(int desktop, QString screenName, Window *window);
    void updateStorage(Window *window);
    bool isSplitWindow(Window *window);
    void createSplitBar(QString);
    void updateSplitWindowsGroup();
    SplitGroup *createGroup(int &, QString &);
    SplitGroup *getGroup(int &, QString &);
    WindowData dataForWindow(Window *window) const;

private:
    int                                 m_inhibitCount = 0;
    QHash<QString, QSet<SplitGroup *>>  m_splitGroupManage;
    QHash<QString, SplitBar *>          m_splitBarManage;
    QHash<QString, Window *>            m_splitBarWindows;
    QHash<Window *, WindowData>         m_data;
    QMutex                              m_mutex;
    Window                              *m_lastWin = nullptr;
    Window                              *m_topWin = nullptr;
    Window                              *m_activeWin = nullptr;
    Layer                               m_topLayer = NormalLayer;
    bool                                m_isCreatePlaceHolder = false;
    int                                 m_numSplitWin = 0;
};
}

#endif
