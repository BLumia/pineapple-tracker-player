import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt.labs.platform 1 as Platform

Item {
    id: instrumentsContainer
    clip: true

    property list<string> instruments

    ListView {
        clip: true
        anchors.fill: parent
        orientation: ListView.Vertical
        flickableDirection: Flickable.VerticalFlick
        model: instrumentsContainer.instruments
        delegate: Button {
            checkable: true
            checked: true
            width: ListView.view.width
            height: 30
            text: modelData === "" ? index : modelData
            onPressed: () => {
                player.setInstrumentMuteStatus(index, checked)
                console.log("instrument toggle mute", index, checked)
            }
        }
    }
}
