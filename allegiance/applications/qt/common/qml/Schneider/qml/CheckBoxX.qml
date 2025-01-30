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
    readonly property bool hovered: checkbox.hovered || label.hovered

    ToolTip.visible: hovered && ToolTip.text.length > 0
    ToolTip.timeout: -1
    ToolTip.delay: Application.styleHints.mousePressAndHoldInterval

    spacing: 5
    Label {
        id: label
        text: "Slider"
        font: Style.fontDefault
        Layout.alignment: Qt.AlignLeft
        readonly property bool hovered: labelMa.containsMouse
        MouseArea {
            id: labelMa
            anchors.fill: parent
            hoverEnabled: true
        }
    }
    Item { Layout.fillWidth: true }
    CheckBox {
        id: checkbox
        Layout.alignment: Qt.AlignRight
        checked: initial
        onCheckedChanged: root.checked(checked)
    }
}
