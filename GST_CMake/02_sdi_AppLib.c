/*
    1. Two Source Pipeline -> Processed Video/Audio -> Two Output Pipeline
    2. Input[3]: 1080i5994 -> Output[0] 1080p2997, format=BGR
    3. References:
        - https://gstreamer.freedesktop.org/documentation/applib/gstappsink.html?gi-language=c
        - https://gstreamer.freedesktop.org/documentation/applib/gstappsrc.html?gi-language=c
*/

#include <gst/gst.h>
#include <stdio.h>
#include <signal.h>
#include <glib.h>

static GMainLoop *loop = NULL;
static GstElement *video_src, *audio_src;

static void handle_sigint(int sig) {
    if (loop) {
        g_print("Received SIGINT, stopping pipelines...\n");
        g_main_loop_quit(loop);
    }
}

// Bus callback for handling pipeline messages
static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *main_loop = (GMainLoop *)data;
    GstElement *pipeline = (GstElement *)data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err = NULL;
            gchar *debug_info = NULL;
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Error from %s in pipeline %s: %s\n", 
                      GST_OBJECT_NAME(msg->src), GST_OBJECT_NAME(pipeline), err->message);
            g_printerr("Debugging info: %s\n", debug_info ? debug_info : "none");
            g_clear_error(&err);
            g_free(debug_info);
            g_main_loop_quit(main_loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached in pipeline %s.\n", GST_OBJECT_NAME(pipeline));
            g_main_loop_quit(main_loop);
            break;
        case GST_MESSAGE_STATE_CHANGED: {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
                g_print("Pipeline %s state changed from %s to %s\n",
                        GST_OBJECT_NAME(pipeline),
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

// Callback for new video sample from appsink
static void on_new_video_sample(GstElement *sink, gpointer user_data) {
    GstSample *sample = NULL;
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample) {
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        if (buffer && video_src) {
            GstFlowReturn ret;
            g_signal_emit_by_name(video_src, "push-buffer", buffer, &ret);
            if (ret != GST_FLOW_OK) {
                g_printerr("Failed to push video buffer: %d\n", ret);
            }
        }
        gst_sample_unref(sample);
    }
}

// Callback for new audio sample from appsink
static void on_new_audio_sample(GstElement *sink, gpointer user_data) {
    GstSample *sample = NULL;
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample) {
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        if (buffer && audio_src) {
            GstFlowReturn ret;
            g_signal_emit_by_name(audio_src, "push-buffer", buffer, &ret);
            if (ret != GST_FLOW_OK) {
                g_printerr("Failed to push audio buffer: %d\n", ret);
            }
        }
        gst_sample_unref(sample);
    }
}

int main(int argc, char *argv[]) {
    GstElement *video_capture_pipeline, *audio_capture_pipeline;
    GstElement *video_output_pipeline, *audio_output_pipeline;
    GstElement *video_sink, *audio_sink;
    GstBus *video_capture_bus, *audio_capture_bus;
    GstBus *video_output_bus, *audio_output_bus;
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

    const gchar *video_capture_pipeline_str =
        "decklinkvideosrc device-number=3 connection=sdi mode=1080i5994 drop-no-signal-frames=true do-timestamp=true ! "
        "deinterlace ! "
        "queue ! "
        "videoconvert ! videorate ! "
        "video/x-raw,format=BGR,width=1920,height=1080,interlace-mode=progressive,framerate=30000/1001 ! "
        "appsink name=video_sink emit-signals=true sync=false max-buffers=30 drop=false";

    video_capture_pipeline = gst_parse_launch(video_capture_pipeline_str, &error);
    if (!video_capture_pipeline || error) {
        g_printerr("Failed to create video capture pipeline: %s\n", error ? error->message : "Unknown error");
        if (error) g_clear_error(&error);
        g_main_loop_unref(loop);
        return -1;
    }
    g_object_set(video_capture_pipeline, "name", "video-capture-pipeline", NULL);

    const gchar *audio_capture_pipeline_str =
        "decklinkaudiosrc device-number=3 ! "
        "queue ! "
        "audioconvert ! audioresample ! "
        "audio/x-raw,format=S16LE,layout=interleaved,channels=2,rate=48000 ! "
        "appsink name=audio_sink emit-signals=true sync=false max-buffers=300 drop=false";

    audio_capture_pipeline = gst_parse_launch(audio_capture_pipeline_str, &error);
    if (!audio_capture_pipeline || error) {
        g_printerr("Failed to create audio capture pipeline: %s\n", error ? error->message : "Unknown error");
        if (error) g_clear_error(&error);
        gst_object_unref(video_capture_pipeline);
        g_main_loop_unref(loop);
        return -1;
    }
    g_object_set(audio_capture_pipeline, "name", "audio-capture-pipeline", NULL);

    const gchar *video_output_pipeline_str =
        "appsrc name=video_src format=GST_FORMAT_TIME is-live=true do-timestamp=true "
        "caps=video/x-raw,format=BGR,width=1920,height=1080,framerate=30000/1001 ! "
        "queue ! "
        "videoconvert ! "
        "videorate ! "
        "video/x-raw,format=UYVY,framerate=30000/1001 ! "
        "decklinkvideosink device-number=0 mode=1080p2997 sync=false video-format=8bit-yuv";

    video_output_pipeline = gst_parse_launch(video_output_pipeline_str, &error);
    if (!video_output_pipeline || error) {
        g_printerr("Failed to create video output pipeline: %s\n", error ? error->message : "Unknown error");
        if (error) g_clear_error(&error);
        gst_object_unref(video_capture_pipeline);
        gst_object_unref(audio_capture_pipeline);
        g_main_loop_unref(loop);
        return -1;
    }
    g_object_set(video_output_pipeline, "name", "video-output-pipeline", NULL);

    const gchar *audio_output_pipeline_str =
        "appsrc name=audio_src format=GST_FORMAT_TIME is-live=true do-timestamp=true "
        "caps=audio/x-raw,format=S16LE,layout=interleaved,channels=2,rate=48000 ! "
        "queue ! "
        "audioconvert ! "
        "decklinkaudiosink device-number=0 sync=true async=false";

    audio_output_pipeline = gst_parse_launch(audio_output_pipeline_str, &error);
    if (!audio_output_pipeline || error) {
        g_printerr("Failed to create audio output pipeline: %s\n", error ? error->message : "Unknown error");
        if (error) g_clear_error(&error);
        gst_object_unref(video_capture_pipeline);
        gst_object_unref(audio_capture_pipeline);
        gst_object_unref(video_output_pipeline);
        g_main_loop_unref(loop);
        return -1;
    }
    g_object_set(audio_output_pipeline, "name", "audio-output-pipeline", NULL);

    // Get appsink and appsrc elements
    video_sink = gst_bin_get_by_name(GST_BIN(video_capture_pipeline), "video_sink");
    audio_sink = gst_bin_get_by_name(GST_BIN(audio_capture_pipeline), "audio_sink");
    video_src = gst_bin_get_by_name(GST_BIN(video_output_pipeline), "video_src");
    audio_src = gst_bin_get_by_name(GST_BIN(audio_output_pipeline), "audio_src");

    if (!video_sink || !audio_sink || !video_src || !audio_src) {
        g_printerr("Failed to get appsink or appsrc elements.\n");
        goto cleanup;
    }

    // Connect new-sample signals
    g_signal_connect(video_sink, "new-sample", G_CALLBACK(on_new_video_sample), NULL);
    g_signal_connect(audio_sink, "new-sample", G_CALLBACK(on_new_audio_sample), NULL);

    // Set up bus for each pipeline
    video_capture_bus = gst_element_get_bus(video_capture_pipeline);
    audio_capture_bus = gst_element_get_bus(audio_capture_pipeline);
    video_output_bus = gst_element_get_bus(video_output_pipeline);
    audio_output_bus = gst_element_get_bus(audio_output_pipeline);

    gst_bus_add_watch(video_capture_bus, bus_callback, video_capture_pipeline);
    gst_bus_add_watch(audio_capture_bus, bus_callback, audio_capture_pipeline);
    gst_bus_add_watch(video_output_bus, bus_callback, video_output_pipeline);
    gst_bus_add_watch(audio_output_bus, bus_callback, audio_output_pipeline);

    // Set pipelines to playing state
    GstStateChangeReturn ret;
    ret = gst_element_set_state(video_capture_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set video capture pipeline to playing state.\n");
        goto cleanup;
    }

    ret = gst_element_set_state(audio_capture_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set audio capture pipeline to playing state.\n");
        goto cleanup;
    }

    ret = gst_element_set_state(video_output_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set video output pipeline to playing state.\n");
        goto cleanup;
    }

    ret = gst_element_set_state(audio_output_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set audio output pipeline to playing state.\n");
        goto cleanup;
    }

    // Run main loop
    g_print("Running pipelines...\n");
    g_main_loop_run(loop);

cleanup:
    // Clean up
    g_print("Cleaning up...\n");
    gst_element_set_state(video_capture_pipeline, GST_STATE_NULL);
    gst_element_set_state(audio_capture_pipeline, GST_STATE_NULL);
    gst_element_set_state(video_output_pipeline, GST_STATE_NULL);
    gst_element_set_state(audio_output_pipeline, GST_STATE_NULL);

    gst_object_unref(video_sink);
    gst_object_unref(audio_sink);
    gst_object_unref(video_src);
    gst_object_unref(audio_src);
    gst_object_unref(video_capture_bus);
    gst_object_unref(audio_capture_bus);
    gst_object_unref(video_output_bus);
    gst_object_unref(audio_output_bus);
    gst_object_unref(video_capture_pipeline);
    gst_object_unref(audio_capture_pipeline);
    gst_object_unref(video_output_pipeline);
    gst_object_unref(audio_output_pipeline);
    g_main_loop_unref(loop);

    return 0;
}