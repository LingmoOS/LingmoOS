// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOUCHSCREENSETTING_H
#define TOUCHSCREENSETTING_H

#include "org_deepin_dde_display1_monitor.h"
#include "org_deepin_dde_display1.h"

#include <DComboBox>
#include <DDialog>

DWIDGET_USE_NAMESPACE

using namespace org::deepin::dde;
using namespace org::deepin::dde::display1;

class RecognizeDialog;
class MonitorIndicator;

class TouchscreenSetting : public DDialog {
    Q_OBJECT
public:
    TouchscreenSetting(const QString &touchscreen, QWidget *parent = nullptr);

private Q_SLOTS:
    void onMonitorChanged();
    void onButtonClicked(int index, const QString &text);
    void showRecognizeDialog();
    bool monitorsIsIntersect() const;
    void markDisplay(int index);

Q_SIGNALS:
    void requestAssociateTouch(const QString &monitor,
                               const QString &touchscreenSerial);

private:
    Display1 *m_displayInter;
    RecognizeDialog *m_recognize;
    QString m_touchscreenUUID;
    DComboBox *m_listCombo;
    QList<Monitor *> m_monitors;
    MonitorIndicator *m_monitorIndicator;
};

#endif // !TOUCHSCREENSETTING_H
