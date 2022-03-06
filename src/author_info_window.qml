import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.1
import QtQuick.Controls.Material 2.12
import "my_components" as MyComponents
import AxelChat.MessageAuthor 1.0
import AxelChat.ChatHandler 1.0
import AxelChat.ChatMessage 1.0

Window {
    id: rootWindow
    title: qsTr("Participant Information")

    Material.theme: Material.Dark
    Material.accent :     "#03A9F4"
    Material.background : "black"
    //Material.elevation :  "#03A9F4"
    Material.foreground : "#03A9F4"
    Material.primary :    "#03A9F4"

    color: Material.background

    property var messageType;
    property string authorName: "";
    property var authorChannelId;
    property var authorAvatarUrl;
    property var authorPageUrl;
    property var authorCustomBadgeUrl;
    property bool authorChatModerator: false;
    property bool authorIsChatOwner:   false;
    property bool authorChatSponsor:   false;
    property bool authorIsVerified:    false;


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
    minimumWidth:  480
    minimumHeight: 256
    maximumHeight: 256

    ScrollView {
        id: rootScrollView
        contentWidth:  480
        contentHeight: 256
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
                rounded: false
                height: 240
                width:  height
                mipmap: true
                source: {
                    if (messageType === ChatMessage.YouTube)
                    {
                        return typeof(rootWindow.authorAvatarUrl) == "object" ? youTube.createResizedAvatarUrl(rootWindow.authorAvatarUrl, height) : ""
                    }

                    return rootWindow.authorAvatarUrl;
                }
            }

            MyComponents.MyTextField {
                id: labelAuthorName
                width: 400
                anchors.left: avatarImage.right
                anchors.leftMargin: 6
                anchors.top: avatarImage.top
                anchors.rightMargin: 6
                anchors.right: parent.right
                text: typeof(rootWindow.authorName) == "string" ? rootWindow.authorName : ""
                color: {
                    /*if (authorIsChatOwner)
                    {
                        return "#FFD90F";
                    }
                    else if (authorIsVerified)
                    {
                        return "#F48FB1";
                    }
                    else if (authorChatModerator)
                    {
                        return "#5F84F1";
                    }
                    else if (authorChatSponsor)
                    {
                        return "#107516";
                    }*/

                    return "#03A9F4";
                }
                font.bold: true
                selectByMouse: true
                readOnly: true
                wrapMode: Text.Wrap
            }

            Label {
                id: labelAuthorType
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

                    if (authorIsVerified)
                    {
                        if (typeName.length > 0) { typeName += ", "; }
                        typeName += qsTr("Verified Account");
                    }

                    return typeName;
                }
                anchors.left: avatarImage.right
                anchors.top: labelAuthorName.bottom
                anchors.topMargin: 8
                anchors.leftMargin: 6

            }

            Label {
                id: labelMessagesSent
                text: qsTr("Messages (current session): %1")
                    .arg(chatHandler.authorMessagesSentCurrent(authorChannelId))
                anchors.left: avatarImage.right
                anchors.top: labelAuthorType.bottom
                anchors.topMargin: 6
                anchors.leftMargin: 6

                Connections {
                    target: chatHandler
                    function onMessageReceived() {
                        labelMessagesSent.text = qsTr("Messages (current session): %1")
                        .arg(chatHandler.authorMessagesSentCurrent(authorChannelId));
                    }
                }
            }

            Button {
                id: buttonAvatar
                text: qsTr("Open Image")
                anchors.top: labelMessagesSent.bottom
                anchors.topMargin: 12
                anchors.left: avatarImage.right
                anchors.leftMargin: 6
                icon.source: "qrc:/resources/images/forward-arrow.svg"
                highlighted: false
                flat: true

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                }

                onClicked: {
                    if (typeof(rootWindow.authorPageUrl) == "object")
                    {
                        Qt.openUrlExternally(youTube.createResizedAvatarUrl(rootWindow.authorAvatarUrl, 720))
                    }
                }
            }

            Button {
                id: buttonChannel
                text: qsTr("Go To Channel")
                anchors.top: buttonAvatar.bottom
                anchors.topMargin: 0
                anchors.left: avatarImage.right
                anchors.leftMargin: 6
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
        }
    }
}

/*##^##
Designer {
    D{i:8}D{i:9}
}
##^##*/
