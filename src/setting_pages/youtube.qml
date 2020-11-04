import QtQuick 2.15
import QtQuick.Controls 2.15
import "../my_components" as MyComponents
import AxelChat.YouTube 1.0

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 640

    property color accentColor: "#ff0000"

    Item {
        id: element1
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Text {
            id: element
            y: 57
            height: 26
            color: accentColor
            text: qsTr("Broadcast link or ID:")
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
            styleColor: "#000000"
            font.bold: false
            style: Text.Normal
            font.weight: Font.Bold
            font.pixelSize: 20
        }

        MyComponents.MyTextField {
            id: textFieldUserSpecifiedLink
            y: 108
            height: 43
            autoTrim: true
            placeholderText: qsTr("Paste the broadcast link or ID here...")
            selectByMouse: true
            anchors.left: parent.left
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

        Switch {
            id: switchEnableProxy
            x: 8
            y: 159
            text: qsTr("Proxy")

            Component.onCompleted: {
                checked = chatHandler.proxyEnabled;
            }

            onCheckedChanged: {
                chatHandler.proxyEnabled = checked;
            }
        }

        MyComponents.MyTextField {
            id: textFieldProxyServerAddress
            anchors.left: switchEnableProxy.right
            anchors.leftMargin: 0
            maximumLength: 15
            autoTrim: true
            y: 162
            width: 150
            height: 43
            anchors.verticalCenter: switchEnableProxy.verticalCenter
            placeholderText: qsTr("Proxy Server IP")
            validator: RegExpValidator {
                regExp:  /^((?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\.){0,3}(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])$/
           }

            Component.onCompleted: {
                text = chatHandler.proxyServerAddress
            }

            onTextChanged: {
                chatHandler.proxyServerAddress = text
            }
        }

        Text {
            id: text1
            y: 80
            text: ":";
            anchors.verticalCenter: textFieldProxyServerAddress.verticalCenter
            anchors.left: textFieldProxyServerAddress.right
            anchors.leftMargin: 6
        }

        MyComponents.MyTextField {
            id: textFieldProxyServerPort
            anchors.left: text1.right
            anchors.leftMargin: 6
            maximumLength: 5
            autoTrim: true
            y: 65
            width: 60
            height: 43
            anchors.verticalCenter: text1.verticalCenter
            placeholderText: qsTr("Port")
            validator: RegExpValidator {
                regExp:  /^[0-9]{1,5}$/
            }

            Component.onCompleted: {
                var port = chatHandler.proxyServerPort;
                if (port !== -1)
                {
                    text = port;
                }
                else
                {
                    text = "";
                }
            }

            onTextChanged: {
                if (text == "")
                {
                    chatHandler.proxyServerPort = -1;
                }
                else
                {
                    chatHandler.proxyServerPort = text;
                }
            }
        }

        Text {
            id: element3
            x: 8
            y: 227
            text: qsTr("Broadcast ID:")
            font.pixelSize: 15
        }

        MyComponents.MyTextField {
            id: textBroadcastId
            y: 213
            height: 46
            text: youTube.broadcastId
            anchors.right: buttonCopyBroadcastId.left
            anchors.rightMargin: 4
            anchors.left: element3.right
            anchors.leftMargin: 6
            selectByMouse: true
            readOnly: true
            horizontalAlignment: Text.AlignLeft
        }

        Text {
            id: element4
            x: 8
            y: 276
            text: qsTr("Broadcast:")
            font.pixelSize: 15
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
            text: qsTr("YouTube")
            font.bold: true
            font.pixelSize: 25
        }

        Image {
            id: image
            x: 8
            y: 13
            mipmap: true
            width: 49
            height: 30
            source: "qrc:/resources/images/youtube-icon.svg"
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
        }

        Button {
            id: buttonPasteUserSpecifiedLink
            x: 360
            y: 51
            width: 100
            height: 39
            text: qsTr("Paste")
            flat: true
            anchors.right: buttonCopyUserSpecifiedLink.left
            anchors.rightMargin: 6
            font.pointSize: 8
            icon.source: "qrc:/resources/images/clipboard-paste-button.svg"

            onClicked: {
                if (clipboard.text.length != 0)
                {
                    textFieldUserSpecifiedLink.text = clipboard.text;
                    textFieldUserSpecifiedLink.deselect();
                }
            }
        }

        Button {
            id: buttonCopyUserSpecifiedLink
            x: 446
            y: 51
            width: 120
            height: 39
            text: qsTr("Copy")
            flat: true
            anchors.right: buttonOpenUserSpecifiedLink.left
            anchors.rightMargin: 6
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
            x: 568
            y: 51
            width: 100
            height: 39
            text: qsTr("Open")
            anchors.right: textFieldUserSpecifiedLink.right
            anchors.rightMargin: 0
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

        Button {
            id: buttonCopyBroadcastId
            x: 550
            y: 217
            width: 39
            height: 39
            text: qsTr("Copy")
            display: AbstractButton.IconOnly
            flat: true
            anchors.right: parent.right
            anchors.rightMargin: 51
            font.pointSize: 8
            icon.source: "qrc:/resources/images/copy-content.svg"

            onClicked: {
                if (textBroadcastId.text.length != 0)
                {
                    clipboard.text = textBroadcastId.text;
                    textBroadcastId.selectAll();
                    Qt.callLater(forceActiveFocus);
                    Qt.callLater(textBroadcastId.forceActiveFocus);
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
    }
}






