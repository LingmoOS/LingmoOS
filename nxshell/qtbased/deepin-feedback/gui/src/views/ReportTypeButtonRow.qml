/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1
import DataConverter 1.0
import "Widgets"

Row {
    id: reportTypeButtonRow
    width: parent.width
    spacing: 12
    property var reportType: DataConverter.DFeedback_Bug

    ReportTypeButton {
        id: bugReportButton
        width: (parent.width - 12) / 2
        actived: parent.reportType == DataConverter.DFeedback_Bug
        iconPath: "qrc:/views/Widgets/images/reporttype_bug.png"
        text: dsTr("I have a problem")
        onClicked: {
            parent.reportType = DataConverter.DFeedback_Bug
        }
    }

    ReportTypeButton {
        id: proposalReportButton
        width: (parent.width - 12) / 2
        actived: parent.reportType == DataConverter.DFeedback_Proposal
        iconPath: "qrc:/views/Widgets/images/reporttype_proposal.png"
        text: dsTr("I have a good idea")
        onClicked: {
            parent.reportType = DataConverter.DFeedback_Proposal
            contentEdited()
        }
    }
}

