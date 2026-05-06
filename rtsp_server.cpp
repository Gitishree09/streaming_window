// #include <gst/gst.h>
// #include <gst/rtsp-server/rtsp-server.h>

// int main(int argc, char *argv[])
// {
//     gst_init(&argc, &argv);

//     GstRTSPServer *server = gst_rtsp_server_new();
//     GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

//     GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

//     gst_rtsp_media_factory_set_launch(factory,
//         "( filesrc location=/home/gitishree/clean.mp4 ! "
//         "qtdemux ! h264parse ! rtph264pay name=pay0 pt=96 )"
//     );

//     gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
//     g_object_unref(mounts);

//     gst_rtsp_server_attach(server, NULL);

//     g_print("RTSP READY: rtsp://127.0.0.1:8554/test\n");

//     GMainLoop *loop = g_main_loop_new(NULL, FALSE);
//     g_main_loop_run(loop);

//     return 0;
// }

// // rtsp://172.29.191.96:8554/test

//for live camera video streaming
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <unistd.h>   // for access()

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    // Check camera exists (IMPORTANT)
    if (access("/dev/video0", F_OK) == -1) {
        g_printerr("Camera NOT found (/dev/video0). Server not started.\n");
        return -1;
    }

    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    // DO NOT reuse old pipeline
    gst_rtsp_media_factory_set_shared(factory, FALSE);

    // LIVE CAMERA PIPELINE
    gst_rtsp_media_factory_set_launch(factory,
        "( v4l2src device=/dev/video0 ! "
        "videoconvert ! "
        "x264enc tune=zerolatency bitrate=512 speed-preset=ultrafast ! "
        "rtph264pay name=pay0 pt=96 config-interval=1 )"
    );

    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);

    // g_print("LIVE RTSP READY: rtsp://127.0.0.1:8554/test\n");

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
