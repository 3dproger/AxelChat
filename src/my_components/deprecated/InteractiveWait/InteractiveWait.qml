import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
    id: root

    width: 100
    height: 100

    property color mainColor: "#ff0000"
    property int curretnImageIndex: getRandomInt(0, imageSources.count)
    property string forcedImage: ""
    ListModel {
        id: imageSources
        ListElement { source: "qrc:/gifs/car_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/cat-meow_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/cd-2_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/cola_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/computer-display_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/cool_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/fidget-spinner_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/fly-elephant_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/ghost_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/hourglass_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/kawaii_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/mug_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/no-connection_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/porridge_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/roller-skating_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/row-boat_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/settings_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/shuttercock_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/skateboarding_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/snow-storm-weather_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/soccer-ball_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/sup_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/tea_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/volleyball_200_transparent.gif" }
        ListElement { source: "qrc:/gifs/walk_200_transparent.gif" }
    }

    state: "shown"

    states: [
        State {
            name: "hiden"
            PropertyChanges {
                target: animatedImage;
                visible: false
                imageScaleX: 0
            }
        },
        State {
            name: "shown"
            PropertyChanges {
                target: animatedImage;
                visible: true
                imageScaleX: 1
            }
        },
        State {
            name: "changing_image_closing"
            PropertyChanges {
                target: animatedImage;
                visible: true
                imageScaleX: 0
            }
            onCompleted: {
                animatedImage.source = getNextImage();
                root.state = "changing_image_opening";
            }
        },
        State {
            name: "changing_image_opening"
            PropertyChanges {
                visible: true
                imageScaleX: 1
                target: animatedImage;
            }
        }
    ]

    transitions:[
        Transition {
            from: "*";
            to: "*";
            reversible: true

            NumberAnimation {
                properties: "imageScaleX";
                easing.type: Easing.Linear;
                duration: 150;
            }
        }
    ]

    function hide()
    {
        state = "hiden"
    }

    function show()
    {
        state = "shown"
    }

    function getNextImage()
    {
        if (forcedImage.length == 0)
        {
            curretnImageIndex++;
            if (curretnImageIndex >= imageSources.count)
            {
                curretnImageIndex = 0;
            }

            return imageSources.get(curretnImageIndex).source;
        }
        else
        {
            return forcedImage;
        }
    }

    function getRandomInt(min, max) {
        min = Math.ceil(min);
        max = Math.floor(max);
        return Math.floor(Math.random() * (max - min)) + min;
    }

    Timer {
        id: timerChangeImage
        interval: 1200;
        running: animatedImage.visible;
        repeat: true
        onTriggered: {
            if (forcedImage == "" || animatedImage.source != forcedImage)
            {
                root.state = "changing_image_closing"
            }
        }
    }

    onForcedImageChanged: {
        timerChangeImage.restart();
        root.state = "changing_image_closing";
    }

    AnimatedImage {
        id: animatedImage
        x: 270
        y: 190
        anchors.fill: parent
        playing: true
        source: imageSources.get(curretnImageIndex).source
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
        id: overlay
        anchors.fill: animatedImage
        source: animatedImage
        smooth: animatedImage.smooth
        antialiasing: animatedImage.antialiasing
        color: mainColor
        opacity: animatedImage.opacity
        transform: Scale {
            xScale: animatedImage.imageScaleX
            origin.x: width  / 2;
            origin.y: height / 2;
        }
    }
}
