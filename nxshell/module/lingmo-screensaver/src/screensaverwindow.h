// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERWINDOW_H
#define SCREENSAVERWINDOW_H

#include <QWindow>
#include <QEvent>

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

class ScreenSaverView;
class ScreenSaverWindow : public QObject
{
    Q_OBJECT

public:
    explicit ScreenSaverWindow(QObject *parent = nullptr);
    ~ScreenSaverWindow();

    bool start(const QString &filePath);
    void stop();

    WId winId() const;
    Qt::WindowFlags flags() const;
    void setFlags(Qt::WindowFlags flags, bool bypassWindowManager);

    QWindow::Visibility visibility() const;
    QScreen *screen() const;

public slots:
    void setGeometry(const QRect &rect);
    void setScreen(QScreen *screen);
    void show();
    void hide();
    void close();

signals:
    void inputEvent(QEvent::Type type);

private:
    ScreenSaverView *m_view;
};

#endif // SCREENSAVERWINDOW_H
