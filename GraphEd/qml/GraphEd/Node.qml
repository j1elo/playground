import QtQuick 2.0

Item {
    id: root
    width: 500; height: 500

    property string nodeId: "node_-1"

    // These are list<Socket>
    property alias socketsTop: socketContainerTop.sockets
    property alias socketsBottom: socketContainerBottom.sockets
    property alias socketsLeft: socketContainerLeft.sockets
    property alias socketsRight: socketContainerRight.sockets

    signal nodePositionChanged()

    Rectangle {
        id: body
        width: 100; height: 100
//        width: 4 * socketContainerLeft.width
//        height: socketContainerLeft.height
        color: "lightgrey"
        border.width: 2
        border.color: "white"

        Text {
            anchors.centerIn: parent
            text: "ACME"
            font.family: "Helvetica"
            font.pointSize: 14
            style: Text.Outline
            color: "red"
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            drag.target: parent
            onPositionChanged: root.nodePositionChanged()
        }
    }

    SocketContainer {
        id: socketContainerTop
        anchors {
            bottom: body.top
            horizontalCenter: body.horizontalCenter
        }
        socketContainerId: nodeId + "_top"
        socketSpacing: 5
        orientation: Qt.Horizontal

        Connections {
            target: mouseArea
            onPositionChanged: socketContainerTop.socketPosUpdate()
        }
    }

    SocketContainer {
        id: socketContainerBottom
        anchors {
            top: body.bottom
            horizontalCenter: body.horizontalCenter
        }
        socketContainerId: nodeId + "_bottom"
        socketSpacing: 5
        orientation: Qt.Horizontal

        Connections {
            target: mouseArea
            onPositionChanged: socketContainerBottom.socketPosUpdate()
        }
    }

    SocketContainer {
        id: socketContainerLeft
        anchors {
            right: body.left
            verticalCenter: body.verticalCenter
        }
        socketContainerId: nodeId + "_left"
        socketSpacing: 5
        orientation: Qt.Vertical

        Connections {
            target: mouseArea
            onPositionChanged: socketContainerLeft.socketPosUpdate()
        }
    }

    SocketContainer {
        id: socketContainerRight
        anchors {
            left: body.right
            verticalCenter: body.verticalCenter
        }
        socketContainerId: nodeId + "_right"
        socketSpacing: 5
        orientation: Qt.Vertical

        Connections {
            target: mouseArea
            onPositionChanged: socketContainerRight.socketPosUpdate()
        }
    }
}
