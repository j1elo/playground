import QtQuick 2.0

// Note
// Use qsTr("String") for translatable strings in UI.

Item {
    id: window
    width: 600
    height: 600

    // Tool items.
    Column {
        id: toolbar
        spacing: 16
        anchors {
            top: window.top
            left: window.left
            bottom: window.bottom
            topMargin: 50
            bottomMargin: 50
            leftMargin: 8
        }

// NOTE - WIP - this would allow to add new Nodes. Not ready yet.
//        Tool {
//            id: newLinkTool
//            source: "qrc:/qml/GraphEd/img/add.png"
//            onClicked: newLink()
//        }

        // "Delete Node" tool.
        //        Tool {
        //            id: clearAllTool
        //            source: "images/clear.png"
        //            onClicked: pagePanel.currentPage.clear()
        //        }
    }

    // ------------ Node

    // Load the Node Component.
//    Component {
//        id: nodeComponent
//        Node { }
//    }

//    // Create an Item element that will be used as a Node container.
//    Item { id: container }

//    // When this Component is loaded,
//    // call loadNodes() to load nodes
//    // from the database.
//    //Component.onCompleted: loadNodes()

//    // Create Node QML objects.
//    function newNodeObject(propertyMap) {
//        // Call createObject() on previously declared
//        // Node Component; 'container' will be the parent
//        // of the new object and 'propertyMap' is a map of property-value pairs.
//        var node = nodeComponent.createObject(container, propertyMap)
//        if (node == null) {
//            console.log("ERROR: creating Node object")
//        }
//    }

//    // Read the Node data from DB.
//    //    function loadNodes() {
//    //        var nodeItems = NodeDB.readNodes()
//    //        for (var i in nodeItems) {
//    //            newNodeObject(nodeItems[i])
//    //        }
//    //    }

//    // Create Node items, not load from DB.
//    function newNode() {
//        // Call the newNodeObject and possibly pass
//        // a set of arguments.
//        //newNodeObject( { "markerId": root.markerId } )
//        newNodeObject( { "x": 75, "y": 100 } )
//    }

//    // Iterate through the children elements of the
//    // container item and call destroy() for deleting them.
//    function clear() {
//        for(var i = 0; i < container.children.length; ++i) {
//            container.children[i].destroy()
//        }
//    }

    // ------------ Link

    // Load the Component which will create Link instances.
    Component {
        id: linkComponent
        Link {
            Component.onCompleted: parent = linkContainer
        }
    }

    // Use an Item as a Link container.
    Item { id: linkContainer }

    // Create Node QML objects.
    function newLinkObject(propertyMap) {
        // 'container' will be the parent of the new object
        // and 'propertyMap' is a map of property-value pairs.
        var link = linkComponent.createObject(linkContainer, propertyMap)
        if (link == null) {
            console.log("ERROR: creating Link object")
        }
    }

    function newLink(tailSocket, headSocket) {
        newLinkObject({
                          "tailX": Qt.binding(function(){ return tailSocket.posX }),
                          "tailY": Qt.binding(function(){ return tailSocket.posY }),
                          "headX": Qt.binding(function(){ return headSocket.posX }),
                          "headY": Qt.binding(function(){ return headSocket.posY })
                      })
        linkCanvas.canvas.requestPaint()
    }

    property Socket linkTailSocket

    function socketClicked(socket) {
        linkCanvas.enableMouseLink = !linkCanvas.enableMouseLink

        if (linkCanvas.enableMouseLink) {
            linkCanvas.mouseLink.tailX = socket.posX
            linkCanvas.mouseLink.tailY = socket.posY
            linkTailSocket = socket
        }
        else {
            newLink(linkTailSocket, socket)
        }
    }


    // ---- Left Nodes.

    Node {
        id: node_1
        nodeId: "node_1"
        x: 25; y: 25
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_1_socket_1; size: 25; onClicked: socketClicked(node_1_socket_1) }
            , Socket { id: node_1_socket_2; size: 25; onClicked: socketClicked(node_1_socket_2) }
        ]
    }

    Node {
        id: node_2
        nodeId: "node_2"
        x: 25; y: 150
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_2_socket_1; size: 25; onClicked: socketClicked(node_2_socket_1) }
            , Socket { id: node_2_socket_2; size: 25; onClicked: socketClicked(node_2_socket_2) }
        ]
    }

    Node {
        id: node_3
        nodeId: "node_3"
        x: 25; y: 275
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_3_socket_1; size: 25; onClicked: socketClicked(node_3_socket_1) }
            , Socket { id: node_3_socket_2; size: 25; onClicked: socketClicked(node_3_socket_2) }
        ]
    }

    Node {
        id: node_4
        nodeId: "node_4"
        x: 25; y: 400
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_4_socket_1; size: 25; onClicked: socketClicked(node_4_socket_1) }
            , Socket { id: node_4_socket_2; size: 25; onClicked: socketClicked(node_4_socket_2) }
        ]
    }


    // ---- Right Nodes.

    Node {
        id: node_5
        nodeId: "node_5"
        x: 400; y: 25
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_5_socket_1; size: 25; onClicked: socketClicked(node_5_socket_1) }
            , Socket { id: node_5_socket_2; size: 25; onClicked: socketClicked(node_5_socket_2) }
        ]
    }

    Node {
        id: node_6
        nodeId: "node_6"
        x: 400; y: 150
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_6_socket_1; size: 25; onClicked: socketClicked(node_6_socket_1) }
            , Socket { id: node_6_socket_2; size: 25; onClicked: socketClicked(node_6_socket_2) }
        ]
    }

    Node {
        id: node_7
        nodeId: "node_7"
        x: 400; y: 275
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_7_socket_1; size: 25; onClicked: socketClicked(node_7_socket_1) }
            , Socket { id: node_7_socket_2; size: 25; onClicked: socketClicked(node_7_socket_2) }
        ]
    }

    Node {
        id: node_8
        nodeId: "node_8"
        x: 400; y: 400
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_8_socket_1; size: 25; onClicked: socketClicked(node_8_socket_1) }
            , Socket { id: node_8_socket_2; size: 25; onClicked: socketClicked(node_8_socket_2) }
        ]
    }

//    Link {
//        id: link_1
//        tailX: node_1.rightSockets[1].posX
//        tailY: node_1.rightSockets[1].posY
//        headX: node_4.leftSockets[0].posX
//        headY: node_4.leftSockets[0].posY
//        Component.onCompleted: parent = linkContainer
//    }

//    Link {
//        id: link_2
//        tailX: node_2.socketsRight[1].posX
//        tailY: node_2.socketsRight[1].posY
//        headX: node_3.socketsLeft[1].posX
//        headY: node_3.socketsLeft[1].posY
//        Component.onCompleted: parent = linkContainer
//    }
//    Component.onCompleted: {
//        newLink(node_2.socketsRight[1], node_3.socketsLeft[1])
//        newLink(node_1.socketsRight[1], node_3.socketsLeft[1])
//    }

//    Link {
//        id: link_3
//        tailX: node_1.rightSockets[0].posX
//        tailY: node_1.rightSockets[0].posY
//        headX: node_3.leftSockets[0].posX
//        headY: node_3.leftSockets[0].posY
//        Component.onCompleted: parent = linkContainer
//    }

    LinkCanvas {
        id: linkCanvas
        z: -1
        anchors.fill: window
        links: linkContainer.children
    }
}
