import QtQuick 2.0

Item {
    id: root
    width: orientation == Qt.Vertical ? socketSize : m_socketCount * socketSize + (m_socketCount - 1) * socketSpacing
    height: orientation == Qt.Horizontal ? socketSize : m_socketCount * socketSize + (m_socketCount - 1) * socketSpacing

    property string socketContainerId: "node_-1"

    property int orientation: Qt.Vertical
    //property int orientation: Qt.Horizontal

    property list<Socket> sockets
    property int m_socketCount: sockets.length

    property real socketSize: m_socketCount ? sockets[0].size : 25
    property real socketSpacing: socketSize

    function socketPosUpdate() {
        //console.log("SocketContainer socketPosUpdate")
        for (var i = 0; i < m_socketCount; ++i) {
            sockets[i].posUpdate()
        }
    }

    Component.onCompleted: {
        //console.log("SocketContainer onCompleted")
        //for (var i = 0; i < m_socketCount; ++i) {
            //console.log("SocketContainer", sockets[i].socketId, "x", sockets[i].x, "y", sockets[i].y)
        //}

        socketsChanged()
        socketPosUpdate()
    }

    onSocketsChanged: {
        for (var i = 0; i < m_socketCount; ++i) {
            if (orientation == Qt.Vertical)
                sockets[i].parent = columnContainer
            else
                sockets[i].parent = rowContainer
        }

//        Alternative (doesn't work for initial values).
//        if (orientation == Qt.Vertical)
//            sockets[m_socketCount - 1].parent = columnContainer
//        else
//            sockets[m_socketCount - 1].parent = rowContainer
    }

//    debug
//    Rectangle {
//        anchors.fill: root
//        color: "brown"
//    }

    Column {
        id: columnContainer
        anchors {
            fill: parent
//            topMargin: socketSize
        }
        spacing: socketSpacing
    }

    Row {
        id: rowContainer
        anchors {
            fill: parent
//            leftMargin: socketSize
        }
        spacing: socketSpacing
    }
}
