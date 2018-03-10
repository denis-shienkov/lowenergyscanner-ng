import QtQuick 2.9
import QtQuick.Controls 2.2

ToolButton {
    property alias source: image.source
    contentItem: Image {
        id: image
        fillMode: Image.Pad
        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter
    }
}
