"""
    1. Two Source Pipeline -> Processed Video/Audio -> Two Output Pipeline
    2. Input[3]: 1080i5994 -> Output[0] 1080p2997, format=BGR
    3. References:
        - https://gstreamer.freedesktop.org/documentation/applib/gstappsink.html?gi-language=python
        - https://gstreamer.freedesktop.org/documentation/applib/gstappsrc.html?gi-language=python
"""

import cv2
import time
import logging
import numpy as np

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

Gst.init([])

frame_count = 0
fps_start_time = time.time()

def video_sample(sink, appsrc):
    global frame_count, fps_start_time
    
    # Pull sample from appsink
    sample = sink.emit('pull-sample')
    if not sample:
        logger.error("Failed to pull sample from appsink")
        return Gst.FlowReturn.ERROR
    
    # Get buffer and caps
    buffer = sample.get_buffer()
    caps = sample.get_caps()
    
    structure = caps.get_structure(0)
    width = structure.get_int('width')[1]
    height = structure.get_int('height')[1]
    
    expected_size = width * height * 3  # BGR format, 3 bytes per pixel
    
    # Map buffer to numpy array
    success, map_info = buffer.map(Gst.MapFlags.READ)
    if not success:
        logger.error("Failed to map input buffer")
        return Gst.FlowReturn.ERROR
    
    try:
        if map_info.size != expected_size:
            logger.error(f"Buffer size mismatch: got {map_info.size}, expected {expected_size}")
            return Gst.FlowReturn.ERROR
        
        # Convert buffer to numpy array (BGR format)
        frame = np.ndarray(
            (height, width, 3),
            dtype=np.uint8,
            buffer=map_info.data
        )
        
        # Create a copy to avoid modifying the original buffer
        frame = frame.copy()
        
        # Perform OpenCV processing -> convert to grayscale and back to BGR
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        frame = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
        
        # Create new buffer for output
        out_buffer = Gst.Buffer.new_allocate(None, expected_size, None)
        success, out_map_info = out_buffer.map(Gst.MapFlags.WRITE)
        if not success:
            logger.error("Failed to map output buffer")
            return Gst.FlowReturn.ERROR
        
        # Copy processed frame to output buffer
        out_map_info.data[:] = frame.flatten()
        
        # Set buffer properties
        out_buffer.pts = buffer.pts
        out_buffer.dts = Gst.CLOCK_TIME_NONE
        out_buffer.duration = buffer.duration
        
        # Push buffer to appsrc
        ret = appsrc.emit('push-buffer', out_buffer)
        if ret != Gst.FlowReturn.OK:
            logger.error(f"Error pushing video buffer: {ret}")
            return ret
        
    finally:
        buffer.unmap(map_info)
        if 'out_map_info' in locals():
            out_buffer.unmap(out_map_info)
    
    frame_count += 1
    current_time = time.time()
    if current_time - fps_start_time > 1:
        fps = frame_count / (current_time - fps_start_time)
        print(f"Current FPS: {fps:.2f}")
        fps_start_time = current_time
        frame_count = 0
    
    return Gst.FlowReturn.OK

def audio_sample(sink, appsrc):
    # Pull sample from appsink
    sample = sink.emit('pull-sample')
    if not sample:
        logger.error("Failed to pull audio sample from appsink")
        return Gst.FlowReturn.ERROR
    
    # Get buffer
    buffer = sample.get_buffer()
    
    # Copy buffer for output
    out_buffer = buffer.copy()
    
    # Push buffer to appsrc
    ret = appsrc.emit('push-buffer', out_buffer)
    if ret != Gst.FlowReturn.OK:
        logger.error(f"Error pushing audio buffer: {ret}")
        return ret
    
    return Gst.FlowReturn.OK

