import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
    width: 800
    height: 600

    ListView {
        id: view1
        anchors { top: parent.top; left: parent.left; bottom: parent.bottom }
        width: parent.width / 2
        spacing: 5

        model: dataList.list
        delegate: Item {
            height: 30
            width: parent.width

            Text { text: model.value }

            MouseArea {
                anchors.fill: parent
                onClicked: model.value += "1";
            }
        }
    }

    ListView {
        id: view2
        anchors { top: parent.top; right: parent.right; bottom: parent.bottom }
        width: parent.width / 2
        spacing: 5

        model: dataList.list
        delegate: Item {
            height: 30
            width: parent.width

            Text { text: model.value }

            MouseArea {
                anchors.fill: parent
                onClicked: model.value += "2";
            }
        }
    }
}
