import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

Rectangle {
  id: root
  width: 360
  height: 800
  color: Style.background

  ColumnLayout {
    id: layout
    anchors.fill: parent
    anchors.margins: 10

    Item {
      id: titleBar
      Layout.fillWidth: true
      height: titleText.height + 30

      Rectangle {
        id: backButton
        height: 25
        width: 33
        radius: 8
        color: Style.button
        visible: pageStack.depth > 1

        Text {
          anchors.centerIn: parent
          text: "Back"
          font: Style.fontDefault
          color: Style.text
        }

        MouseArea {
          anchors.fill: parent
          onClicked: pageStack.pop(StackView.Immediate)
        }
      }

      Rectangle {
        id: homeButton
        height: 25
        width: 33
        radius: 8
        color: Style.button
        visible: pageStack.depth > 1
        anchors.right: parent.right
        anchors.top: parent.top

        Text {
          anchors.centerIn: parent
          text: "Home"
          font: Style.fontDefault
          color: Style.text
        }

        MouseArea {
          anchors.fill: parent
          onClicked: {
            while (pageStack.depth > 1) {
              pageStack.pop();
            }
          }
        }
      }

      Text {
        id: titleText
        font: Style.fontH1
        color: Style.text
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: pageStack.currentItem.title
      }
    }

    StackView {
      id: pageStack
      Layout.fillWidth: true
      Layout.fillHeight: true

      initialItem: MainMenu {
        id: mainMenu
      }
    }
  }
}
