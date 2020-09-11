import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.1
import QtQuick.Controls.Material 2.12
import "my_components" as MyComponents
import AxelChat.MessageAuthor 1.0
import AxelChat.ChatHandler 1.0

Window {
    id: rootWindow
    title: qsTr("Participant Information")

    Material.theme: Material.Dark
    Material.accent :     "#03A9F4"
    Material.background : "black"
    //Material.elevation :  "#03A9F4"
    Material.foreground : "#03A9F4"
    Material.primary :    "#03A9F4"

    property var authorName: "";
    property var authorChannelId;
    property var authorAvatarUrl;
    property var authorPageUrl;
    property var authorCustomBadgeUrl;
    property var authorChatModerator: false;
    property var authorIsChatOwner:   false;
    property var authorChatSponsor:   false;
    property var authorIsVerified:    false;

    flags: Qt.Dialog |
           Qt.CustomizeWindowHint |
           Qt.WindowTitleHint |
           Qt.WindowCloseButtonHint //|
           //Qt.WindowMaximizeButtonHint

    Settings {
        category: "author_info_window"
        property alias window_width:  rootWindow.width;
        property alias window_height: rootWindow.height;
    }

    width: rootScrollView.contentWidth
    height: rootScrollView.contentHeight
    minimumWidth:  380
    minimumHeight: 240

    color: Material.background

    ScrollView {
        id: rootScrollView
        contentWidth:  380
        contentHeight: 240
        width: rootWindow.width
        height: rootWindow.height
        Item {
            id: root
            width:  Math.max(rootScrollView.width,  rootScrollView.contentWidth)
            height: Math.max(rootScrollView.height, rootScrollView.contentHeight)

            MyComponents.ImageRounded {
                id: avatarImage
                x: 8
                y: 8
                rounded: true
                height: 64
                width: 64
                mipmap: true
                source: typeof(rootWindow.authorAvatarUrl) == "object" ? rootWindow.authorAvatarUrl : ""
            }

            Button {
                id: buttonChannel
                x: 8
                y: 78
                text: qsTr("Go To Channel")
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 6
                anchors.right: parent.right
                anchors.rightMargin: 6
                icon.source: "qrc:/resources/images/forward-arrow.svg"
                highlighted: true
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                }

                onClicked: {
                    if (typeof(rootWindow.authorPageUrl) == "object")
                    {
                        Qt.openUrlExternally(rootWindow.authorPageUrl)
                    }
                }
            }

            MyComponents.MyTextField {
                id: labelAuthorName
                width: 400
                height: 40
                anchors.left: avatarImage.right
                anchors.leftMargin: 6
                anchors.top: avatarImage.top
                anchors.bottom: avatarImage.bottom
                anchors.rightMargin: 6
                anchors.right: parent.right
                text: typeof(rootWindow.authorName) == "string" ? rootWindow.authorName : ""
                color: {
                    if (authorIsChatOwner)
                    {
                        return "#FFD90F";
                    }
                    else if (authorChatSponsor)
                    {
                        return "#107516";
                    }
                    else if (authorChatModerator)
                    {
                        return "#5F84F1";
                    }
                    else if (authorIsVerified)
                    {
                        return "#F48FB1";
                    }
                    else
                    {
                        return "#03A9F4";
                    }
                }
                font.bold:
                    authorChatModerator |
                    authorIsChatOwner   |
                    authorChatSponsor   |
                    authorIsVerified
                selectByMouse: true
                readOnly: true
                wrapMode: Text.Wrap
            }

            Label {
                id: labelAuthorType
                x: 8
                y: 78
                text: {
                    var typeName = "";
                    if (authorIsChatOwner)
                    {
                        if (typeName.length > 0) { typeName += ", "; }
                        typeName += qsTr("Channel Author");
                    }
                    if (authorChatModerator)
                    {
                        if (typeName.length > 0) { typeName += ", "; }
                        typeName += qsTr("Moderator");
                    }
                    if (authorChatSponsor)
                    {
                        if (typeName.length > 0) { typeName += ", "; }
                        typeName += qsTr("Sponsor");
                    }
                    if (typeName === "")
                    {
                        typeName = qsTr("Regular Participant");
                    }
                    return typeName;
                }

            }

            Label {
                id: labelMessagesSent
                x: 8
                y: 101
                text: qsTr("Messages (Current Session): %1")
                    .arg(chatHandler.authorMessagesSentCurrent(authorChannelId))

                Connections {
                    target: chatHandler
                    function onMessagesReceived() {
                        labelMessagesSent.text = qsTr("Messages (Current Session): %1")
                        .arg(chatHandler.authorMessagesSentCurrent(authorChannelId));
                    }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:1.5}D{i:5}D{i:8}
}
##^##*/
