/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

.pragma library

var copyToClipboardText = "Copy to Clipboard";

var formats = [
    "RRR,GGG,BBB",
    "#rrggbb",
    "rrggbb",
    "#RRGGBB",
    "RRGGBB",
    "rgb",
    "rgba",
    "Qt.rgba",
    "LaTeX"
]

function formatColor(color, format) {
    var hexR = padHex((Math.round(color.r * 255)).toString(16))
    var hexG = padHex((Math.round(color.g * 255)).toString(16))
    var hexB = padHex((Math.round(color.b * 255)).toString(16))

    switch (format) {
    case "RRR,GGG,BBB":
        return [Math.round(color.r * 255), Math.round(color.g * 255), Math.round(color.b * 255)].join(", ")
    case "#rrggbb":
        return "#" + formatColor(color, "rrggbb")
    case "rrggbb":
        return formatColor(color, "RRGGBB").toLowerCase()
    case "#RRGGBB":
        return "#" + formatColor(color, "RRGGBB")
    case "RRGGBB":
        return (hexR + hexG + hexB).toUpperCase()
    case "rgb":
        return "rgb(" + formatColor(color, "RRR,GGG,BBB") + ")"
    case "rgba":
        return "rgba(" + formatColor(color, "RRR,GGG,BBB") + ", 1)"
    case "Qt.rgba":
        return "Qt.rgba(" + [roundComponent(color.r), roundComponent(color.g), roundComponent(color.b)].join(", ") + ", 1)"
    case "LaTeX":
        return "\\definecolor{ColorName}{rgb}{" + [roundComponent(color.r), roundComponent(color.g), roundComponent(color.b)].join(",") + "}"
    }
}

function padHex(n) {
    return ("0" + n).substr(-2, 2) // fancy
}

function roundComponent(n) {
    return Math.round(n * 100) / 100
}

function menuForColor(color) {
    return [
        {text: copyToClipboardText, section: true},
        {text: formatColor(color, "RRR,GGG,BBB")},
        {text: formatColor(color, "rgb")},
        {text: formatColor(color, "rgba")},
        {separator: true},
        {text: formatColor(color, "#rrggbb")},
        {text: formatColor(color, "rrggbb")},
        {text: formatColor(color, "#RRGGBB")},
        {text: formatColor(color, "RRGGBB")},
        {separator: true},
        {text: formatColor(color, "Qt.rgba")},
        {text: formatColor(color, "LaTeX")}
    ]
}

function createContextMenu(visualParent, currentColor, picker, colorLabel, copyIndicatorLabel, colorLabelRestoreTimer) {
    const initialArgs = {
        model: menuForColor(currentColor),
        visualParent,
        picker,
        colorLabel,
        copyIndicatorLabel,
        colorLabelRestoreTimer,
    };
    const component = Qt.createComponent("ColorContextMenu.qml");
    const menu = component.createObject(visualParent, initialArgs);
    component.destroy();
    return menu;
}

function showLoadingIndicator(parent, urls) {
    if (parent.loadingIndicator === null) {
        const component = Qt.createComponent(Qt.resolvedUrl("LoadingIndicator.qml"));
        parent.loadingIndicator = component.createObject(parent, {
            "jobRemaining": urls.length,
        });
        component.destroy();
    } else {
        parent.loadingIndicator.jobRemaining += urls.length;
    }
}
