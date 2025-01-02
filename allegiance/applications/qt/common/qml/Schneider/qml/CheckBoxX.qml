import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

RowLayout {
    id: root
    signal checked(bool checkValue)
    property alias title: label.text
    property bool initial: false
    readonly property bool isChecked: checkbox.checked

    Layout.fillWidth: true
    spacing: 5
    Label {
        id: label
        text: "Slider"
        font: Style.fontDefault
        Layout.alignment: Qt.AlignLeft
    }
    Item { Layout.fillWidth: true }
    CheckBox {
        id: checkbox
        Layout.alignment: Qt.AlignRight
        checked: initial
        onCheckedChanged: root.checked(checked)
    }
}
