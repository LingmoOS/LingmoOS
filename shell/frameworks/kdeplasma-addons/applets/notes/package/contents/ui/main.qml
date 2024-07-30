/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2014, 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2 // just for desktop-styled context menus
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs

import org.kde.draganddrop 2.0 as DragDrop

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.notes 0.1

PlasmoidItem {
    id: root

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    // this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
    readonly property real horizontalMargins: fullRepresentationItem.width * 0.07
    readonly property real verticalMargins: fullRepresentationItem.height * 0.07
    readonly property PlasmaComponents3.TextArea mainTextArea: fullRepresentationItem.mainTextArea

    // note is of type Note
    property QtObject note: noteManager.loadNote(Plasmoid.configuration.noteId);

    // define colors used for icons in ToolButtons and for text in TextArea.
    // this is deliberately _NOT_ the theme color as we are over a known bright background!
    // an unknown colour over a known colour is a bad move as you end up with white on yellow.
    readonly property color textIconColor: {
        if (Plasmoid.configuration.color === "black" || Plasmoid.configuration.color === "translucent-light") {
            return "#dfdfdf";
        }
        return "#202020";
    }

    onExternalData: (mimetype, data) => {
        // if we dropped a text file, we want its contents,
        // otherwise we take the external data verbatim
        var contents = NotesHelper.fileContents(data) || data
        mainTextArea.text = String(contents).replace(/\n/g, "<br>") // what about richtext?

        // place cursor at the end of text, there's no "just move the cursor" function
        mainTextArea.moveCursorSelection(mainTextArea.length)
        mainTextArea.deselect()
    }

    Timer {
        id: forceFocusTimer
        interval: 1
        onTriggered: mainTextArea.forceActiveFocus()
    }

    Connections {
        target: Plasmoid
        function onActivated() {
            // FIXME doing forceActiveFocus here directly doesn't work
            forceFocusTimer.restart()
        }
    }

    NoteManager {
        id: noteManager
    }

    // Only exists because the default CompactRepresentation doesn't open on drag.
    // TODO remove once it gains that feature (perhaps optionally?)
    compactRepresentation: DragDrop.DropArea {
        id: compactDropArea
        onDragEnter: activationTimer.restart()
        onDragLeave: activationTimer.stop()

        Timer {
            id: activationTimer
            interval: 250 // matches taskmanager delay
            onTriggered: root.expanded = true
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            property bool wasExpanded

            onPressed: wasExpanded = root.expanded
            onClicked: root.expanded = !wasExpanded

            Kirigami.Icon {
                anchors.fill: parent
                source: "knotes-symbolic"
                active: parent.containsMouse
            }
        }
    }

    preloadFullRepresentation: true
    fullRepresentation: KSvg.SvgItem {
        id: backgroundItem

        property alias mainTextArea: mainTextArea
        Layout.preferredWidth: Kirigami.Units.gridUnit * 25
        Layout.preferredHeight: Kirigami.Units.gridUnit * 25
        Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        Layout.minimumHeight: Kirigami.Units.gridUnit * 2

        imagePath: "widgets/notes"
        elementId: Plasmoid.configuration.color + "-notes"

        DocumentHandler {
            id: documentHandler
            target: mainTextArea
            cursorPosition: mainTextArea.cursorPosition
            selectionStart: mainTextArea.selectionStart
            selectionEnd: mainTextArea.selectionEnd
            defaultFontSize: Plasmoid.configuration.fontSize
        }

        Connections {
            target: root
            function onExpandedChanged(expanded) {
                // don't autofocus when we're on the desktop
                if (expanded && (Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.formFactor === PlasmaCore.Types.Horizontal)) {
                    mainTextArea.forceActiveFocus()
                }
            }
        }
        Component.onDestruction: {
            note.save(mainTextArea.text);
        }

        FocusScope {
            id: focusScope
            anchors {
                fill: parent
                leftMargin: horizontalMargins
                rightMargin: horizontalMargins
                topMargin: verticalMargins
                bottomMargin: verticalMargins
            }

            PlasmaComponents3.ScrollView {
                id: scrollview
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    bottom: fontButtons.top
                    bottomMargin: Kirigami.Units.largeSpacing
                }

                clip: true

                PlasmaComponents3.TextArea {
                    id: mainTextArea
                    property int cfgFontPointSize: Plasmoid.configuration.fontSize

                    textFormat: TextEdit.RichText
                    onLinkActivated: Qt.openUrlExternally(link)
                    background: null
                    color: textIconColor
                    persistentSelection: true
                    wrapMode: TextEdit.Wrap

                    font.pointSize: cfgFontPointSize

                    Keys.onPressed: event => {
                        if (event.key === Qt.Key_Escape) {
                            root.expanded = false;
                            event.accepted = true;
                        } else if (event.modifiers === Qt.ControlModifier) {
                            if (event.key === Qt.Key_B) {
                                documentHandler.bold = !documentHandler.bold;
                                event.accepted = true;
                            } else if (event.key === Qt.Key_I) {
                                documentHandler.italic = !documentHandler.italic;
                                event.accepted = true;
                            } else if (event.key === Qt.Key_U) {
                                documentHandler.underline = !documentHandler.underline;
                                event.accepted = true;
                            } else if (event.key === Qt.Key_S) {
                                documentHandler.strikeOut = !documentHandler.strikeOut;
                                event.accepted = true;
                            } else if (event.matches(StandardKey.Paste)) {
                                documentHandler.pasteWithoutFormatting();
                                documentHandler.reset();
                                event.accepted = true;
                            } else if (event.matches(StandardKey.Cut)) {
                                cut();
                                documentHandler.reset();
                                event.accepted = true;
                            }
                        }
                    }

                    // Apply the font size change to existing texts
                    onCfgFontPointSizeChanged: {
                        const start = selectionStart;
                        const end = selectionEnd;

                        selectAll();
                        documentHandler.fontSize = cfgFontPointSize;
                        select(start, end);
                    }

                    // update the note if the source changes, but only if the user isn't editing it currently
                    Binding {
                        target: mainTextArea
                        property: "text"
                        value: note.noteText
                        when: !mainTextArea.activeFocus
                        // don't restore an empty value (which IS empty by default when the applet starts up),
                        // instead only remove this binding for the time when the user edits the content.
                        restoreMode: Binding.RestoreBinding
                    }

                    onActiveFocusChanged: {
                        const window = Window.window;
                        if (activeFocus && window && (window.flags & Qt.WindowDoesNotAcceptFocus)) {
                            Plasmoid.status = PlasmaCore.Types.AcceptingInputStatus
                        } else {
                            Plasmoid.status = PlasmaCore.Types.ActiveStatus
                            note.save(text);
                        }
                    }

                    onPressed: event => {
                        if (event.button === Qt.RightButton) {
                            event.accepted = true;
                            contextMenu.popup();
                            forceActiveFocus();
                        }
                        if (event.button === Qt.LeftButton && contextMenu.visible === true) {
                            event.accepted = true;
                            contextMenu.dismiss();
                            forceActiveFocus();
                        }
                    }

                    Component.onCompleted: {
                        if (!Plasmoid.configuration.fontSize) {
                            // Set fontSize to default if it is not set
                            Plasmoid.configuration.fontSize = font.pointSize
                        }
                    }

                    QQC2.Menu {
                        id: contextMenu

                        ShortcutMenuItem {
                            _sequence: StandardKey.Undo
                            _enabled: mainTextArea.canUndo
                            _iconName: "edit-undo"
                            _text: i18n("Undo")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.undo())
                        }

                        ShortcutMenuItem {
                            _sequence: StandardKey.Redo
                            _enabled: mainTextArea.canRedo
                            _iconName: "edit-redo"
                            _text: i18n("Redo")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.redo())
                        }

                        QQC2.MenuSeparator {}

                        ShortcutMenuItem {
                            _sequence: StandardKey.Cut
                            _enabled: mainTextArea.selectedText.length > 0
                            _iconName: "edit-cut"
                            _text: i18n("Cut")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.cut())
                        }

                        ShortcutMenuItem {
                            _sequence: StandardKey.Copy
                            _enabled: mainTextArea.selectedText.length > 0
                            _iconName: "edit-copy"
                            _text: i18n("Copy")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.copy())
                        }

                        ShortcutMenuItem {
                            _sequence: StandardKey.Paste
                            _enabled: mainTextArea.canPaste
                            _iconName: "edit-paste"
                            _text: i18n("Paste")
                            onTriggered: contextMenu.retFocus(() => documentHandler.pasteWithoutFormatting())
                        }

                        ShortcutMenuItem {
                            _enabled: mainTextArea.canPaste
                            _text: i18n("Paste with Full Formatting")
                            _iconName: "edit-paste"
                            onTriggered: contextMenu.retFocus(() => mainTextArea.paste())
                        }

                        ShortcutMenuItem {
                            _enabled: mainTextArea.selectedText.length > 0
                            _text: i18nc("@action:inmenu", "Remove Formatting")
                            _iconName: "edit-clear-all"
                            onTriggered: {
                                var richText = mainTextArea.getFormattedText(mainTextArea.selectionStart, mainTextArea.selectionEnd)
                                var unformattedText = documentHandler.strip(richText)
                                unformattedText = unformattedText.replace(/\n/g, "<br>")
                                mainTextArea.remove(mainTextArea.selectionStart, mainTextArea.selectionEnd)
                                contextMenu.retFocus(() => mainTextArea.insert(mainTextArea.selectionStart, unformattedText))
                            }
                        }

                        ShortcutMenuItem {
                            _sequence: StandardKey.Delete
                            _enabled: mainTextArea.selectedText.length > 0
                            _iconName: "edit-delete"
                            _text: i18n("Delete")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.remove(mainTextArea.selectionStart, mainTextArea.selectionEnd))
                        }

                        ShortcutMenuItem {
                            _enabled: mainTextArea.text.length > 0
                            _iconName: "edit-clear"
                            _text: i18n("Clear")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.clear())
                        }

                        QQC2.MenuSeparator {}

                        ShortcutMenuItem {
                            _sequence: StandardKey.SelectAll
                            _enabled: mainTextArea.text.length > 0
                            _iconName: "edit-select-all"
                            _text: i18n("Select All")
                            onTriggered: contextMenu.retFocus(() => mainTextArea.selectAll())
                        }

                        function retFocus(f) {
                            f()
                            documentHandler.reset()
                            mainTextArea.forceActiveFocus()
                        }
                    }
                }

                // Save scrolling position when it changes, but throttle to avoid
                // killing a storage disk.
                Connections {
                    target: scrollview.contentItem
                    function onContentXChanged() {
                        throttedScrollSaver.restart();
                    }
                    function onContentYChanged() {
                        throttedScrollSaver.restart();
                    }
                }
                Connections {
                    target: mainTextArea
                    function onCursorPositionChanged() {
                        throttedScrollSaver.restart();
                    }
                }

                Timer {
                    id: throttedScrollSaver
                    interval: Kirigami.Units.humanMoment
                    repeat: false
                    running: false
                    onTriggered: scrollview.saveScroll()
                }

                function saveScroll() {
                    const flickable = scrollview.contentItem;
                    Plasmoid.configuration.scrollX = flickable.contentX;
                    Plasmoid.configuration.scrollY = flickable.contentY;
                    Plasmoid.configuration.cursorPosition = mainTextArea.cursorPosition;
                }

                function restoreScroll() {
                    const flickable = scrollview.contentItem;
                    flickable.contentX = Plasmoid.configuration.scrollX;
                    flickable.contentY = Plasmoid.configuration.scrollY;
                    mainTextArea.cursorPosition = Plasmoid.configuration.cursorPosition;
                }

                // Give it some time to lay out the text, because at this
                // point in time content size is not reliable yet.
                Component.onCompleted: Qt.callLater(restoreScroll)
                Component.onDestruction: saveScroll()
            }

            DragDrop.DropArea {
                id: dropArea
                anchors.fill: scrollview

                function positionOfDrop(event) {
                    return mainTextArea.positionAt(event.x, event.y)
                }

                onDrop: {
                    var mimeData = event.mimeData
                    var text = ""
                    if (mimeData.hasUrls) {
                        var urls = mimeData.urls
                        for (var i = 0, j = urls.length; i < j; ++i) {
                            var url = urls[i]
                            text += "<a href=\"" + url + "\">" + url + "</a><br>"
                        }
                    } else {
                        text = mimeData.text.replace(/\n/g, "<br>")
                    }

                    mainTextArea.insert(positionOfDrop(event), text)
                    event.accept(Qt.CopyAction)
                }
                onDragMove: {
                    // there doesn't seem to be a "just move the cursor", so we move
                    // the selection and then unselect so the cursor follows the mouse
                    mainTextArea.moveCursorSelection(positionOfDrop(event))
                    mainTextArea.deselect()
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: mainTextArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                    acceptedButtons: Qt.NoButton
                }
            }

            RowLayout {
                id: fontButtons
                spacing: Kirigami.Units.smallSpacing
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: visible ? implicitHeight : 0
                visible: opacity > 0
                opacity: focusScope.activeFocus ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration } }

                readonly property int requiredWidth: formatButtonsRow.width + spacing + settingsButton.width + removeButton.width
                readonly property bool showFormatButtons: width > requiredWidth

                Row {
                    id: formatButtonsRow
                    spacing: Kirigami.Units.smallSpacing
                    // show format buttons if TextField or any of the buttons have focus
                    enabled: opacity > 0
                    visible: fontButtons.showFormatButtons

                    PlasmaComponents3.ToolButton {
                        focusPolicy: Qt.TabFocus
                        icon.name: "format-text-bold"
                        icon.color: textIconColor
                        checked: documentHandler.bold
                        onClicked: documentHandler.bold = !documentHandler.bold
                        Accessible.name: boldTooltip.text
                        PlasmaComponents3.ToolTip {
                            id: boldTooltip
                            text: i18nc("@info:tooltip", "Bold")
                        }
                    }
                    PlasmaComponents3.ToolButton {
                        focusPolicy: Qt.TabFocus
                        icon.name: "format-text-italic"
                        icon.color: textIconColor
                        checked: documentHandler.italic
                        onClicked: documentHandler.italic = !documentHandler.italic
                        Accessible.name: italicTooltip.text
                        PlasmaComponents3.ToolTip {
                            id: italicTooltip
                            text: i18nc("@info:tooltip", "Italic")
                        }
                    }
                    PlasmaComponents3.ToolButton {
                        focusPolicy: Qt.TabFocus
                        icon.name: "format-text-underline"
                        icon.color: textIconColor
                        checked: documentHandler.underline
                        onClicked: documentHandler.underline = !documentHandler.underline
                        Accessible.name: underlineTooltip.text
                        PlasmaComponents3.ToolTip {
                            id: underlineTooltip
                            text: i18nc("@info:tooltip", "Underline")
                        }
                    }
                    PlasmaComponents3.ToolButton {
                        focusPolicy: Qt.TabFocus
                        icon.name: "format-text-strikethrough"
                        icon.color: textIconColor
                        checked: documentHandler.strikeOut
                        onClicked: documentHandler.strikeOut = !documentHandler.strikeOut
                        Accessible.name: strikethroughTooltip.text
                        PlasmaComponents3.ToolTip {
                            id: strikethroughTooltip
                            text: i18nc("@info:tooltip", "Strikethrough")
                        }
                    }
                }

                Item { // spacer
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                PlasmaComponents3.ToolButton {
                    id: settingsButton
                    focusPolicy: Qt.TabFocus
                    icon.name: "configure"
                    icon.color: textIconColor
                    onClicked: Plasmoid.internalAction("configure").trigger()
                    Accessible.name: settingsTooltip.text
                    PlasmaComponents3.ToolTip {
                        id: settingsTooltip
                        text: Plasmoid.internalAction("configure").text
                    }
                }

                PlasmaComponents3.ToolButton {
                    id: removeButton
                    focusPolicy: Qt.TabFocus
                    icon.name: "edit-delete"
                    icon.color: textIconColor
                    onClicked: {
                        // No need to ask for confirmation in the cases when...
                        // ...the note is blank
                        if (mainTextArea.length === 0 ||
                            // ...the note's content is equal to the clipboard text

                            // Note that we are intentionally not using
                            // mainTextArea.getText() because it has a method of
                            // converting the text to plainText that does not produce
                            // the same exact output of various other methods, and if
                            // we go out of our way to match it, we will be
                            // depending on an implementation detail. So we instead
                            // roll our own version to ensure that the conversion
                            // is done in the same way every time.
                            documentHandler.stripAndSimplify(mainTextArea.text) === documentHandler.strippedClipboardText()
                        ) {
                            Plasmoid.internalAction("remove").trigger();
                        } else {
                            discardConfirmationDialogLoader.open();
                        }
                    }
                    Accessible.name: removeTooltip.text
                    PlasmaComponents3.ToolTip {
                        id: removeTooltip
                        text: Plasmoid.internalAction("remove").text
                    }
                }
            }
        }

        Loader {
            id: discardConfirmationDialogLoader

            function open() {
                if (item) {
                    item.open();
                } else {
                    active = true;
                }
                item.visible = true;
            }

            active: false

            sourceComponent: MessageDialog {
                visible: false
                title: i18n("Discard this note?")
                text: i18n("Are you sure you want to discard this note?")

                buttons: MessageDialog.Discard | MessageDialog.Cancel

                onButtonClicked: (button, role) => {
                    if (button === MessageDialog.Discard) {
                        Plasmoid.internalAction("remove").trigger()
                        visible = false;
                    }
                }

                onRejected: {
                    visible = false
                }
            }
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "White")
            onTriggered: Plasmoid.configuration.color = "white"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Black")
            onTriggered: Plasmoid.configuration.color = "black"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Red")
            onTriggered: Plasmoid.configuration.color = "red"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Orange")
            onTriggered: Plasmoid.configuration.color = "orange"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Yellow")
            onTriggered: Plasmoid.configuration.color = "yellow"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Green")
            onTriggered: Plasmoid.configuration.color = "green"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Blue")
            onTriggered: Plasmoid.configuration.color = "blue"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Pink")
            onTriggered: Plasmoid.configuration.color = "pink"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Transparent")
            onTriggered: Plasmoid.configuration.color = "translucent"
        },
        PlasmaCore.Action {
            text: i18nc("@item:inmenu", "Transparent Light")
            onTriggered: Plasmoid.configuration.color = "translucent-light"
        },
        PlasmaCore.Action {
            isSeparator: true
        }
    ]

    Component.onCompleted: {
        // Plasmoid configuration doesn't check before emitting change signal
        // explicit check is needed (at time of writing)
        if (note.id != Plasmoid.configuration.noteId) {
            Plasmoid.configuration.noteId = note.id;
        }
    }
}
