import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import Qt.labs.settings 1.1
import AxelChat.ChatHandler 1.0
import AxelChat.UpdateChecker 1.0
import AxelChat.MessageAuthor 1.0
import AxelChat.ChatMessage 1.0
import AxelChat.Tray 1.0
import QtQuick.Window 2.15
import "my_components" as MyComponents
import "setting_pages" as SettingPages
import "."

ApplicationWindow {
    id: root
    visible: true
    width: 300
    height: 600
    minimumHeight: 200
    minimumWidth:  150
    title: Qt.application.name

    flags: {
        var windowFlags = Qt.Window
        if (_customizeWindowHint) {
            windowFlags |= Qt.CustomizeWindowHint
        }

        if (Global.windowChatTransparentForInput) {
            windowFlags |= Qt.WindowTransparentForInput
        }

        if (Global.windowChatSystemWindowFrame) {
            windowFlags |= Qt.WindowTitleHint | Qt.WindowSystemMenuHint |
                    Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint | Qt.WindowCloseButtonHint
        }
        else {
            windowFlags |= Qt.FramelessWindowHint
        }

        if (Global.windowChatStayOnTop) {
            windowFlags |= Qt.WindowStaysOnTopHint
        }

        return windowFlags
    }

    property bool _customizeWindowHint: false
    function refreshByFlags() {
        _customizeWindowHint = !_customizeWindowHint
        _customizeWindowHint = !_customizeWindowHint
    }

    opacity: Global.windowChatWindowOpacity
    color: Qt.rgba(0, 0, 0, Global.windowChatBackgroundOpacity)

    property var settingsWindow;
    property var authorInfoWindow;
    property var updatesWindow;

    Settings {
        category: "chat_window"
        property alias window_width:  root.width;
        property alias window_height: root.height;
    }

    function loadGlobalSettings() {
        Global.windowChatTransparentForInput = qmlUtils.valueBool("windowChatTransparentForInput",      Global.windowChatTransparentForInput)

        Global.windowChatStayOnTop          = qmlUtils.valueBool("windowChatStayOnTop",                 Global.windowChatStayOnTop)
        Global.windowChatSystemWindowFrame  = qmlUtils.valueBool("windowChatSystemWindowFrame",         Global.windowChatSystemWindowFrame)
        Global.windowChatWindowOpacity      = qmlUtils.valueReal("windowChatWindowOpacity",             Global.windowChatWindowOpacity)
        Global.windowChatBackgroundOpacity  = qmlUtils.valueReal("windowChatBackgroundOpacity",         Global.windowChatBackgroundOpacity)

        Global.windowChatMessageShowAvatar          = qmlUtils.valueBool("windowChatMessageShowAvatar",             Global.windowChatMessageShowAvatar)
        Global.windowChatMessageShowAuthorName      = qmlUtils.valueBool("windowChatMessageShowAuthorName",         Global.windowChatMessageShowAuthorName)
        Global.windowChatMessageShowTime            = qmlUtils.valueBool("windowChatMessageShowTime",               Global.windowChatMessageShowTime)
        Global.windowChatMessageShowPlatformIcon    = qmlUtils.valueBool("windowChatMessageShowPlatformIcon",       Global.windowChatMessageShowPlatformIcon)

        Global.windowChatMessageAvatarSize          = qmlUtils.valueReal("windowChatMessageAvatarSize",             Global.windowChatMessageAvatarSize)

        Global.windowChatMessageAuthorNameFontSize  = qmlUtils.valueReal("windowChatMessageAuthorNameFontSize",     Global.windowChatMessageAuthorNameFontSize)
        Global.windowChatMessageTextFontSize        = qmlUtils.valueReal("windowChatMessageTextFontSize",           Global.windowChatMessageTextFontSize)
        Global.windowChatMessageTimeFontSize        = qmlUtils.valueReal("windowChatMessageTimeFontSize",           Global.windowChatMessageTimeFontSize)

        Global.windowChatShowViewersCount           = qmlUtils.valueReal("windowChatShowViewersCount",              Global.windowChatShowViewersCount)
    }

    function saveGlobalSettings() {
        qmlUtils.setValue("windowChatTransparentForInput",  Global.windowChatTransparentForInput)

        qmlUtils.setValue("windowChatStayOnTop",            Global.windowChatStayOnTop)
        qmlUtils.setValue("windowChatSystemWindowFrame",    Global.windowChatSystemWindowFrame)
        qmlUtils.setValue("windowChatWindowOpacity",        Global.windowChatWindowOpacity)
        qmlUtils.setValue("windowChatBackgroundOpacity",    Global.windowChatBackgroundOpacity)

        qmlUtils.setValue("windowChatMessageShowAvatar",        Global.windowChatMessageShowAvatar)
        qmlUtils.setValue("windowChatMessageShowAuthorName",    Global.windowChatMessageShowAuthorName)
        qmlUtils.setValue("windowChatMessageShowTime",          Global.windowChatMessageShowTime)
        qmlUtils.setValue("windowChatMessageShowPlatformIcon",  Global.windowChatMessageShowPlatformIcon)

        qmlUtils.setValue("windowChatMessageAvatarSize",  Global.windowChatMessageAvatarSize)

        qmlUtils.setValue("windowChatMessageAuthorNameFontSize",    Global.windowChatMessageAuthorNameFontSize)
        qmlUtils.setValue("windowChatMessageTextFontSize",          Global.windowChatMessageTextFontSize)
        qmlUtils.setValue("windowChatMessageTimeFontSize",          Global.windowChatMessageTimeFontSize)

        qmlUtils.setValue("windowChatShowViewersCount",             Global.windowChatShowViewersCount)
    }

    onClosing: {
        saveGlobalSettings()
    }

    Component.onCompleted: {
        Global.windowChat = this
        loadGlobalSettings()

        //Update notification window
        if (updateChecker.autoRequested)
        {
            var component = Qt.createComponent("qrc:/updatesnotification.qml");
            root.updatesWindow = component.createObject(root);
        }

        refreshByFlags()
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

    Connections {
        target: tray

        function onTriggered(actionName) {
            if (actionName === "settings") {
                openSettingsWindow()
            }
            else if (actionName === "input_transparent_toggle") {
                Global.windowChatTransparentForInput = !Global.windowChatTransparentForInput
            }
            else if (actionName === "close_application") {
                Qt.quit()
            }
            else if (actionName === "clear_messages") {
                chatHandler.clearMessages()
            }
            else {
                console.log("Unknown tray action '", actionName, "'")
            }
        }
    }

    Connections {
        target: Global

        function onWindowChatTransparentForInputChanged() {
            tray.ignoredMouse = Global.windowChatTransparentForInput
        }
    }

    function openSettingsWindow() {
        if (typeof(settingsWindow) == "undefined")
        {
            var component = Qt.createComponent("qrc:/settings.qml")
            settingsWindow = component.createObject(root)
        }

        settingsWindow.show()
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: {
            openSettingsWindow()
        }

        onWheel: {
            listMessages.flick(wheel.angleDelta.x * 8, wheel.angleDelta.y * 8)
            wheel.accepted=true
        }
    }

    ListView {
        id: listMessages
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: bottomPanel.visible ? bottomPanel.top : parent.bottom
        clip: true

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
                        y: listMessages.height + 8 + (bottomPanel.visible ? bottomPanel.height : 0)
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

        function autoScroll()
        {
            if (needAutoScrollToBottom)
            {
                listMessages.positionViewAtEnd();
                listMessages.needAutoScrollToBottom = false;
            }
        }

        Connections {
            target: chatHandler
            function onMessagesDataChanged() {
                listMessages.needAutoScrollToBottom = listMessages.scrollbarOnBottom() || Global.windowChatTransparentForInput;
                Qt.callLater(listMessages.autoScroll);
                deferredAutoScrollTimer.running = true
                deferredAutoScrollTimer.couterTimes = 5
            }
        }

        Timer {
            id: deferredAutoScrollTimer
            property int couterTimes: 5
            interval: 50
            running: false
            repeat: true
            onTriggered: {
                // additional scroll to fix the problem when the first time the scroll does not reach the end
                listMessages.needAutoScrollToBottom = listMessages.scrollbarOnBottom() || Global.windowChatTransparentForInput;
                listMessages.autoScroll()
                couterTimes -= 1
                if (couterTimes <= 0)
                {
                    running = false
                }
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
        if (messageType === ChatMessage.SoftwareNotification || messageType === ChatMessage.TestMessage)
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

        root.authorInfoWindow.messageType = messageType;

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

    function redrawTextEdit(_textEditObject)
    {
        // this non-obvious code solves the bug https://github.com/3dproger/AxelChat/issues/193

        var selectionStart  =_textEditObject.selectionStart
        var selectionEnd =_textEditObject.selectionEnd

        _textEditObject.selectAll()
        _textEditObject.deselect()

        _textEditObject.select(selectionStart, selectionEnd)
    }

    function getPlatformIcon(platform)
    {
        switch (platform)
        {
        case ChatMessage.YouTube:               return "qrc:/resources/images/youtube-icon.svg"
        case ChatMessage.Twitch:                return "qrc:/resources/images/twitch-icon.svg"
        case ChatMessage.GoodGame:              return "qrc:/resources/images/goodgame-icon.svg"

        case ChatMessage.SoftwareNotification:  return "qrc:/resources/images/axelchat-rounded.svg"
        case ChatMessage.TestMessage:           return "qrc:/resources/images/flask2.svg"

        case ChatMessage.Unknown:               return ""
        }

        return ""
    }

    Component {
        id: messageDelegate

        Rectangle {
            id: messageContent

            width: listMessages.width
            height: {
                if (avatarImage.visible) {
                    return Math.max(Math.max(textEditMessageText.y + textEditMessageText.height + 4, 40), avatarImage.height + 4)
                }
                else {
                    return Math.max(textEditMessageText.y + textEditMessageText.height + 4, 40)
                }
            }

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

            border.width: Global.windowChatMessageFrameBorderWidth
            border.color: Global.windowChatMessageFrameBorderColor
            color: {
                if (messageBodyBackgroundForcedColor != "")
                {
                    return messageBodyBackgroundForcedColor
                }

                if (messageIsDonateSimple || messageIsDonateWithText || messageIsDonateWithImage)
                {
                    return "#1DE9B6"
                }

                if (messageIsYouTubeChatMembership)
                {
                    return "#0F9D58"
                }

                return Global.windowChatMessageFrameBackgroundColor
            }

            radius: Global.windowChatMessageFrameCornerRadius

            Row {
                id: authorRow
                y: 3
                anchors.left:  avatarImage.visible ? avatarImage.right : parent.left
                anchors.right: labelTime.visible ? labelTime.left : messageContent.right
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                spacing: 4

                //Author Name
                Text {
                    id: authorNameText
                    visible: Global.windowChatMessageShowAuthorName
                    //readOnly: true
                    style: Text.Outline
                    color: {
                        if (messageType == ChatMessage.YouTube)
                        {
                            if (authorIsChatOwner)
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
                            }
                        }
                        else if (messageType == ChatMessage.Twitch)
                        {
                            if (authorNicknameColor != "#000000")
                            {
                                return authorNicknameColor;
                            }
                        }

                        return "#03A9F4";
                    }
                    font.bold: true
                        /*authorChatModerator |
                        authorIsChatOwner   |
                        authorChatSponsor   |
                        authorIsVerified    |
                        messageType == ChatMessage.SoftwareNotification |
                        messageType == ChatMessage.TestMessage*/
                    font.pointSize: Global.windowChatMessageAuthorNameFontSize
                    //font.hintingPreference: Font.PreferFullHinting
                    //style: Text.Outline;
                    //styleColor: "red"

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: messageType != ChatMessage.SoftwareNotification &&
                                      messageType != ChatMessage.TestMessage &&
                                      !messageIsPlatformGeneric;
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

                    //anchors.margins: 4
                    textFormat: Text.RichText
                    //wrapMode: Text.Wrap

                    property int badgePixelSize: font.pixelSize * 1.25
                    text: {
                        var prefix = "";
                        var postfix = "";

                        if (Global.windowChatMessageShowPlatformIcon &&
                                messageType != ChatMessage.SoftwareNotification &&
                                messageType != ChatMessage.TestMessage)
                        {
                            prefix += createImgTag(getPlatformIcon(messageType), badgePixelSize) + " "
                        }

                        if (messageType === ChatMessage.YouTube)
                        {
                            if (authorIsChatOwner)
                            {
                                postfix += " " + createImgTag("qrc:/resources/images/king.svg", badgePixelSize)
                            }

                            if (authorIsVerified)
                            {
                                postfix += " " + createImgTag("qrc:/resources/images/youtube-verified-icon.svg", badgePixelSize)
                            }

                            if (authorChatModerator)
                            {
                                postfix += " " + createImgTag("qrc:/resources/images/youtube-moderator-icon.svg", badgePixelSize)
                            }


                            if (authorCustomBadgeUrl != "undefined")
                            {
                                postfix += " " + createImgTag(authorCustomBadgeUrl, badgePixelSize)
                            }
                        }
                        else if (messageType === ChatMessage.Twitch)
                        {
                            for (var i = 0; i < authorTwitchBadgesUrls.length; i++)
                            {
                                prefix += createImgTag(authorTwitchBadgesUrls[i], badgePixelSize) + " "
                            }
                        }

                        return prefix + authorName + postfix
                    }

                    function createImgTag(imgurl, size)
                    {
                        return "<img align=\"top\" src=\"" + imgurl + "\" height=\"" + size.toString() + "\" width=\"" + size.toString() + "\"/>"
                    }
                }
            }

            //Time
            Label {
                id: labelTime
                visible: Global.windowChatMessageShowTime
                color: "#039BE5"
                anchors.right: messageContent.right
                anchors.margins: 4
                text: messagePublishedAt.toLocaleTimeString(Qt.locale(), "hh:mm")
                font.pixelSize: Global.windowChatMessageTimeFontSize
                style: Text.Outline
            }

            //Author Avatar
            MyComponents.ImageRounded {
                id: avatarImage
                visible: Global.windowChatMessageShowAvatar

                rounded: messageType != ChatMessage.SoftwareNotification &&
                         messageType != ChatMessage.TestMessage &&
                         !messageIsPlatformGeneric;

                height: Global.windowChatMessageAvatarSize
                width: Global.windowChatMessageAvatarSize
                mipmap: true

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.margins: 4

                asynchronous: true
                source: {
                    if (authorAvatarUrl.toString() !== "")
                    {
                        return authorAvatarUrl;
                    }

                    return getPlatformIcon(messageType)
                }

                MouseArea {
                    anchors.fill: parent;
                    hoverEnabled: messageType != ChatMessage.SoftwareNotification &&
                                  messageType != ChatMessage.TestMessage &&
                                  !messageIsPlatformGeneric;
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

                anchors.left: avatarImage.visible ? avatarImage.right : parent.left
                anchors.right: parent.right
                anchors.top: authorRow.bottom
                anchors.margins: 4
                wrapMode: Text.Wrap
                textFormat: TextEdit.RichText
                text: messageText
                font.weight: messageIsBotCommand ? Font.Black : Font.DemiBold
                font.italic: messageMarkedAsDeleted || messageIsTwitchAction
                font.bold: messageType == ChatMessage.SoftwareNotification
                selectByMouse: true
                selectByKeyboard: true
                readOnly: true
                color: "#00000000"

                font.letterSpacing: 0.5
                font.pointSize: Global.windowChatMessageTextFontSize

                onLinkActivated: Qt.openUrlExternally(link)
                onHoveredLinkChanged: {
                    // here the code can be empty. The slot is created to change the shape of the cursor when hovering over a link
                }

                onTextChanged: {
                    if (textEditMessageTextTimer.running)
                    {
                        textEditMessageTextTimer.couterTimes = 20
                        textEditMessageTextTimer.running = true
                    }
                }

                Timer {
                    id: textEditMessageTextTimer
                    property int couterTimes: 20
                    interval: 500
                    running: true
                    repeat: true
                    onTriggered: {
                        // this non-obvious code solves the bug https://github.com/3dproger/AxelChat/issues/193
                        redrawTextEdit(parent)
                        couterTimes -= 1
                        if (couterTimes <= 0)
                        {
                            running = false
                        }
                    }
                }

                Text {
                    anchors.fill: parent
                    text: parent.text
                    textFormat: parent.textFormat
                    wrapMode: parent.wrapMode
                    style: Text.Outline
                    visible: true
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

                    font: parent.font
                    //font.pointSize: parent.font.pointSize
                }
            }
        }
    }

    Item {
        id: waitAnimation
        x: parent.width  / 2 - width  / 2
        y: parent.height / 2 - height / 2
        width: 100
        height: 100
        visible: !chatHandler.connectedCount > 0 && listMessages.count == 0

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
        style: Text.Outline
        font.pointSize: 12
        horizontalAlignment: Text.AlignHCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: waitAnimation.bottom
        anchors.topMargin: 20
        visible: chatHandler.connectedCount === 0 && listMessages.count == 0
    }

    Rectangle {
        id: bottomPanel
        visible: Global.windowChatShowViewersCount
        height: 22
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: "transparent"

        Text {
            id: textViewersCount
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: textViewersTotalCount.left
            anchors.margins: 2
            style: Text.Outline
            font.pointSize: 10
            color: "white"
            text:{
                if (chatHandler.connectedCount === 0)
                {
                    return ""
                }

                var t = ""
                var size = 18

                if (youTube.connectionStateType === 30)//ConnectionStateType.Connected
                {
                    t += "<img align=\"middle\" height=\"" + size.toString() + "\" width=\"" + size.toString() + "\" src=\"qrc:/resources/images/youtube-icon-18x18.png\">: "
                    if (youTube.viewersCount !== -1)
                        t += String("%1   ").arg(youTube.viewersCount)
                    else
                        t += "?"
                }

                if (twitch.connectionStateType === 30)//ConnectionStateType.Connected
                {
                    t += "<img align=\"middle\" height=\"" + size.toString() + "\" width=\"" + size.toString() + "\" src=\"qrc:/resources/images/twitch-icon-18x18.png\">: "
                    if (twitch.viewersCount !== -1)
                        t += String("%1   ").arg(twitch.viewersCount)
                    else
                        t += "?"
                }

                return t
            }

        }

        Text {
            id: textViewersTotalCount
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 2
            style: Text.Outline
            font.pointSize: 10
            color: "white"
            text:{
                if (chatHandler.connectedCount <= 1 || chatHandler.viewersTotalCount < 0)
                {
                    return ""
                }

                var t = ""
                var size = 18

                t += "<img align=\"middle\" height=\"" + size.toString() + "\" width=\"" + size.toString() + "\" src=\"qrc:/resources/images/viewer.svg\">: "
                if (chatHandler.viewersTotalCount !== -1)
                    t += String("%1   ").arg(chatHandler.viewersTotalCount)
                else
                    t += "?"

                return t
            }
        }
    }
}


/*##^##
Designer {
    D{i:0;formeditorZoom:2}
}
##^##*/
