# GStreamer

GStreamer repository for C and Python development, with a focus on Blackmagic DeckLink Duo integration.

## 📦 Installing GStreamer

Run the following command to install the full set of development packages and commonly used plugins:

```bash
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
```

🛠️ Building Applications Using GStreamer (C/C++)

To build GStreamer applications in C/C++, you’ll need the GCC compiler and pkg-config.
Dependencies:

```bash
sudo apt install build-essential pkg-config cmake
```