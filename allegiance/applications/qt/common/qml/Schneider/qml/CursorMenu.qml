import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Schneider

GroupBox {
    id: cursor
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignTop

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        CheckBoxX {
            Layout.fillWidth: true
            title: "Visible"
            initial: Cursor.visible
            onChecked: bchecked => Cursor.visible = bchecked
        }

        // Cursor Settings if visible
        ColumnLayout {
            id: cursorlayout
            Layout.fillWidth: true
            visible: Cursor.visible

            // Cursor Type
            RowLayout {
                Layout.fillWidth: true
                Label {
                    Layout.alignment: Qt.AlignLeft
                    text: "Type"
                    font: Style.fontDefault
                }
                ComboBox {
                    Layout.fillWidth: true
                    model: [
                        { value: Cursor.CursorType.Default, text: "Default" },
                        { value: Cursor.CursorType.Cross, text: "Cross"},
                        { value: Cursor.CursorType.CrossHair, text: "Cross Hair"},
                        { value: Cursor.CursorType.Dot, text: "Dot"}
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: Cursor.cursor
                    onCurrentIndexChanged: {
                        if (currentIndex !== -1 && Cursor.cursor != currentIndex) {
                            Cursor.cursor = currentIndex;
                        }
                    }
                }
            }

            // Cursor Color
            RowLayout{
                id: colorlayout
                Layout.fillWidth: true
                Label {
                    id: colorLabel
                    Layout.alignment: Qt.AlignLeft
                    text: "Color"
                    font: Style.fontDefault
                }
                Item {
                    Layout.fillWidth: true
                }
                Button {
                    id: button
                    text: "            "
                    background: Rectangle {
                        id: rect
                        color: Cursor.cursorTint
                        border.color: Style.link
                        border.width: 2
                        radius: 2
                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                            }
                        }
                    }
                }
            }

            SliderValue {
                id: cursorhue
                Layout.fillWidth: true
                from: 1
                to: 360
                title: "Hue"
                unit: "°"
                precision: 1
                value: Cursor.hue
                enabled: true
                defaultValue: 180
                onMoved: current => Cursor.hue = current
                onAdjusted: adjustedValue => Cursor.hue += adjustedValue
                ToolTip.text: "Set cursor Hue."
            }

            // Cursor Scaling
            CheckBoxX {
                Layout.fillWidth: true
                visible: Cursor.visible
                title: "Scaling"
                initial: Cursor.scalingEnabled
                onChecked: bchecked => Cursor.scalingEnabled = bchecked
            }

            // Cursor Factor
            SliderValue {
                id: scalefactor
                visible: Cursor.scalingEnabled
                Layout.fillWidth: true
                from: 0.1
                to: 2.0
                value: Cursor.scaleFactor
                title: "Factor"
                fineAdjustmentFactor: 0.001

                onMoved: current => Cursor.scaleFactor = current
                onAdjusted: adjustedValue => Cursor.scaleFactor += adjustedValue
            }
        }
    }
}
