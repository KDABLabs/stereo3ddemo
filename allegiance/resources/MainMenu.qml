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
        fileDialog.open();
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
          from: 0
          to: 0.5
          value: Camera.eyeDistance
          title: "Eye Separation"

          onMoved: current => Camera.eyeDistance = current
        }

        SliderValue {
          id: focus
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignTop
          Layout.bottomMargin: 20
          from: 60.0
          to: 89.9
          value: Camera.focusAngle
          title: "Eye Angle"

          onMoved: current => Camera.focusAngle = current
        }
        
        SliderValue {
          id: focusdist
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignTop
          Layout.bottomMargin: 20
          from: 0.1
          to: 50.0
          value: Camera.focusDistance
          title: "Focus Distance"

          onMoved: current => Camera.focusDistance = current
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
      }
    }

    FileDialog {
      id: fileDialog
      title: "Load Model"
      currentFolder: "./scene"
      nameFilters: ["Object files (*.obj *.fbx *.gltf)", "All Files (*)"]
      onAccepted: {
        Scene.model = fileDialog.selectedFile;
      }
      onRejected: {
        console.log("Rejected");
      }
    }
  }
}
