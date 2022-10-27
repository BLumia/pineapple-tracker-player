import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt.labs.platform 1 as Platform
import "./components"

Item {
    id: instrumentsContainer
    clip: true

    property list<string> instruments

    onInstrumentsChanged: () => {
        instrumentModel.clear()
        instruments.forEach((instrument) => {
            instrumentModel.append({
                name: instrument,
                mute: false
            })
        })
        console.log(instrumentModel)
    }

    ListModel {
        id: instrumentModel

        ListElement {
            name: "Instrument will be displayed here"
            mute: false
        }
    }

    ListView {
        clip: true
        anchors.fill: parent
        orientation: ListView.Vertical
        flickableDirection: Flickable.VerticalFlick
        model: instrumentModel
        delegate: InstrumentItem {
            width: ListView.view.width
            fontFamily: fontDialog.selectedFont.family
            muted: mute
            instrumentName: name
            instrumentIndex: index
            onClicked: () => {
                instrumentModel.set(index, {'mute': !mute})
                player.setInstrumentMuteStatus(index, muted)
            }
        }
    }
}
