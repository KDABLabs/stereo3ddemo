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
    readonly property bool hovered: slider.hovered || label.hovered
    property int precision: 2
    property string unit: ""
    readonly property real distance: (to - from)

    signal moved(real fvalue)

    ToolTip.visible: hovered && ToolTip.text.length > 0
    ToolTip.timeout: 5000
    ToolTip.delay: Application.styleHints.mousePressAndHoldInterval

    // TODO: Investigate switching to [0, 100] range to handle having a different
    // value mapping when shift is pressed
    function fromSliderValue(sliderValue) // [0, 100] -> [from, to]
    {
        return from + distance * (sliderValue * 0.01)
    }

    function toSliderValue(displayValue) // [from, to] -> [0, 100]
    {
        return (displayValue - from) / distance * 100.0
    }

    readonly property bool shiftPressed: Scene.shiftPressed
    property real valueOnShiftPressed: 0
    onShiftPressedChanged: {
        valueOnShiftPressed = (shiftPressed) ? slider.value : 0
    }

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

    Slider {
        id: slider
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight

        onMoved: {
            root.moved(value)
        }
        enabled: root.enabled
    }
    TextField {
        id: textBox
        Layout.alignment: Qt.AlignRight
        padding: 0
        validator: DoubleValidator {
            bottom: slider.from
            top: slider.to
            decimals: root.precision
        }
        text: slider.value.toFixed(root.precision) + unit
        onTextChanged: {
            if (textBox.focus) {
                moved(parseFloat(textBox.text));
            }
        }
        font: Style.fontDefault
        enabled: root.enabled
    }
}
