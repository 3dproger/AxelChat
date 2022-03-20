import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Controls.Material 2.12
import "my_components" as MyComponents

Window {
    id: root

    property bool showSkipVersionCheckBox: true

    flags: Qt.Dialog |
           Qt.CustomizeWindowHint |
           Qt.WindowTitleHint |
           Qt.WindowCloseButtonHint

    width: 500
    height: 520
    minimumWidth: 500
    minimumHeight: 400

    Material.theme: Material.Dark
    Material.accent: "#03A9F4"
    Material.foreground: "#FFFFFF"
    color: "#202225"

    onVisibleChanged: {
        if (visible)
        {
            checkBoxSkipVersion.checked = false
        }
    }

    Button {
        id: buttonDownload
        x: 539
        y: 424
        text: qsTr("Download")
        icon.source: "qrc:/resources/images/download-button.svg"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8
        autoExclusive: true
        highlighted: true

        onClicked: {
            Qt.openUrlExternally(updateChecker.lastVersionDownloadUrl)
        }
    }

    Button {
        id: buttonDetailsBrowser
        x: 421
        y: 424
        text: qsTr("More Details")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        anchors.right: buttonDownload.left
        anchors.rightMargin: 6

        onClicked: {
            Qt.openUrlExternally(updateChecker.lastVersionHtmlUrl)
        }
    }

    CheckBox {
        id: checkBoxSkipVersion
        visible: showSkipVersionCheckBox
        x: 8
        y: 424
        text: qsTr("Skip This Version")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8

        onClicked: {
            updateChecker.setSkipCurrentVersion(checked);
        }
    }

    Label {
        id: elementHeading
        text: qsTr("New Version Available %1").arg(updateChecker.lastVersionName)
        color: Material.accent
        anchors.right: parent.right
        anchors.rightMargin: 163
        anchors.left: parent.left
        anchors.leftMargin: 164
        anchors.top: parent.top
        anchors.topMargin: 10
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 22
    }

    Label {
        id: labelVersion
        y: 43
        height: 29
        text: qsTr("Current version: %1").arg(Qt.application.version)
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 12
        opacity: 0.75
    }

    ScrollView {
        id: scrollView
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 62
        anchors.top: parent.top
        anchors.topMargin: 78

        MyComponents.MyTextArea {
            id: description
            text: {
                return qsTr("Published: %1")
                    .arg(updateChecker.lastVersionDate.toLocaleDateString(Qt.locale())) + "\n\n" +
                updateChecker.lastVersionDescription;
            }
            readOnly: true
            selectByMouse: true
            selectByKeyboard: true
            font.pointSize: 14
            wrapMode: Text.WordWrap
            textFormat: Text.MarkdownText

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                hoverEnabled: true
                cursorShape: Qt.IBeamCursor
            }
        }
    }
}



/*##^##
Designer {
    D{i:4;anchors_x:164;anchors_y:6}D{i:5;anchors_width:484;anchors_x:8}
}
##^##*/
