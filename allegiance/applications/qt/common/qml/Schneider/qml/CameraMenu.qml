import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

ColumnLayout {
    id: camera_details

    component GroupBoxTitleLabel : Label {
        font: Style.fontDefault
        text: parent.title
    }

    GridLayout {
        Layout.fillWidth: true
        columnSpacing: 10

        // Camera Model
        Label {
            Layout.alignment: Qt.AlignLeft
            text: "Camera Model"
            font: Style.fontDefault
            Layout.column: 0
        }
        ComboBox {
            Layout.fillWidth: true
            model: ["ToeIn", "AsymmetricFrustum"]
            currentIndex: Camera.stereoMode
            onCurrentIndexChanged: Camera.stereoMode = currentIndex
            Layout.columnSpan: 2
            Layout.column: 1
        }

        // Display Mode
        Label {
            Layout.alignment: Qt.AlignLeft
            text: "Display Mode"
            font: Style.fontDefault
            Layout.column: 0
            Layout.row: 1
        }
        ComboBox {
            Layout.fillWidth: true
            model: [
                { value: Camera.CameraMode.Stereo, text: "Stereo" },
                { value: Camera.CameraMode.Mono, text: "Mono" },
                { value: Camera.CameraMode.Left, text: "Left Eye" },
                { value: Camera.CameraMode.Right, text: "Right Eye"}
            ]
            textRole: "text"
            valueRole: "value"
            currentIndex: Camera.cameraMode
            onCurrentIndexChanged: {
                if (currentIndex !== -1 && Camera.cameraMode != currentIndex) {
                    Camera.cameraMode = currentIndex;
                }
            }
            Layout.columnSpan: 2
            Layout.column: 1
            Layout.row: 1
        }

        CheckBoxX {
            title: "Show Frustum"
            initial: Camera.frustumViewEnabled
            onChecked: checkValue => Camera.frustumViewEnabled = checkValue
            Layout.column: 0
            Layout.columnSpan: 3
            Layout.row: 2
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: "Focus"
        label: GroupBoxTitleLabel{}

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            CheckBoxX {
                title: "Auto Focus"
                initial: Camera.autoFocus
                onChecked: checkValue => Camera.autoFocus = checkValue
            }

            CheckBoxX {
                title: "Show AF Area"
                initial: Camera.showAutoFocusArea
                onChecked: checkValue => Camera.showAutoFocusArea = checkValue
            }

            SliderValue {
                Layout.fillWidth: true
                from: 0.1
                to: 100
                title: "Distance:"
                unit: "%"
                value: Camera.focusDistance
                onMoved: current => Camera.focusDistance = current
                enabled: !Camera.autoFocus
            }

            SliderValue {
                Layout.fillWidth: true
                from: -100
                to: 100
                title: "Pop Out:"
                unit: "%"
                value: Camera.popOut
                onMoved: current => Camera.popOut = current
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: "Eye Separation"
        label: GroupBoxTitleLabel{}

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            SliderValue {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                from: 0.01
                to: 25
                value: Camera.eyeDistance
                title: "Separation"
                precision: 4
                unit: "m"

                onMoved: current => Camera.eyeDistance = current

                ToolTip.visible: hovered
                ToolTip.text: "F3 -  F12 +"
            }

            CheckBoxX {
                Layout.fillWidth: true
                title: "Flipped"
                initial: Camera.flipped
                onChecked: bchecked => Camera.flipped = bchecked
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: "Field of View"
        label: GroupBoxTitleLabel{}

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            CheckBoxX {
                id: fovByPhysicalDimCheckBox
                title: "By Physical Dimension"
                Layout.fillWidth: true
                initial: Camera.fovByPhysicalDim
                onChecked: bchecked => Camera.fovByPhysicalDim = bchecked
            }

            SliderValue {
                Layout.fillWidth: true
                from: 0.1
                to: 2
                title: "Screen Height"
                enabled: fovByPhysicalDimCheckBox.isChecked
                unit: "m"
                value: Camera.screenHeight
                onMoved: current => Camera.screenHeight = current
            }

            SliderValue {
                Layout.fillWidth: true
                from: 0.1
                to: 10.0
                title: "Viewer Distance"
                enabled: fovByPhysicalDimCheckBox.isChecked
                unit: "m"
                value: Camera.viewerDistance
                onMoved: current => Camera.viewerDistance = current
            }

            SliderValue {
                Layout.fillWidth: true
                from: 10
                to: 180
                title: "FOV"
                unit: "°"
                value: Camera.fov
                onMoved: current => Camera.fov = current
                enabled: !fovByPhysicalDimCheckBox.isChecked
            }
        }
    }
}
