/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIRIGAMIPLASMATHEME_H
#define KIRIGAMIPLASMATHEME_H

#include <Kirigami/Platform/PlatformTheme>

#include <QColor>
#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

class StyleSingleton;

class PlasmaDesktopTheme : public Kirigami::Platform::PlatformTheme
{
    Q_OBJECT

public:
    explicit PlasmaDesktopTheme(QObject *parent = nullptr);
    ~PlasmaDesktopTheme() override;

    Q_INVOKABLE QIcon iconFromTheme(const QString &name, const QColor &customColor = Qt::transparent) override;

    void syncWindow();
    void syncColors();

protected:
    bool event(QEvent *event) override;

private:
    friend class StyleSingleton;
    QPointer<QWindow> m_window;
};

#endif // KIRIGAMIPLASMATHEME_H
