import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "../my_components" as MyComponents
import AxelChat.Twitch 1.0

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 640

    Item {
        id: element1
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Text {
            id: element
            y: 57
            height: 26
            color: "#FF0000"
            text: qsTr("Nick or link to channel:")
            anchors.left: parent.left
            anchors.leftMargin: 8
            //styleColor: "#000000"
            font.bold: false
            style: Text.Normal
            font.weight: Font.Bold
            font.pixelSize: 20
        }

        MyComponents.MyTextField {
            id: textFieldUserSpecifiedLink
            y: 49
            height: 43
            autoTrim: true
            placeholderText: qsTr("Paste the broadcast link or ID here...")
            selectByMouse: true
            anchors.left: element.right
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8

            Component.onCompleted: {
                text = youTube.userSpecifiedLink
            }

            onTextChanged: {
                youTube.userSpecifiedLink = text
            }
        }

        MyComponents.MyTextField {
            id: textFieldOAuthToken
            y: 98
            height: 43
            autoTrim: true
            placeholderText: qsTr("Paste OAuth token here...")
            selectByMouse: true
            anchors.left: element3.right
            anchors.leftMargin: 8
            anchors.right: buttonGetOAuthToken.left
            anchors.rightMargin: 8

            Component.onCompleted: {
                text = youTube.userSpecifiedLink
            }

            onTextChanged: {
                youTube.userSpecifiedLink = text
            }
        }

        MyComponents.MyTextField {
            id: textFieldChannelToConnect
            y: 147
            height: 43
            autoTrim: true
            placeholderText: qsTr("Paste channel to connect or leave it blank...")
            selectByMouse: true
            anchors.left: element7.right
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8

            Component.onCompleted: {
                text = youTube.userSpecifiedLink
            }

            onTextChanged: {
                youTube.userSpecifiedLink = text
            }
        }

        Text {
            id: element4
            x: 8
            y: 276
            text: qsTr("Broadcast:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textBroadcastURL
            y: 262
            height: 46
            text: youTube.broadcastShortUrl
            anchors.left: element4.right
            anchors.leftMargin: 6
            anchors.right: buttonCopyBroadcastUrl.left
            anchors.rightMargin: 4
            readOnly: true
            selectByMouse: true
        }

        Text {
            id: element5
            x: 8
            y: 380
            text: qsTr("Chat:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textChatURL
            y: 366
            height: 46
            text: youTube.chatUrl
            anchors.left: element5.right
            anchors.leftMargin: 17
            anchors.right: buttonCopyChatUrl.left
            anchors.rightMargin: 8
            readOnly: true
            selectByMouse: true
        }

        Text {
            id: element2
            x: 63
            y: 15
            text: qsTr("Twitch")
            font.bold: true
            font.pixelSize: 25
            color: Material.foreground
        }

        Image {
            id: image
            x: 8
            y: 9
            mipmap: true
            width: 49
            height: 42
            source: "../resources/images/twitch-round.svg"
            sourceSize.height: 67
            fillMode: Image.PreserveAspectFit
        }

        MyComponents.MyTextField {
            id: textControlPanelURL
            y: 314
            height: 46
            text: youTube.controlPanelUrl
            anchors.left: element6.right
            anchors.leftMargin: 8
            anchors.right: buttonCopyControlPanelCopy.left
            anchors.rightMargin: 4
            readOnly: true
            selectByMouse: true
        }

        Text {
            id: element6
            x: 8
            y: 329
            text: qsTr("Control Panel:")
            font.pixelSize: 15
            color: Material.foreground
        }

        Button {
            id: buttonOpenBroadcastUrl
            x: 593
            y: 266
            width: 39
            height: 39
            text: qsTr("Open")
            display: AbstractButton.IconOnly
            anchors.right: parent.right
            anchors.rightMargin: 8
            flat: true
            highlighted: true
            font.pointSize: 8
            icon.source: "qrc:/resources/images/forward-arrow.svg"

            onClicked: {
                if (textBroadcastURL.text.length != 0)
                {
                    Qt.openUrlExternally(textBroadcastURL.text)
                }
            }
        }

        Button {
            id: buttonOpenControlPanelUrl
            x: 593
            y: 318
            width: 39
            height: 39
            text: qsTr("Open")
            anchors.right: parent.right
            anchors.rightMargin: 8
            display: AbstractButton.IconOnly
            flat: true
            highlighted: true
            font.pointSize: 8
            icon.source: "qrc:/resources/images/forward-arrow.svg"

            onClicked: {
                if (textControlPanelURL.text.length != 0)
                {
                    Qt.openUrlExternally(textControlPanelURL.text)
                }
            }
        }

        Button {
            id: buttonOpenChatUrl
            x: 595
            y: 370
            width: 39
            height: 39
            text: qsTr("Open")
            anchors.right: parent.right
            anchors.rightMargin: 6
            display: AbstractButton.IconOnly
            flat: true
            highlighted: true
            font.pointSize: 8
            icon.source: "qrc:/resources/images/forward-arrow.svg"

            onClicked: {
                if (textChatURL.text.length != 0)
                {
                    Qt.openUrlExternally(textChatURL.text)
                }
            }
        }

        Button {
            id: buttonCopyBroadcastUrl
            x: 550
            y: 266
            width: 39
            height: 39
            text: qsTr("Copy")
            display: AbstractButton.IconOnly
            flat: true
            anchors.right: buttonOpenBroadcastUrl.left
            anchors.rightMargin: 4
            font.pointSize: 8
            icon.source: "qrc:/resources/images/copy-content.svg"

            onClicked: {
                if (textBroadcastURL.text.length != 0)
                {
                    clipboard.text = textBroadcastURL.text;
                    textBroadcastURL.selectAll();
                    Qt.callLater(forceActiveFocus);
                    Qt.callLater(textBroadcastURL.forceActiveFocus);
                }
            }
        }

        Button {
            id: buttonCopyControlPanelCopy
            x: 550
            y: 318
            width: 39
            height: 39
            text: qsTr("Copy")
            anchors.right: buttonOpenControlPanelUrl.left
            anchors.rightMargin: 4
            display: AbstractButton.IconOnly
            flat: true
            font.pointSize: 8
            icon.source: "qrc:/resources/images/copy-content.svg"

            onClicked: {
                if (textControlPanelURL.text.length != 0)
                {
                    clipboard.text = textControlPanelURL.text;
                    textControlPanelURL.selectAll();
                    Qt.callLater(forceActiveFocus);
                    Qt.callLater(textControlPanelURL.forceActiveFocus);
                }
            }
        }

        Button {
            id: buttonCopyChatUrl
            x: 552
            y: 370
            width: 39
            height: 39
            text: qsTr("Copy")
            anchors.right: buttonOpenChatUrl.left
            anchors.rightMargin: 4
            display: AbstractButton.IconOnly
            flat: true
            font.pointSize: 8
            icon.source: "qrc:/resources/images/copy-content.svg"

            onClicked: {
                if (textChatURL.text.length != 0)
                {
                    clipboard.text = textChatURL.text;
                    textChatURL.selectAll();
                    Qt.callLater(forceActiveFocus);
                    Qt.callLater(textChatURL.forceActiveFocus);
                }
            }
        }

        BusyIndicator {
            id: busyIndicator
            y: 4
            visible: {
                if (youTube.broadcastId.length == 0)
                {
                    if (youTube.userSpecifiedLink.trim().length == 0)
                    {
                        return false
                    }
                    else
                    {
                        return false
                    }
                }
                else if (!youTube.connected)
                {
                    return true
                }
                else
                {
                    return false
                }
            }
            height: 50
            width: height
            anchors.verticalCenter: image.verticalCenter
            anchors.left: element2.right
            anchors.leftMargin: 12
            antialiasing: false
            smooth: false
        }

        Image {
            id: imageState
            visible: !busyIndicator.visible
            height: 35
            width: height
            mipmap: true
            anchors.verticalCenter: image.verticalCenter
            anchors.left: element2.right
            anchors.leftMargin: 12
            source: {
                if (youTube.broadcastId.length == 0)
                {
                    if (youTube.userSpecifiedLink.trim().length == 0)
                    {
                        return "qrc:/resources/images/alert1.svg"
                    }
                    else
                    {
                        return "qrc:/resources/images/alert1.svg"
                    }
                }
                else if (!youTube.connected)
                {
                    return "qrc:/resources/images/alert1.svg"
                }
                else
                {
                    return "qrc:/resources/images/tick.svg"
                }
            }
        }

        Label {
            id: labelState
            anchors.verticalCenter: busyIndicator.verticalCenter
            anchors.left: busyIndicator.right
            anchors.leftMargin: 12
            font.pointSize: 14
            x: 241
            y: 20
            text: {
                if (youTube.broadcastId.length == 0)
                {
                    if (youTube.userSpecifiedLink.trim().length == 0)
                    {
                        return qsTr("Link or broadcast ID is not specified");
                    }
                    else
                    {
                        return qsTr("Incorrect link or broadcast ID specified");
                    }
                }
                else if (!youTube.connected)
                {
                    return qsTr("Connecting...");
                }
                else
                {
                    return qsTr("Successfully Connected!");
                }
            }
        }

        Text {
            id: element3
            y: 107
            height: 26
            color: "#ff0000"
            text: qsTr("OAuth token:")
            anchors.left: parent.left
            font.pixelSize: 20
            font.bold: false
            font.weight: Font.Bold
            style: Text.Normal
            anchors.leftMargin: 8
        }

        Button {
            id: buttonGetOAuthToken
            x: 543
            y: 96
            text: qsTr("Get token")
            anchors.right: parent.right
            anchors.rightMargin: 8
        }

        Text {
            id: element7
            y: 156
            height: 26
            color: "#ff0000"
            text: qsTr("Nick or link to connect:")
            anchors.left: parent.left
            font.pixelSize: 20
            font.weight: Font.Bold
            font.bold: false
            style: Text.Normal
            anchors.leftMargin: 8
        }
    }
}






