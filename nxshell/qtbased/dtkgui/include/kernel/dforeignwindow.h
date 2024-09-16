// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFOREIGNWINDOW_H
#define DFOREIGNWINDOW_H

#include <dtkgui_global.h>
#include <DObject>

#include <QWindow>

DGUI_BEGIN_NAMESPACE

class DForeignWindowPrivate;
class DForeignWindow : public QWindow, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    Q_PROPERTY(QString wmClass READ wmClass NOTIFY wmClassChanged)
    Q_PROPERTY(quint32 pid READ pid NOTIFY pidChanged)

public:
    explicit DForeignWindow(QWindow *parent = 0);

    static DForeignWindow *fromWinId(WId id);

    QString wmClass() const;
    quint32 pid() const;

Q_SIGNALS:
    void wmClassChanged();
    void pidChanged();

protected:
    bool event(QEvent *) Q_DECL_OVERRIDE;

private:
    D_DECLARE_PRIVATE(DForeignWindow)
};

DGUI_END_NAMESPACE

#endif // DFOREIGNWINDOW_H
