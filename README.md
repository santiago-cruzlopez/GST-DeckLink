# GStreamer

GStreamer and the DeckLink SDK repository for C/C++ and Python development, with a specific focus on integrating the Blackmagic DeckLink Duo card. Below are the instructions for installing GStreamer and the Blackmagic Desktop Video Drivers.

- [DeckLink SDK Installation](https://github.com/santiago-cruzlopez/GStreamer/blob/master/DeckLink_SDK/README.md) 
- [Application Development Manual](https://gstreamer.freedesktop.org/documentation/application-development/index.html?gi-language=c)

### Installing on Linux (Ubuntu or Debian)

- Run the following command to install the development packages and plugins:
```bash
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
```
- To verify that GStreamer is installed correctly, run the following commands:
```bash
gst-inspect-1.0 --version
gst-device-monitor-1.0 Video/Source

# Python Development (Conda Env)
conda list | grep gst
```

## Building C/C++ Applications

- GCC compiler and pkg-config dependencies:
```bash
sudo apt install build-essential pkg-config cmake
```
- Clone the GStreamer Repository, build and compile the first script tutorial:
```bash
git clone https://gitlab.freedesktop.org/gstreamer/gstreamer

# Change directory to find basic-tutorials-1:
cd gstreamer/subprojects/gst-docs/examples/tutorials/

# Compile it:
gcc basic-tutorial-1.c -o basic-tutorial-1 `pkg-config --cflags --libs gstreamer-1.0`

# Execute the binary:
./basic-tutorial-1
```

## Python Development

- To use GStreamer in Python applications, you need the gi bindings.
```bash
sudo apt install python3-gi gir1.2-gstreamer-1.0
```
- Installation (Conda Environment):
```bash
conda install -c conda-forge gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav gst-python pygobject
```

## External Installations and Configurations

- [OpenCV Python with Gstreamer Backend](https://discuss.bluerobotics.com/t/opencv-python-with-gstreamer-backend/8842/1)
- [Guide to build OpenCV from source with GPU support (CUDA and cuDNN)](https://gist.github.com/minhhieutruong0705/8f0ec70c400420e0007c15c98510f133)
- [Compiling and installing ffmpeg with Decklink support](https://gist.github.com/afriza/879fed4ede539a5a6501e0f046f71463)
- [FFmpeg for DekTec](https://dektec.com/products/SDK/ffmpeg/linux/#)
- [Compile FFmpeg for Ubuntu](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu)
- [NVIDIA DeepStream SDK](https://developer.nvidia.com/deepstream-sdk)
