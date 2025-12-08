import sys
import platform

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# Structure to contain all our information, so we can pass it around
class CustomData:
    def __init__(self):
        self.playbin = None  # Our one and only element
        self.n_video = 0     # Number of embedded video streams
        self.n_audio = 0     # Number of embedded audio streams
        self.n_text = 0      # Number of embedded subtitle streams
        self.current_video = -1  # Currently playing video stream
        self.current_audio = -1  # Currently playing audio stream
        self.current_text = -1   # Currently playing subtitle stream
        self.main_loop = None    # GLib's Main Loop

# playbin flags
GST_PLAY_FLAG_VIDEO = (1 << 0)  # We want video output
GST_PLAY_FLAG_AUDIO = (1 << 1)  # We want audio output
GST_PLAY_FLAG_TEXT = (1 << 2)   # We want subtitle output

# Forward definition for the message and keyboard processing functions
def handle_message(bus, msg, data):
    mtype = msg.type
    if mtype == Gst.MessageType.ERROR:
        err, debug = msg.parse_error()
        print(f"Error received from element {msg.src.get_name()}: {err.message}")
        print(f"Debugging information: {debug if debug else 'none'}")
        data.main_loop.quit()
    elif mtype == Gst.MessageType.EOS:
        print("End-Of-Stream reached.")
        data.main_loop.quit()
    elif mtype == Gst.MessageType.STATE_CHANGED:
        if msg.src == data.playbin:
            old_state, new_state, pending_state = msg.parse_state_changed()
            if new_state == Gst.State.PLAYING:
                # Once we are in the playing state, analyze the streams
                analyze_streams(data)

def handle_keyboard(source, cond, data):
    if cond == GLib.IOCondition.IN:
        line = source.readline()
        if line:
            try:
                index = int(line.strip())
                if index < 0 or index >= data.n_audio:
                    print("Index out of bounds")
                else:
                    print(f"Setting current audio stream to {index}")
                    data.playbin.set_property("current-audio", index)
            except ValueError:
                print("Invalid input")
    return True

def main():
    Gst.init(None)

    data = CustomData()

    # Create the elements
    data.playbin = Gst.ElementFactory.make("playbin", "playbin")

    if not data.playbin:
        print("Not all elements could be created.")
        sys.exit(1)

    # Set the URI to play
    data.playbin.set_property("uri", "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm")

    # Set flags to show Audio and Video but ignore Subtitles
    flags = data.playbin.get_property("flags")
    flags |= GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO
    flags &= ~GST_PLAY_FLAG_TEXT
    data.playbin.set_property("flags", flags)

    # Set connection speed. This will affect some internal decisions of playbin
    data.playbin.set_property("connection-speed", 56)

    # Add a bus watch, so we get notified when a message arrives
    bus = data.playbin.get_bus()
    bus.add_signal_watch()
    bus.connect("message", handle_message, data)

    # Add a keyboard watch so we get notified of keystrokes
    if platform.system().lower() == "windows":
        io_stdin = GLib.IOChannel.win32_new_fd(sys.stdin.fileno())
    else:
        io_stdin = GLib.IOChannel.unix_new(sys.stdin.fileno())
    GLib.io_add_watch(io_stdin, GLib.IOCondition.IN, handle_keyboard, data)

    # Start playing
    ret = data.playbin.set_state(Gst.State.PLAYING)
    if ret == Gst.StateChangeReturn.FAILURE:
        print("Unable to set the pipeline to the playing state.")
        sys.exit(1)

    # Create a GLib Main Loop and set it to run
    data.main_loop = GLib.MainLoop.new(None, False)
    data.main_loop.run()

    # Free resources
    data.main_loop = None
    data.playbin.set_state(Gst.State.NULL)

# Extract some metadata from the streams and print it on the screen
def analyze_streams(data):
    data.n_video = data.playbin.get_property("n-video")
    data.n_audio = data.playbin.get_property("n-audio")
    data.n_text = data.playbin.get_property("n-text")

    print(f"{data.n_video} video stream(s), {data.n_audio} audio stream(s), {data.n_text} text stream(s)")

    print("\n")
    for i in range(data.n_video):
        tags = data.playbin.emit("get-video-tags", i)
        if tags:
            print(f"video stream {i}:")
            success, str_val = tags.get_string(Gst.TAG_VIDEO_CODEC)
            print(f"  codec: {str_val if success else 'unknown'}")

    print("\n")
    for i in range(data.n_audio):
        tags = data.playbin.emit("get-audio-tags", i)
        if tags:
            print(f"audio stream {i}:")
            success, str_val = tags.get_string(Gst.TAG_AUDIO_CODEC)
            if success:
                print(f"  codec: {str_val}")
            success, str_val = tags.get_string(Gst.TAG_LANGUAGE_CODE)
            if success:
                print(f"  language: {str_val}")
            success, rate = tags.get_uint(Gst.TAG_BITRATE)
            if success:
                print(f"  bitrate: {rate}")

    print("\n")
    for i in range(data.n_text):
        tags = data.playbin.emit("get-text-tags", i)
        if tags:
            print(f"subtitle stream {i}:")
            success, str_val = tags.get_string(Gst.TAG_LANGUAGE_CODE)
            if success:
                print(f"  language: {str_val}")

    data.current_video = data.playbin.get_property("current-video")
    data.current_audio = data.playbin.get_property("current-audio")
    data.current_text = data.playbin.get_property("current-text")

    print("\n")
    print(f"Currently playing video stream {data.current_video}, audio stream {data.current_audio} and text stream {data.current_text}")
    print("Type any number and hit ENTER to select a different audio stream")

if __name__ == "__main__":
    main()