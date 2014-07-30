import QtQuick 2.2
import QtQuick.Controls 1.2

ApplicationWindow {
    id: root
    visible: true
    width: 300; height: 300

    ListView {
        id: view
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: column.top
        model: model1
        spacing: 2
        delegate: Component {
            Rectangle {
                width: view.width
                implicitHeight: textInput.implicitHeight + 10
                color: "transparent"
                border.color: "red"
                border.width: 2
                radius: 5

                TextInput {
                    id: textInput
                    anchors.margins: 1.5 * parent.border.width
                    anchors.fill: parent
                    text: model.edit // "edit" role of the model, to break the binding loop
                    onTextChanged: model.display = text;
                }
            }
        }
    }

    Column {
        id: column;
        width: parent.width
        anchors.bottom: parent.bottom

        Text { text: "Type";  }
        ComboBox {
            id: box1
            width: parent.width
            model: model1
            textRole: "display"
            onCurrentTextChanged: generator.generate(currentText);
        }

        Text { text: "Unit"; }
        ComboBox {
            id: box2
            width: parent.width
            model: model2
            textRole: "display"
        }
    }
}
