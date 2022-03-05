import QtQuick 2.0
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import "my_components" as MyComponents
import "."

Window {
    id: rootWindow
    title: qsTr("Font Settings")
    width: 500
    height: 300
    minimumWidth: 50
    minimumHeight: 50

    Material.theme: Material.Dark
    Material.accent: "#03A9F4"
    Material.foreground: "#FFFFFF"
    color: "#202225"

    flags: {
        var windowFlags = Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint |
                Qt.WindowCloseButtonHint

        return windowFlags
    }

    property string colorDialogTarget: ""

    ColorDialog {
        id: colorDialog
        title: qsTr("Text Color")
        showAlphaChannel: true
        onVisibleChanged: {
            if (visible) {
                if (colorDialogTarget === "text") {
                    color = exampleText.color
                }
                else if (colorDialogTarget === "style") {
                    color = exampleText.styleColor
                }
            }
        }

        onAccepted: {
            if (colorDialogTarget === "text") {
                exampleText.color = color
            }
            else if (colorDialogTarget === "style") {
                exampleText.styleColor = color
            }
        }
    }

    Row {
        id: headerRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        /*Label {
            id: labelExample
            text: qsTr("Label")
        }*/

        //MyComponents.MyTextArea
        Label
        {
            id: exampleText
            width: parent.width
            text: "Это пример текста.\nThis is an example text\nこれはサンプルテキストです\n1234567890.,!"
            style: Text.Outline
        }
    }

    ScrollView {
        id: rootScrollView
        clip: true
        padding: 6
        contentHeight: rootColumn.implicitHeight
        contentWidth: rootColumn.implicitWidth
        ScrollBar.horizontal.policy: ScrollBar.AsNeeded
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: headerRow.bottom

        Column {
            id: rootColumn

            Row {
                spacing: 6

                Label {
                    text: qsTr("Font")
                    anchors.verticalCenter: parent.verticalCenter
                }

                ComboBox {
                    width: 250
                    anchors.verticalCenter: parent.verticalCenter
                    model: Qt.fontFamilies()

                    onActivated: {
                        exampleText.font.family = currentValue
                    }
                }
            }

            MyComponents.SliderRow {
                name: qsTr("Font Size")
                from: 6
                to: 50
                stepSize: 0.5
                defaultValue: 12

                value: exampleText.font.pointSize
                onValueChanged: {
                    exampleText.font.pointSize = value
                }
            }

            Row {
                spacing: 6

                Button {
                    text: qsTr("Text Color")
                    onClicked: {
                        colorDialogTarget = "text"
                        colorDialog.open()
                    }
                }

                Button {
                    text: qsTr("Style Color")
                    onClicked: {
                        colorDialogTarget = "style"
                        colorDialog.open()
                    }
                }
            }

            CheckBox {
                text: qsTr("Bold")
                onCheckStateChanged: {
                    exampleText.font.bold = checked
                }
            }

            CheckBox {
                text: qsTr("Italic")
                onCheckStateChanged: {
                    exampleText.font.italic = checked
                }
            }

            CheckBox {
                text: qsTr("Underline")
                onCheckStateChanged: {
                    exampleText.font.underline = checked
                }
            }
        }
    }
}
