import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import AxelChat.QMLUtils 1.0
import AxelChat.I18n 1.0
import "../my_components" as MyComponents

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 640
    Item {
        id: element
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Dialog {
            id: restartDialog
            anchors.centerIn: parent
            title: qsTr("Changes will take effect after restarting the program");
            modal: true
            footer: DialogButtonBox {
                Button {
                    flat: true
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                    onClicked: {
                        restartDialog.close();
                    }
                }
                Button {
                    flat: true
                    text: qsTr("Restart")
                    DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
                    onClicked: {
                        restartDialog.close();
                        Qt.callLater(qmlUtils.restartApplication);
                    }
                }
            }
        }

        ComboBox {
            id: comboBoxLanguage
            anchors.left: labelLanguage.right
            anchors.leftMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 8
            model: ListModel {
                id: model
                ListElement { text: "English"; }
                ListElement { text: "Русский"; }
            }

            property bool enableForEditing: false
            Component.onCompleted: {
                if (i18n.language == "ru")
                    currentIndex = 1;
                else
                    currentIndex = 0;
                enableForEditing = true;
            }

            onCurrentIndexChanged: {
                if (!enableForEditing)
                {
                    return;
                }

                if (currentIndex == 0)
                    i18n.setLanguage("C");
                if (currentIndex == 1)
                    i18n.setLanguage("ru");
            }
        }

        Image {
            id: imageLanguageFlag
            y: 12
            width: 40
            height: 40
            anchors.verticalCenter: comboBoxLanguage.verticalCenter
            anchors.left: comboBoxLanguage.right
            anchors.leftMargin: 6
            mipmap: true
            fillMode: Image.PreserveAspectFit
            source: {
                if (comboBoxLanguage.currentText.toLowerCase() === "english")
                    return "qrc:/resources/images/flags/usa.svg";
                if (comboBoxLanguage.currentText.toLowerCase() === "русский")
                    return "qrc:/resources/images/flags/russia.svg";
            }
        }

        Switch {
            id: switchClearMessagesOnLinkChange
            x: 10
            y: 58
            text: qsTr("Clear Messages on Link Change")

            Component.onCompleted: {
                checked = chatHandler.enabledClearMessagesOnLinkChange;
            }

            onCheckedChanged: {
                chatHandler.enabledClearMessagesOnLinkChange = checked;
            }
        }

        Switch {
            id: switchEnabledHardwareGraphicsAccelerator
            x: 8
            y: 112
            text: qsTr("Enabled Hardware Graphics Accelerator")

            Component.onCompleted: {
                checked = qmlUtils.enabledHardwareGraphicsAccelerator;
            }

            onCheckedChanged: {
                qmlUtils.enabledHardwareGraphicsAccelerator = checked;
            }

            onClicked: {
                restartDialog.open();
            }
        }

        Switch {
            id: switchEnableSoundNewMessage
            x: 8
            y: 166
            text: qsTr("Enable Sound when New Message Received")

            Component.onCompleted: {
                checked = chatHandler.enabledSoundNewMessage;
            }

            onCheckedChanged: {
                chatHandler.enabledSoundNewMessage = checked;
            }

            onClicked: {
                if (checked)
                {
                    chatHandler.playNewMessageSound();
                }
            }
        }

        Label {
            id: labelLanguage
            y: 24
            text: qsTr("Language")
            anchors.verticalCenter: comboBoxLanguage.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
        }

        Switch {
            id: switchEnableProxy
            x: 10
            y: 220
            text: qsTr("Proxy (SOCKS5)")

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
            placeholderText: qsTr("Host name...")

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
            placeholderText: qsTr("Port...")
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
    }
}



/*##^##
Designer {
    D{i:0;formeditorZoom:0.8999999761581421}
}
##^##*/
