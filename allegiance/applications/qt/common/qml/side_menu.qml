import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import Schneider

ColumnLayout {
    id: root
    width: 360
    height: 1080

    Material.theme: Material.Dark
    Material.accent: Material.Blue

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
