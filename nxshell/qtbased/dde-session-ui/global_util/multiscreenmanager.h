// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTISCREENMANAGER_H
#define MULTISCREENMANAGER_H

#include <QObject>
#include <QWidget>
#include <QScreen>
#include <QMap>
#include <functional>
#include <QTimer>

class MultiScreenManager : public QObject
{
    Q_OBJECT
public:
    explicit MultiScreenManager(QObject *parent = nullptr);

    void register_for_mutil_screen(std::function<QWidget* (QScreen *)> function);
    void startRaiseContentFrame();

private:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void raiseContentFrame();

private:
    std::function<QWidget* (QScreen *)> m_registerFunction;
    QMap<QScreen*, QWidget*> m_frames;
    QTimer *m_raiseContentFrameTimer;
};

#endif // MULTISCREENMANAGER_H
