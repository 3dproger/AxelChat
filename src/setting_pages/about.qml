import QtQuick 2.15
import QtQuick.Controls 2.15
import AxelChat.UpdateChecker 1.0
import AxelChat.QMLUtils 1.0

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 640
    Item {
        id: element5
        width:  Math.max(root.width,  root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Dialog {
            id: noNewVersionsDialog
            anchors.centerIn: parent
            modal: true
            standardButtons: Dialog.Ok
        }

        Text {
            id: textVersion
            y: 76
            text: Qt.application.version
            anchors.left: textApplicationName.right
            anchors.leftMargin: 6
            font.pixelSize: 20
        }

        Text {
            id: textApplicationName
            x: 256
            y: 84
            height: 29
            text: Qt.application.name
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 24
        }

        Text {
            id: textBuildInfo
            x: 309
            y: 119
            text:  {
                if (qmlUtils.buildCpuArchitecture == "i386")
                    return qsTr("32-bit Build");
                else if (qmlUtils.buildCpuArchitecture == "x86_64")
                    return qsTr("64-bit Build");
                else
                    return qsTr("Build Architecture: %1").arg(qmlUtils.buildCpuArchitecture);
            }
            font.pixelSize: 12
            anchors.horizontalCenter: textApplicationName.horizontalCenter
        }

        Text {
            id: element
            y: 278
            height: 24
            text: qsTr("Alexander 'Axel_k' Kirsanov (c) 2020")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 20
        }

        Button {
            id: buttonYouTubeChannel
            x: 196
            y: 308
            width: 249
            height: 48
            text: qsTr("YouTube Channel")
            anchors.horizontalCenter: parent.horizontalCenter

            onClicked: {
                Qt.openUrlExternally("https://www.youtube.com/channel/UCujkj8ZgVkscm34GA1Z8wTQ");
            }
        }

        Image {
            id: image
            x: 275
            y: 147
            width: 100
            height: 100
            anchors.horizontalCenter: parent.horizontalCenter
            mipmap: true
            source: "qrc:/resources/images/axelchat.svg"
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: element2
            x: 8
            textFormat: Text.RichText
            text: "Some icons made by <a href=\"http://www.freepik.com/\" title=\"Freepik\">Freepik</a> and <a href=\"https://www.flaticon.com/authors/google\" title=\"Google\">Google</a> from <a href=\"https://www.flaticon.com/\" title=\"Flaticon\"> www.flaticon.com</a> and <a href=\"https://icons8.com\">https://icons8.com</a>"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            font.pixelSize: 12

            onLinkActivated: {
                Qt.openUrlExternally(link)
            }
        }

        Button {
            id: buttonCheckUpdates
            x: 482
            y: 8
            text: qsTr("Check for Updates")
            anchors.right: parent.right
            anchors.rightMargin: 8

            onClicked: {
                updateChecker.checkForNewVersion();
            }
        }

        Text {
            id: element1
            x: 93
            y: 320
            text: "(⌒ω⌒)ﾉ"
            anchors.right: buttonYouTubeChannel.left
            anchors.rightMargin: 6
            font.pixelSize: 20
        }

        Text {
            id: element3
            x: 380
            y: 20
            text: "(づ ◕‿◕ )づ"
            anchors.right: buttonCheckUpdates.left
            anchors.rightMargin: 6
            font.pixelSize: 20
        }

        Text {
            id: element4
            x: 114
            y: 306
            text: qsTr("subscribe")
            anchors.horizontalCenterOffset: -5
            anchors.horizontalCenter: element1.horizontalCenter
            anchors.bottom: element1.top
            anchors.bottomMargin: 0
            font.pixelSize: 12
        }
    }

    Component.onCompleted: {
        var component = Qt.createComponent("qrc:/updatesnotification.qml");
        root.updatesWindow = component.createObject(root);
    }

    property var updatesWindow;

    Connections {
        target: updateChecker

        property bool showUpdateWindowIfNotNewVersionAvailable: false;

        function onReplied() {
            if (updateChecker.replyState === UpdateChecker.NewVersionAvailable
                    || updateChecker.replyState === UpdateChecker.NewVersionAvailableButSkipped)
            {
                root.updatesWindow.showSkipVersionCheckBox = false;
                root.updatesWindow.show();
            }
            else if (updateChecker.replyState === UpdateChecker.LatestVersionYouHave)
            {
                noNewVersionsDialog.title = qsTr("You are using the most current version!");
                noNewVersionsDialog.open();

            }
            else if (updateChecker.replyState === UpdateChecker.FailedToConnect)
            {
                noNewVersionsDialog.title = qsTr("Failed to connect to server. Check your internet connection!");
                noNewVersionsDialog.open();
            }
            else if (updateChecker.replyState === UpdateChecker.RequestLimitsExceeded)
            {
                noNewVersionsDialog.title = qsTr("Request limits exceeded! Please try again later");
                noNewVersionsDialog.open();
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:1.100000023841858;height:480;width:640}
}
##^##*/
