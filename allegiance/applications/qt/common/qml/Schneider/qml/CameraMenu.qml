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

    component ToolTipLabel: Label {
        ToolTip.visible: hovered && ToolTip.text.length > 0
        ToolTip.timeout: -1
        ToolTip.delay: Application.styleHints.mousePressAndHoldInterval

        readonly property bool hovered: labelMa.containsMouse
        font: Style.fontDefault
        MouseArea {
            id: labelMa
            anchors.fill: parent
            hoverEnabled: true
        }
    }

    function join(textStrs)
    {
        let str = "";
        for (let idx in textStrs) {
            str += textStrs[idx] + "\n";
        }
        return str
    }

    GridLayout {
        Layout.fillWidth: true
        columnSpacing: 10

        // Camera Model
        ToolTipLabel {
            Layout.alignment: Qt.AlignLeft
            text: "Camera Model"
            Layout.column: 0
            ToolTip.text: join(["Select the camera model:",
                                " - Asymmetric Frustum: Accurate stereo projection with sheared frustums.",
                                " - Toe-In: Simplistic method with two perspective cameras."])
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
        ToolTipLabel {
            Layout.alignment: Qt.AlignLeft
            text: "Display Mode"
            Layout.column: 0
            Layout.row: 1
            ToolTip.text:  join(["Select the display mode:",
                                 "- Stereo: renders in stereo with left and right eye cameras.",
                                 "- Mono: render using a centered camera between left and right eyes.",
                                 "- Left Eye: render using the left eye camera only.",
                                 "- Right Eye: render using the right eye camera only."])
        }
        ComboBox {
            Layout.fillWidth: true
            model: [
                { value: Camera.DisplayMode.Stereo, text: "Stereo" },
                { value: Camera.DisplayMode.Mono, text: "Mono" },
                { value: Camera.DisplayMode.Left, text: "Left Eye" },
                { value: Camera.DisplayMode.Right, text: "Right Eye"}
            ]
            textRole: "text"
            valueRole: "value"
            currentIndex: Camera.displayMode
            onCurrentIndexChanged: {
                if (currentIndex !== -1 && Camera.displayMode != currentIndex) {
                    Camera.displayMode = currentIndex;
                }
            }
            Layout.columnSpan: 2
            Layout.column: 1
            Layout.row: 1
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
                ToolTip.text: join(["Automatically adjusts focus within the auto focus area.",
                                    "Use Ctrl+RMB to focus manually, when switched off.",
                                    "F2"])
            }

            CheckBoxX {
                title: "Show AF Area"
                enabled: Camera.autoFocus
                visible: enabled
                initial: Camera.showAutoFocusArea
                onChecked: checkValue => Camera.showAutoFocusArea = checkValue
                ToolTip.text: join(["Shows the auto focus area handle.",
                                    "Shift + F2"])
            }

            CheckBoxX {
                title: "Show Focus Plane"
                initial: Camera.showFocusPlane
                onChecked: checkValue => Camera.showFocusPlane = checkValue
                ToolTip.text: "Shows the focus plane"
            }

            SliderValue {
                Layout.fillWidth: true
                from: 0.1
                to: 100
                title: "Distance:"
                precision: 0
                unit: "%"
                value: Camera.focusDistance
                defaultValue: Camera.focusDistanceDefaultValue
                onMoved: current => Camera.focusDistance = current
                enabled: !Camera.autoFocus
                ToolTip.text: join(["Sets the focus plane distance as a % of the camera near and far planes.",
                                    "(Shift) F3 -",
                                    "(Shift) F4 +",
                                    "Auto set distance to 3D Cursor intersection with Ctrl+RMB"])
            }

            SliderValue {
                Layout.fillWidth: true
                from: -100
                to: 100
                title: "Pop Out:"
                unit: "%"
                precision: 0
                defaultValue: Camera.popOutDefaultValue
                value: Camera.popOut
                onMoved: current => Camera.popOut = current
                ToolTip.text: join(["Controls if the object appears inside the screen or pops out of the screen.",
                                    "(Shift) F5 -",
                                    "(Shift) F6 +"])
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: "Camera Separation"
        label: GroupBoxTitleLabel{}

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            NonLinearSliderValue {
                enabled: !_1_30th_separation_mode_checkbox.isChecked
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                from: 0.1
                mid: 6.5
                to: 50
                defaultValue: Camera.eyeDistanceDefaultValue * 100
                value: Camera.eyeDistance * 100
                title: "Separation"
                precision: 2
                unit: ""

                onMoved: current => Camera.eyeDistance = current * 0.01

                ToolTip.text: join(["Sets the virtual distance between the cameras for the left and the right eye.",
                                    "(Shift) F7 -",
                                    "(Shift) F8 +"])
            }

            CheckBoxX {
                Layout.fillWidth: true
                title: "Flipped"
                initial: Camera.flipped
                onChecked: bchecked => Camera.flipped = bchecked

                ToolTip.text: "Flip the Right and Left cameras."
            }

            CheckBoxX {
                id: _1_30th_separation_mode_checkbox
                Layout.fillWidth: true
                title: "Set to 1/%1th of Focus Distance".arg(Camera.separationBasedOnFocusDistanceDivider)
                initial: Camera.separationBasedOnFocusDistance
                onChecked: bchecked => Camera.separationBasedOnFocusDistance = bchecked

                ToolTip.text: "Automatically set the separation as 1/30th of the focus distance."
            }

            SliderValue {
                Layout.fillWidth: true
                from: 10
                visible: Camera.separationBasedOnFocusDistance
                defaultValue: Camera.separationBasedOnFocusDistanceDividerDefaultValue
                to: 50
                precision: 0
                title: "Divider:"
                value: Camera.separationBasedOnFocusDistanceDivider
                onMoved: current => Camera.separationBasedOnFocusDistanceDivider = Math.round(current / 1)
                ToolTip.text: "Dividers by which we device the focusDistance to obtain the eyeSeparation"
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
                ToolTip.text: "Set FOV by physical dimension of projection area and viewer distance."
            }

            NonLinearSliderValue {
                Layout.fillWidth: true
                from: 0.1
                mid: 0.5
                to: 2
                title: "Screen Height"
                enabled: fovByPhysicalDimCheckBox.isChecked
                visible: enabled
                unit: "m"
                value: Camera.screenHeight
                defaultValue: Camera.screenHeightDefaultValue
                onMoved: current => Camera.screenHeight = current
                ToolTip.text: "The physical HEIGHT of the screen or projection area respectively in meters."
            }

            NonLinearSliderValue {
                Layout.fillWidth: true
                from: 0.1
                mid: 2
                to: 10.0
                title: "Viewer Distance"
                enabled: fovByPhysicalDimCheckBox.isChecked
                visible: enabled
                unit: "m"
                value: Camera.viewerDistance
                defaultValue: Camera.viewerDistanceDefaultValue
                onMoved: current => Camera.viewerDistance = current
                ToolTip.text: "The physical viewer distance to the screenin meters."
            }

            SliderValue {
                Layout.fillWidth: true
                from: 10
                to: 170
                title: "FOV"
                unit: "°"
                precision: 1
                value: Camera.fov
                defaultValue: Camera.fovDefaultValue
                onMoved: current => Camera.fov = current
                enabled: !fovByPhysicalDimCheckBox.isChecked
                ToolTip.text: "Set field of view (vertical angle) directly."
            }
        }
    }

    ButtonX {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        text: "View All"
        onClicked: {
            Camera.viewAll()
        }
    }
}
