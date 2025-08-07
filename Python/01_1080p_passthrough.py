'''
    1. First code for TESTING the SDI Input and Output using DeckLink with GStreamer
    2. Input[1]: 1080p5994 -> Output[0]: 1080p5994
    3. GStreamer Command:
        gst-launch-1.0 \
        decklinkvideosrc device-number=1 connection=sdi mode=1080p5994 ! decklinkvideosink device-number=0 mode=1080p5994 \
        decklinkaudiosrc device-number=1 ! decklinkaudiosink device-number=0
'''

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

Gst.init(None)

# Create pipeline
pipeline = Gst.Pipeline()

# Create video elements
videosrc = Gst.ElementFactory.make("decklinkvideosrc", "videosrc")
if not videosrc:
    print("Error: Could not create decklinkvideosrc element")
    exit(1)
videosrc.set_property("device-number", 1)
videosrc.set_property("connection", "sdi")
videosrc.set_property("mode", "1080p5994")

videosink = Gst.ElementFactory.make("decklinkvideosink", "videosink")
if not videosink:
    print("Error: Could not create decklinkvideosink element")
    exit(1)
videosink.set_property("device-number", 0)
videosink.set_property("mode", "1080p5994")

# Create audio elements
audiosrc = Gst.ElementFactory.make("decklinkaudiosrc", "audiosrc")
if not audiosrc:
    print("Error: Could not create decklinkaudiosrc element")
    exit(1)
audiosrc.set_property("device-number", 1)

audiosink = Gst.ElementFactory.make("decklinkaudiosink", "audiosink")
if not audiosink:
    print("Error: Could not create decklinkaudiosink element")
    exit(1)
audiosink.set_property("device-number", 0)

# Add elements to pipeline
pipeline.add(videosrc)
pipeline.add(videosink)
pipeline.add(audiosrc)
pipeline.add(audiosink)

# Link elements
if not videosrc.link(videosink):
    print("Error: Could not link videosrc to videosink")
    exit(1)
if not audiosrc.link(audiosink):
    print("Error: Could not link audiosrc to audiosink")
    exit(1)

# Set up message handling
bus = pipeline.get_bus()
bus.add_signal_watch()

def on_message(bus, message, loop):
    t = message.type
    if t == Gst.MessageType.ERROR:
        err, debug = message.parse_error()
        print(f"Error: {err}, {debug}")
        loop.quit()
    elif t == Gst.MessageType.WARNING:
        err, debug = message.parse_warning()
        print(f"Warning: {err}, {debug}")
    elif t == Gst.MessageType.EOS:
        print("End of stream")
        loop.quit()
    return True


bus.connect("message", on_message, GLib.MainLoop())

# Start pipeline
pipeline.set_state(Gst.State.PLAYING)

# Run main loop
loop = GLib.MainLoop()
try:
    loop.run()
except KeyboardInterrupt:
    print("Interrupted by user")
finally:
    pipeline.set_state(Gst.State.NULL)