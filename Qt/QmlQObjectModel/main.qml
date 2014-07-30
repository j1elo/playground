import QtQuick 2.2
import QtQuick.Controls 1.2

import MyApp.DataItems 1.0

ApplicationWindow {
    visible: true
    width: 640; height: 480

    // Avoid having to test everywhere against 'null',
    // by always having a valid Item instance
    property ParentItem defaultItem: ParentItem {}
    property ParentItem dataItem: defaultItem
    onDataItemChanged: {
        if (!dataItem)
            dataItem = defaultItem;
    }

    signal buttonClicked(int index)

    Row {
        anchors.centerIn: parent
        Column {
            Repeater {
                model: 3
                Button { text: "Item "+index; onClicked: buttonClicked(index) }
            }
        }
        Grid {
            columns: 3

            Label { text: " " }
            Label { text: "ParentItem" }
            Label { text: "ChildItem" }

            Label { text: "Text:" }
            TextField {
                id: tf1
                onEditingFinished: dataItem.text = text
                Binding { target: tf1; property: "text"; value: dataItem.text }
            }
            TextField {
                id: tf2
                onEditingFinished: dataItem.child.text = text
                Binding { target: tf2; property: "text"; value: dataItem.child.text }
            }

            Label { text: "Check:" }
            CheckBox {
                id: ck1
                onClicked: dataItem.check = checked
                Binding { target: ck1; property: "checked"; value: dataItem.check }
            }
            CheckBox {
                id: ck2
                onClicked: dataItem.child.check = checked
                Binding { target: ck2; property: "checked"; value: dataItem.child.check }
            }

            Label { text: "Type:" }
            ComboBox {
                id: cb1
                model: itemTypes
                onActivated: dataItem.type = index
                Binding { target: cb1; property: "currentIndex"; value: dataItem.type }
            }
            ComboBox {
                id: cb2
                model: itemTypes
                onActivated: dataItem.child.type = index
                Binding { target: cb2; property: "currentIndex"; value: dataItem.child.type }
            }
        }
    }

    ListModel {
        id: itemTypes
        ListElement { data: "type0"; text: "The Type 0" }
        ListElement { data: "type1"; text: "The Type 1" }
        ListElement { data: "type2"; text: "The Type 2" }
    }
}
