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
                enabled: !devicesModel.running
                visible: stackView.depth === 1
                source: "qrc:/images/search.png"
                onClicked: {
                    errorPopup.close();
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
                        errorPopup.close();
                        stackView.pop();
                    } else {
                        // do something
                    }
                }
                Layout.fillHeight: true
            }

            ComboBox {
                id: searchTimeoutsBox
                enabled: !devicesModel.running
                visible: stackView.depth === 1
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

    Popup {
        id: errorPopup

        function showError(errorString) {
            label.text = errorString;
            open();
            activityTimer.start();
        }

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        contentItem: Label {
            id: label
            color: "red"
        }

        onClosed: {
            activityTimer.stop();
            label.text = "";
        }

        Timer {
            id: activityTimer
            interval: 5000
            onTriggered: errorPopup.close();
        }
    }

    DevicesModel {
        id: devicesModel
        discoveryTimeout: searchTimeoutsBox.currentText
        onErrorOccurred: errorPopup.showError(errorString);
    }

    ServicesModel {
        id: servicesModel
        onErrorOccurred: errorPopup.showError(errorString);
    }

    CharacteriticsModel {
        id: characteristicsModel
        onErrorOccurred: errorPopup.showError(errorString);
    }

    DescriptorsModel {
        id: descriptorsModel
    }
}
