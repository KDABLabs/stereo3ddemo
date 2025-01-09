import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

RowLayout {
    id: root

    property alias title: label.text
    property real from
    property real mid
    property real to
    property real value
    property real defaultValue
    readonly property bool hovered: slider.hovered || label.hovered
    property int precision: 2
    property string unit: ""

    readonly property real cA: ((from * to - mid * mid) / (from - 2 * mid + to))
    readonly property real cB: ((mid - from) * (mid - from) / (from - 2 * mid + to))
    readonly property real cC: (2.0 * Math.log((to - mid) / (mid - from)))

    function fromSliderValue(sliderValue)
    {
        return cA + cB * Math.exp(cC * sliderValue);
    }

    function toSliderValue(displayValue)
    {
        return Math.log((displayValue - cA) / cB) / cC;
    }

    signal moved(real fvalue)

    ToolTip.visible: hovered && ToolTip.text.length > 0
    ToolTip.timeout: 5000
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
        from: 0
        to: 1
        value: toSliderValue(root.value)
        defaultValue: toSliderValue(root.defaultValue)
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight
        onMoved: (proposedValue) => root.moved(fromSliderValue(proposedValue))
        enabled: root.enabled
    }
    TextField {
        id: textBox
        Layout.alignment: Qt.AlignRight
        validator: DoubleValidator {
            bottom: from
            top: to
            decimals: root.precision
        }
        text: fromSliderValue(slider.value).toFixed(root.precision) + unit
        onTextChanged: {
            if (textBox.focus) {
                moved(parseFloat(textBox.text));
            }
        }
        font: Style.fontDefault
        enabled: root.enabled
    }
}
