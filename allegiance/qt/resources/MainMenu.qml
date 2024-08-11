import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Schneider

PageBase {
  id: page
  title: "Menu"

  ColumnLayout {
    anchors.left: parent.left
    anchors.right: parent.right

    ButtonX {
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignTop
      text: "Load Model"
      onClicked: {
        Scene.OpenLoadModelDialog()
      }
    }

    GroupBox {
      id: camera
      label: Label {
        text: "Camera"
        font: Style.fontButton
      }
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignTop

      ColumnLayout {
        anchors.margins: 10
        anchors.left: parent.left
        anchors.right: parent.right

        SliderValue {
          id: eyesep
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignTop
          from: 0.01
          to: 0.5
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
      }
    }

    GroupBox {
      id: cursor
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignTop
      label: Label {
        text: "Cursor"
        font: Style.fontButton
      }
      ColumnLayout {
        anchors.margins: 10
        anchors.left: parent.left
        anchors.right: parent.right
        CheckBoxX {
          text: "Cursor Visible"
          initial: Cursor.visible
          onChecked: bchecked => Cursor.visible = bchecked
        }
        Item{
          visible: Cursor.visible
          implicitWidth: cursorlayout.implicitWidth
          implicitHeight: cursorlayout.implicitHeight
		      ColumnLayout {
            id: cursorlayout
			      anchors.margins: 0
			      anchors.left: parent.left
			      anchors.right: parent.right
            spacing: 10

            RowLayout {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignTop
              spacing: 10
              Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                text: "Cursor Type:"
                font: Style.fontDefault
              }
              ComboBox {
                model: [{
                    value: Cursor.CursorType.Default,
                    text: "Default"
                  }, {
                    value: Cursor.CursorType.Cross,
                    text: "Cross"
                  }, {
                    value: Cursor.CursorType.CrossHair,
                    text: "Cross Hair"
                  }, {
                    value: Cursor.CursorType.Dot,
                    text: "Dot"
                  }]
                  textRole: "text"
                  valueRole: "value"
                  currentIndex: Cursor.cursor
                  onCurrentIndexChanged: {
                  if (currentIndex != -1 && Cursor.cursor != currentIndex) {
                    Cursor.cursor = currentIndex;
                  }
                }
              }
            }

            CheckBoxX {
              visible: Cursor.visible
              text: "Cursor Scaling"
              initial: Cursor.scalingEnabled
              onChecked: bchecked => Cursor.scalingEnabled = bchecked
            }
            SliderValue {
              id: scalefactor
    		      visible: Cursor.scalingEnabled
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignTop
              from: 0.1
              to: 2.0
              value: Cursor.scaleFactor
              title: "Scale Factor"
              onMoved: current => Cursor.scaleFactor = current
            }

            CheckBoxX {
              visible: Cursor.visible
              text: "Cursor Focus"
              initial: Cursor.cursorFocus
              onChecked: bchecked => Cursor.cursorFocus = bchecked
            }
          }
        }
      }
    }

    GroupBox {
        id: navigation
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
