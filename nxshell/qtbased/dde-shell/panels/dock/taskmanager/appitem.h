// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "abstractitem.h"
#include "abstractwindow.h"

#include <QMap>
#include <QObject>
#include <QPointer>

namespace dock {
class TaskManager;
class DesktopfileAbstractParser;

class AppItem : public AbstractItem
{
    Q_OBJECT

public:
    ~AppItem();

    ItemType itemType() const override;

    QString id() const override;
    QString type() const override;
    QString icon() const override;
    QString name() const override;
    QString menus() const override;

    QString desktopfileID() const;

    bool isActive() const override;
    void active() const override;

    bool isAttention() const override;

    bool isDocked() const override;
    void setDocked(bool docked) override;

    void handleClick(const QString& clickItem) override;

    QVariant data() override;

    bool hasWindow() const;

    void appendWindow(QPointer<AbstractWindow> window);
    void removeWindow(QPointer<AbstractWindow> window);

    void setDesktopFileParser(QSharedPointer<DesktopfileAbstractParser> desktopfile);
    QPointer<DesktopfileAbstractParser> getDesktopFileParser();

    void launch();
    void requestQuit();
    void handleMenu(const QString& menuId);

    const QList<QPointer<AbstractWindow>>& getAppendWindows();

    QString getCurrentActiveWindowName() const;
    QString getCurrentActiveWindowIcon() const;

protected:
    friend class TaskManager;
    AppItem(QString id, QObject *parent = nullptr);

Q_SIGNALS:
    void currentActiveWindowChanged();
    void appendedWindow(const QPointer<AbstractWindow> &window);

private:
    void updateCurrentActiveWindow(QPointer<AbstractWindow> window);
    void checkAppItemNeedDeleteAndDelete();

private Q_SLOTS:
    void onWindowDestroyed();

private:
    QString m_id;
    QList<QPointer<AbstractWindow>> m_windows;
    QPointer<AbstractWindow> m_currentActiveWindow;
    QSharedPointer<DesktopfileAbstractParser> m_desktopfileParser;

};
}
