import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import Schneider

Item {
    id: root

    signal moved(real proposedValue)
    signal adjusted(real adjustedValue)

    property real from: 0
    property real to: 1
    property real value: 0
    property real padding: 5
    property real stepSize: 1
    property real defaultValue
    property real fineAdjustmentFactor: 0.01

    readonly property bool hovered: ma.containsMouse

    implicitHeight: handle.height + padding

    Item {
        id: priv
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: root.padding
        }
        implicitHeight: handleHighlight.height

        readonly property int accentColorEnum: Material.Blue
        readonly property color accentColor: Material.color(accentColorEnum, Material.Shade200);
        readonly property color disabledColor: Material.color(Material.Grey, Material.Shade700)
        readonly property color activeColor: enabled ? accentColor : disabledColor
        readonly property color backgroundColor: enabled ? Qt.rgba(accentColor.r, accentColor.g, accentColor.b, accentColor.a * 0.33) : disabledColor

        readonly property real distance: (root.to - root.from)
        readonly property real handlePos: toNormalized(root.value) * width

        function fromNormalized(sliderValue) // [0, 1] -> [from, to]
        {
            return root.from + distance * (sliderValue)
        }

        function toNormalized(displayValue) // [from, to] -> [0, 1]
        {
            return (displayValue - root.from) / distance
        }

        Rectangle {
            id: sliderBar
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            height: 4
            color: priv.backgroundColor
            radius: 15

            Rectangle {
                id: sliderBarActive
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                width: priv.handlePos
                height: 4
                visible: root.enabled
                color: priv.activeColor
                radius: 15
            }
        }

        Rectangle {
            id: handleHighlight
            height: 30
            width: height
            radius: width * 0.5
            color: priv.backgroundColor
            visible: root.hovered
            enabled: root.hovered
            x: priv.handlePos - width * 0.5
            anchors {
                verticalCenter: parent.verticalCenter
            }
        }

        Rectangle {
            id: handle
            height: 13
            width: height
            radius: width * 0.5
            color: priv.activeColor
            x: priv.handlePos - width * 0.5
            anchors {
                verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            id: ma
            hoverEnabled: true
            anchors.fill: parent
            preventStealing: true

            property bool active: false
            readonly property bool shiftPressed: Scene.shiftPressed
            property real valueOnShiftPressed: 0

            onShiftPressedChanged: { valueOnShiftPressed = (shiftPressed) ? priv.toNormalized(root.value) : 0 }
            onPressed: (mouse) => {
                if (shiftPressed)
                    Scene.lockMouseInPlace = true
                active = true
            }
            onReleased: (mouse) => {
                if (shiftPressed)
                    Scene.lockMouseInPlace = false
                active = false
            }

            onPositionChanged: (mouse) => {
                if (!active)
                    return;

                // fine, incremental adjustment
                if (shiftPressed) {
                    var globXY = mapToGlobal(mouse.x, mouse.y);
                    var calculatedAdjustedValue = (globXY.x - Scene.mousePressedX) * root.fineAdjustmentFactor
                    if (root.value + calculatedAdjustedValue < root.from) return;
                    if (root.value + calculatedAdjustedValue > root.to) return;

                    root.adjusted(calculatedAdjustedValue)
                    return;
                }

                // direct, coarse adjustment
                var normalizedValue = Math.min(Math.max(0, mouse.x / root.width), 1)
                root.moved(priv.fromNormalized(normalizedValue))
            }

            onDoubleClicked: root.moved(root.defaultValue)
        }
    }
}
