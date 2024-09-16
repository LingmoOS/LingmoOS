// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QWindow>
#include <QString>
#include <QObject>
#include <QPointer>

namespace dock {
class AppItem;

class AbstractWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint32_t id READ id)
    Q_PROPERTY(pid_t pid READ pid NOTIFY pidChanged FINAL)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged FINAL)
    Q_PROPERTY(bool shouldSkip READ shouldSkip NOTIFY shouldSkipChanged FINAL)

public:
    virtual ~AbstractWindow() {};

    virtual uint32_t id() = 0;
    virtual pid_t pid() = 0;
    virtual QString icon() = 0;
    virtual QString title() = 0;
    virtual bool isActive() = 0;
    virtual bool allowClose() = 0;
    virtual bool shouldSkip() = 0;
    virtual bool isMinimized() = 0;
    virtual bool isAttention() =0;

    virtual void close() = 0;
    virtual void activate() = 0;
    virtual void maxmize() = 0;
    virtual void minimize() = 0;
    virtual void killClient() = 0;
    virtual void setWindowIconGeometry(const QWindow* baseWindow, const QRect& gemeotry) = 0;

    void setAppItem(QPointer<AppItem> item) {m_appitem = item;}
    QPointer<AppItem> getAppItem() {return m_appitem;}

protected:
    /** only windowmonitor can call updateXXX(func) to update window data
      */
    friend class AbstractWindowMonitor;

    /** AbstractWindow will only created in windowmonitor and stored as below
      * 1. window monitor as a list (own it)
      * 2. appitem as launched window (but not owned, can not destructor)
      */
    AbstractWindow(QObject *parent = nullptr) : QObject(parent) {}

private:
    QPointer<AppItem> m_appitem;

private:
    virtual void updatePid() = 0;
    virtual void updateIcon() = 0;
    virtual void updateTitle() = 0;
    virtual void updateIsActive() = 0;
    virtual void updateShouldSkip() = 0;
    virtual void updateAllowClose() = 0;
    virtual void updateIsMinimized() = 0;

Q_SIGNALS:
    void pidChanged();
    void iconChanged();
    void titleChanged();
    void isActiveChanged();
    void shouldSkipChanged();
    void stateChanged();
};
}
