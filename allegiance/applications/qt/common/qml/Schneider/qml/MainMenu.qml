import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Schneider

ScrollView {
    Layout.fillWidth: true
    Layout.fillHeight: true
    anchors.left: parent.left
    anchors.right: parent.right

    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.horizontal.interactive: false

    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    ScrollBar.vertical.interactive: true

    ColumnLayout {
        width: root.width - 40

        ButtonX {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            text: "Load Model"
            onClicked: {
                Scene.OpenLoadModelDialog()
            }
        }
        Pane {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            RowLayout {
                SectionTitle {
                    Layout.maximumWidth: 40
                    Layout.alignment: Qt.AlignRight
                    id: camera_section_title
                }
                Label {
                    text: "Camera"
                    font: Style.fontButton
                }
            }
        }

        GroupBox {
            id: camera
            visible: camera_section_title.isVisible
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            ColumnLayout {
                anchors.margins: 10
                anchors.left: parent.left
                anchors.right: parent.right
                id: camera_details

                SliderValue {
                    id: eyesep
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    from: 0.01
                    to: 25
                    value: Camera.eyeDistance
                    title: "Eye Separation"

                    onMoved: current => Camera.eyeDistance = current

                    ToolTip.visible: eyesep.hovered
                    ToolTip.text: "F3 -  F12 +"
                }

                SliderValue {
                    id: focus
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.bottomMargin: 20
                    from: 79.0
                    to: 89.9
                    value: Camera.focusAngle
                    title: "Z Focus"

                    onMoved: current => Camera.focusAngle = current
                }

                SliderValue {
                    id: fov
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.bottomMargin: 20
                    from: 30.0
                    to: 89.9
                    value: Camera.fov
                    title: "FOV"

                    onMoved: current => Camera.fov = current
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 10
                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: "Display Mode:"
                        font: Style.fontDefault
                    }
                    ComboBox {
                        model: [{
                            value: Camera.CameraMode.Stereo,
                            text: "Stereo"
                        }, {
                            value: Camera.CameraMode.Mono,
                            text: "Mono"
                        }, {
                            value: Camera.CameraMode.Left,
                            text: "Left Eye"
                        }, {
                            value: Camera.CameraMode.Right,
                            text: "Right Eye"
                        }]
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: Camera.cameraMode
                        onCurrentIndexChanged: {
                            if (currentIndex != -1 && Camera.cameraMode != currentIndex) {
                                Camera.cameraMode = currentIndex;
                            }
                        }
                    }
                }
                CheckBoxX {
                    text: "Eyes Flipped"
                    initial: Camera.flipped
                    onChecked: bchecked => Camera.flipped = bchecked
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 10
                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: "Stereo Mode:"
                        font: Style.fontDefault
                    }
                    ComboBox {
                        model: ["ToeIn", "AsymmetricFrustum"]
                        currentIndex: Camera.stereoMode
                        onCurrentIndexChanged: Camera.stereoMode = currentIndex
                    }
                }

                CheckBoxX {
                    text: "Show Frustum"
                    initial: Camera.frustumViewEnabled
                    onChecked: checkValue => Camera.frustumViewEnabled = checkValue
                }

            }
        }

        Pane {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            RowLayout {
                SectionTitle {
                    Layout.maximumWidth: 40
                    Layout.alignment: Qt.AlignRight
                    id: cursor_section_title
                }
                Label {
                    text: "Cursor"
                    font: Style.fontButton
                }
            }
        }
        CursorMenu {
            visible: cursor_section_title.isVisible
        }

        Pane {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            RowLayout {
                SectionTitle {
                    Layout.maximumWidth: 40
                    Layout.alignment: Qt.AlignRight
                    id: navigation_section_title
                }
                Label {
                    text: "Navigation"
                    font: Style.fontButton
                }
            }
        }

        GroupBox {
            id: navigation
            visible: navigation_section_title.isVisible
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            label: Label {
                text: "Navigation"
                font: Style.fontButton
            }

            ColumnLayout {
                anchors.margins: 10
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 10

                SliderValue {
                    id: mousesensitivity
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    from: 20
                    to: 300
                    value: Scene.mouseSensitivity
                    title: "Mouse Sensitivity"

                    onMoved: current => Scene.mouseSensitivity = current
                }
                /*      Still needs to be implemented on Application Side
                            Item {
                                Layout.fillWidth: true
                                height: 30
                                anchors.margins: 10
                                Button {
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.topMargin: 10
                                    anchors.bottomMargin: 10
                                    text: "Reset Pivot Point"
                                }
                                Button {
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    anchors.topMargin: 10
                                    anchors.bottomMargin: 10
                                    text: "Reset Camera Position"
                                }
                            }
                 */
            }
        }
    }
}
