/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A component used to forward questions from KNewStuff's engine to the UI
 *
 * This component is equivalent to the WidgetQuestionListener
 * @see KNewStuff::WidgetQuestionListener
 * @see KNewStuffCore::Question
 * @since 5.63
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff
import org.kde.newstuff.core as NewStuffCore

QQC2.Dialog {
    id: dialog

    property int questionType

    anchors.centerIn: parent

    modal: true
    focus: true

    margins: Kirigami.Units.largeSpacing
    padding: Kirigami.Units.largeSpacing

    standardButtons: {
        switch (questionType) {
            case NewStuffCore.Question.SelectFromListQuestion:
            case NewStuffCore.Question.InputTextQuestion:
            case NewStuffCore.Question.PasswordQuestion:
            case NewStuffCore.Question.ContinueCancelQuestion:
                // QQC2 Dialog standardButtons does not have a Continue button...
                return QQC2.Dialog.Ok | QQC2.Dialog.Cancel;
            case NewStuffCore.Question.YesNoQuestion:
                return QQC2.Dialog.Yes | QQC2.Dialog.No;
            default:
                return QQC2.Dialog.NoButton;
        }
    }

    Connections {
        target: NewStuff.QuickQuestionListener

        function onAskListQuestion(title, question, list) {
            dialog.questionType = NewStuffCore.Question.SelectFromListQuestion;
            dialog.title = title;
            questionLabel.text = question;
            for (var i = 0; i < list.length; i++) {
                listView.model.append({ text: list[i] });
            }
            listView.currentIndex = 0;
            listView.visible = true;
            dialog.open();
        }

        function onAskContinueCancelQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.ContinueCancelQuestion;
            dialog.title = title;
            questionLabel.text = question;
            dialog.open();
        }

        function onAskTextInputQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.InputTextQuestion;
            dialog.title = title;
            questionLabel.text = question;
            textInput.visible = true;
            dialog.open();
        }

        function onAskPasswordQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.PasswordQuestion;
            dialog.title = title;
            questionLabel.text = question;
            textInput.echoMode = QQC2.TextInput.PasswordEchoOnEdit;
            textInput.visible = true;
            dialog.open();
        }

        function onAskYesNoQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.YesNoQuestion;
            dialog.title = title;
            questionLabel.text = question;
            dialog.open();
        }
    }

    function passResponse(responseIsContinue) {
        let input = "";
        switch (dialog.questionType) {
        case NewStuffCore.Question.SelectFromListQuestion:
            input = listView.currentItem.text;
            listView.model.clear();
            listView.visible = false;
            break;
        case NewStuffCore.Question.InputTextQuestion:
            input = textInput.text;
            textInput.text = "";
            textInput.visible = false;
            break;
        case NewStuffCore.Question.PasswordQuestion:
            input = textInput.text;
            textInput.text = "";
            textInput.visible = false;
            textInput.echoMode = QQC2.TextInput.Normal;
            break;
        case NewStuffCore.Question.ContinueCancelQuestion:
        case NewStuffCore.Question.YesNoQuestion:
        default:
            // Nothing special to do for these types of question, we just pass along the positive or negative response
            break;
        }
        NewStuff.QuickQuestionListener.passResponse(responseIsContinue, input);
    }

    ColumnLayout {
        id: layout

        readonly property real maxWidth: {
            const bounds = dialog.parent;
            if (!bounds) {
                return 0;
            }
            return bounds.width - (dialog.leftPadding + dialog.leftMargin + dialog.rightMargin + dialog.rightPadding);
        }

        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            id: questionLabel
            Layout.maximumWidth: layout.maxWidth
            wrapMode: Text.Wrap
        }

        ListView {
            id: listView
            spacing: Kirigami.Units.smallSpacing
            Layout.maximumWidth: layout.maxWidth
            Layout.fillWidth: true

            visible: false

            model: ListModel { }

            delegate: QQC2.ItemDelegate {
                width: listView.width
                text: model.text
            }
        }

        QQC2.TextField {
            id: textInput

            Layout.maximumWidth: layout.maxWidth
            Layout.fillWidth: true

            visible: false
        }
    }

    onAccepted: {
        passResponse(true);
    }

    onRejected: {
        passResponse(false);
    }
}
