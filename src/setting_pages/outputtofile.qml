import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.2
import "../my_components" as MyComponents
import "../"

ScrollView {
    id: root
    clip: true
    padding: 6
    contentHeight: column.implicitHeight
    contentWidth: column.implicitWidth

    function urlToFilename(url) { // ToDo: is running too slowly
        var path = url.toString();
        // remove prefixed "file:///"
        path = path.replace(/^(file:\/{3})/,"");
        // unescape html codes like '%23' for '#'
        return decodeURIComponent(path);
    }

    Dialog {
        id: dialogMain
        title: qsTr("Export to txt");
        standardButtons: Dialog.Ok

        contentItem: Rectangle {
            color: "lightskyblue"
            implicitWidth: 400
            implicitHeight: 100
            Text {
                text: qsTr("Failed to save!")
                color: Material.foreground
                anchors.centerIn: parent

            }
        }
    }

    Column {
        id: column
        spacing: 6

        Row {
            Switch {
                text: qsTr("Enable Output to ini-file")

                Component.onCompleted: {
                    checked = outputToFile.enabled;
                }

                onCheckedChanged: {
                    outputToFile.enabled = checked;
                }
            }
        }

        Row {
            Text {
                text: qsTr("Folder where to save:")
                font.bold: true
                font.pixelSize: 20
                color: Material.accent
            }
        }

        Row {
            spacing: 6
            MyComponents.MyTextField {
                id: textFieldFolder
                width: 400
                placeholderText: qsTr("Folder where to save...")
                selectByMouse: true

                Component.onCompleted: {
                    text = outputToFile.outputFolderPath;
                }

                onTextChanged: {
                    outputToFile.outputFolderPath = text;
                }
            }

            Button {
                text: qsTr("Find")

                onClicked: {
                    fileDialogOutputToFilePath.open();
                }

                FileDialog {
                    id: fileDialogOutputToFilePath

                    selectFolder: true
                    selectMultiple: false
                    selectExisting: true

                    title: qsTr("Please select a folder")
                    folder: "file:///" + textFieldFolder.text
                    onAccepted: {
                        textFieldFolder.text = urlToFilename(fileDialogOutputToFilePath.folder);
                    }
                }
            }
        }

        Row {
            spacing: 20

            Button {
                text: qsTr("Show in Explorer")

                onClicked: {
                    outputToFile.showInExplorer();
                }
            }

            Button {
                text: qsTr("Reset Path")

                Material.foreground : "white"
                Material.background :  "red"

                onClicked: {
                    textFieldFolder.text = outputToFile.standardOutputFolder;
                }
            }
        }

        Row {
            spacing: 6

            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Codec:")
            }

            ComboBox {
                id: comboBoxCodecs

                property bool enableForEditing: false

                width: 250
                valueRole: "value"
                textRole: "text"
                model: ListModel {
                    id: comboBoxCodecsModel
                    ListElement { text: "UTF-8"; value: 0}
                    ListElement { text: "ANSI"; value: 100}
                    ListElement { text: qsTr("ANSI and UTF-8 numbers"); value: 200}
                }

                Component.onCompleted: {
                    var option = outputToFile.codecOption();
                    switch (option)
                    {
                    case 0:
                        currentIndex = 0;
                        break;
                    case 100:
                        currentIndex = 1;
                        break;
                    case 200:
                        currentIndex = 2;
                        break;
                    default:
                        console.log("unknown codec option ", option)
                    }
                }

                onActivated: {
                    outputToFile.setCodecOption(currentValue, false);
                    Global.windowSettings.showRestartDialog()
                }


            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:480}
}
##^##*/
