import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Schneider

Rectangle {
    color: Style.background

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 100
            Layout.preferredWidth: 100
            Layout.maximumWidth: 300
            Layout.minimumHeight: 50

            Image {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                source: "kdab.png"
                fillMode: Image.PreserveAspectFit
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.openUrlExternally("https://www.kdab.com")
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 100
            Layout.preferredWidth: 100
            Layout.maximumWidth: 300
            Layout.minimumHeight: 50
            Image {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5

                source: "schneider_white.svg"
                fillMode: Image.PreserveAspectFit
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.openUrlExternally("https://www.schneider-digital.com/")
                }
            }
        }
    }
}
