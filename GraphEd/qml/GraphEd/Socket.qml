import QtQuick 2.0

Rectangle {
    id: root
    width: size; height: size; radius: size
    color: mouseArea.containsMouse ? Qt.lighter("red") : "red"
    border.width: 3
    border.color: "white"

    property string socketId: "node_-1_socket_-1"
    property real size: 50
    property real posX
    property real posY

    // MouseArea::clicked(MouseEvent mouse)
    signal clicked(var mouse)  // QML bug? "Invalid signal parameter type: MouseEvent"
    //onClicked: console.log("[Socket onClicked]")

    //onSocketIdChanged: console.log("Socket", "socketId", socketId)
    //onPosXChanged: console.log("Socket", "socketId", socketId, "posX", posX)
    //onPosYChanged: console.log("Socket", "socketId", socketId, "posY", posY)
    onXChanged: posUpdate()
    onYChanged: posUpdate()

    function posUpdate() {
        var obj = mapToItem(null, width / 2.0, height / 2.0)
        posX = obj.x
        posY = obj.y
        //console.log("Socket posUpdate", "socketId", socketId, "posX", posX, "posY", posY)
        //console.log("Socket posUpdate", "root.x", root.x, "root.y", root.y, "mapToItem x", mapToItem(null, root.x, root.y).x, "mapToItem y", mapToItem(null, root.x, root.y).y)
    }

    Component.onCompleted: {
        posUpdate()
        //console.log("Socket onCompleted", "socketId", socketId)
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
//        onClicked: root.clicked(mouse)
        Component.onCompleted: {
            mouseArea.clicked.connect(root.clicked)
        }
    }
}
