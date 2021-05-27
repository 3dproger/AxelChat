import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "../my_components" as MyComponents
import AxelChat.AbstractChatService 1.0
import AxelChat.YouTube 1.0

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
            text: qsTr("Broadcast:")
            anchors.left: parent.left
            anchors.leftMargin: 8
            font.bold: false
            style: Text.Normal
            font.weight: Font.Bold
            font.pixelSize: 20
        }

        MyComponents.MyTextField {
            id: textFieldUserSpecifiedLink
            y: 59
            height: 43
            placeholderText: qsTr("Paste the broadcast link or ID here...")
            anchors.left: element.right
            anchors.leftMargin: 10
            anchors.right: buttonPasteUserSpecifiedLink.left
            anchors.rightMargin: 4

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
            y: 105
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
            anchors.verticalCenterOffset: 0
            anchors.leftMargin: 0
            maximumLength: 15
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
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textFieldProxyServerPort
            anchors.left: text1.right
            anchors.leftMargin: 6
            maximumLength: 5
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
            y: 165
            text: qsTr("Broadcast ID:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textBroadcastId
            y: 151
            height: 46
            text: youTube.broadcastId
            anchors.right: buttonCopyBroadcastId.left
            anchors.rightMargin: 4
            anchors.left: element3.right
            anchors.leftMargin: 6
            readOnly: true
            horizontalAlignment: Text.AlignLeft
        }

        Text {
            id: element4
            x: 8
            y: 214
            text: qsTr("Broadcast:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textBroadcastURL
            y: 200
            height: 46
            text: youTube.broadcastUrl
            anchors.left: element4.right
            anchors.leftMargin: 6
            anchors.right: buttonCopyBroadcastUrl.left
            anchors.rightMargin: 4
            readOnly: true
        }

        Text {
            id: element5
            x: 8
            y: 318
            text: qsTr("Chat:")
            font.pixelSize: 15
            color: Material.foreground
        }

        MyComponents.MyTextField {
            id: textChatURL
            y: 304
            height: 46
            text: youTube.chatUrl
            anchors.left: element5.right
            anchors.leftMargin: 17
            anchors.right: buttonCopyChatUrl.left
            anchors.rightMargin: 8
            readOnly: true
        }

        Text {
            id: element2
            x: 63
            y: 15
            text: qsTr("YouTube")
            font.bold: true
            font.pixelSize: 25
            color: Material.foreground
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
            y: 252
            height: 46
            text: youTube.controlPanelUrl
            anchors.left: element6.right
            anchors.leftMargin: 8
            anchors.right: buttonCopyControlPanelCopy.left
            anchors.rightMargin: 4
            readOnly: true
        }

        Text {
            id: element6
            x: 8
            y: 267
            text: qsTr("Control Panel:")
            font.pixelSize: 15
            color: Material.foreground
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
            x: 593
            y: 204
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
            y: 256
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
            y: 308
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
            y: 204
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
            y: 256
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
            y: 308
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
            y: 155
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
            visible: youTube.connectionStateType === 20 // ToDo: need refactoring
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
            anchors.verticalCenter: busyIndicator.verticalCenter
            anchors.left: busyIndicator.right
            anchors.leftMargin: 12
            font.pointSize: 14
            x: 241
            y: 20
            text: youTube.stateDescription
        }
    }
}






