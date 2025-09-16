#include <gst/gst.h>
#include <stdio.h>
#include <signal.h>
#include <glib.h>

static GMainLoop *loop = NULL;

static void handle_sigint(int sig) {
    if (loop) {
        g_print("Received SIGINT, stopping pipeline...\n");
        g_main_loop_quit(loop);
    }
}

static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *main_loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err = NULL;
            gchar *debug_info = NULL;
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
            g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error(&err);
            g_free(debug_info);
            g_main_loop_quit(main_loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached.\n");
            g_main_loop_quit(main_loop);
            break;
        case GST_MESSAGE_STATE_CHANGED: {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data)) {
                g_print("Pipeline state changed from %s to %s\n",
                        gst_element_state_get_name(old_state),
                        gst_element_state_get_name(new_state));
            }
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    GstElement *pipeline;
    GstBus *bus;
    GError *error = NULL;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create main loop
    loop = g_main_loop_new(NULL, FALSE);
    if (!loop) {
        g_printerr("Failed to create main loop.\n");
        return -1;
    }

    // Set up signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    const gchar *pipeline_str =
        "decklinkvideosrc device-number=3 connection=sdi mode=1080i5994 ! "
        "decklinkvideosink device-number=0 mode=1080i5994 "
        "decklinkaudiosrc device-number=3 ! decklinkaudiosink device-number=0";

    pipeline = gst_parse_launch(pipeline_str, &error);
    if (!pipeline || error) {
        g_printerr("Failed to create pipeline: %s\n", error ? error->message : "Unknown error");
        if (error) g_clear_error(&error);
        g_main_loop_unref(loop);
        return -1;
    }

    // Set up bus to handle messages
    bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, bus_callback, pipeline);

    // Set pipeline to playing state
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(bus);
        gst_object_unref(pipeline);
        g_main_loop_unref(loop);
        return -1;
    }

    g_print("Running pipeline...\n");
    g_main_loop_run(loop);

    g_print("Cleaning up...\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}