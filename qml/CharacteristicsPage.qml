import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

import qt.example.com 1.0

ListView {
    model: characteristicsModel
    delegate: Button {
        width: parent.width
        contentItem: ColumnLayout {
            spacing: 0
            Label {
                id: captionsLabel
                text: qsTr("%1\n%2\n%3").arg(name).arg(uuid).arg(props)
                font.capitalization: Font.AllUppercase
                horizontalAlignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }
            RowLayout {
                id: controlButtonsLayout
                ToolButton {
                    visible: writable
                    text: qsTr("W")
                    onClicked: writeDialog.openDialog(uuid);
                }
                ToolButton {
                    visible: readable
                    text: qsTr("R")
                    onClicked: characteristicsModel.read(uuid)
                }
                ToolButton {
                    visible: notifyable
                    text: qsTr("N")
                    checkable: true
                    checked: notificationEnabled
                    onClicked: characteristicsModel.enableNotification(uuid, checked);
                }
                ToolButton {
                    visible: indicatable
                    text: qsTr("I")
                    checkable: true
                    checked: indicationEnabled
                }
                Layout.alignment: Qt.AlignHCenter
            }
            Label {
                id: valueLabel
                visible: readable || notifyable || indicatable
                text: qsTr("%1").arg(value)
                font.capitalization: Font.AllUppercase
                horizontalAlignment: Qt.AlignHCenter
                Layout.fillWidth: true
            }
        }
        onClicked: {
            errorPopup.close();
            var service = characteristicsModel.service();
            descriptorsModel.update(service, uuid);
            stackView.push("qrc:/qml/DescriptorsPage.qml");
        }
    }

    Dialog {
        id: writeDialog

        function openDialog(uuid) {
            characteristicUuid = uuid;
            textInput.clear();
            open();
        }

        property string characteristicUuid;

        width: 200; height: 200
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        title: "Write value"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: TextField {
            id: textInput
            validator: RegExpValidator { regExp: /[0-9A-Fa-f]+/ }
        }

        onAccepted: characteristicsModel.write(characteristicUuid, textInput.text)
    }
}
