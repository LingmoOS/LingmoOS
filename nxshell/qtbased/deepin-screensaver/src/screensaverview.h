// SPDX-FileCopyrightText: 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERVIEW_H
#define SCREENSAVERVIEW_H

#include <QQuickView>

QT_BEGIN_NAMESPACE
class QProcess;
QT_END_NAMESPACE

class ScreenSaverView : public QQuickView
{
    Q_OBJECT

public:
    explicit ScreenSaverView(QWindow *parent = nullptr);
    ~ScreenSaverView();

    bool start(const QString &filePath);
    void stop();

signals:
    void inputEvent(QEvent::Type type);

private:
    bool event(QEvent *event) override;

    QProcess *m_process = nullptr;

    friend class ScreenSaverWindow;
};

#endif // SCREENSAVERVIEW_H
