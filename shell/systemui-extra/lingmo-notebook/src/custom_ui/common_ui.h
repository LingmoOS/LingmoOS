#ifndef COMMON_UI_H
#define COMMON_UI_H

#include <QString>
#include <QWidget>

extern QString dealMessage(const QString msg);

extern void centerTheWindow(QWidget *self, QWidget *parentWindow);

#ifdef TEST_DEBUG
extern void showWindowFrame(QWidget *self);
#endif

#endif // COMMON_UI_H