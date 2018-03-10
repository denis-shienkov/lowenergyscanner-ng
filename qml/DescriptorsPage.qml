import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import qt.example.com 1.0

ListView {
    model: descriptorsModel
    delegate: Button {
        width: parent.width
        height: 100
        text: qsTr("%1\n%2\n%3").arg(name).arg(uuid).arg(value)
    }
}
