import QtQuick 2.0
import QtQuick.Controls 2.15
import "../my_components" as MyComponents
import "../"

ScrollView {
    id: root
    clip: true
    padding: 6
    contentHeight: rootColumn.implicitHeight
    contentWidth: rootColumn.implicitWidth
    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn

    property var fontWindow;

    function openFontWindow() {
        if (typeof(fontWindow) == "undefined")
        {
            var component = Qt.createComponent("qrc:/font_settings.qml")
            fontWindow = component.createObject(root)
        }

        fontWindow.show()
    }

    Column {
        id: rootColumn

        Switch {
            text: qsTr("System Window Frame")

            Component.onCompleted: {
                checked = Global.windowChatSystemWindowFrame
            }

            onCheckedChanged: {
                Global.windowChatSystemWindowFrame = checked
                Global.save()
            }
        }

        MyComponents.SliderRow {
            name: qsTr("Background Opacity")
            from: 0
            to: 1
            stepSize: 0.01
            defaultValue: 0.40
            valueToPercent: true

            value: Global.windowChatBackgroundOpacity
            onValueChanged: {
                Global.windowChatBackgroundOpacity = value
                Global.save()
                Global.windowChat.refreshByFlags()
            }
        }

        MyComponents.SliderRow {
            name: qsTr("Window Opacity")
            from: 0.1
            to: 1
            stepSize: 0.01
            defaultValue: 1
            valueToPercent: true

            value: Global.windowChatWindowOpacity
            onValueChanged: {
                Global.windowChatWindowOpacity = value
                Global.save()
                Global.windowChat.refreshByFlags()
            }
        }

        Switch {
            text: qsTr("Show Viewers Count")

            Component.onCompleted: {
                checked = Global.windowChatShowViewersCount
            }
            onCheckedChanged: {
                Global.windowChatShowViewersCount = checked
                Global.save()
            }
        }

        Switch {
            text: qsTr("Show Avatars")

            Component.onCompleted: {
                checked = Global.windowChatMessageShowAvatar
            }
            onCheckedChanged: {
                Global.windowChatMessageShowAvatar = checked
                Global.save()
            }
        }

        Switch {
            text: qsTr("Show Message Author Name")

            Component.onCompleted: {
                checked = Global.windowChatMessageShowAuthorName
            }
            onCheckedChanged: {
                Global.windowChatMessageShowAuthorName = checked
                Global.save()
            }
        }

        Switch {
            text: qsTr("Show Platform Icon")

            Component.onCompleted: {
                checked = Global.windowChatMessageShowPlatformIcon
            }
            onCheckedChanged: {
                Global.windowChatMessageShowPlatformIcon = checked
                Global.save()
            }
        }

        Switch {
            text: qsTr("Show Message Time")

            Component.onCompleted: {
                checked = Global.windowChatMessageShowTime
            }
            onCheckedChanged: {
                Global.windowChatMessageShowTime = checked
                Global.save()
            }
        }

        MyComponents.SliderRow {
            name: qsTr("Avatar Size")
            from: 10
            to: 300
            stepSize: 1
            defaultValue: 40

            value: Global.windowChatMessageAvatarSize
            onValueChanged: {
                Global.windowChatMessageAvatarSize = value
                Global.save()
            }
        }

        MyComponents.SliderRow {
            name: qsTr("Font Size (Author Name)")
            from: 6
            to: 40
            stepSize: 0.5
            defaultValue: 12

            value: Global.windowChatMessageAuthorNameFontSize
            onValueChanged: {
                Global.windowChatMessageAuthorNameFontSize = value
                Global.save()
            }
        }

        MyComponents.SliderRow {
            name: qsTr("Font Size (Message Text)")
            from: 6
            to: 40
            stepSize: 0.5
            defaultValue: 12

            value: Global.windowChatMessageTextFontSize
            onValueChanged: {
                Global.windowChatMessageTextFontSize = value
                Global.save()
            }
        }

        MyComponents.SliderRow {
            name: qsTr("Font Size (Message Time)")
            from: 6
            to: 40
            stepSize: 0.5
            defaultValue: 12

            value: Global.windowChatMessageTimeFontSize
            onValueChanged: {
                Global.windowChatMessageTimeFontSize = value
                Global.save()
            }
        }

        /*Button {
            text: "Font"
            onClicked: {
                openFontWindow()
            }
        }*/
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
