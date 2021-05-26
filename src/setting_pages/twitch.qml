import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "../my_components" as MyComponents
import AxelChat.Twitch 1.0
import AxelChat.AbstractChatService 1.0

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
            y: 68
            height: 26
            color: Material.accentColor
            text: qsTr("Channel:")
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
            y: 60
            height: 43
            placeholderText: qsTr("Paste the link or channel name here...")
            selectByMouse: true
            anchors.left: element.right
            anchors.leftMargin: 10
            anchors.right: buttonPasteUserSpecifiedLink.left
            anchors.rightMargin: 6

            Component.onCompleted: {
                text = twitch.userSpecifiedChannel
            }

            onTextChanged: {
                twitch.userSpecifiedChannel = text
            }
        }

        Button {
            id: buttonPasteUserSpecifiedLink
            x: 505
            y: 61
            height: 39
            text: qsTr("Paste")
            flat: true
            anchors.right: buttonCopyUserSpecifiedLink.left
            display: AbstractButton.TextBesideIcon
            anchors.rightMargin: 4
            font.pointSize: 8
            icon.source: "qrc:/resources/images/clipboard-paste-button.svg"

            onClicked: {
                if (clipboard.text.length !== 0)
                {
                    textFieldUserSpecifiedLink.text = clipboard.text;
                    textFieldUserSpecifiedLink.deselect();
                }
            }
        }

        Button {
            id: buttonCopyUserSpecifiedLink
            x: 548
            y: 61
            width: 39
            height: 39
            text: qsTr("Copy")
            flat: true
            anchors.right: buttonOpenUserSpecifiedLink.left
            display: AbstractButton.IconOnly
            anchors.rightMargin: 4
            font.pointSize: 8
            icon.source: "qrc:/resources/images/copy-content.svg"

            onClicked: {
                if (textFieldUserSpecifiedLink.text.length != 0)
                {
                    clipboard.text = textFieldUserSpecifiedLink.text;
                    textFieldUserSpecifiedLink.selectAll();
                    Qt.callLater(forceActiveFocus);
                    Qt.callLater(textFieldUserSpecifiedLink.forceActiveFocus);
                    buttonCopyUserSpecifiedLink.text = qsTr("Copied!");
                    buttonCopyUserSpecifiedLinkTimer.restart();
                    buttonCopyUserSpecifiedLinkTimer.running = true;
                }
            }

            Timer {
                id: buttonCopyUserSpecifiedLinkTimer
                interval: 5000
                onTriggered: {
                    buttonCopyUserSpecifiedLink.text = qsTr("Copy")
                }
            }
        }

        Button {
            id: buttonOpenUserSpecifiedLink
            x: 593
            y: 61
            width: 39
            height: 39
            text: qsTr("Open")
            anchors.right: parent.right
            anchors.rightMargin: 8
            display: AbstractButton.IconOnly
            highlighted: true
            flat: true
            font.pointSize: 8
            icon.source: "qrc:/resources/images/forward-arrow.svg"

            onClicked: {
                if (textFieldUserSpecifiedLink.text.length != 0)
                {
                    if (youTube.isBroadcastIdUserSpecified)
                    {
                        Qt.openUrlExternally(textBroadcastURL.text)
                    }
                    else if (textBroadcastURL.text.length != 0)
                    {
                        Qt.openUrlExternally(textFieldUserSpecifiedLink.text)
                    }
                }
            }
        }

        Button {
            id: buttonOpenBroadcastUrl
            x: 595
            y: 167
            width: 39
            height: 39
            text: qsTr("Open")
            display: AbstractButton.IconOnly
            anchors.right: parent.right
            anchors.rightMargin: 6
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

        MyComponents.MyTextField {
            id: textFieldOAuthToken
            y: 111
            height: 43
            placeholderText: qsTr("Paste OAuth token here...")
            echoMode: TextInput.Password
            selectByMouse: true
            anchors.left: element3.right
            anchors.leftMargin: 8
            anchors.right: buttonPasteOAuthToken.left
            anchors.rightMargin: 6

            Component.onCompleted: {
                text = twitch.oauthToken
            }

            onTextChanged: {
                twitch.oauthToken = text
            }
        }

        Button {
            id: buttonPasteOAuthToken
            x: 498
            y: 113
            //width: 39
            height: 39
            text: qsTr("Paste")
            anchors.right: buttonGetOAuthToken.left
            flat: true
            anchors.rightMargin: 6
            icon.source: "qrc:/resources/images/clipboard-paste-button.svg"
            font.pointSize: 8
            display: AbstractButton.TextBesideIcon//IconOnly

            onClicked: {
                if (clipboard.text.length !== 0)
                {
                    textFieldOAuthToken.text = clipboard.text;
                    textFieldOAuthToken.deselect();
                }
            }
        }

        Text {
            id: element4
            x: 8
            y: 177
            text: qsTr("Broadcast:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textBroadcastURL
            y: 163
            height: 46
            text: twitch.broadcastUrl
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
            y: 281
            text: qsTr("Chat:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textChatURL
            y: 267
            height: 46
            text: twitch.chatUrl
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
            y: 215
            height: 46
            text: twitch.controlPanelUrl
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
            y: 230
            text: qsTr("Control Panel:")
            font.pixelSize: 15
            color: Material.foreground
        }

        Button {
            id: buttonOpenControlPanelUrl
            x: 593
            y: 219
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
            y: 271
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
            y: 167
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
            y: 219
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
            y: 271
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
            visible: twitch.connectionStateType === AbstractChatService.Connecting

            /*{
                if (twitch.broadcastUrl.length === 0)
                {
                    if (twitch.userSpecifiedChannel.trim().length === 0)
                    {
                        return false
                    }
                    else
                    {
                        return false
                    }
                }
                else if (twitch.connectionStateType === AbstractChatService.Connecting)
                {
                    return true
                }
                else
                {
                    return false
                }
            }*/
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
                console.log(twitch.connectionStateType)

                console.log(AbstractChatService.Connected)

                if (twitch.connectionStateType === AbstractChatService.NotConnected)
                {
                    return "qrc:/resources/images/alert1.svg"
                }
                else if (twitch.connectionStateType === AbstractChatService.Connected)
                {
                    return "qrc:/resources/images/tick.svg"
                }
            }

            /*{
                if (twitch.broadcastUrl.length === 0)
                {
                    if (twitch.userSpecifiedChannel.trim().length === 0)
                    {
                        return "qrc:/resources/images/alert1.svg"
                    }
                    else
                    {
                        return "qrc:/resources/images/alert1.svg"
                    }
                }
                else if (!twitch.connected)
                {
                    return "qrc:/resources/images/alert1.svg"
                }
                else
                {
                    return "qrc:/resources/images/tick.svg"
                }
            }*/
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
                if (twitch.broadcastUrl.length === 0)
                {
                    if (twitch.userSpecifiedChannel.trim().length === 0)
                    {
                        return qsTr("Link or channel name is not specified");
                    }
                    else
                    {
                        return qsTr("Incorrect link or channel name specified");
                    }
                }
                else if (!twitch.connected)
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
            y: 120
            height: 26
            color: Material.accentColor
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
            y: 109
            text: qsTr("Get token")
            anchors.right: parent.right
            anchors.rightMargin: 8

            onClicked: {
                Qt.openUrlExternally(twitch.requesGetAOuthTokenUrl);
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:1.1}D{i:28}
}
##^##*/
