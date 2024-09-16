// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QDBusConnection>

class QLabel;
class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();
    bool SleepLock();

private:
    QLabel * m_image;
    QLabel * m_text;
    QPixmap m_pix;
    bool m_bSleepLock;

    void setupSize();
    void setupUI();
public slots:
    void HideShowToRaise(bool visible);
};

class LowPowerAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.LowPower1")

public:
    explicit LowPowerAdaptor(Window * parent);
    ~LowPowerAdaptor();

public slots:
    void Raise();
    void Quit();
};

#endif // WINDOW_H
