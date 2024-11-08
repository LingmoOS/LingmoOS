/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMO_DIALOGSHADOWS_H
#define LINGMO_DIALOGSHADOWS_H

#include <QSet>

#include <KSvg/FrameSvg>
#include <KSvg/Svg>

class DialogShadows : public KSvg::Svg
{
    Q_OBJECT

public:
    static DialogShadows *instance(const QString &prefix = QLatin1String("dialogs/background"));

    explicit DialogShadows(QObject *parent, const QString &prefix);
    ~DialogShadows() override;

    void addWindow(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders = KSvg::FrameSvg::AllBorders);
    void removeWindow(QWindow *window);

    void setEnabledBorders(QWindow *window, KSvg::FrameSvg::EnabledBorders enabledBorders = KSvg::FrameSvg::AllBorders);

    bool enabled() const;

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void updateShadows())
    Q_PRIVATE_SLOT(d, void windowDestroyed(QObject *deletedObject))
};

#endif
