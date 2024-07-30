/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.kde.kholidays 1.0 as KHolidays

ApplicationWindow {
    visible: true
    width: 720
    height: 480

    ColumnLayout {
        RowLayout {
            Label {
                text: "Position:"
            }
            TextField {
                id: lat
                placeholderText: "latitude"
                text: "46.09902"
            }
            TextField {
                id: lon
                placeholderText: "longitude"
                text: "7.78328"
            }
        }

        Label {
            text: "Dawn: " + KHolidays.SunRiseSet.utcDawn(new Date(), lat.text, lon.text)
        }
        Label {
            text: "Sunrise: " + KHolidays.SunRiseSet.utcSunrise(new Date(), lat.text, lon.text)
        }
        Label {
            text: "Sunset: " + KHolidays.SunRiseSet.utcSunset(new Date(), lat.text, lon.text)
        }
        Label {
            text: "Dusk: " + KHolidays.SunRiseSet.utcDusk(new Date(), lat.text, lon.text)
        }
        Label {
            text: "Polar day: " + KHolidays.SunRiseSet.isPolarDay(new Date(), lat.text)
        }
        Label {
            text: "Polar twilight: " + KHolidays.SunRiseSet.isPolarTwilight(new Date(), lat.text)
        }
        Label {
            text: "Polar night: " + KHolidays.SunRiseSet.isPolarNight(new Date(), lat.text)
        }

        Label {
            text: "Lunar phase: " + KHolidays.Lunar.phaseNameAtDate(new Date())
        }
        Label {
            text: KHolidays.Lunar.phaseAtDate(new Date()) == KHolidays.LunarPhase.FullMoon ? KHolidays.Lunar.phaseName(KHolidays.LunarPhase.FullMoon) : "not a " + KHolidays.Lunar.phaseName(KHolidays.LunarPhase.FullMoon)
        }
    }

    Component.onCompleted: console.log(KHolidays.SunRiseSet)
}
