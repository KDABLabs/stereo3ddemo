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
    anchors.margins: 10
    anchors.left: parent.left
    anchors.right: parent.right
    CheckBoxX {
      text: "Cursor Visible"
      initial: Cursor.visible
      onChecked: bchecked => Cursor.visible = bchecked
    }

    // Cursor Settings if visible
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

        // Cursor Type
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

        // Cursor Color
        RowLayout{
          id: colorlayout
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignTop
          spacing: 10
          Label {
            //Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            text: "Cursor Color:"
            font: Style.fontDefault
          }
          Button {
            id: button
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop

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
            onClicked: colorDialog.open()
          }
          ColorDialog {
              id: colorDialog
              selectedColor: Cursor.cursorTint
              onAccepted: Cursor.cursorTint = selectedColor
          }
        }

        // Cursor Scaling
        CheckBoxX {
          visible: Cursor.visible
          text: "Cursor Scaling"
          initial: Cursor.scalingEnabled
          onChecked: bchecked => Cursor.scalingEnabled = bchecked
        }

        // Cursor Factor
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

        // Cursor Focus
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
