import cv2

url = "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"

cap = cv2.VideoCapture(url)

if not cap.isOpened():
    print("Error: Could not open video stream.")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("End of stream or error reading frame.")
        break
    
    cv2.imshow('Video Playback', frame)
    
    if cv2.waitKey(25) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()