// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTASKBARCONTROL_H
#define DTASKBARCONTROL_H

#include <dtkgui_global.h>

#include <QObject>
#include <QColor>

#include <DObject>

DGUI_BEGIN_NAMESPACE
class DTaskbarControlPrivate;

class DTaskbarControl : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

public:
    explicit DTaskbarControl(QObject *parent = nullptr);
    ~DTaskbarControl();

    void setProgress(bool progressVisible, double progress);
    void setCounter(bool counterVisible, int counter);
    int counter() const;
    void setCounterVisible(bool counterVisible);
    bool counterVisible() const;
    void setUrgency(bool val);

Q_SIGNALS:
    void counterChanged(int counter);
    void counterVisibleChanged(bool visible);
    void progressChanged(double progress);
    void progressVisibleChanged(bool visible);

protected:
    virtual void sendMessage(const QVariantMap &params);

private:
    D_DECLARE_PRIVATE(DTaskbarControl)
};



DGUI_END_NAMESPACE

#endif // DTASKBARCONTROL_H
