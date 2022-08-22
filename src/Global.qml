pragma Singleton
import QtQuick 2.0

QtObject {
    id: qtObject

    readonly property int _UnknownServiceType: -1
    readonly property int _SoftwareServiceType: 10
    readonly property int _YouTubeServiceType: 100
    readonly property int _TwitchServiceType: 101
    readonly property int _GoodGameServiceType: 102

    readonly property int _NotConnectedConnectionStateType: 10
    readonly property int _ConnectingConnectionStateType: 20
    readonly property int _ConnectedConnectionStateType: 30

    property var windowChat
    property var windowSettings

    property bool windowChatTransparentForInput : false

    property bool windowChatShowViewersCount : true

    property bool windowChatStayOnTop : true
    property bool windowChatSystemWindowFrame : true
    property real windowChatWindowOpacity: 1
    property real windowChatBackgroundOpacity: 0.40

    property bool windowChatMessageShowAvatar : true
    property bool windowChatMessageShowAuthorName : true
    property bool windowChatMessageShowPlatformIcon : true
    property bool windowChatMessageShowTime : false

    property real windowChatMessageAvatarSize : 40
    property string windowChatMessageAvatarShape : "round"

    property real windowChatMessageAuthorNameFontSize : 12
    property real windowChatMessageTextFontSize : 12
    property real windowChatMessageTimeFontSize : 12

    property real windowChatMessageFrameBorderWidth: 0//1
    property color windowChatMessageFrameBorderColor: "#003760"
    property color windowChatMessageFrameBackgroundColor: "transparent"
    property real windowChatMessageFrameCornerRadius: 4//0

    function save(){
        windowChat.saveGlobalSettings()
    }
}
