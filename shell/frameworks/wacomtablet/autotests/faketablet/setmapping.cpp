/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include "common/property.h"
#include "common/screenspace.h"
#include "kded/xinputadaptor.h"
#include "kded/xsetwacomadaptor.h"

#include <QApplication>
#include <QObject>
#include <QThread>
#include <QTimer>

using namespace Wacom;

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

public slots:
    void run()
    {
        QString device = QString::fromLatin1("Wacom Bamboo One S Pen stylus");

        Wacom::XinputAdaptor xi(device);
        XsetwacomAdaptor xsetwacom(device);

        auto defmode = xsetwacom.getProperty(Property::Mode);
        // auto defmapping = xi.getProperty(Property::ScreenSpace);

        Wacom::ScreenSpace space(QLatin1String("speedx0.5x0.5"));

        xsetwacom.setProperty(Property::Mode, QString::fromLatin1("relative"));
        xi.setProperty(Property::ScreenSpace, space.toString());
        std::cout << "Mapping set\n";

        QThread::sleep(10);

        xsetwacom.setProperty(Property::Mode, defmode);

        std::cout << "Mapping reset\n";

        emit finished();
    }

signals:
    void finished();
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    Task *task = new Task(&a);
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
    QTimer::singleShot(0, task, SLOT(run()));

    return a.exec();
}

#include "setmapping.moc"
