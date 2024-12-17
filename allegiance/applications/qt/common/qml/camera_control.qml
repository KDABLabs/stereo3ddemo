import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

Rectangle {
    id: root
    width: 360
    height: 1080
    color: Style.background

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: 10

        StackView {
            id: pageStack
            Layout.fillWidth: true
            Layout.fillHeight: true

            initialItem: MainMenu {
                id: mainMenu
            }
        }

        Logos {
            Layout.fillWidth: true
            height: 100
        }

    }

}
