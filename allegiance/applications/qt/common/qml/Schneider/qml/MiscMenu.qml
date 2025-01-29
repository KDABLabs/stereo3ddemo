import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

ColumnLayout {
    id: misc_menu

    GridLayout {
        Layout.fillWidth: true
        columnSpacing: 10

        CheckBoxX {
            title: "Wireframe"
            initial: Misc.wireframeEnabled
            onChecked: checkValue => Misc.wireframeEnabled = checkValue
            Layout.column: 0
            Layout.columnSpan: 3
            Layout.row: 0
            ToolTip.text: "Enable WireFrame Rendering."
        }

        CheckBoxX {
            title: "Show Frustum"
            initial: Misc.frustumViewEnabled
            onChecked: checkValue => Misc.frustumViewEnabled = checkValue
            Layout.column: 0
            Layout.columnSpan: 3
            Layout.row: 1
            ToolTip.text: "Display the Camera Frustum Overlay."
        }
    }
}
