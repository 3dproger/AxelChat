import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import QtQuick.Window 2.15
import AxelChat.ChatManager 1.0
import AxelChat.OutputToFile 1.0
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import "my_components" as MyComponents
import "."

Window {
    id: root
    title: qsTr("Settings")
    flags: {
        var windowFlags = Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint |
                Qt.WindowCloseButtonHint

        return windowFlags
    }


    Material.theme: Material.Dark
    Material.accent: "#03A9F4"
    Material.foreground: "#FFFFFF"
    color: "#202225"

    Component.onCompleted: {
        Global.windowSettings = this
    }

    Settings {
        id: settings
        category: "settings_window"
        property alias window_width:  root.width;
        property alias window_height: root.height;
        property alias category_index: listViewCategories.currentIndex
    }

    function showRestartDialog() {
        restartDialog.open()
    }

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

    function showInfo(text) {
        infoDialog.infoText = text
        infoDialog.open()
    }

    Dialog {
        id: infoDialog
        x: parent.width / 2 - width / 2
        y: parent.height / 2 - height / 2
        margins: 12
        width: parent.width * 0.75
        property string infoText: "info"
        modal: true
        header: Label {
            width: infoDialog.width - 24
            padding: 12
            wrapMode: Text.WordWrap
            text: infoDialog.infoText
        }

        footer: DialogButtonBox {
            Button {
                flat: true
                text: qsTr("Ok")
                DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                onClicked: {
                    infoDialog.close();
                }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            qmlUtils.updateWindowStyle(this)
        }
        else {
            commandsEditor.close();
        }
    }

    width: 880
    height: 500
    minimumHeight: 300
    minimumWidth:  550

    function urlToFilename(url) {
        var path = url.toString();
        // remove prefixed "file:///"
        path = path.replace(/^(file:\/{3})/,"");
        // unescape html codes like '%23' for '#'
        return decodeURIComponent(path);
    }

    FocusScope {
        id: categories
        width: 150
        height: parent.height
        Layout.minimumWidth: 124
        Layout.preferredWidth: parent.width / 3
        Layout.maximumWidth: 330
        Layout.fillWidth: true
        Layout.fillHeight: true
        focus: true
        activeFocusOnTab: true
        signal recipeSelected(url url)

        ColumnLayout {
            spacing: 0
            anchors.fill: parent

            ListView {
                id: listViewCategories
                Layout.fillWidth: true
                Layout.fillHeight: true
                keyNavigationWraps: true
                clip: true
                focus: true
                ScrollBar.vertical: ScrollBar {
                    interactive: false
                    policy: ScrollBar.AlwaysOn
                }

                Component.onCompleted: {
                    currentIndex = settings.category_index
                }

                model: ListModel {
                    ListElement {
                        name: qsTr("Common")
                        category: "common"
                        iconSource: ""
                    }
                    ListElement {
                        name: qsTr("Widgets")
                        category: "widgets"
                        iconSource: ""
                    }
                    ListElement {
                        name: qsTr("Appearance")
                        category: "appearance"
                        iconSource: ""
                    }
                    ListElement {
                        name: qsTr("Chat Commands")
                        category: "chat_commands"
                        iconSource: ""
                    }
                    ListElement {
                        name: qsTr("Output to Files")
                        category: "output_to_files"
                        iconSource: ""
                    }
                    ListElement {
                        name: qsTr("About AxelChat")
                        category: "about_software"
                        iconSource: ""
                    }
                }

                delegate: ItemDelegate {
                    id: categoryDelegate
                    width: listViewCategories.width
                    text: model.name
                    property var iconSource: model.iconSource
                    property string category: model.category
                    highlighted: ListView.isCurrentItem

                    onClicked: {
                        listViewCategories.forceActiveFocus();
                        listViewCategories.currentIndex = model.index;
                    }

                    contentItem: Row {
                        anchors.fill: parent

                        spacing: 8
                        leftPadding: 8

                        Rectangle {
                            visible: categoryDelegate.highlighted
                            width: 4
                            color: categoryDelegate.Material.accentColor
                        }

                        Image {
                            mipmap: true
                            height: 24
                            width: height
                            anchors.verticalCenter: parent.verticalCenter
                            source: categoryDelegate.iconSource
                            fillMode: Image.PreserveAspectFit
                            visible: categoryDelegate.iconSource.length > 0
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter

                            text: categoryDelegate.text
                            font: categoryDelegate.font
                            color: categoryDelegate.enabled ? categoryDelegate.Material.primaryTextColor
                                                  : categoryDelegate.Material.hintTextColor
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.Wrap
                        }
                    }
                }

                onCurrentItemChanged: {
                    if (!currentItem) {
                        return
                    }

                    if (currentItem.category === "common")
                    {
                        stackViewCategories.replace("setting_pages/common.qml");
                    }
                    else if (currentItem.category === "widgets")
                    {
                        stackViewCategories.replace("setting_pages/widgets.qml");
                    }
                    else if (currentItem.category === "appearance")
                    {
                        stackViewCategories.replace("setting_pages/appearance.qml")
                    }
                    else if (currentItem.category === "members")
                    {
                        stackViewCategories.replace("setting_pages/authors.qml")
                    }
                    else if (currentItem.category === "output_to_files")
                    {
                        stackViewCategories.replace("setting_pages/outputtofile.qml");
                    }
                    else if (currentItem.category === "chat_commands")
                    {
                        stackViewCategories.replace("setting_pages/chatcommands.qml");
                    }
                    else if (currentItem.category === "about_software")
                    {
                        stackViewCategories.replace("setting_pages/about.qml");
                    }
                }
            }
        }
    }

    StackView{
        id: stackViewCategories
        x: categories.width
        width: parent.width - x
        height: parent.height
        initialItem: "setting_pages/common.qml"

        replaceExit: Transition {
            OpacityAnimator {
                from: 1;
                to: 0;
                duration: 200
            }
        }

        replaceEnter: Transition {
            OpacityAnimator {
                from: 0;
                to: 1;
                duration: 200
            }
        }
    }
}
