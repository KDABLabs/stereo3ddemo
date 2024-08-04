import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

Column {
  id: root
  default property alias item: ld.sourceComponent
  property string title: "--"
  Item {
    height: 40
    anchors.margins: 0
    anchors.left: parent.left
    anchors.right: parent.right

    RowLayout {
      anchors.fill: parent
      spacing: 0

      Label {
        text: root.title
        font: Style.fontButton
      }
      Label {
          anchors{
              right: parent.right
              top: parent.top
              bottom: parent.bottom
              margins: 10
          }
          horizontalAlignment: Text.AlignRight
          verticalAlignment: Text.AlignVCenter
          text: "^"
          font: Style.fontButton
          rotation: info.show ? "180" : 0
      }

      MouseArea {
        anchors.fill: parent
        onClicked: info.show = !info.show
      }
    }
  }
  Item {
    id: info
    anchors.left: parent.left
    anchors.right: parent.right
    height: show ? ld.height : 0
    anchors.margins: 0
    property bool show : true
    clip: true
    Loader {
      id: ld
      y: info.height - height
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.horizontalCenter: info.horizontalCenter
    }
    Behavior on height {
      NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
    }
  }
}