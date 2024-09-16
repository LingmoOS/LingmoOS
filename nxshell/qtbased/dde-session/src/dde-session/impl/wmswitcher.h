// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WMSWITCHER_H
#define WMSWITCHER_H

#include <QObject>

#include "com_deepin_Wm.h"

class WMSwitcher : public QObject
{
    Q_OBJECT
public:
    explicit WMSwitcher(QObject *parent = nullptr);

public Q_SLOTS:
    bool AllowSwitch() const;
    QString CurrentWM() const;
    void RequestSwitchWM() const;

Q_SIGNALS:
    void WMChanged(QString );

private Q_SLOTS:
    void onCompositingEnabledChanged(bool enable);

private:
    void showOSD(const QString &osd) const;

private:
    com::deepin::wm *m_wmInter;
};

#endif // WMSWITCHER_H
