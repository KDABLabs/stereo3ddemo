import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

RowLayout {
    id: root

    property alias title: label.text
    property alias from: slider.from
    property alias to: slider.to
    property alias value: slider.value
    property alias stepSize: slider.stepSize
    property alias defaultValue: slider.defaultValue
    property alias fineAdjustmentFactor: slider.fineAdjustmentFactor
    property alias fillSliderBar: slider.fillSliderBar
    property alias background: slider.background
    property bool enableTextField: true
    readonly property bool hovered: slider.hovered || label.hovered
    property int precision: 2
    property string unit: ""

    signal moved(real fvalue)

    ToolTip.visible: hovered && ToolTip.text.length > 0
    ToolTip.timeout: -1
    ToolTip.delay: Application.styleHints.mousePressAndHoldInterval

    Label {
        id: label
        text: "Slider"
        font: Style.fontDefault
        Layout.alignment: Qt.AlignLeft
        enabled: root.enabled
        readonly property bool hovered: labelMa.containsMouse
        MouseArea {
            id: labelMa
            anchors.fill: parent
            hoverEnabled: true
        }
    }
    CustomSlider {
        id: slider
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight
        onMoved: (proposedValue) => root.moved(proposedValue)
        enabled: root.enabled
    }
    TextField {
        id: textBox
        Layout.alignment: Qt.AlignRight
        Layout.preferredWidth: 80
        padding: 0
        text: slider.value.toFixed(root.precision) + unit

        Timer {
            id: acceptEntryTimer
            running: false
            repeat: false
            interval: 700
            onTriggered: root.moved(Math.max(root.from, Math.min(parseFloat(textBox.text), root.to)));
        }

        onTextEdited: {
            if (textBox.focus) {
                acceptEntryTimer.restart()
            }
        }
        font: Style.fontDefault
        enabled: root.enabled
        visible: root.enableTextField
    }
}
