import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt.labs.platform 1 as Platform
import Pineapple.TrackerPlayer 1

ApplicationWindow {
    visible: true
    width: 600
    height: 500
    // QtQuick.Controls do have MenuBar and related stuff, but it don't have the native look and feel.
    Platform.MenuBar {
        Platform.Menu {
            title: qsTr("&File")

            Platform.MenuItem {
                text: qsTr("&Open")
                shortcut: StandardKey.Open
                onTriggered: () => {
                    fileDialog.open()
                }
            }
        }

        Platform.Menu {
            title: qsTr("Options")

            Platform.MenuItem {
                text: qsTr("Set Mono &Font")
                onTriggered: () => {
                    fontDialog.open()
                }
            }
        }
    }

    DropArea {
        id: root
        anchors.fill: parent
        onEntered: (drag) => {
            drag.accept(Qt.LinkAction);
        }
        onDropped: (drop) => {
            if (drop.urls.length <= 0) return;
            player.load(drop.urls[0]);
            player.play();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            Label {
                id: titleLabel
                text: "Song Title (Drop file to load/play)"
            }
            Label {
                id: artistTrackerLabel
                text: "Artist (Tracker)"
            }
            Slider {
                id: progressSlider
                Layout.fillWidth: true
                value: player.currentOrder
                to: player.totalOrders - 1
                stepSize: 1
                onMoved: () => {
                    player.seek(progressSlider.value)
                }
            }
            RowLayout {
                Button {
                    text: "Play"
                    onClicked: {
                        player.play()
                    }
                }
                Button {
                    text: "Pause"
                    onClicked: {
                        player.pause()
                    }
                }
                Button {
                    text: "Toggle Compact"
                    onClicked: {
                        trackerContainer.compactView = !trackerContainer.compactView
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                Slider {
                    id: gainSlider
                    from: -1500
                    to: 1000
                    value: 0
                    onMoved: () => {
                        player.setGain(gainSlider.value)
                    }
                }
            }
            Label {
                id: playbackStatusLabel
                text: "Playback Status"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stackLayout.currentIndex = stackLayout.currentIndex === 0 ? 1 : 0
                    }
                }
            }
            StackLayout {
                id: stackLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    TextArea {
                        id: messageArea
                        readOnly: true
                        textFormat: Text.PlainText
                        font.family: fontDialog.selectedFont.family
                    }
                }
                TrackerView {
                    id: trackerContainer
                    channelCount: player.qml_channelCount
                    rowCount: player.qml_rowCount
                    currentRow: player.currentRow
                    patternContent: player.qml_patternContent
                }
            }
        }
    }

    // Non-widgets:
    TrackerPlayer {
        id: player

        property int qml_channelCount
        property int qml_rowCount
        property var qml_patternContent: [[]] // how to declare a vector<QStringList> ?

        onFileLoaded: {
            titleLabel.text = player.title()
            let artist = player.artist()
            artistTrackerLabel.text = artist === "" ? player.tracker() : artist + " (" + player.tracker() + ")"
            messageArea.text = player.message()
            qml_channelCount = player.totalChannels()
        }
        onCurrentPatternChanged: {
            qml_rowCount = player.patternTotalRows(player.currentPattern)
            qml_patternContent = player.patternContent(player.currentPattern)
        }

        onCurrentRowChanged: {
            let order = player.currentOrder.toString().padStart(3, " ")
            let orders = player.totalOrders.toString().padStart(3, " ")
            let row = player.currentRow.toString().padStart(3, " ")
            let rows = player.patternTotalRows(player.currentPattern)
            let epoch = new Date(1970, 0, 1)
            epoch.setSeconds(player.positionSec)
            let time = epoch.toLocaleTimeString(Qt.locale("C"), "h:mm:ss")
            playbackStatusLabel.text = `Order: ${order}/${orders} Row: ${row}/${rows} Time: ${time}`
        }
    }

    Component.onCompleted: {
        if (fileList.length > 0) {
            player.load(fileList[0]);
            player.play();
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select module file"
        nameFilters: [
            "Module Files (*.xm *.it *.mod *.s3m *.mptm)"
        ]
        onAccepted: {
            player.load(fileDialog.currentFile);
            player.play();
        }
    }

    FontDialog {
        id: fontDialog
        selectedFont.family: monoFontFamily
        options: FontDialog.MonospacedFonts
    }
}
