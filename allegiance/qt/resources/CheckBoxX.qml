/*****************************************************************************************
 *
 * Copyright (c) 2013, Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 * All rights reserved.
 *
 * See the LICENSE.txt file shipped along with this file for the license.
 *
 ******************************************************************************************/

import QtQuick
import Schneider

Item {
  id: checkbox
  signal checked(bool checkValue)

  implicitWidth: checkImage.implicitWidth + textItem.implicitWidth + spacing + margin * 2
  implicitHeight: checkImage.implicitHeight + spacing

  property alias text: textItem.text
  property real margin: 5
  property real spacing: textItem.text.length > 0 ? 5 : 0
  property bool initial: false

  Image {
    id: checkImage
    anchors.left: parent.left
    anchors.verticalCenter: checkbox.verticalCenter
    height: parent.height / 3
    fillMode: Image.PreserveAspectFit
  }

  Text {
    id: textItem
    anchors.left: checkImage.right
    anchors.leftMargin: spacing
    anchors.verticalCenter: checkImage.verticalCenter

    font: Style.fontDefault
    color: Style.text

    elide: Text.ElideRight
    width: checkbox.width - checkImage.width - margin * 2 - spacing
  }

  MouseArea {
    anchors.fill: parent
    onClicked: {
      parent.state = (parent.state == "checked") ? "unchecked" : "checked";
      parent.checked(parent.state == "checked");
    }
  }

  state: initial ? "checked" : "unchecked"

  states: [
    State {
      name: "checked"
      PropertyChanges {
        target: checkImage
        source: "checked.svg"
      }
    },
    State {
      name: "unchecked"
      PropertyChanges {
        target: checkImage
        source: "unchecked.svg"
      }
    }
  ]
}
