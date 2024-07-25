import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

Item {
  id: root
  implicitWidth: rootLayout.implicitWidth
  implicitHeight: rootLayout.implicitHeight

  property alias title: label.text
  property alias from: slider.from
  property alias to: slider.to
  property alias value: slider.value
  property alias hovered: slider.hovered

  signal moved(real fvalue)

  ColumnLayout {
    id: rootLayout
    anchors.fill: parent
    anchors.margins: 10

    Label {
      id: label
      text: "Slider"
      font: Style.fontDefault
    }

    RowLayout {
      Layout.fillWidth: true
      Slider {
        id: slider
        Layout.fillWidth: true
        onMoved: root.moved(value)
      }
      TextInput {
        id: textBox
        validator: DoubleValidator {
          bottom: slider.from
          top: slider.to
          decimals: 4
        }
        text: slider.value.toFixed(4)
        onTextChanged: {
          if (textBox.focus) {
            slider.value = parseFloat(textBox.text);
            moved(slider.value);
          }
        }
        font: Style.fontDefault
        color: Style.text
      }
    }
  }
}
