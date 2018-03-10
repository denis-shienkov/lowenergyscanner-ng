import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import qt.example.com 1.0

ListView {
    model: servicesModel
    delegate: Button {
        width: parent.width
        height: 70
        text: qsTr("%1\n%2").arg(name).arg(uuid)
        onClicked: {
            errorLabel.text = qsTr("");
            var service = servicesModel.service(uuid);
            characteristicsModel.update(service);
            stackView.push("qrc:/qml/CharacteristicsPage.qml");
        }
    }
}
