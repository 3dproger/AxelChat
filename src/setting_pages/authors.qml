import QtQuick 2.15
import QtQuick.Controls 2.15
import AxelChat.ChatMessage 1.0
import "../my_components" as MyComponents

Item {
    id: root

    ListView {
        id: listAuthors
        anchors {
            fill: parent;
            margins: 2
        }

        property bool needAutoScrollToBottom: false

        RoundButton {
            id: roundButtonScrollToBottom
            anchors.horizontalCenter: listAuthors.horizontalCenter
            y: 420
            text: "↓"
            opacity: 0.85

            state: "hiden"

            states: [
                State {
                    name: "hiden"
                    PropertyChanges {
                        y: listAuthors.height + 1;
                        target: roundButtonScrollToBottom;
                    }
                },
                State {
                    name: "shown"
                    PropertyChanges {
                        y: listAuthors.height - width - 20;
                        target: roundButtonScrollToBottom;
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation {
                    properties: "y";
                    easing.type: Easing.InOutQuad;
                    duration: 400;
                }
            }

            onClicked: {
                Qt.callLater(listAuthors.positionViewAtEnd);
                state = "hiden";
            }
        }

        ScrollBar.vertical: ScrollBar {}

        spacing: 2
        width: parent.width
        model: messagesModel
        delegate: authorDelegate
        /*delegate: Component {
            Loader { source: "message_delegate.qml" }
        }*/

        visibleArea.onHeightRatioChanged: {
            if (scrollbarOnBottom())
            {
                roundButtonScrollToBottom.state = "hiden"
            }
            else
            {
                roundButtonScrollToBottom.state = "shown"
            }
        }

        visibleArea.onYPositionChanged: {
            if (scrollbarOnBottom())
            {
                roundButtonScrollToBottom.state = "hiden"
            }
            else
            {
                roundButtonScrollToBottom.state = "shown"
            }
        }

        onCountChanged: {
            needAutoScrollToBottom = scrollbarOnBottom();
            Qt.callLater(listAuthors.autoScroll);
        }

        function autoScroll()
        {
            if (needAutoScrollToBottom)
            {
                listAuthors.positionViewAtEnd();
                needAutoScrollToBottom = false;
            }
        }

        function scrollbarOnBottom()
        {
            return visibleArea.yPosition * contentHeight + listAuthors.height + 40 >= contentHeight;
        }
    }

    Component {
        id: authorDelegate

        Rectangle {
            id: messageContent

            width: listMessages.width
            height: Math.max(textEditMessageText.y + textEditMessageText.height, 40)

            state: "hiden"

            Component.onCompleted: {
                state = "shown"
            }

            states: [
                State {
                    name: "hiden"
                    PropertyChanges {
                        x: listMessages.width;
                        target: messageContent;
                    }
                },
                State {
                    name: "shown"
                    PropertyChanges {
                        x: 0;
                        target: messageContent;
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation {
                    properties: "x";
                    easing.type: Easing.InOutQuad;
                    duration: 500;
                }
            }

            border.width: 1
            border.color: "#003760"
            color: {
                if (messageType === ChatMessage.SoftwareNotification ||
                    messageType === ChatMessage.TestMessage)
                {
                    return border.color
                }
                else
                {
                    return chromoKeyColor
                }
            }

            radius: 0

            Row {
                id: authorRow
                anchors.left:  avatarImage.right
                anchors.right: labelTime.visible ? labelTime.left : messageContent.right
                anchors.margins: 4
                spacing: 4

                //Author Name
                //TextEdit
                Label {
                    id: authorNameText
                    color: {
                        if (authorChatModerator)
                        {
                            return "#5F84F1";
                        }
                        else if (authorIsChatOwner)
                        {
                            return "#FFD90F";
                        }
                        /*else if (authorIsVerified)
                        {
                            return "#F48FB1";
                        }*/
                        else if (authorChatSponsor)
                        {
                            return "#107516";
                        }
                        else
                        {
                            return "#03A9F4";
                        }
                    }
                    font.bold:
                        authorChatModerator |
                        authorIsChatOwner |
                        authorChatSponsor |
                        messageType === ChatMessage.SoftwareNotification |
                        messageType === ChatMessage.TestMessage

                    /*selectByKeyboard: true
                    selectByMouse: true
                    readOnly: true*/
                    style: Text.Outline
                    styleColor: "black"

                    /*anchors.left: avatarImage.right
                    //anchors.right: labelTime.visible ? labelTime.left : messageContent.right

                    anchors.right: {
                        if (imageStandardBadge.source !== "")
                        {
                            return imageStandardBadge.left;
                        }
                        else
                        {
                            if (customBadgeImage.source !== "")
                            {
                                return customBadgeImage.left;
                            }
                            else
                            {
                                if (labelTime.visible)
                                {
                                    return labelTime.left;
                                }
                                else
                                {
                                    return messageContent.right;
                                }
                            }
                        }
                    }*/

                    //anchors.margins: 4
                    wrapMode: Text.Wrap
                    text: authorName
                }

                //Badge Standard
                Image {
                    id: imageStandardBadge
                    //x: authorNameText.x + authorNameText.width + 4
                    y: authorNameText.y
                    height: authorNameText.height
                    width: height

                    asynchronous: true
                    fillMode: Image.PreserveAspectFit
                    source: {
                        if (authorChatModerator)
                        {
                            return "qrc:/resources/images/tool.svg";
                        }
                        else if (authorIsChatOwner)
                        {
                            return "qrc:/resources/images/king.svg";
                        }
                        else
                        {
                            return "";
                        }
                    }
                }

                //Badge Custom
                MyComponents.ImageRounded {
                    id: customBadgeImage
                    rounded: false

                    height: authorNameText.height
                    width: height
                    /*x: {
                        if (imageStandardBadge.source.toString().length === 0)
                        {
                            return imageStandardBadge.x;
                        }
                        else
                        {
                            return imageStandardBadge.x + imageStandardBadge.width + 4;
                        }
                    }*/

                    y: authorNameText.y

                    asynchronous: true
                    source: authorCustomBadgeUrl
                }
            }

            //Time
            Label {
                id: labelTime
                visible: showTime
                color: "#039BE5"
                anchors.right: messageContent.right
                anchors.margins: 4
                text: messagePublishedAt.toLocaleTimeString(Qt.locale(), "hh:mm")
            }

            //Author Avatar
            MyComponents.ImageRounded {
                id: avatarImage

                rounded: messageType !== ChatMessage.SoftwareNotification &&
                         messageType !== ChatMessage.TestMessage

                height: 32
                width: 32
                mipmap: true

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.margins: 4

                asynchronous: true
                source: authorAvatarUrl
            }

            //Text Message
            TextEdit {
                id: textEditMessageText
                color: "white"
                anchors.left: avatarImage.right
                anchors.right: parent.right
                anchors.top: authorRow.bottom
                anchors.margins: 4
                wrapMode: Text.Wrap
                text: messageText
                font.weight: Font.DemiBold
                selectByMouse: true
                selectByKeyboard: true
                readOnly: true

                font.letterSpacing: 0.5
                font.pointSize: 11.5
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1;anchors_x:142}
}
##^##*/
