import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
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
            color: Material.foreground
        }

        Text {
            id: textApplicationName
            x: 256
            y: 84
            height: 29
            text: Qt.application.name
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 24
            color: Material.foreground
        }

        Text {
            id: textBuildInfo
            x: 309
            y: 119
            text:  {
                if (qmlUtils.buildCpuArchitecture === "i386")
                    return qsTr("32-bit Build");
                else if (qmlUtils.buildCpuArchitecture === "x86_64")
                    return qsTr("64-bit Build");
                else
                    return qsTr("Build Architecture: %1").arg(qmlUtils.buildCpuArchitecture);
            }
            font.pixelSize: 12
            anchors.horizontalCenter: textApplicationName.horizontalCenter
            color: Material.foreground
        }

        Text {
            id: element
            y: 352
            height: 24
            text: qsTr("Alexander 'Axel_k' Kirsanov %1").arg("© 2020-2022")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 20
            color: Material.foreground
        }

        Button {
            id: buttonYouTubeChannel
            x: 196
            y: 382
            width: 226
            height: 48
            text: qsTr("YouTube Channel")
            anchors.horizontalCenterOffset: 0
            icon.source: "qrc:/resources/images/youtube-black.svg"
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
            y: 445
            textFormat: Text.RichText
            text: "Some icons made by <a href=\"http://www.freepik.com/\" title=\"Freepik\">Freepik</a> and <a href=\"https://www.flaticon.com/authors/google\" title=\"Google\">Google</a> from <a href=\"https://www.flaticon.com/\" title=\"Flaticon\"> www.flaticon.com</a> and <a href=\"https://icons8.com\">https://icons8.com</a>"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 21
            font.pixelSize: 12

            onLinkActivated: {
                Qt.openUrlExternally(link)
            }

            color: Material.foreground
        }

        Button {
            id: buttonCheckUpdates
            x: 482
            y: 8
            text: qsTr("Check for Updates")
            anchors.right: parent.right
            highlighted: true
            anchors.rightMargin: 8
            icon.source: "qrc:/resources/images/refresh.svg"

            onClicked: {
                updateChecker.checkForNewVersion();
            }
        }

        Text {
            id: element1
            x: 109
            text: "(⌒ω⌒)ﾉ"
            anchors.right: buttonYouTubeChannel.left
            anchors.top: buttonYouTubeChannel.bottom
            anchors.rightMargin: 6
            font.pixelSize: 20
            anchors.topMargin: -38
            color: Material.foreground
        }

        Text {
            id: element3
            x: 380
            y: 20
            text: "(づ ◕‿◕ )づ"
            anchors.right: buttonCheckUpdates.left
            anchors.rightMargin: 6
            font.pixelSize: 20
            color: Material.foreground
        }

        Text {
            id: element4
            x: 114
            y: 390
            text: qsTr("subscribe")
            anchors.horizontalCenterOffset: -5
            anchors.horizontalCenter: element1.horizontalCenter
            anchors.bottom: element1.top
            font.pixelSize: 12
            color: Material.foreground
        }

        /*Button {
            id: buttonDiscussions
            x: 10
            y: 62
            text: qsTr("Forum (Discussions)")
            icon.source: "qrc:/resources/images/feedback.svg"

            //Material.background: "#ffffff"
            //Material.foreground: "#c51a4a"

            //Material.background: "#003760"
            //Material.foreground: "#03A9F4"

            //Material.background: "#03A9F4"
            //Material.foreground: "#FFFFFF"

            onClicked: {
                Qt.openUrlExternally("https://github.com/3dproger/AxelChat/discussions");
            }
        }*/

        Button {
            id: buttonReleases
            x: 521
            y: 62
            text: qsTr("Releases")
            anchors.right: parent.right
            anchors.rightMargin: 8
            icon.source: "qrc:/resources/images/new-release.svg"

            onClicked: {
                Qt.openUrlExternally("https://github.com/3dproger/AxelChat/releases");
            }
        }

        Button {
            id: buttonSource
            x: 10
            y: 116
            text: qsTr("Source Code")
            icon.source: "qrc:/resources/images/github.svg"

            onClicked: {
                Qt.openUrlExternally("https://github.com/3dproger/AxelChat");
            }
        }

        Text {
            id: element6
            y: 270
            height: 66
            text: qsTr("AxelChat is a free and cross-platform app for adding interactivity to live-streaming and interacting with viewers simultaneously for multiple live-streaming platforms")
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: 17
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            anchors.rightMargin: 8
            anchors.leftMargin: 8
            color: Material.foreground
        }

        Button {
            id: buttonGitHub
            y: 380
            text: qsTr("GitHub")
            anchors.verticalCenter: buttonYouTubeChannel.verticalCenter
            anchors.left: buttonYouTubeChannel.right
            anchors.leftMargin: 6
            icon.source: "qrc:/resources/images/github.svg"

            onClicked: {
                Qt.openUrlExternally("https://github.com/3dproger");
            }
        }

        /*Button {
            id: buttonLicense
            x: 10
            y: 170
            text: qsTr("License")
            //icon.source: "qrc:/resources/images/feedback.svg"

            onClicked: {
                Qt.openUrlExternally("https://github.com/3dproger/AxelChat/blob/master/LICENSE");
            }
        }*/

        Button {
            id: buttonPatreon
            x: 8
            y: 8
            text: qsTr("Support the developer")
            icon.source: "qrc:/resources/images/heart.svg"
            Material.background: "#DB61A2"

            onClicked: {
                Qt.openUrlExternally("https://patreon.com/axel_k");
            }
        }

        Button {
            id: buttonIssues
            x: 8
            y: 62
            text: qsTr("Complaints and suggestions (Issues)")
            icon.source: "qrc:/resources/images/feedback.svg"

            onClicked: {
                Qt.openUrlExternally("https://github.com/3dproger/AxelChat/issues");
            }
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
                noNewVersionsDialog.title = qsTr("You are using the most current version!\n\nCurrent version: %1\nLatest version available: %2")
                    .arg(Qt.application.version)
                    .arg(updateChecker.lastVersionName);
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
