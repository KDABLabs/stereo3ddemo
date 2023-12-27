import QtQuick
import QtQuick.Controls
import Schneider

Button {
  id: button
  width: parent.width
  height: 50
  background: Rectangle {
    id: rect
    color: button.enabled ? Style.button : Style.buttonDisabled
    border.color: "transparent"
    border.width: 2
    radius: 0
    Behavior on color {
      ColorAnimation {
        duration: 200
      }
    }
  }
  contentItem: Text {
    text: button.text
    font: Style.fontButton
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
    color: Style.buttonText
  }
  MouseArea {
    anchors.fill: parent
    hoverEnabled: true
    onEntered: {
      rect.color = Style.buttonHover;
    }
    onExited: {
      rect.color = button.enabled ? Style.button : Style.buttonDisabled;
    }
    onPressed: {
      rect.border.color = Style.accent;
    }
    onClicked: button.clicked()
    onReleased: {
      rect.border.color = "transparent";
    }
  }
}
