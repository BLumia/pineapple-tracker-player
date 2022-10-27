import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Control {
    id: instrumentItem
    clip: true

    property string instrumentName
    property string instrumentIndex
    property string fontFamily
    property bool muted
    signal clicked()

    height: 30

    background: Rectangle {
        anchors.fill: parent
        color: instrumentItem.hovered ? "#1B1C20" : "#0B0C10"
    }

    contentItem: RowLayout {
        anchors.fill: parent
        spacing: 10

        Item {}

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            width: instrumentItem.height / 2
            height: width
            color: instrumentItem.muted ? "#2B2C30" : "#00439F"
        }

        Text {
            Layout.fillWidth: true
            font.family: instrumentItem.fontFamily
            text: instrumentItem.instrumentName
            color: "#ffffff"
        }

        Text {
            font.family: instrumentItem.fontFamily
            text: "#" + instrumentItem.instrumentIndex
            color: "#ffffff"
        }

        Item {}
    }

    MouseArea {
        anchors.fill: parent
        onClicked: instrumentItem.clicked()
    }
}
