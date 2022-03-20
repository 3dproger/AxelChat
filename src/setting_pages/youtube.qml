import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "../my_components" as MyComponents
import AxelChat.AbstractChatService 1.0
import AxelChat.YouTube 1.0

ScrollView {
    id: root
    clip: true
    padding: 6
    contentHeight: column.implicitHeight
    contentWidth: column.implicitWidth

    Column {
        id: column
        spacing: 6

        Row {
            spacing: 12

            Image {
                mipmap: true
                height: 40
                width: height
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/resources/images/youtube-icon.svg"
                fillMode: Image.PreserveAspectFit
            }

            Label {
                text: qsTr("YouTube")
                font.bold: true
                font.pixelSize: 25
                color: Material.foreground
                anchors.verticalCenter: parent.verticalCenter
            }

            BusyIndicator {
                id: busyIndicator
                visible: youTube.connectionStateType === 20 // ToDo: need refactoring
                height: 40
                width: height
                anchors.verticalCenter: parent.verticalCenter
                antialiasing: false
                smooth: false
            }

            Image {
                visible: !busyIndicator.visible
                height: busyIndicator.height
                width: height
                anchors.verticalCenter: parent.verticalCenter
                mipmap: true
                source: {
                    if (youTube.connectionStateType === 10) // ToDo: need refactoring
                    {
                        return "qrc:/resources/images/alert1.svg"
                    }
                    else if (youTube.connectionStateType === 30) // ToDo: need refactoring
                    {
                        return "qrc:/resources/images/tick.svg"
                    }

                    return "";
                }
            }

            Label {
                id: labelState
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: 14
                text: youTube.stateDescription
            }
        }

        Row {
            spacing: 6

            Label {
                color: Material.accentColor
                text: qsTr("Broadcast:")
                anchors.verticalCenter: parent.verticalCenter
                font.bold: true
                font.pixelSize: 20
            }

            MyComponents.MyTextField {
                id: textFieldUserSpecifiedLink
                placeholderText: qsTr("Paste the broadcast link or ID here...")
                anchors.verticalCenter: parent.verticalCenter
                width: 400

                Component.onCompleted: {
                    text = youTube.userSpecifiedLink
                }

                onTextChanged: {
                    youTube.userSpecifiedLink = text
                }
            }

            Button {
                anchors.verticalCenter: parent.verticalCenter
                highlighted: true
                text: qsTr("Paste")
                display: AbstractButton.TextBesideIcon
                icon.source: "qrc:/resources/images/clipboard-paste-button.svg"

                onClicked: {
                    if (clipboard.text.length !== 0)
                    {
                        textFieldUserSpecifiedLink.text = clipboard.text;
                        textFieldUserSpecifiedLink.deselect();
                    }
                }
            }
        }

        Row {
            spacing: 6

            Button {
                text: qsTr("Control Panel")
                anchors.verticalCenter: parent.verticalCenter
                enabled: youTube.controlPanelUrl.toString().length !== 0
                icon.source: "qrc:/resources/images/youtube-control-panel.svg"
                onClicked: {
                    Qt.openUrlExternally(youTube.controlPanelUrl)
                }
            }

            Button {
                text: qsTr("Broadcast")
                anchors.verticalCenter: parent.verticalCenter
                enabled: youTube.broadcastUrl.toString().length !== 0
                onClicked: {
                    Qt.openUrlExternally(youTube.broadcastLongUrl)
                }
            }

            Button {
                text: qsTr("Chat")
                anchors.verticalCenter: parent.verticalCenter
                enabled: youTube.chatUrl.toString().length !== 0
                onClicked: {
                    Qt.openUrlExternally(youTube.chatUrl)
                }
            }
        }

        /*Row {
            MyComponents.MyTextArea {
                readOnly: true
                text: youTube.detailedInformation
            }
        }*/
    }
}






