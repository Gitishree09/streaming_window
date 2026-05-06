// #include <QGuiApplication>
// #include <QQmlApplicationEngine>
// #include <QApplication>
// #include <QWidget>
// #include <QVBoxLayout>
// #include <QPushButton>
// #include <QLineEdit>
// #include <QHBoxLayout>
// #include <QLabel>
// #include <QDebug>
// #include <QMessageBox>

// #include <gst/gst.h>

// int main(int argc, char *argv[])
// {
//     gst_init(&argc, &argv);
//     QApplication app(argc, argv);

//     QWidget window;
//     window.setWindowTitle("RTSP Viewer");
//     window.resize(900, 600);

//     QVBoxLayout *mainLayout = new QVBoxLayout(&window);
//     mainLayout->setSpacing(15);
//     mainLayout->setContentsMargins(15, 15, 15, 15);

//     // Title
//     QLabel *title = new QLabel("RTSP Camera Viewer");
//     title->setStyleSheet("font-size: 22px; font-weight: bold;");
//     title->setAlignment(Qt::AlignCenter);

//     // Input + Button
//     QHBoxLayout *topBar = new QHBoxLayout();

//     QLineEdit *input = new QLineEdit();
//     input->setPlaceholderText("Enter RTSP URL (rtsp://...)");
//     input->setMinimumHeight(40);

//     QPushButton *button = new QPushButton("▶ Start Stream");
//     button->setMinimumHeight(40);

//     topBar->addWidget(input);
//     topBar->addWidget(button);

//     // Placeholder box (no embedding in WSL)
//     QWidget *videoWidget = new QWidget();
//     videoWidget->setStyleSheet("background:black; border: 2px solid #333;");

//     mainLayout->addWidget(title);
//     mainLayout->addLayout(topBar);
//     mainLayout->addWidget(videoWidget, 1);

//     GstElement *pipeline = nullptr;

//     QObject::connect(button, &QPushButton::clicked, [&]() {

//         // Stop previous stream
//         if (pipeline) {
//             gst_element_set_state(pipeline, GST_STATE_NULL);
//             gst_object_unref(pipeline);
//             pipeline = nullptr;
//         }

//         QString url = input->text().trimmed();

//         if (url.isEmpty()) {
//             QMessageBox::warning(&window, "Error", "Enter RTSP URL");
//             return;
//         }

//         // WSL-safe pipeline (NO embedding)
//         // QString pipelineStr = QString(
//         //     "rtspsrc location=\"%1\" latency=0 ! "
//         //     "decodebin ! videoconvert ! "
//         //     "ximagesink sync=false"
//         // ).arg(url);

//         QString pipelineStr = QString(
//             "rtspsrc location=\"%1\" latency=0 ! "
//             "decodebin ! videoconvert ! "
//             "autovideosink"
//         ).arg(url);

//         qDebug() << "Pipeline:" << pipelineStr;

//         GError *error = nullptr;
//         pipeline = gst_parse_launch(pipelineStr.toStdString().c_str(), &error);

//         if (error) {
//             g_printerr("Error: %s\n", error->message);
//             g_error_free(error);
//             return;
//         }

//         // Start pipeline
//         gst_element_set_state(pipeline, GST_STATE_PLAYING);
//     });

//     window.show();
//     return app.exec();
// }

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

// Global UI
QLabel *videoLabel = nullptr;
GstElement *pipeline = nullptr;

// CLEAR SCREEN FUNCTION
void showNoSignal()
{
    QMetaObject::invokeMethod(videoLabel, []() {
        videoLabel->setText("No Signal / Camera Not Found");
        videoLabel->setStyleSheet("color: white; background:black;");
        videoLabel->setAlignment(Qt::AlignCenter);
    });
}

// FRAME CALLBACK
GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer)
{
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) return GST_FLOW_ERROR;

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *s = gst_caps_get_structure(caps, 0);

    int width, height;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    QImage img((uchar*)map.data, width, height, QImage::Format_RGB888);

    QMetaObject::invokeMethod(videoLabel, [img]() {
        videoLabel->setPixmap(QPixmap::fromImage(img).scaled(
            videoLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        ));
    });

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

// ERROR HANDLING (VERY IMPORTANT)
gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer)
{
    switch (GST_MESSAGE_TYPE(msg)) {

    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error(msg, &err, &debug);

        qDebug() << "GStreamer Error:" << err->message;

        g_error_free(err);
        g_free(debug);

        showNoSignal();   // CLEAR SCREEN
        break;
    }

    case GST_MESSAGE_EOS:
        qDebug() << "Stream ended";
        showNoSignal();   // CLEAR SCREEN
        break;

    default:
        break;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("RTSP Viewer (Embedded)");
    window.resize(900, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    QLabel *title = new QLabel("RTSP Camera Viewer");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 22px; font-weight: bold;");

    QHBoxLayout *topBar = new QHBoxLayout();

    QLineEdit *input = new QLineEdit();
    input->setPlaceholderText("Enter RTSP URL (rtsp://...)");

    QPushButton *button = new QPushButton("▶ Start Stream");

    topBar->addWidget(input);
    topBar->addWidget(button);

    videoLabel = new QLabel("No Video");
    videoLabel->setStyleSheet("background:black; color:white;");
    videoLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(title);
    mainLayout->addLayout(topBar);
    mainLayout->addWidget(videoLabel, 1);

    QObject::connect(button, &QPushButton::clicked, [&]() {

        // Stop old pipeline
        if (pipeline) {
            gst_element_set_state(pipeline, GST_STATE_NULL);
            gst_object_unref(pipeline);
            pipeline = nullptr;
        }

        QString url = input->text().trimmed();

        if (url.isEmpty()) {
            QMessageBox::warning(&window, "Error", "Enter RTSP URL");
            return;
        }

        // PIPELINE (appsink)
        QString pipelineStr = QString(
            "rtspsrc location=\"%1\" latency=0 ! "
            "decodebin ! videoconvert ! video/x-raw,format=RGB ! "
            "appsink name=sink"
        ).arg(url);

        qDebug() << "Pipeline:" << pipelineStr;

        GError *error = nullptr;
        pipeline = gst_parse_launch(pipelineStr.toStdString().c_str(), &error);

        if (error) {
            qDebug() << "Error:" << error->message;
            g_error_free(error);
            showNoSignal();
            return;
        }

        // Attach BUS (error listener)
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_add_watch(bus, bus_call, NULL);
        gst_object_unref(bus);

        // appsink setup
        GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

        gst_app_sink_set_emit_signals((GstAppSink*)sink, true);
        gst_app_sink_set_drop((GstAppSink*)sink, true);

        g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

        gst_object_unref(sink);

        // Start
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    });

        // if (stream_not_available) {
        //     showFallbackVideo();
        //     return;
        // }

    window.show();
    return app.exec();
}

//User enters RTSP URL → clicks button →
//GStreamer pipeline starts →
//Video is rendered inside Qt widget