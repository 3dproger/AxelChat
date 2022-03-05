import QtQuick 2.15
import QtQuick.Controls 2.15
import ClipboardQml.Clipboard 1.0

TextField {
    id: root
    selectByMouse: true

    persistentSelection: true
    background.visible: !readOnly

    onFocusChanged: {
        if (!focus)
        {
            deselect();
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        hoverEnabled: true
        cursorShape: Qt.IBeamCursor

        onClicked: {
            contextMenu.x = mouse.x;
            contextMenu.y = mouse.y;
            contextMenu.open();
        }

        Menu {
            id: contextMenu
            MenuItem {
                enabled: !readOnly && selectedText.length != 0 && echoMode != TextInput.Password
                text: qsTr("Cut")
                icon.source: "qrc:/resources/images/cut-content-button.svg"
                onTriggered: {
                    root.cut();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
            MenuItem {
                enabled: selectedText.length != 0 && echoMode != TextInput.Password
                text: qsTr("Copy")
                icon.source: "qrc:/resources/images/copy-content.svg"
                onTriggered: {
                    root.copy();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
            MenuItem {
                enabled: !readOnly && clipboard.text.length !== 0
                text: qsTr("Paste")
                icon.source: "qrc:/resources/images/clipboard-paste-button.svg"
                onTriggered: {
                    root.paste();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
            MenuSeparator {}
            MenuItem {
                enabled: root.selectedText.length != root.text.length
                text: qsTr("Select All")
                onTriggered: {
                    root.selectAll();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
        }
    }
}
