"""
    1. DeckLink Video using GStreamer with CUDA
    2. Input[1]: 1080p5994 -> Output[0] 1080p2997, format=BGRA
    3. GStreamer Command:
        gst-launch-1.0 \
            decklinkvideosrc device-number=1 connection=sdi mode=1080p5994 ! \
            queue max-size-time=1000000000 name=input_buffer ! \
            videoconvert ! \
            video/x-raw,format=BGRA,width=1920,height=1080 ! \
            cudaupload ! \
            cudaconvert ! \
            video/x-raw\(memory:CUDAMemory\),format=BGRA ! \
            identity name=gpu_processing ! \
            cudadownload ! \
            queue max-size-time=1000000000 name=output_buffer ! \
            videorate ! \
            video/x-raw,format=BGRA,width=1920,height=1080,framerate=30000/1001 ! \
            videoconvert ! \
            decklinkvideosink device-number=0 mode=1080p2997 \
            decklinkaudiosrc device-number=1 ! \
            queue max-size-time=1000000000 ! \
            decklinkaudiosink device-number=0
"""

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

def GStreamer_Pipeline():
    Gst.init(None)

    pipeline = Gst.Pipeline()

    # Video elements
    videosrc = Gst.ElementFactory.make("decklinkvideosrc", None)
    videosrc.set_property("device-number", 1)
    videosrc.set_property("connection", "sdi")
    videosrc.set_property("mode", "1080p5994")

    queue_input = Gst.ElementFactory.make("queue", "input_buffer")
    queue_input.set_property("max-size-time", 1000000000)

    videoconvert1 = Gst.ElementFactory.make("videoconvert", None)

    capsfilter1 = Gst.ElementFactory.make("capsfilter", None)
    caps1 = Gst.Caps.from_string("video/x-raw,format=BGRA,width=1920,height=1080")
    capsfilter1.set_property("caps", caps1)

    cudaupload = Gst.ElementFactory.make("cudaupload", None)

    cudaconvert = Gst.ElementFactory.make("cudaconvert", None)

    capsfilter2 = Gst.ElementFactory.make("capsfilter", None)
    caps2 = Gst.Caps.from_string("video/x-raw(memory:CUDAMemory),format=BGRA")
    capsfilter2.set_property("caps", caps2)

    identity_gpu = Gst.ElementFactory.make("identity", "gpu_processing")

    cudadownload = Gst.ElementFactory.make("cudadownload", None)

    queue_output = Gst.ElementFactory.make("queue", "output_buffer")
    queue_output.set_property("max-size-time", 1000000000)

    videorate = Gst.ElementFactory.make("videorate", None)

    capsfilter3 = Gst.ElementFactory.make("capsfilter", None)
    caps3 = Gst.Caps.from_string("video/x-raw,format=BGRA,width=1920,height=1080,framerate=30000/1001")
    capsfilter3.set_property("caps", caps3)

    videoconvert2 = Gst.ElementFactory.make("videoconvert", None)

    videosink = Gst.ElementFactory.make("decklinkvideosink", None)
    videosink.set_property("device-number", 0)
    videosink.set_property("mode", "1080p2997")

    # Add video elements to pipeline
    pipeline.add(videosrc)
    pipeline.add(queue_input)
    pipeline.add(videoconvert1)
    pipeline.add(capsfilter1)
    pipeline.add(cudaupload)
    pipeline.add(cudaconvert)
    pipeline.add(capsfilter2)
    pipeline.add(identity_gpu)
    pipeline.add(cudadownload)
    pipeline.add(queue_output)
    pipeline.add(videorate)
    pipeline.add(capsfilter3)
    pipeline.add(videoconvert2)
    pipeline.add(videosink)

    # Link video chain
    videosrc.link(queue_input)
    queue_input.link(videoconvert1)
    videoconvert1.link(capsfilter1)
    capsfilter1.link(cudaupload)
    cudaupload.link(cudaconvert)
    cudaconvert.link(capsfilter2)
    capsfilter2.link(identity_gpu)
    identity_gpu.link(cudadownload)
    cudadownload.link(queue_output)
    queue_output.link(videorate)
    videorate.link(capsfilter3)
    capsfilter3.link(videoconvert2)
    videoconvert2.link(videosink)

    # Audio elements
    audiosrc = Gst.ElementFactory.make("decklinkaudiosrc", None)
    audiosrc.set_property("device-number", 1)

    queue_audio = Gst.ElementFactory.make("queue", None)
    queue_audio.set_property("max-size-time", 1000000000)

    audiosink = Gst.ElementFactory.make("decklinkaudiosink", None)
    audiosink.set_property("device-number", 0)

    # Add audio elements to pipeline
    pipeline.add(audiosrc)
    pipeline.add(queue_audio)
    pipeline.add(audiosink)

    # Link audio chain
    audiosrc.link(queue_audio)
    queue_audio.link(audiosink)

    # Bus handler
    bus = pipeline.get_bus()
    bus.add_signal_watch()

    def on_message(bus, message):
        if message.type == Gst.MessageType.EOS:
            loop.quit()
        elif message.type == Gst.MessageType.ERROR:
            err, debug = message.parse_error()
            print(f"Error: {err} {debug}")
            loop.quit()

    bus.connect("message", on_message)

    # Start pipeline
    pipeline.set_state(Gst.State.PLAYING)

    loop = GLib.MainLoop()
    try:
        loop.run()
    except KeyboardInterrupt:
        pass
    finally:
        pipeline.set_state(Gst.State.NULL)

if __name__ == "__main__":
    GStreamer_Pipeline()