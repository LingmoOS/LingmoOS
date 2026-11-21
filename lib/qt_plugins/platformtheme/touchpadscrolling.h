// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOUCHPADSCROLLING_H
#define TOUCHPADSCROLLING_H

#include <QObject>

// Gives every scroll view of the application the touchpad scrolling of FishUI.
// The Qt Quick part is loaded when a Qt Quick window first gets a wheel event,
// so widget applications never load Qt Quick.
class TouchpadScrolling : public QObject
{
    Q_OBJECT

public:
    static void install();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    TouchpadScrolling();
    void loadQuickScrolling();
};

#endif // TOUCHPADSCROLLING_H
