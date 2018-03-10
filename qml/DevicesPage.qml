import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import qt.example.com 1.0

ListView {
    model: devicesModel
    delegate: Button {
        width: parent.width
        height: 70
        text: qsTr("%1\n%2").arg(name).arg(address)
        onClicked: {
            errorLabel.text = qsTr("");
            servicesModel.update(address);
            stackView.push("qrc:/qml/ServicesPage.qml");
        }
    }
}
