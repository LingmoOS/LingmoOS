/*
 *  SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
import QtQuick

import org.kde.coreaddons

Column {

    height: 500

    Text {
        text: "formatByteSize(1234): %1".arg(Format.formatByteSize(1234))
    }

    Text {
        text: "formatByteSize(1234, 2): %1".arg(Format.formatByteSize(1234, 2))
    }

    Text {
        text: "formatDuration(12345678, FormatTypes.DefaultDuration): %1".arg(Format.formatDuration(12345678, FormatTypes.DefaultDuration))
    }

    Text {
        text: "formatDuration(12345678, FormatTypes.InitialDuration): %1".arg(Format.formatDuration(12345678, FormatTypes.InitialDuration))
    }

    Text {
        text: "formatDuration(12345678, FormatTypes.ShowMilliseconds): %1".arg(Format.formatDuration(12345678, FormatTypes.ShowMilliseconds))
    }

    Text {
        text: "formatDuration(12345678, FormatTypes.HideSeconds): %1".arg(Format.formatDuration(12345678, FormatTypes.HideSeconds))
    }

    Text {
        text: "formatDuration(12345678, FormatTypes.FoldHours): %1".arg(Format.formatDuration(12345678, FormatTypes.FoldHours))
    }

    Text {
        text: "formatDuration(12345678, FormatTypes.InitialDuration | FormatTypes.HideSeconds): %1".arg(Format.formatDuration(12345678, FormatTypes.InitialDuration | FormatTypes.HideSeconds))
    }

    Text {
        text: "formatDecimalDuration(6000): %1".arg(Format.formatDecimalDuration(6000))
    }

    Text {
        text: "formatDecimalDuration(6123, 2): %1".arg(Format.formatDecimalDuration(6123, 2))
    }

    Text {
        text: "formatSpelloutDuration(60001): %1".arg(Format.formatSpelloutDuration(60001))
    }

    Text {
        text: "Format.formatRelativeDate(new Date(), Qt.LongFormat): %1".arg(Format.formatRelativeDate(new Date(), Qt.LongFormat))
    }

}
