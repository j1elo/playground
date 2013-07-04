//import QtQuick 1.1
import QtQuick 2.0

Image {
    id: root

    signal clicked()

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
