/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kdatepicker.h"
#include <QApplication>
#include <QTimer>

class Manager : public QObject
{
    Q_OBJECT
public:
    Manager()
    {
        picker.show();
        QTimer::singleShot(1000, this, &Manager::timeout1);
        QTimer::singleShot(5000, this, &Manager::timeout2);
    }

private Q_SLOTS:
    void timeout1()
    {
        picker.setEnabled(false);
    }
    void timeout2()
    {
        picker.setEnabled(true);
    }

private:
    KDatePicker picker;
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KDatePickertest"));
    QApplication app(argc, argv);

    Manager mgr;

    return app.exec();
}

#include <kdatepicktest.moc>
