import QtQuick 2.0

Item {
    id: window
    width: 100; height: 100

    Rectangle {
        id: button_1
        x: 0; y: 0
        width: 25; height: 25
        color: "red"

        Component.onCompleted: {
            var obj = mapToItem(null, x, y)
            console.log("[button_1]", "x:", x, "y:", y, "mapToItem().x:", obj.x, "mapToItem().y", obj.y)
        }
    }

    Rectangle {
        id: button_2
        x: 0; y: 25
        width: 25; height: 25
        color: "pink"

        Component.onCompleted: {
            //var obj = mapToItem(null, x, y)
            var obj = mapToItem(null, width / 2, height / 2)
            console.log("[button_2]", "x:", x, "y:", y, "mapToItem().x:", obj.x, "mapToItem().y", obj.y)
        }
    }
}
