import QtQuick 2.0

Item {
    id: window
    width: 100; height: 100

    Rectangle {
        id: mainRect
        width: 30; height: 30
    }

    Item { id: myContainer }

    Component {
        id: myComponent
        Rectangle {
            width: 50; height: 50
            color: "red"
            onWidthChanged: console.log("[Rectangle onWidthChanged()]", "width", width)
        }
    }

    function loadItems() {
        //var item = myComponent.createObject(myContainer, {"width": mainRect.width})
        var item = myComponent.createObject(myContainer, {"width": Qt.binding(function(){ return mainRect.width })})
        if (item == null) {
            console.log("[loadItems()]", "ERROR", "Component::createObject()")
        }
    }

    Component.onCompleted: {
        loadItems()
        mainRect.width = 31
        mainRect.width = 32
        mainRect.width = 33
    }
//    Rectangle {
//        width: mainRect.width; height: 50
//        color: "red"
//        onWidthChanged: console.log("[Rectangle onWidthChanged()]", "width", width)
//    }

//    Component.onCompleted: {
//        mainRect.width = 31
//        mainRect.width = 32
//        mainRect.width = 33
//    }
}
