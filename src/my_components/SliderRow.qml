import QtQuick 2.0
import QtQuick.Controls 2.15

Row {
    id: root

    property string name: "slider"
    property real from: 0
    property real to: 1
    property real defaultValue: 0.5
    property real value: 0.5
    property real stepSize: 0
    property bool valueToPercent: false

    onValueChanged: {
        slider.value = value
    }

    spacing: 6

    Label {
        id: labelName
        anchors.verticalCenter: root.verticalCenter
        text: root.name
    }

    Slider {
        id: slider
        anchors.verticalCenter: root.verticalCenter
        from: root.from
        to: root.to
        stepSize: root.stepSize
        onValueChanged: {
            root.value = value
        }
    }

    Label {
        id: labelValue
        anchors.verticalCenter: parent.verticalCenter
        text: {
            if (valueToPercent) {
                return String("%1 %").arg(root.value * 100)
            }
            else {
                return String("%1").arg(root.value)
            }
        }
    }

    RoundButton {
        id: buttonReset
        visible: root.value !== root.defaultValue
        anchors.verticalCenter: parent.verticalCenter
        flat: true
        icon.source: "qrc:/resources/images/refresh-button-1.svg"
        text: ""
        onClicked: {
            slider.value = defaultValue
        }
    }
}
