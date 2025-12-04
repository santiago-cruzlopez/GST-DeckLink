import sys
import time
import gi
gi.require_version('GLib', '2.0')
gi.require_version('GObject', '2.0')
gi.require_version('Gst', '1.0')
gi.require_version('GstPbutils', '1.0')
from gi.repository import Gst, GObject, GLib, GstPbutils

# Initialize GStreamer
Gst.init(sys.argv)

uri = "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"

# Discover and print video/audio specifications
discoverer = GstPbutils.Discoverer.new(60 * Gst.SECOND)
info = discoverer.discover_uri(uri)

# Video specs
video_streams = info.get_video_streams()
if video_streams:
    for vs in video_streams:
        print("Video Specifications:")
        print(f"  Width: {vs.get_width()}")
        print(f"  Height: {vs.get_height()}")
        fr = vs.get_framerate_num() / vs.get_framerate_denom() if vs.get_framerate_denom() != 0 else 0
        print(f"  Framerate: {fr} fps")
        print(f"  Bitrate: {vs.get_bitrate()} bits/s")
        caps = vs.get_caps()
        if caps:
            struct = caps.get_structure(0)
            codec = struct.get_name()
            print(f"  Codec: {codec}")

# Audio specs
audio_streams = info.get_audio_streams()
if audio_streams:
    for a in audio_streams:
        print("Audio Specifications:")
        print(f"  Sample Rate: {a.get_sample_rate()} Hz")
        print(f"  Channels: {a.get_channels()}")
        print(f"  Bitrate: {a.get_bitrate()} bits/s")
        caps = a.get_caps()
        if caps:
            struct = caps.get_structure(0)
            codec = struct.get_name()
            print(f"  Codec: {codec}")

# Get total duration
duration_seconds = info.get_duration() / Gst.SECOND if info.get_duration() else 0

# Build the pipeline
pipeline = Gst.parse_launch(f"playbin uri={uri}")

# Function to recursively find element by factory name
def find_element(bin, factory_name):
    iterator = bin.iterate_elements()
    while iterator.next() == Gst.IteratorResult.OK:
        elem = iterator.get()
        if elem.get_factory().get_name() == factory_name:
            return elem
        if isinstance(elem, Gst.Bin):
            found = find_element(elem, factory_name)
            if found:
                return found
    return None

# Dictionary to store entry times keyed by PTS (in nanoseconds)
entry_times = {}

def sink_probe(pad, info, user_data):
    buffer = info.get_buffer()
    if buffer:
        entry_times[buffer.pts] = time.monotonic()
    return Gst.PadProbeReturn.OK

def src_probe(pad, info, user_data):
    buffer = info.get_buffer()
    if buffer:
        pts = buffer.pts
        if pts in entry_times:
            delta = time.monotonic() - entry_times[pts]
            print(f"Processing time for frame (PTS {pts / Gst.SECOND:.2f}s): {delta:.6f} seconds")
            del entry_times[pts]
    return Gst.PadProbeReturn.OK

# Set up main loop and bus
loop = GLib.MainLoop()
bus = pipeline.get_bus()

def on_message(bus, msg):
    t = msg.type
    if t == Gst.MessageType.EOS:
        pipeline.set_state(Gst.State.NULL)
        loop.quit()
    elif t == Gst.MessageType.ERROR:
        err, dbg = msg.parse_error()
        print(f"Error: {err} {dbg}")
        pipeline.set_state(Gst.State.NULL)
        loop.quit()
    elif t == Gst.MessageType.STATE_CHANGED:
        if msg.src == pipeline:
            old, new, pending = msg.parse_state_changed()
            if new == Gst.State.PAUSED:
                # Now find the decoder and add probes
                decoder = find_element(pipeline, "vp8dec")
                if decoder:
                    sink_pad = decoder.get_static_pad("sink")
                    src_pad = decoder.get_static_pad("src")
                    if sink_pad and src_pad:
                        sink_pad.add_probe(Gst.PadProbeType.BUFFER, sink_probe, None)
                        src_pad.add_probe(Gst.PadProbeType.BUFFER, src_probe, None)
                        print("Probes added to video decoder for per-frame timing.")
                    else:
                        print("Could not get decoder pads.")
                else:
                    print("Video decoder not found.")
                # Now resume to PLAYING
                pipeline.set_state(Gst.State.PLAYING)
    return True

bus.add_signal_watch()
bus.connect("message", on_message)

# Start in PAUSED to preroll and access elements
pipeline.set_state(Gst.State.PAUSED)
loop.run()

# Print total duration at the end
print(f"Total duration of the file: {duration_seconds} seconds")