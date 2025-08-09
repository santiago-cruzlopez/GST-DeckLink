# GStreamer

GStreamer repository for C and Python development, with a focus on Blackmagic DeckLink Duo integration.

- [Application Development Manual](https://gstreamer.freedesktop.org/documentation/application-development/index.html?gi-language=c) 

### 📦 Install GStreamer on Ubuntu 22.04

Run the following command to install the development packages and plugins:

```bash
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
```

### 🛠️ Building Applications Using GStreamer (C/C++)

To build GStreamer applications in C/C++, you’ll need the GCC compiler and pkg-config.
Dependencies:

```bash
sudo apt install build-essential pkg-config cmake
```

### 🧱 Compile with GCC
Use the following command to compile your GStreamer application:

```bash
pkg-config --cflags --libs gstreamer-1.0
```

### 🐍 Using GStreamer with Python

To use GStreamer in Python applications, you need the gi bindings.
```bash
sudo apt install python3-gi gir1.2-gstreamer-1.0
```

**Installation (Conda):**
```bash
conda install -c conda-forge gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav gst-python pygobject
```

### ✅ Verification
You can verify that GStreamer is installed correctly by running:
```bash
$ gst-inspect-1.0 --version
$ gst-device-monitor-1.0 Video/Source
$ conda list | grep gst
```

## 🔧 External Installations and Configurations

- [OpenCV Python with Gstreamer Backend](https://discuss.bluerobotics.com/t/opencv-python-with-gstreamer-backend/8842/1)
- [Guide to build OpenCV from source with GPU support (CUDA and cuDNN)](https://gist.github.com/minhhieutruong0705/8f0ec70c400420e0007c15c98510f133)
- [Compiling and installing ffmpeg with Decklink support](https://gist.github.com/afriza/879fed4ede539a5a6501e0f046f71463)
