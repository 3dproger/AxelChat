import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.1
import AxelChat.ChatHandler 1.0
import AxelChat.UpdateChecker 1.0
import AxelChat.MessageAuthor 1.0
import AxelChat.ChatMessage 1.0
import QtQuick.Window 2.15
import "my_components" as MyComponents
import "setting_pages" as SettingPages

ApplicationWindow {
    id: root
    visible: true
    width: 300
    height: 480
    minimumHeight: 300
    minimumWidth:  250
    title: Qt.application.name

    property bool showTime: true

    property color chromoKeyColor: "#000000"//"#008800"//"#1B5F1E"

    property var settingsWindow;
    property var authorInfoWindow;
    property var updatesWindow;

    Settings {
        category: "chat_window"
        property alias window_width:  root.width;
        property alias window_height: root.height;
    }

    Component.onCompleted: {
        //Update notification window
        if (updateChecker.autoRequested)
        {
            var component = Qt.createComponent("qrc:/updatesnotification.qml");
            root.updatesWindow = component.createObject(root);
        }
    }



    Connections {
        target: updateChecker

        function onReplied() {
            if (updateChecker.replyState === UpdateChecker.NewVersionAvailable)
            {
                if (root.updatesWindow)
                {
                    root.updatesWindow.show()
                    root.updatesWindow = undefined;
                }
            }
        }
    }

    background: Rectangle {
        color: chromoKeyColor
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: {
            if (typeof(settingsWindow) == "undefined")
            {
                var component = Qt.createComponent("qrc:/settings.qml")
                settingsWindow = component.createObject(root)
            }

            settingsWindow.show()
        }

        onWheel: {
            listMessages.flick(wheel.angleDelta.x * 8, wheel.angleDelta.y * 8)
            wheel.accepted=true
        }
    }

    ListView {
        id: listMessages
        anchors {
            fill: parent;
            margins: 2
        }

        interactive: false

        property bool needAutoScrollToBottom: false

        Button {
            id: roundButtonScrollToBottom
            anchors.horizontalCenter: listMessages.horizontalCenter
            y: 420
            text: "↓"
            width: 38
            height: width
            opacity: down ? 0.8 : 1.0

            background: Rectangle {
                anchors.fill: parent
                color: "#03A9F4"
                radius: Math.min(width, height) / 2
            }

            MouseArea {
                function containsMouseRound() {
                    var x1 = width / 2;
                    var y1 = height / 2;
                    var x2 = mouseX;
                    var y2 = mouseY;
                    var distanceFromCenter = Math.pow(x1 - x2, 2) + Math.pow(y1 - y2, 2);
                    var radiusSquared = Math.pow(Math.min(width, height) / 2, 2);
                    var isWithinOurRadius = distanceFromCenter < radiusSquared;
                    return isWithinOurRadius;
                }

                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                cursorShape: {
                    if (containsMouseRound())
                    {
                        return Qt.PointingHandCursor;
                    }
                }
                onClicked: {
                    Qt.callLater(listMessages.positionViewAtEnd);
                    state = "hiden";
                }
            }

            state: "hiden"

            states: [
                State {
                    name: "hiden"
                    PropertyChanges {
                        y: listMessages.height + 8;
                        target: roundButtonScrollToBottom;
                    }
                },
                State {
                    name: "shown"
                    PropertyChanges {
                        y: listMessages.height - width - 20;
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
        }

        ScrollBar.vertical: ScrollBar {
            contentItem: Rectangle {
                color: "#03A9F4"
                opacity: 0
                onOpacityChanged: {
                    if (opacity > 0.5)
                    {
                        opacity = 0.5
                    }
                }
            }
        }

        spacing: 2
        width: parent.width
        model: messagesModel
        delegate: messageDelegate
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
            Qt.callLater(listMessages.autoScroll);
        }

        function autoScroll()
        {
            if (needAutoScrollToBottom)
            {
                listMessages.positionViewAtEnd();
                needAutoScrollToBottom = false;
            }
        }

        function scrollbarOnBottom()
        {
            return visibleArea.yPosition * contentHeight + listMessages.height + 160 >= contentHeight;
        }
    }
    function openAuthorWindow(authorChannelId,
                              authorName,
                              messageType,
                              authorAvatarUrl,
                              authorPageUrl,
                              authorChatModerator,
                              authorIsChatOwner,
                              authorChatSponsor,
                              authorIsVerified)
    {
        if (messageType === ChatMessage.SoftwareNotification && messageType !== ChatMessage.TestMessage)
        {
            return;
        }

        var posX, posY;
        if (typeof(root.authorInfoWindow) != "undefined")
        {
            posX = root.authorInfoWindow.x;
            posY = root.authorInfoWindow.y;
            root.authorInfoWindow.destroy();
        }

        var component = Qt.createComponent("qrc:/author_info_window.qml");
        root.authorInfoWindow = component.createObject(root);

        root.authorInfoWindow.close();

        root.authorInfoWindow.authorChannelId = authorChannelId;
        root.authorInfoWindow.authorName      = authorName;
        root.authorInfoWindow.authorAvatarUrl = authorAvatarUrl;
        root.authorInfoWindow.authorPageUrl   = authorPageUrl;

        root.authorInfoWindow.authorChatModerator = authorChatModerator;
        root.authorInfoWindow.authorIsChatOwner   = authorIsChatOwner;
        root.authorInfoWindow.authorChatSponsor   = authorChatSponsor;
        root.authorInfoWindow.authorIsVerified    = authorIsVerified;

        if (typeof(posX) != "undefined")
        {
            root.authorInfoWindow.x = posX;
            root.authorInfoWindow.y = posY;
        }

        root.authorInfoWindow.show();
    }

    Component {
        id: messageDelegate

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
                if (messageType == ChatMessage.SoftwareNotification ||
                    messageType == ChatMessage.TestMessage)
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
                Text {
                    id: authorNameText
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
                        authorIsVerified    |
                        messageType == ChatMessage.SoftwareNotification |
                        messageType == ChatMessage.TestMessage
                    font.pointSize: 10

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: authorPageUrl.toString().length > 0 && authorName.length > 0;
                        cursorShape: {
                            if (hoverEnabled)
                                return Qt.PointingHandCursor;
                        }
                        onClicked: {
                            if (hoverEnabled)
                                openAuthorWindow(authorChannelId,
                                             authorName,
                                             messageType,
                                             authorAvatarUrl,
                                             authorPageUrl,
                                             authorChatModerator,
                                             authorIsChatOwner,
                                             authorChatSponsor,
                                             authorIsVerified);
                        }
                    }

                    //selectByKeyboard: true
                    //selectByMouse: true
                    //readOnly: true
                    //style: Text.Outline
                    //styleColor: "black"

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
                        if (authorIsChatOwner)
                        {
                            return "qrc:/resources/images/king.svg";
                        }
                        else if (authorChatModerator)
                        {
                            return "qrc:/resources/images/tool.svg";
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

                rounded: messageType != ChatMessage.SoftwareNotification &&
                         messageType != ChatMessage.TestMessage;

                height: 32
                width: 32
                mipmap: true

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.margins: 4

                asynchronous: true
                source: authorAvatarUrl

                MouseArea {
                    anchors.fill: parent;
                    hoverEnabled: authorPageUrl.toString().length > 0;
                    acceptedButtons: Qt.LeftButton;
                    cursorShape: {
                        if (hoverEnabled)
                            return Qt.PointingHandCursor;
                    }
                    onClicked: {
                        if (hoverEnabled)
                            openAuthorWindow(authorChannelId,
                                         authorName,
                                         messageType,
                                         authorAvatarUrl,
                                         authorPageUrl,
                                         authorChatModerator,
                                         authorIsChatOwner,
                                         authorChatSponsor,
                                         authorIsVerified);
                    }
                }
            }

            //Text Message
            TextEdit {
                id: textEditMessageText

                color: {
                    if (messageMarkedAsDeleted)
                    {
                        return "gray";
                    }
                    else
                    {
                        if (messageIsBotCommand)
                        {
                            return "yellow";
                        }
                        else
                        {
                            //Normal message
                            return "white";
                        }
                    }
                }

                anchors.left: avatarImage.right
                anchors.right: parent.right
                anchors.top: authorRow.bottom
                anchors.margins: 4
                wrapMode: Text.Wrap
                text: messageText
                font.weight: messageIsBotCommand ? Font.Black : Font.DemiBold
                font.italic: messageMarkedAsDeleted
                selectByMouse: true
                selectByKeyboard: true
                readOnly: true

                font.letterSpacing: 0.5
                font.pointSize: 11.5
            }
        }
    }

    Item {
        id: waitAnimation
        x: parent.width  / 2 - width  / 2
        y: parent.height / 2 - height / 2
        width: 100
        height: 100
        visible: !chatHandler.connectedSome && listMessages.count == 0

        AnimatedImage {
            id: animatedImage
            x: 270
            y: 190
            anchors.fill: parent
            playing: true
            source: "qrc:/resources/images/sleeping_200_transparent.gif"
            smooth: true
            antialiasing: true
            asynchronous: true

            property real imageScaleX: 1.0
            transform: Scale {
                xScale: animatedImage.imageScaleX;
                origin.x: width  / 2;
                origin.y: height / 2;
            }
        }

        ColorOverlay {
            width: 100
            height: 100
            source: animatedImage
            smooth: animatedImage.smooth
            antialiasing: animatedImage.antialiasing
            color: "#03A9F4"
            opacity: animatedImage.opacity
            transform: Scale {
                xScale: animatedImage.imageScaleX
                origin.x: width  / 2;
                origin.y: height / 2;
            }
        }

        Text {
            text: "Z z z"
            color: "#03A9F4"
            x: animatedImage.x + animatedImage.width - 20
            anchors.bottom: parent.top
            font.pointSize: 20
        }
    }

    Text {
        text: {
            var s = qsTr("Nothing connected");

            if (typeof(root.settingsWindow) == "undefined" || !root.settingsWindow.visible)
            {
               s += "\n\n" + qsTr("Right click on the window to open the settings");
            }

            s += "\n\n" + "(^=◕ᴥ◕=^)";

            return s;
        }

        color: "#03A9F4"
        wrapMode: Text.Wrap
        font.pointSize: 12
        horizontalAlignment: Text.AlignHCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: waitAnimation.bottom
        anchors.topMargin: 20
        visible: !chatHandler.connectedSome && listMessages.count == 0
    }
}


/*##^##
Designer {
    D{i:0;formeditorZoom:2}
}
##^##*/
