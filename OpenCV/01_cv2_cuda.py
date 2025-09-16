import cv2
import time
import logging
import subprocess
import numpy as np

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

SDI_CAPTURE_PIPELINE = (
    "decklinkvideosrc device-number=3 connection=sdi mode=1080i5994 drop-no-signal-frames=true do-timestamp=true ! "
    "deinterlace fields=all ! "
    "queue leaky=upstream max-size-buffers=8 max-size-bytes=0 max-size-time=0 ! "
    "videoconvert ! videorate ! "
    "video/x-raw,format=BGR,width=1920,height=1080,interlace-mode=progressive,framerate=30000/1001 ! "
    "appsink emit-signals=true sync=false max-buffers=8 drop=true"
)

cap = cv2.VideoCapture(SDI_CAPTURE_PIPELINE, cv2.CAP_GSTREAMER)

if not cap.isOpened():
    logger.error("Cannot Capture the SDI Input")
    exit()

SDI_OUTPUT_PIPELINE  = (
    "appsrc format=GST_FORMAT_TIME is-live=true do-timestamp=true block=false "
    "caps=video/x-raw,format=BGR,width=1920,height=1080,framerate=30000/1001,interlace-mode=progressive ! "
    "queue leaky=upstream max-size-buffers=30 max-size-bytes=0 max-size-time=2000000000 ! "
    "videoconvert ! "
    "videorate ! "
    "video/x-raw,format=UYVY,framerate=30000/1001 ! "
    "decklinkvideosink device-number=0 mode=1080p2997 sync=false video-format=8bit-yuv"
)

writer = cv2.VideoWriter(SDI_OUTPUT_PIPELINE , cv2.CAP_GSTREAMER, 0, 30000 / 1001.0, (1920, 1080), True)

if not writer.isOpened():
    logger.error("Cannot Output to the SDI - Port 0")
    cap.release()
    exit()

printed = False
start_time = time.time()
frame_count = 0
try:
    while True:
        success, frame = cap.read()
        if not success:
            logger.error("Could not receive frame. Possible signal loss or pipeline issue.")
            break
        if not printed:
            logger.info("Successfully capturing frames.")
            printed = True

        gpu_frame = cv2.cuda_GpuMat()
        gpu_frame.upload(frame)
        # Example GPU processing: Convert to grayscale and back to BGR
        gray_gpu = cv2.cuda.cvtColor(gpu_frame, cv2.COLOR_BGR2GRAY)
        processed_gpu = cv2.cuda.cvtColor(gray_gpu, cv2.COLOR_GRAY2BGR)
        frame = processed_gpu.download()
        writer.write(frame)  # Write frames to output

        frame_count += 1
        if time.time() - start_time > 1:
            fps = frame_count / (time.time() - start_time)
            logger.info(f"Current FPS: {fps:.2f}")
            start_time = time.time()
            frame_count = 0

except KeyboardInterrupt:
    end_time = time.time()
    duration = end_time - start_time
    hours, rem = divmod(duration, 3600)
    minutes, seconds = divmod(rem, 60)
    logger.info("Interrupted by user")
    logger.info(f"Execution ended after {int(hours):02d}:{int(minutes):02d}:{seconds:09.6f}")

finally:
    cap.release()
    writer.release()