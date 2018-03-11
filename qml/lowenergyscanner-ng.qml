import QtQuick 2.10
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import qt.example.com 1.0

ApplicationWindow {
    visible: true
    width: 360
    height: 640
    title: qsTr("Bluetooth LE Scanner")

    Material.accent: Material.primary

    header: ToolBar {
        Material.foreground: "white"

        height: 50

        RowLayout {
            anchors.fill: parent
            spacing: 0

            ImageButton {
                id: searchButton
                visible: stackView.depth === 1
                source: "qrc:/images/search.png"
                onClicked: {
                    errorLabel.text = qsTr("");
                    devicesModel.update();
                }
                Layout.fillHeight: true
            }

            ImageButton {
                id: backButton
                visible: stackView.depth > 1
                source: "qrc:/images/back.png"
                onClicked: {
                    if (stackView.depth > 1) {
                        stackView.pop();
                        errorLabel.text = qsTr("");
                    } else {
                        // do something
                    }
                }
                Layout.fillHeight: true
            }

            ComboBox {
                id: searchTimeoutsBox
                enabled: !devicesModel.running
                model: [ 1000, 2000, 5000 ]
            }

            Label {
                id: titleLabel
                visible: stackView.depth > 0
                text: {
                    switch (stackView.depth) {
                    case 1:
                        return qsTr("Devices");
                    case 2:
                        return qsTr("Services");
                    case 3:
                        return qsTr("Characteristics");
                    case 4:
                        return qsTr("Descriptors");
                    default:
                        return qsTr("");
                    }
                }

                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        onDepthChanged: {
            if (depth > 1) {

            } else {

            }
        }

        Component.onCompleted: stackView.push("qrc:/qml/DevicesPage.qml");
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: devicesModel.running || servicesModel.running || characteristicsModel.running
        Label {
            anchors.centerIn: parent
            text: {
                if (devicesModel.running)
                    return qsTr("Scanning for devices");
                else if (servicesModel.running)
                    return qsTr("Scanning for services");
                else if (characteristicsModel.running)
                    return qsTr("Scanning for characteristics");
                else
                    return qsTr("")
            }
        }
    }

    Label {
        id: errorLabel
        anchors.centerIn: parent
        color: "red"
    }

    DevicesModel {
        id: devicesModel
        discoveryTimeout: searchTimeoutsBox.currentText
        onErrorOccurred: errorLabel.text = errorString;
    }

    ServicesModel {
        id: servicesModel
        onErrorOccurred: errorLabel.text = errorString;
    }

    CharacteriticsModel {
        id: characteristicsModel
        onErrorOccurred: errorLabel.text = errorString;
    }

    DescriptorsModel {
        id: descriptorsModel
    }
}