def GStreamer_Pipeline():
    start_time = time.time()

    SDI_CAPTURE_PIPELINE = (
        f"decklinkvideosrc device-number=3 connection=sdi mode=1080i5994 drop-no-signal-frames=true do-timestamp=true ! "
        f"deinterlace ! "
        f"queue ! "
        f"videoconvert ! videorate ! "
        f"video/x-raw,format=BGR,width=1920,height=1080,interlace-mode=progressive,framerate=30000/1001 ! "
        f"appsink name=video_sink emit-signals=true sync=false max-buffers=30 drop=false"
    )

    SDI_OUTPUT_PIPELINE = (
        f"appsrc name=video_src format=GST_FORMAT_TIME is-live=true do-timestamp=true "
        f"caps=video/x-raw,format=BGR,width=1920,height=1080,framerate=30000/1001 ! "
        f"queue ! "
        f"videoconvert ! "
        f"videorate ! "
        f"video/x-raw,format=UYVY,framerate=30000/1001 ! "
        f"decklinkvideosink device-number=0 mode=1080p2997 sync=false video-format=8bit-yuv"
    )

    AUDIO_CAPTURE = (
        f"decklinkaudiosrc device-number=3 ! "
        f"queue ! " 
        f"audioconvert ! audioresample ! "
        f"audio/x-raw,format=S16LE,layout=interleaved,channels=2,rate=48000 ! "
        f"appsink name=audio_sink emit-signals=true sync=false max-buffers=300 drop=false"
    )

    AUDIO_OUTPUT = (
        f"appsrc name=audio_src format=GST_FORMAT_TIME is-live=true do-timestamp=true "
        f"caps=audio/x-raw,format=S16LE,layout=interleaved,channels=2,rate=48000 ! "
        f"queue ! "
        f"audioconvert ! "
        f"decklinkaudiosink device-number=0 sync=true async=false"
    )

    # Video/Audio Pipelines
    try:
        video_in_pipeline = Gst.parse_launch(SDI_CAPTURE_PIPELINE)
        audio_in_pipeline = Gst.parse_launch(AUDIO_CAPTURE)
    except GLib.GError as e:
        logger.error(f"Failed to create input pipelines: {e}")
        return
    
    try:
        video_out_pipeline = Gst.parse_launch(SDI_OUTPUT_PIPELINE)
        audio_out_pipeline = Gst.parse_launch(AUDIO_OUTPUT)
    except GLib.GError as e:
        logger.error(f"Failed to create output pipelines: {e}")
        return
    
    # Get appsink and appsrc elements for Video/Audio
    video_appsink = video_in_pipeline.get_by_name('video_sink')
    video_appsrc = video_out_pipeline.get_by_name('video_src')
    audio_appsink = audio_in_pipeline.get_by_name('audio_sink')
    audio_appsrc = audio_out_pipeline.get_by_name('audio_src')
    
    # Set caps on Video/Audio appsrc
    caps = Gst.Caps.from_string("video/x-raw,format=BGR,width=1920,height=1080,framerate=30000/1001")
    video_appsrc.set_property("caps", caps)
    audio_caps = Gst.Caps.from_string("audio/x-raw,format=S16LE,layout=interleaved,channels=2,rate=48000")
    audio_appsrc.set_property("caps", audio_caps)
    
    # Connect new-sample signal to callback for Video/Audio
    video_appsink.connect('new-sample', video_sample, video_appsrc)
    audio_appsink.connect('new-sample', audio_sample, audio_appsrc)
    
    # Create main loop
    loop = GLib.MainLoop()
    
    # Create bus for Video/Audio input/output pipeline
    video_in_bus = video_in_pipeline.get_bus()
    video_in_bus.add_signal_watch()
    video_out_bus = video_out_pipeline.get_bus()
    video_out_bus.add_signal_watch()
    audio_in_bus = audio_in_pipeline.get_bus()
    audio_in_bus.add_signal_watch()
    audio_out_bus = audio_out_pipeline.get_bus()
    audio_out_bus.add_signal_watch()
    
    def bus_message(bus, message, loop, pipeline_name):
        """Handle messages from the GStreamer bus."""
        if message.type == Gst.MessageType.EOS:
            logger.error(f"End of stream in {pipeline_name}")
            loop.quit()
        elif message.type == Gst.MessageType.ERROR:
            err, debug = message.parse_error()
            logger.error(f"Error: {err.message}, Domain: {err.domain}, Code: {err.code}")
            logger.error(f"Debug Info: {debug} for {pipeline_name}")
            if err.domain == Gst.CoreError.quark() and err.code == Gst.CoreError.MISSING_PLUGIN:
                logger.error("Specific issue: Missing codec/plugin")
            elif err.domain == Gst.StreamError.quark() and err.code == Gst.StreamError.DECODE:
                logger.error("Specific issue: Decoding error (possible pixelation or corruption)")
            loop.quit()
        elif message.type == Gst.MessageType.WARNING:
            err, debug = message.parse_warning()
            logger.warning(f"Warning: {err.message} (Domain: {err.domain}, Code: {err.code})")
            logger.warning(f"Debug Info: {debug} for {pipeline_name}")
            if "sync" in err.message.lower() or "latency" in err.message.lower():
                logger.warning("Specific issue: Potential audio-video synchronization problem")
        return True
    
    # Connect bus message handlers
    video_in_bus.connect("message", bus_message, loop, "video input pipeline")
    video_out_bus.connect("message", bus_message, loop, "video output pipeline")
    audio_in_bus.connect("message", bus_message, loop, "audio input pipeline")
    audio_out_bus.connect("message", bus_message, loop, "audio output pipeline")
    
    try:
        video_in_pipeline.set_state(Gst.State.PLAYING)
        video_out_pipeline.set_state(Gst.State.PLAYING)
        audio_in_pipeline.set_state(Gst.State.PLAYING)
        audio_out_pipeline.set_state(Gst.State.PLAYING)
        logger.info("All pipelines set to PLAYING state")
    except Exception as e:
        logger.error(f"Failed to set pipeline states: {e}")
        return
    
    try:
        logger.info("Starting main loop")
        loop.run()
    except KeyboardInterrupt:
        end_time = time.time()
        duration = end_time - start_time
        hours, rem = divmod(duration, 3600)
        minutes, seconds = divmod(rem, 60)
        logger.info(f"Interrupted by user")
        logger.info(f"Execution ended after {int(hours):02d}:{int(minutes):02d}:{seconds:09.6f}")
    finally:
        logger.info("Cleaning up pipelines")
        video_in_pipeline.set_state(Gst.State.NULL)
        video_out_pipeline.set_state(Gst.State.NULL)
        audio_in_pipeline.set_state(Gst.State.NULL)
        audio_out_pipeline.set_state(Gst.State.NULL)
        loop.quit()
        logger.info("Main loop Terminated")

if __name__ == '__main__':
    GStreamer_Pipeline()