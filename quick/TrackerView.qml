import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt.labs.platform 1 as Platform

Item {
    id: trackerContainer
    clip: true

    property int currentRow: 0
    property int rowCount: 1
    property int channelCount: 1
    property var patternContent
    readonly property int colWidth: 40

    onChannelCountChanged:() => {
        console.log("changed:", channelCount)
    }

    ListView {
        id: trackerListView
        anchors.fill: parent

//        interactive: false
        currentIndex: currentRow
        orientation: ListView.Vertical

        model: trackerContainer.patternContent
        preferredHighlightBegin: height / 2
        preferredHighlightEnd: height / 2
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightMoveDuration: 0 // disable the highlight scrolling animation
        contentWidth: trackerContainer.channelCount * trackerContainer.colWidth
        flickableDirection: Flickable.HorizontalFlick

        headerPositioning: ListView.OverlayHeader
        header: Rectangle {
            height: 50
            z: 2 // keep header on the top of pattern rows
            color: "grey"
            width: parent.width
            ListView {
                interactive: false
                anchors.fill: parent
                model: trackerContainer.channelCount
                orientation: ListView.Horizontal
                delegate: Button {
                    flat: true
                    width: trackerContainer.colWidth
                    height: 50
                    checkable: true
                    checked: true
                    text: index
                    onPressed: () => {
                        player.setChannelMuteStatus(index, checked)
                        console.log(index, checked)
                    }
                }
            }
        }

        // The rows
        delegate: Rectangle {
            id: delegateRow
            height: 30
            color: "blue"
            width: trackerContainer.channelCount * trackerContainer.colWidth
            ListView {
                interactive: false
                anchors.fill: parent
                model: modelData
                orientation: ListView.Horizontal
                delegate: Rectangle {
                    color: delegateRow.ListView.isCurrentItem ? "yellow" : "green"
                    width: trackerContainer.colWidth
                    height: 30
                    Text {
                        text: `<pre>${modelData}</pre>`;
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        color: delegateRow.ListView.isCurrentItem ? "green" : "yellow"
                    }
                }
            }
        }
    }
}
