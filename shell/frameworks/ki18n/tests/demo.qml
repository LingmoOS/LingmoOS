/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2
import org.kde.i18n.localeData 1.0

QQC2.ApplicationWindow {
    visible: true
    width: 480
    height: 720

    property var currentCountry
    property var currentSubdiv

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        QQC2.ComboBox {
            id: countrySelector
            Layout.fillWidth: true
            model: Country.allCountries
            delegate: QQC2.ItemDelegate {
                text: modelData.emojiFlag + " " + modelData.name + " (" + modelData.alpha2 + ")"
                onClicked: {
                    countrySelector.currentIndex = index
                    currentCountry = modelData
                }
            }
            displayText: currentValue.emojiFlag + " " + currentValue.name + " (" + currentValue.alpha2 + ")"
        }

        QQC2.Label {
            text: "Localized name: " + (currentCountry ? currentCountry.name : "N/A")
        }
        QQC2.Label {
            text: "ISO 3166-1 alpha2: " + (currentCountry ? currentCountry.alpha2 : "N/A")
        }
        QQC2.Label {
            text: "ISO 3166-2 aplha3: " + (currentCountry ? currentCountry.alpha3 : "N/A")
        }
        QQC2.Label {
            text: "Flag: " + (currentCountry ? currentCountry.emojiFlag : "N/A")
        }
        QQC2.Label {
            text: "Currency: " + (currentCountry ? currentCountry.currencyCode : "N/A")
        }
        QQC2.Label {
            text: "Timezones: " + (currentCountry ? currentCountry.timeZoneIds.join(', '): "N/A")
        }

        QQC2.TextField {
            Layout.fillWidth: true
            placeholderText: "search country by name or ISO code"
            onEditingFinished: currentCountry = Country.fromName(text)
        }

        QQC2.ComboBox {
            Layout.fillWidth: true
            model: currentCountry ? currentCountry.subdivisions : undefined
            delegate: QQC2.ItemDelegate {
                text: modelData.name + " (" + modelData.code + ")"
                onClicked: {
                    countrySelector.currentIndex = index
                    currentSubdiv = modelData
                }
            }
            displayText: currentValue.name + " (" + currentValue.code + ")"
        }

        QQC2.Label {
            text: "Localized name: " + (currentSubdiv ? currentSubdiv.name : "N/A")
        }
        QQC2.Label {
            text: "ISO 3166-2: " + (currentSubdiv ? currentSubdiv.code: "N/A")
        }
        QQC2.Label {
            text: "Parent: " + (currentSubdiv ? currentSubdiv.parent.code : "N/A")
        }
        QQC2.Label {
            text: "Country: " + (currentSubdiv ? currentSubdiv.country.alpha2 : "N/A")
        }
        QQC2.Label {
            text: "Timezones: " + (currentSubdiv ? currentSubdiv.timeZoneIds.join(', '): "N/A")
        }
        QQC2.ComboBox {
            Layout.fillWidth: true
            visible: currentSubdiv != undefined && currentSubdiv.subdivisions.length > 0
            model: currentSubdiv ? currentSubdiv.subdivisions : undefined
            delegate: QQC2.ItemDelegate {
                text: modelData.name + " (" + modelData.code + ")"
                onClicked: {
                    countrySelector.currentIndex = index
                    currentSubdiv = modelData
                }
            }
            displayText: currentValue.name + " (" + currentValue.code + ")"
        }

        QQC2.TextField {
            Layout.fillWidth: true
            placeholderText: "search country subdivision by ISO code"
            onEditingFinished: {
                currentSubdiv = CountrySubdivision.fromCode(text)
                currentCountry = currentSubdiv ? currentSubdiv.country : undefined
            }
        }

        RowLayout {
            function searchByLocation() {
                currentSubdiv = CountrySubdivision.fromLocation(latitude.text, longitude.text);
                if (currentSubdiv) {
                    currentCountry = currentSubdiv.country;
                } else {
                    currentCountry = Country.fromLocation(latitude.text, longitude.text);
                }
                timezoneAtLocationLabel.text = TimeZone.fromLocation(latitude.text, longitude.text);
            }

            QQC2.TextField {
                id: latitude
                placeholderText: "latitude"
                onEditingFinished: parent.searchByLocation()
            }
            QQC2.TextField {
                id: longitude
                placeholderText: "longitude"
                onEditingFinished: parent.searchByLocation()
            }

        }

        QQC2.Label {
            id: timezoneAtLocationLabel
        }

        Item { Layout.fillHeight: true }
    }
}
