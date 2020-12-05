import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import "../my_components" as MyComponents

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 480

    function urlToFilename(url) {
        var path = url.toString();
        // remove prefixed "file:///"
        path = path.replace(/^(file:\/{3})/,"");
        // unescape html codes like '%23' for '#'
        return decodeURIComponent(path);
    }

    Item {
        id: element
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Switch {
            id: switchEnable
            y: 8
            height: 48
            text: qsTr("Enable Output to File")
            anchors.left: parent.left
            anchors.right: comboBoxLanguage.left
            anchors.rightMargin: 8
            anchors.leftMargin: 8

            Component.onCompleted: {
                checked = outputToFile.enabled;
            }

            onCheckedChanged: {
                outputToFile.enabled = checked;
            }
        }

        Text {
            id: element1
            y: 62
            height: 26
            text: qsTr("Folder where to save:")
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
            font.pixelSize: 20
        }

        MyComponents.MyTextField {
            id: textFieldFolder
            y: 94
            height: 43
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
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
            id: buttonFind
            x: 568
            y: 143
            text: qsTr("Find")
            anchors.right: parent.right
            anchors.rightMargin: 8

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

        Button {
            id: buttonReset
            x: 358
            y: 143
            text: qsTr("Reset")
            anchors.right: buttonFind.left
            anchors.rightMargin: 6

            onClicked: {
                textFieldFolder.text = outputToFile.standardOutputFolder;
            }
        }

        Button {
            id: buttonShowInExplorer
            x: 8
            y: 143
            text: qsTr("Show in Explorer")

            onClicked: {
                outputToFile.showInExplorer();
            }
        }

        ComboBox {
            id: comboBoxCodecs
            anchors.left: element2.right
            anchors.leftMargin: 8
            anchors.top: buttonShowInExplorer.bottom
            anchors.topMargin: 6
            model: ListModel {
                id: model
            }

            Component.onCompleted: {
                var currentCodec = outputToFile.codec();
                //console.log(currentCodec);

                var codecs = outputToFile.codecs();
                var i;
                for (i = 0; i < codecs.length; i++)
                {
                    model.append({text: codecs[i]});

                    if (currentCodec === codecs[i])
                    {
                        currentIndex = i;
                    }
                }
            }

            onCurrentValueChanged: {
                outputToFile.setCodec(currentText);
            }

            property bool enableForEditing: false
            width: 250

            //x: 352
            /*Component.onCompleted: {
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
            }*/
        }

        Text {
            id: element2
            y: 208
            height: 26
            text: qsTr("Codec:")
            anchors.left: parent.left
            //anchors.right: parent.right
            font.pixelSize: 20
            anchors.rightMargin: 383
            anchors.leftMargin: 8
        }

    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:480}D{i:2}D{i:3}D{i:4}D{i:9}
}
##^##*/
