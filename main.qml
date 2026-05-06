import QtQuick 6.4
import QtQuick.Controls 6.4
import QtMultimedia 6.4

ApplicationWindow {
    visible: true
    width: 600
    height: 500
    title: "RTSP Viewer"

    Column {
        anchors.fill: parent
        spacing: 20
        padding: 20

        TextField {
            id: rtspInput
            width: parent.width
            placeholderText: "Enter RTSP URL"
        }

        Button {
            text: "Start Stream"

            onClicked: {
                player.source = rtspInput.text
                player.play()
            }
        }

        Rectangle {
            width: parent.width
            height: parent.height - 120
            color: "black"

            VideoOutput {
                id: videoOut
                anchors.fill: parent
            }
        }
    }

    MediaPlayer {
        id: player
        videoOutput: videoOut   // KEY FIX (instead of VideoOutput.source)
        //autoPlay: false
    }
}

// // [run]
// ./appQtRTSP 

// //[build]
// cd ~/qt_rtsp_app 
// mkdir -p build
// cd build
// cmake ..
// make

// //rtsp server run
// ./rtsp_server
// LIVE RTSP: rtsp://127.0.0.1:8554/test

