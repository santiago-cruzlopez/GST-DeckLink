import cv2
from datetime import datetime

url = "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"

cap = cv2.VideoCapture(url)

if not cap.isOpened():
    print("Error: Could not open video stream.")
    exit()

# Get video specifications
frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fps = cap.get(cv2.CAP_PROP_FPS)
total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
fourcc = int(cap.get(cv2.CAP_PROP_FOURCC))

# Print video specifications
print("=" * 50)
print("VIDEO SPECIFICATIONS")
print("=" * 50)
print(f"Resolution: {frame_width} x {frame_height}")
print(f"FPS: {fps}")
print(f"Total Frames: {total_frames}")
print(f"Video Format (FourCC): {chr(fourcc & 255)}{chr((fourcc >> 8) & 255)}{chr((fourcc >> 16) & 255)}{chr((fourcc >> 24) & 255)}")
print("=" * 50)

# Start timing with date
start_time = datetime.now()
print(f"Start Time: {start_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}")
print("=" * 50)

frame_count = 0

while True:
    ret, frame = cap.read()
    if not ret:
        print("End of stream or error reading frame.")
        break
    
    frame_count += 1
    # Print frame count every frame
    print(f"Frame: {frame_count}/{total_frames} ({(frame_count/total_frames)*100:.1f}%)", end='\r')
    
    cv2.imshow('Video Playback', frame)
    
    if cv2.waitKey(25) & 0xFF == ord('q'):
        break

# End timing with date
end_time = datetime.now()
duration = end_time - start_time

print("=" * 50)
print(f"End Time: {end_time.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}")
print(f"Frames Processed: {frame_count}")
print(f"Total Duration: {duration}")
print("=" * 50)

cap.release()
cv2.destroyAllWindows()