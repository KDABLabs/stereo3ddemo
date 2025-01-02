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
    property alias hovered: slider.hovered
    property int precision: 2
    property string unit: ""

    signal moved(real fvalue)

    Label {
        id: label
        text: "Slider"
        font: Style.fontDefault
        Layout.alignment: Qt.AlignLeft
        enabled: root.enabled
    }
    Slider {
        id: slider
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight
        onMoved: root.moved(value)
        enabled: root.enabled
    }
    TextField {
        id: textBox
        Layout.alignment: Qt.AlignRight
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
