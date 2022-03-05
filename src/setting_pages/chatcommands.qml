import QtQuick 2.15
import QtQuick.Controls 2.15
import "../my_components" as MyComponents

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 480

    property var allCommandsWindow;

    Item {
        id: element1
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Dial {
            id: dialBotVolume
            x: 8
            y: 140
            from: 1
            stepSize: 5
            to: 100

            Component.onCompleted: {
                value = chatBot.volume
            }

            onMoved: {
                chatBot.volume = value
            }
        }

        Label {
            id: element
            x: 8
            y: 116
            text: qsTr("Volume: %1 %").arg(chatBot.volume)
        }

        Switch {
            id: switchEnableSoundCommands
            y: 8
            height: 48
            text: qsTr("Enable Sound Commands")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8

            Component.onCompleted: {
                checked = chatBot.enabledCommands;
            }

            onCheckedChanged: {
                chatBot.enabledCommands = checked;
            }
        }

        MyComponents.MyTextField {
            id: textFieldTestMessage
            y: 271
            height: 43
            anchors.leftMargin: 8
            anchors.left: parent.left
            anchors.right: buttonSendTestMessage.left
            anchors.rightMargin: 8
            placeholderText: qsTr("Test message...")

            onAccepted: {
                sendTestMessage();
            }
        }

        Label {
            id: label
            y: 271
            height: 17
            text: qsTr("Send test message:")
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.left: textFieldTestMessage.left
            anchors.leftMargin: -2
            anchors.bottom: textFieldTestMessage.top
            anchors.bottomMargin: 6
        }

        Button {
            id: buttonSendTestMessage
            x: 303
            y: 312
            width: 169
            height: 48
            text: qsTr("Send")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenterOffset: 0
            anchors.verticalCenter: textFieldTestMessage.verticalCenter
            highlighted: true
            icon.source: "qrc:/resources/images/flask.svg"

            onClicked: {
                sendTestMessage();
            }
        }

        Button {
            id: buttonAllCommands
            y: 320
            text: qsTr("All Commands")
            anchors.left: buttonCommandsEditor.right
            anchors.leftMargin: 6
            onClicked: {
                if (typeof(allCommandsWindow) == "undefined")
                {
                    var component = Qt.createComponent("qrc:/setting_pages/all_commands_window.qml")
                    allCommandsWindow = component.createObject(root)
                }

                allCommandsWindow.show()
            }
        }

        Button {
            id: buttonCommandsEditor
            x: 8
            y: 320
            text: qsTr("Commands Editor")
            onClicked: {
                commandsEditor.open();
            }
        }

        Switch {
            id: switchIncludeBuiltInSoundCommands
            y: 62
            height: 48
            text: qsTr("Include Built-in Sound Commands")
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            Component.onCompleted: {
                checked = chatBot.includeBuiltInCommands;
            }

            onCheckedChanged: {
                chatBot.includeBuiltInCommands = checked;
            }
        }
    }

    function sendTestMessage(){
        if (textFieldTestMessage.text.length > 0)
        {
            chatHandler.sendTestMessage(textFieldTestMessage.text);
            textFieldTestMessage.text = "";
        }
    }
}


