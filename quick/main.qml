import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Pineapple.TrackerPlayer 1

ApplicationWindow {
    visible: true
    width: 600
    height: 500
    // QQC2 do have MenuBar and related stuff, but it don't have the native look and feel.
    // Qt.labs.platform 1 has a native one but it will cause linking issue while using
    // qt6-static 6.2.1-2 from MSYS2, so switch to QQC2 for now...
    menuBar: MenuBar {
        id: appMenu
        Menu {
            title: qsTr("&File")

            Action {
                text: qsTr("&Open")
                shortcut: StandardKey.Open
                onTriggered: () => {
                    fileDialog.open()
                }
            }
        }

        Menu {
            title: qsTr("Options")

            Action {
                text: qsTr("Set Mono &Font")
                onTriggered: () => {
                    fontDialog.open()
                }
            }

            Action {
                text: qsTr("Toggle &Menu Bar Visibility")
                shortcut: "Ctrl+Shift+M"
                onTriggered: () => {
                    appMenu.visible = !appMenu.visible
                }
            }
        }

        Menu {
            title: qsTr("&Help")

            Action {
                text: qsTr("About")
                onTriggered: () => {
                    aboutDialog.open()
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
            playlistManager.loadPlaylist(drop.urls)
            player.load(drop.urls[0]);
            player.play();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            Label {
                id: titleLabel
                text: qsTr("Song Title (Drop file to load/play)")
            }
            Label {
                id: artistTrackerLabel
                text: qsTr("Artist (Tracker)")
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
                    text: player.isPlaying ? qsTr("Pause") : qsTr("Play")
                    onClicked: {
                        player.isPlaying ? player.pause() : player.play()
                    }
                }
                Button {
                    property bool replayMode: false
                    text: replayMode ? qsTr("Replay") : qsTr("Repeat")
                    onClicked: {
                        replayMode = !replayMode
                        if (replayMode) {
                            // replay
                            player.repeatCount = 1
                            player.restartAfterFinished = true
                        } else {
                            // repeat
                            player.repeatCount = 0
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
                Slider {
                    id: gainSlider
                    from: -2000
                    to: 1000
                    value: player.gain
                    onMoved: () => {
                        player.gain = value
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Label {
                    id: playbackStatusLabel
                    text: qsTr("Playback Status")
                }
                Item {
                    Layout.fillWidth: true
                }
                Label {
                    text: "Playlist"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackLayout.currentIndex = 3
                        }
                    }
                }
                Label {
                    text: "Message"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackLayout.currentIndex = 0
                        }
                    }
                }
                Label {
                    text: "Pattern"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (stackLayout.currentIndex !== 1) {
                                stackLayout.currentIndex = 1
                                return
                            }
                            trackerContainer.compactView = !trackerContainer.compactView
                        }
                    }
                }
                Label {
                    text: "Instruments"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            stackLayout.currentIndex = 2
                        }
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
                InstrumentView {
                    id: instrumentsContainer
                    instruments: player.qml_instrumentNames
                }
                ListView {
                    id: playlistView
                    model: playlistManager.model
                    clip: true
                    delegate: ItemDelegate {
                        width: parent.width
                        text: model.display
                        onClicked: function() {
                            playlistManager.currentIndex = index
                            player.load(model.url);
                            player.play();
                        }
                    }
                }
            }
        }
    }

    // Non-widgets:
    TrackerPlayer {
        id: player

        property int qml_channelCount
        property int qml_rowCount
        property var qml_instrumentNames: [] // since we cannot declare list<string>
        property var qml_patternContent: [[]] // how to declare a vector<QStringList> ?

        onFileLoaded: {
            titleLabel.text = player.title()
            let artist = player.artist()
            artistTrackerLabel.text = artist === "" ? player.tracker() : artist + " (" + player.tracker() + ")"
            messageArea.text = player.message()
            qml_channelCount = player.totalChannels()
            qml_instrumentNames = player.instrumentNames()
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

    PlaylistManager {
        id: playlistManager
        autoLoadFilterSuffixes: ["*.xm", "*.it", "*.mod", "*.s3m", "*.mptm"]
    }

    Component.onCompleted: {
        if (fileList.length > 0) {
            playlistManager.loadPlaylist(fileList)
            player.load(fileList[0]);
            player.play();
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Select module file")
        nameFilters: [
            "Module Files (*.xm *.it *.mod *.s3m *.mptm)"
        ]
        onAccepted: {
            playlistManager.loadPlaylist(fileDialog.currentFile)
            player.load(fileDialog.currentFile);
            player.play();
        }
    }

    FontDialog {
        id: fontDialog
        selectedFont.family: monoFontFamily
        options: FontDialog.MonospacedFonts
    }

    Dialog {
        id: aboutDialog
        title: qsTr("About")
        anchors.centerIn: parent
        ColumnLayout {
            Label {
                textFormat: Text.MarkdownText
                text: `Pineapple Tracker Player

Based on the following free software libraries:

- [Qt](https://www.qt.io/)
- [PortAudio](https://www.portaudio.com/)
- [libopenmpt](https://lib.openmpt.org/libopenmpt/)

[Source Code](https://github.com/BLumia/pineapple-tracker-player)

Copyright &copy; 2025 [BLumia](https://github.com/BLumia/)
`
                onLinkActivated: function(link) {
                    Qt.openUrlExternally(link)
                }
            }
            DialogButtonBox {
                standardButtons: DialogButtonBox.Ok
                onAccepted: aboutDialog.close()
            }
        }
    }
}
