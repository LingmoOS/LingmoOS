// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DWidget>

#include <QScopedPointer>

DWIDGET_USE_NAMESPACE

class DDropdownPrivate;

class DDropdown : public DWidget
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
public:
    explicit DDropdown(QWidget *parent = Q_NULLPTR);
    ~DDropdown() override;

public:
    QString status() const;
    QList<QAction *> actions() const;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void statusChanged(QString status);
    void requestContextMenu();
    void triggeredAll();
    void triggeredInstalled();
    void triggeredUnInstalled();

public slots:
    void setText(const QString &text);
    void setCurrentAction(QAction *action);
    void setCurrentAction(int index = 0);
    QAction *addAction(const QString &item, const QVariant &var);
    void setStatus(QString status);

protected:
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const QString m_strAllText = tr("All");
    const QString m_strInstallText = tr("Installed");
    const QString m_strUnInstallText = tr("Not installed");
    QAction *m_pAllAction;
    QAction *m_pInstalledAction;
    QAction *m_pUnInstalledAction;
    QScopedPointer<DDropdownPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DDropdown)
};
