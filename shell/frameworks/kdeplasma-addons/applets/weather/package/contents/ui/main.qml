/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasma5support as P5Support

import org.kde.plasma.private.weather

PlasmoidItem {
    id: root

    Plasmoid.backgroundHints: PlasmaCore.Types.DefaultBackground | PlasmaCore.Types.ConfigurableBackground

    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
        PlasmaCore.Types.LeftEdge,
    ].includes(Plasmoid.location)

    readonly property string weatherSource: Plasmoid.configuration.source
    readonly property int updateInterval: Plasmoid.configuration.updateInterval
    readonly property int displayTemperatureUnit: Plasmoid.configuration.temperatureUnit
    readonly property int displaySpeedUnit: Plasmoid.configuration.speedUnit
    readonly property int displayPressureUnit: Plasmoid.configuration.pressureUnit
    readonly property int displayVisibilityUnit: Plasmoid.configuration.visibilityUnit

    property int status: Util.Normal

    readonly property int invalidUnit: -1 //TODO: make KUnitConversion::InvalidUnit usable here

    // model providing final display strings for observation properties
    readonly property var observationModel: {
        const model = {};
        const data = weatherDataSource.currentData || {};

        function getNumber(key) {
            const number = data[key];
            if (typeof number === "string") {
                const parsedNumber = parseFloat(number);
                return isNaN(parsedNumber) ? null : parsedNumber;
            }
            return (typeof number !== "undefined") && (number !== "") ? number : null;
        }
        function getNumberOrString(key) {
            const number = data[key];
            return (typeof number !== "undefined") && (number !== "") ? number : null;
        }

        const reportTemperatureUnit = data["Temperature Unit"] || invalidUnit;
        const reportPressureUnit =    data["Pressure Unit"] || invalidUnit;
        const reportVisibilityUnit =  data["Visibility Unit"] || invalidUnit;
        const reportWindSpeedUnit =   data["Wind Speed Unit"] || invalidUnit;

        model["conditions"] = data["Current Conditions"] || "";

        const conditionIconName = data["Condition Icon"] || null;
        model["conditionIconName"] = conditionIconName ? Util.existingWeatherIconName(conditionIconName) : "weather-none-available";

        const temperature = getNumber("Temperature");
        model["temperature"] = temperature !== null ? Util.temperatureToDisplayString(displayTemperatureUnit, temperature, reportTemperatureUnit, true, false) : "";

        const windchill = getNumber("Windchill");
        // Use temperature unit to convert windchill temperature
        // we only show degrees symbol not actual temperature unit
        model["windchill"] = windchill !== null ?
            Util.temperatureToDisplayString(displayTemperatureUnit, windchill, reportTemperatureUnit, false, true) :
            "";

        const humidex = getNumber("Humidex");
        // TODO: this seems wrong, does the humidex have temperature as units?
        // Use temperature unit to convert humidex temperature
        // we only show degrees symbol not actual temperature unit
        model["humidex"] = humidex !== null ?
            Util.temperatureToDisplayString(displayTemperatureUnit, humidex, reportTemperatureUnit, false, true) :
            "";

        const dewpoint = getNumber("Dewpoint");
        model["dewpoint"] = dewpoint !== null ?
            Util.temperatureToDisplayString(displayTemperatureUnit, dewpoint, reportTemperatureUnit) : "";

        const pressure = getNumber("Pressure");
        model["pressure"] = pressure !== null ?
            Util.valueToDisplayString(displayPressureUnit, pressure, reportPressureUnit, 2) : "";

        const pressureTendency = (data && data["Pressure Tendency"]) || null;
        model["pressureTendency"] =
            pressureTendency === "rising"  ? i18nc("pressure tendency", "Rising")  :
            pressureTendency === "falling" ? i18nc("pressure tendency", "Falling") :
            pressureTendency === "steady"  ? i18nc("pressure tendency", "Steady")  :
            /* else */                       "";

        const visibility = getNumberOrString("Visibility");
        model["visibility"] = visibility !== null ?
            ((reportVisibilityUnit !== invalidUnit) ?
                Util.valueToDisplayString(displayVisibilityUnit, visibility, reportVisibilityUnit, 1) : visibility) :
            "";

        const humidity = getNumber("Humidity");
        model["humidity"] = humidity !== null ? Util.percentToDisplayString(humidity) : "";

        // TODO: missing check for windDirection validness
        const windDirection = data["Wind Direction"] || "";
        const windSpeed = getNumberOrString("Wind Speed");
        let windSpeedText;
        if (windSpeed !== null && windSpeed !== "") {
            const windSpeedNumeric = (typeof windSpeed !== 'number') ? parseFloat(windSpeed) : windSpeed;
            if (!isNaN(windSpeedNumeric)) {
                if (windSpeedNumeric !== 0) {
                    windSpeedText = Util.valueToDisplayString(displaySpeedUnit, windSpeedNumeric, reportWindSpeedUnit, 1);
                } else {
                    windSpeedText = i18nc("Wind condition", "Calm");
                }
            } else {
                // TODO: i18n?
                windSpeedText = windSpeed;
            }
        }
        model["windSpeed"] = windSpeedText || "";
        model["windDirectionId"] = windDirection;
        model["windDirection"] = windDirection ? i18nc("wind direction", windDirection) : "";

        const windGust = getNumber("Wind Gust");
        model["windGust"] = windGust !== null ? Util.valueToDisplayString(displaySpeedUnit, windGust, reportWindSpeedUnit, 1) : "";

        return model;
    }

    readonly property var generalModel: {
        const model = {};
        const data = weatherDataSource.currentData || {};

        const todayForecastTokens = (data["Short Forecast Day 0"] || "").split("|");

        model["location"] =  data["Place"] || "";
        model["courtesy"] =  data["Credit"] || "";
        model["creditUrl"] = data["Credit Url"] || "";

        let forecastDayCount = parseInt(data["Total Weather Days"] || "");

        // Some providers use a 12h forecast, with day and night distinct info
        const hasNightForecasts = weatherSource && forecastDayCount > 8;
        model["forecastNightRow"] = hasNightForecasts;
        if (hasNightForecasts) {
            model["forecastStartsAtNight"] = data["Forecast Starts at Night"] || false;
            forecastDayCount = Math.ceil(forecastDayCount / 2);
        }

        const forecastTitle = (!isNaN(forecastDayCount) && forecastDayCount > 0) ?
                                i18ncp("Forecast period timeframe", "1 Day", "%1 Days", forecastDayCount) : ""
        model["forecastTitle"] = forecastTitle;

        let conditionIconName = observationModel.conditionIconName;
        if (!conditionIconName || conditionIconName === "weather-none-available") {
            // try icon from current weather forecast
            if (todayForecastTokens.length === 6 && todayForecastTokens[1] !== "N/U") {
                conditionIconName = Util.existingWeatherIconName(todayForecastTokens[1]);
            } else {
                conditionIconName = "weather-none-available";
            }
        }
        model["currentConditionIconName"] = conditionIconName;

        return model;
    }

    readonly property var detailsModel: {
        const model = [];

        if (observationModel.windchill) {
            model.push({
                "label": i18nc("@label", "Windchill:"),
                "text":  observationModel.windchill
            });
        };

        if (observationModel.humidex) {
            model.push({
                "label": i18nc("@label", "Humidex:"),
                "text":  observationModel.humidex
            });
        }

        if (observationModel.dewpoint) {
            model.push({
                "label": i18nc("@label ground temperature", "Dewpoint:"),
                "text":  observationModel.dewpoint
            });
        }

        if (observationModel.pressure) {
            model.push({
                "label": i18nc("@label", "Pressure:"),
                "text":  observationModel.pressure
            });
        }

        if (observationModel.pressureTendency) {
            model.push({
                "label": i18nc("@label pressure tendency, rising/falling/steady", "Pressure Tendency:"),
                "text":  observationModel.pressureTendency
            });
        }

        if (observationModel.visibility) {
            model.push({
                "label": i18nc("@label", "Visibility:"),
                "text":  observationModel.visibility
            });
        }

        if (observationModel.humidity) {
            model.push({
                "label": i18nc("@label", "Humidity:"),
                "text":  observationModel.humidity
            });
        }

        if (observationModel.windGust) {
            model.push({
                "label": i18nc("@label", "Wind Gust:"),
                "text":  observationModel.windGust
            });
        }

        return model;
    }

    readonly property var forecastModel: {
        const model = [];
        const data = weatherDataSource.currentData;

        const forecastDayCount = parseInt((data && data["Total Weather Days"]) || "");
        if (isNaN(forecastDayCount) || forecastDayCount <= 0) {
            return model;
        }

        const reportTemperatureUnit = (data && data["Temperature Unit"]) || invalidUnit;

        if (generalModel.forecastNightRow) {
            model.push({placeholder: i18nc("Time of the day (from the duple Day/Night)", "Day")})
            model.push({placeholder: i18nc("Time of the day (from the duple Day/Night)", "Night")})
        }

        for (let i = 0; i < forecastDayCount; ++i) {
            const forecastInfo = {
                period: "",
                icon: "",
                condition: "",
                probability: "",
                tempHigh: "",
                tempLow: "",
            }

            const forecastDayKey = "Short Forecast Day " + i;
            const forecastDayTokens = ((data && data[forecastDayKey]) || "").split("|");
            if (forecastDayTokens.length !== 6) {
                // We don't have the right number of tokens, abort trying
                continue;
            }

            // If the first item is a night forecast and we are showing them on second row,
            // add an empty placeholder
            if (i === 0 && generalModel.forecastNightRow && generalModel.forecastStartsAtNight) {
                model.push({placeholder: ""})
            }

            forecastInfo["period"] = forecastDayTokens[0];

            // If we see N/U (Not Used) we skip the item
            const weatherIconName = forecastDayTokens[1];
            if (weatherIconName && weatherIconName !== "N/U") {
                forecastInfo["icon"] = Util.existingWeatherIconName(weatherIconName);
                forecastInfo["condition"] = forecastDayTokens[2];

                const probability = forecastDayTokens[5];
                if (probability !== "N/U" && probability !== "N/A" && probability > 7.5) {
                    forecastInfo["probability"] = Math.round(probability / 5 ) * 5;
                }
            }

            const tempHigh = forecastDayTokens[3];
            if (tempHigh !== "N/U" && tempHigh !== "N/A" && tempHigh) {
                forecastInfo["tempHigh"] = Util.temperatureToDisplayString(displayTemperatureUnit, tempHigh, reportTemperatureUnit, true);
            }

            const tempLow = forecastDayTokens[4];
            if (tempLow !== "N/U" && tempLow !== "N/A" && tempLow) {
                forecastInfo["tempLow"] = Util.temperatureToDisplayString(displayTemperatureUnit, tempLow, reportTemperatureUnit, true);
            }

            model.push(forecastInfo);
        }

        return model;
    }

    readonly property var noticesModel: {
        const model = [];
        const data = weatherDataSource.currentData;

        let warningsCount = parseInt((data && data["Total Warnings Issued"]) || "");
        if (isNaN(warningsCount)) {
            warningsCount = 0;
        }
        for (let i = 0; i < warningsCount; ++i) {
            model.push({
                'description': data[`Warning Description ${i}`],
                'infoUrl': data[`Warning Info ${i}`],
                'timestamp': data[`Warning Timestamp ${i}`],
                'priority': data[`Warning Priority ${i}`] ?? 0,
            });
        }

        return model;
    }

    function symbolicizeIconName(iconName) {
        const symbolicSuffix = "-symbolic";
        if (iconName.endsWith(symbolicSuffix)) {
            return iconName;
        }

        return iconName + symbolicSuffix;
    }

    P5Support.DataSource {
        id: weatherDataSource

        readonly property var currentData: data[weatherSource]

        engine: "weather"
        connectedSources: weatherSource
        interval: updateInterval * 60 * 1000
        onConnectedSourcesChanged: {
            if (weatherSource) {
                status = Util.Connecting
                connectionTimeoutTimer.start();
            }
        }
        onCurrentDataChanged: {
            if (currentData) {
                status = Util.Normal
                connectionTimeoutTimer.stop();
            }
        }
    }

    Timer {
        id: connectionTimeoutTimer

        interval: 60 * 1000 // 1 min
        repeat: false
        onTriggered: {
            status = Util.Timeout;
        }
    }

    Plasmoid.icon: {
        let iconName;
        // workaround for now to ensure "Please configure" tooltip
        // TODO: remove when configurationRequired works
        if (status === Util.NeedsConfiguration) {
            iconName = "configure";
        } else {
            iconName = generalModel.currentConditionIconName;
        }

        if (inPanel) {
            iconName = symbolicizeIconName(iconName);
        }

        return iconName;
    }
    Plasmoid.busy: status === Util.Connecting
    Plasmoid.configurationRequired: status === Util.NeedsConfiguration

    toolTipMainText: (status === Util.NeedsConfiguration) ?
        i18nc("@info:tooltip %1 is the translated plasmoid name", "Click to configure %1", Plasmoid.title) :
        generalModel.location

    toolTipSubText: {
        if (!generalModel.location) {
            return "";
        }
        const tooltips = [];
        const temperature = Plasmoid.configuration.showTemperatureInTooltip ? observationModel.temperature : null;
        if (observationModel.conditions && temperature) {
            tooltips.push(i18nc("weather condition + temperature",
                                "%1 %2", observationModel.conditions, temperature));
        } else if (observationModel.conditions || temperature) {
            tooltips.push(observationModel.conditions || temperature);
        }
        if (Plasmoid.configuration.showWindInTooltip && observationModel.windSpeed) {
            if (observationModel.windDirection) {
                if (observationModel.windGust) {
                    tooltips.push(i18nc("winddirection windspeed (windgust)", "%1 %2 (%3)",
                                        observationModel.windDirection, observationModel.windSpeed, observationModel.windGust));
                } else {
                    tooltips.push(i18nc("winddirection windspeed", "%1 %2",
                                        observationModel.windDirection, observationModel.windSpeed));
                }
            } else {
                tooltips.push(observationModel.windSpeed);
            }
        }
        if (Plasmoid.configuration.showPressureInTooltip && observationModel.pressure) {
            if (observationModel.pressureTendency) {
                tooltips.push(i18nc("pressure (tendency)", "%1 (%2)",
                                    observationModel.pressure, observationModel.pressureTendency));
            } else {
                tooltips.push(observationModel.pressure);
            }
        }
        if (Plasmoid.configuration.showHumidityInTooltip && observationModel.humidity) {
            tooltips.push(i18n("Humidity: %1", observationModel.humidity));
        }

        return tooltips.join("\n");
    }

    // Only exists because the default CompactRepresentation doesn't expose:
    // - Icon overlays, or a generic way to overlay something on top of the icon
    // - The ability to show text below or beside the icon
    // TODO remove once it gains those features.
    compactRepresentation: CompactRepresentation {
        generalModel: root.generalModel
        observationModel: root.observationModel
    }

    fullRepresentation: FullRepresentation {
        generalModel: root.generalModel
        observationModel: root.observationModel
    }

    Binding {
        target: Plasmoid
        property: "needsToBeSquare"
        value: (Plasmoid.containmentType & PlasmaCore.Types.CustomEmbeddedContainment)
                | (Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentForcesSquarePlasmoids)
    }

    onWeatherSourceChanged: {
        if (weatherSource.length === 0) {
            status = Util.NeedsConfiguration
        }
    }

    Component.onCompleted: weatherSourceChanged()
}
