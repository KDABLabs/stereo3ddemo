import QtQuick
import QtQuick.Controls
import Schneider

Button {
    property bool isVisible: true
    text: isVisible ? "▼" : "▶"
    onClicked: isVisible = !isVisible
    width: 50
}
