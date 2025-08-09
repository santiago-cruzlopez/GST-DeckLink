#include <gst/gst.h>

int main(int argc, char *argv[]) {
    
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // build the pipeline
    pipeline = gst_parse_launch(
        "playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm",
        NULL);

    // Start playing
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait until error or EOS
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(
            bus, 
            GST_CLOCK_TIME_NONE, 
            GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    
    // Error handling
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        GError *err;
        gchar *debug_info;

        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error(&err);
        g_free(debug_info);
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
        g_print("End-Of-Stream reached.\n");
    }

    // Free resources
    gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}