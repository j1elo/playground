import QtQuick 2.0

// Note
// Use qsTr("String") for translatable strings in UI.

/**
  Basic Graph Editor.
  This sample program allows you to create "Node" instances, which are movable entities
  containing arbitrary contents (in this case it's just text), and allowing to connect them
  by means of "Link"s, which will maintain the connection even while moving the connected Nodes.

  Link connections are actually made between "Socket"s, which are specific placeholders
  contained inside each Node instance. Every Link is drawn inside a QML Canvas element,
  maintaining it's connection between two Sockets.
  */

Item {
    id: window
    width: 600
    height: 600

    // Tool items.
    Row {
        id: toolbar
        spacing: 16
        anchors {
            bottom: window.bottom
            left: window.left
            right: window.right
            bottomMargin: 25
            leftMargin: 50
            rightMargin: 50
        }

        Tool {
            id: newNodeTool
            width: 35
            height: 35
            source: "qrc:/qml/GraphEd/img/add.png"
            onClicked: newNode()
        }

        // "Delete Node" tool.
        //        Tool {
        //            id: clearAllTool
        //            source: "images/clear.png"
        //            onClicked: pagePanel.currentPage.clear()
        //        }
    }


    // --------------------------------------------------------------
    // Node creation
    // --------------------------------------------------------------

    // This Item contains all the created Node instances.
    Item { id: nodeContainer }

    // Component which will create Node instances.
    Component {
        id: nodeComponent
        Node {
            Component.onCompleted: parent = nodeContainer

            // Default values.
            x: 75; y: 25
            onNodePositionChanged: linkCanvas.canvas.requestPaint()
            socketsLeft: [
                Socket { id: socket_left_1; size: 25; onClicked: socketClicked(socket_left_1) }
                , Socket { id: socket_left_2; size: 25; onClicked: socketClicked(socket_left_2) }
            ]
            socketsRight: [
                Socket { id: socket_right_1; size: 25; onClicked: socketClicked(socket_right_1) }
                , Socket { id: socket_right_2; size: 25; onClicked: socketClicked(socket_right_2) }
            ]
        }
    }

    // When this Component is loaded, call loadNodes() to load nodes from the database.
    //Component.onCompleted: loadNodes()

    // Create Node instances.
    function newNodeObject(propertyMap) {
        // "nodeContainer": the parent of the new object.
        // "propertyMap": a map of property-value pairs to be set on the object.
        var node = nodeComponent.createObject(nodeContainer, propertyMap)
        if (node == null) {
            console.log("ERROR: creating Node object")
        }
    }

    // Read the Node data from DB.
//    function loadNodes() {
//        var nodeItems = NodeDB.readNodes()
//        for (var i in nodeItems) {
//            newNodeObject(nodeItems[i])
//        }
//    }

    // Convenience function which creates a default constructed Node.
    function newNode() {
        newNodeObject( {} )
    }

    // Iterate through the children elements of the
    // container item and call destroy() for deleting them.
//    function clear() {
//        for(var i = 0; i < container.children.length; ++i) {
//            container.children[i].destroy()
//        }
//    }


    // --------------------------------------------------------------
    // Link creation
    // --------------------------------------------------------------

    // This Item contains all the created Link instances.
    Item { id: linkContainer }

    // Component which will create Link instances.
    Component {
        id: linkComponent
        Link {
            Component.onCompleted: parent = linkContainer
        }
    }

    // Create Link instances.
    function newLinkObject(propertyMap) {
        // "linkContainer": the parent of the new object.
        // "propertyMap": a map of property-value pairs to be set on the object.
        var link = linkComponent.createObject(linkContainer, propertyMap)
        if (link == null) {
            console.log("ERROR: creating Link object")
        }
    }

    // Convenience function which creates a Link between two Sockets.
    function newLink(tailSocket, headSocket) {
        newLinkObject({
                          "tailX": Qt.binding(function(){ return tailSocket.posX }),
                          "tailY": Qt.binding(function(){ return tailSocket.posY }),
                          "headX": Qt.binding(function(){ return headSocket.posX }),
                          "headY": Qt.binding(function(){ return headSocket.posY })
                      })
        linkCanvas.canvas.requestPaint()
    }


    // --------------------------------------------------------------
    // Socket click
    // --------------------------------------------------------------

    property Socket linkTailSocket

    function socketClicked(socket) {
        // Switch the Canvas state between "Normal" and "Follow mouse".
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


    // --------------------------------------------------------------
    // Left Nodes
    // --------------------------------------------------------------

    Node {
        id: node_1
        nodeId: "node_1"
        x: 25; y: 150
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_1_socket_1; size: 25; onClicked: socketClicked(node_1_socket_1) }
            , Socket { id: node_1_socket_2; size: 25; onClicked: socketClicked(node_1_socket_2) }
        ]
    }

    Node {
        id: node_2
        nodeId: "node_2"
        x: 25; y: 275
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_2_socket_1; size: 25; onClicked: socketClicked(node_2_socket_1) }
            , Socket { id: node_2_socket_2; size: 25; onClicked: socketClicked(node_2_socket_2) }
        ]
    }

    Node {
        id: node_3
        nodeId: "node_3"
        x: 25; y: 400
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsRight: [
            Socket { id: node_3_socket_1; size: 25; onClicked: socketClicked(node_3_socket_1) }
            , Socket { id: node_3_socket_2; size: 25; onClicked: socketClicked(node_3_socket_2) }
        ]
    }


    // --------------------------------------------------------------
    // Right Nodes
    // --------------------------------------------------------------

    Node {
        id: node_4
        nodeId: "node_4"
        x: 400; y: 150
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_4_socket_1; size: 25; onClicked: socketClicked(node_4_socket_1) }
            , Socket { id: node_4_socket_2; size: 25; onClicked: socketClicked(node_4_socket_2) }
        ]
    }

    Node {
        id: node_5
        nodeId: "node_5"
        x: 400; y: 275
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_5_socket_1; size: 25; onClicked: socketClicked(node_5_socket_1) }
            , Socket { id: node_5_socket_2; size: 25; onClicked: socketClicked(node_5_socket_2) }
        ]
    }

    Node {
        id: node_6
        nodeId: "node_6"
        x: 400; y: 400
        onNodePositionChanged: linkCanvas.canvas.requestPaint()
        socketsLeft: [
            Socket { id: node_6_socket_1; size: 25; onClicked: socketClicked(node_6_socket_1) }
            , Socket { id: node_6_socket_2; size: 25; onClicked: socketClicked(node_6_socket_2) }
        ]
    }


    // --------------------------------------------------------------
    // Canvas for drawing Links
    // --------------------------------------------------------------

    LinkCanvas {
        id: linkCanvas
        z: -1
        anchors.fill: window
        links: linkContainer.children
    }
}
