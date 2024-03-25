import QtQuick 2.15
import QtQuick.Controls 2.15
import Schneider

Rectangle {
  required property string title
  readonly property StackView stack: StackView.view
  color: Style.background
}
