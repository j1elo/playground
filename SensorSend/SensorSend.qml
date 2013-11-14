import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Window 2.0
import QtSensors 5.0

ApplicationWindow {
    id: root
    title: "Irontek Sensor Sender"
    width: 640
    height: 480
    
//    menuBar: MenuBar {
//        Menu {
//            title: qsTr("File")
//            MenuItem {
//                text: qsTr("Exit")
//                onTriggered: Qt.quit();
//            }
//        }
//    }
    
    Button {
        text: "Start"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    Accelerometer {
            id: accel

            // See http://qt-project.org/doc/qt-5.1/qtsensors/qaccelerometer.html#AccelerationMode-enum
            accelerationMode: Combined // Gravity // User

            dataRate: 100
            active:true

            onReadingChanged: {
                var newx = (bubble.x + calcRoll(accel.reading.x, accel.reading.y, accel.reading.z) * .1)
                var newy = (bubble.y - calcPitch(accel.reading.x, accel.reading.y, accel.reading.z) * .1)

                if (newx < 0)
                    newx = 0

                if (newx > mainPage.width - bubble.width)
                    newx = mainPage.width - bubble.width

                if (newy < 18)
                    newy = 18

                if (newy > mainPage.height - bubble.height)
                    newy = mainPage.height - bubble.height

                    bubble.x = newx
                    bubble.y = newy
            }
        }

}
