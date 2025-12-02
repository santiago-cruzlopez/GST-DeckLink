# GStreamer-DeckLink

This document is a comprehensive setup guide for integrating GStreamer and the DeckLink SDK with the Blackmagic DeckLink Duo card. It outlines the system requirements, installation steps, and initial configuration instructions for both C/C++ and Python development. For a more detailed overview and summary of this GitHub repository, please follow this link: [DeepWiki Index](https://deepwiki.com/santiago-cruzlopez/GST-DeckLink).

- [Developing with Blackmagic Design](https://www.blackmagicdesign.com/developer/)
- [GStreamer Application Development Manual](https://gstreamer.freedesktop.org/documentation/application-development/index.html?gi-language=c)

## Prerequisites

<p align="center">

| Component                 | Requirement	                  | Verification Command               |
|:-------------------------:|:-------------------------------:|:----------------------------------:|
| Operating System          | Ubuntu/Debian Linux	          | `lsb_release -a`                   |
| Hardware                  | DeckLink Duo card	              | `BlackmagicFirmwareUpdater status` |
| Build Tools	            | GCC, CMake, pkg-config	      | `gcc --version && cmake --version` |
| Python 	                | Python 3.x with gi bindings	  | `python3 -c "import gi"`           |

</p>

## Core Installation Steps

1. System Dependencies
    - Update package information and install the necessary packages:
    ```bash
    sudo apt update 
    sudo apt-get install build-essential pkg-config cmake make unzip yasm dkms git checkinstall libsdl2-dev libgtk2.0-dev libavcodec-dev libavformat-dev libswscale-dev
    ```
2. DeckLink Drivers Installation
    - Download the latest Desktop Video software for Linux from the official Blackmagic Design website: [Desktop Video Downloads](https://www.blackmagicdesign.com/support/family/capture-and-playback)
    ```bash
    tar -xf Blackmagic_Desktop_Video_Linux_*.tar
    cd Blackmagic_Desktop_Video_Linux_*/deb/x86_64/
    sudo dpkg -i desktopvideo_*.deb
    sudo apt-get install -f
    ```
    - Verify firmware status and update if necessary:    
    ```bash
    BlackmagicFirmwareUpdater status
    BlackmagicFirmwareUpdater update 0
    sudo reboot
    ```
3. DeckLink SDK Configuration
    - Download and unzip the Desktop Video SDK from the official developer page.
    ```bash
    unzip Blackmagic_DeckLink_SDK_*.zip
    ```
    - Update the `DECKLINK_SDK_PATH` variable in [CMakeLists.txt](https://github.com/santiago-cruzlopez/GStreamer/blob/master/DeckLink_SDK/CMakeLists.txt) to point to your SDK extraction directory.

4. GStreamer Framework Installation
    - Install GStreamer development packages and plugins:
    ```bash
    sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
    ```

5. OpenCV Python with Gstreamer and FFmpeg Compilation
    - Clone and Build OpenCV from Source with GStreamer Enabled
    ```bash
    git clone https://github.com/opencv/opencv.git
    cd opencv
    git checkout 4.12.0
    mkdir build
    cd build
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=$(python3 -c "import sys; print(sys.prefix)") \
          -D PYTHON_EXECUTABLE=$(which python3) \
          -D WITH_GSTREAMER=ON \
          -D WITH_FFMPEG=ON \
          -D WITH_TBB=ON \
          -D BUILD_EXAMPLES=OFF \
          -D BUILD_TESTS=OFF \
          -D BUILD_PERF_TESTS=OFF ..

    make -j$(nproc)
    make install
    ```
    - Verify GStreamer Support:
    ```bash
    python3 -c "import cv2; print(cv2.__version__); print(cv2.getBuildInformation())"
    ```

6. Python Environment Setup
    - Install the gi bindings for Python applications:
    ```bash
    sudo apt-get install python3-gi gir1.2-gstreamer-1.0 python3-dev python3-numpy python3-pip python3-testresources
    ```
    - UV Installation:
    ```bash
    curl -LsSf https://astral.sh/uv/install.sh | sh
    uv init
    uv add flask requests
    uv python install 3.10
    uv venv --python 3.10
    ```
    - Installation for Conda Environments:
    ```bash
    conda install -c conda-forge gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav gst-python pygobject
    ```

### Installation Verification
- Verify DeckLink hardware is properly detected:
  ```bash
  lspci | grep Blackmagic
  
  # Check that libDeckLinkAPI.so is correctly installed
  ls -l /usr/lib | grep libDeckLink
  ```
- Verify that GStreamer is installed correctly
  ```bash
  gst-inspect-1.0 --version
  gst-device-monitor-1.0 Video/Source
  ```
- Check the Python Environment:
  ``` bash
  python3 -c "import gi"
  conda list | grep gst
  ```

## Building C++ Applications with Blackmagic Design DeckLink SDK

### Project Structure
- `src/`: Contains source code files.
- `bin/`: Houses the compiled executable.
- `build/`: Temporary build directory (generated during compilation).
- `CMakeLists.txt`: Defines the build process.     

### Building the Project
- Path to [DeckLink_SDK](https://github.com/santiago-cruzlopez/GStreamer/tree/master/DeckLink_SDK)
  ```bash
  cd /GST-DeckLink/DeckLink_SDK
  mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  make
  ```
- Execute the binary:
  ```bash
  cd ../bin/Linux64/Debug
  ./DeckLink-SDK
  ```
- Terminate with Ctrl+C to trigger cleanup, displaying the end time and performance metrics.


## Building C Applications with GStreamer
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
- Check [GST_CMake](https://github.com/santiago-cruzlopez/GStreamer/tree/master/GST_CMake) for the implementation with the BlackMagic DeckLink Duo card.

## Troubleshooting
- **Device Not Detected:** Confirm the card appears in `lspci` and add your user to the video group if access is denied: `sudo usermod -aG video $USER`.
- **API Failures:** Consult `HRESULT` error codes in the DeckLink SDK Manual for debugging.
- **Compilation Issues:** Ensure the SDK path is correct and headers like `DeckLinkAPI.h` are found.
- **Runtime Permissions:** Elevated privileges may be needed for hardware interaction in some setups.
- **Blackmagic Desktop Video Drivers Installation Problems:** If you encounter a dependency issue where the Blackmagic Desktop Video drivers need the `dkms` (Dynamic Kernel Module Support) package, ensure you install the missing dkms package along with its dependencies before proceeding with the installation of the Blackmagic Desktop Video drivers.

  ```bash
  # Step 1: Install DKMS and Dependencies
  sudo apt update
  sudo apt install dkms

  # Step 2: Install Build Dependencies (if needed)
  sudo apt install build-essential linux-headers-$(uname -r)

  # Step 3: Install the Blackmagic Desktop Video Drivers
  sudo dpkg -i desktopvideo_*.deb

  # Step 4: Fix Any Remaining Dependencies
  sudo apt --fix-broken install
  sudo reboot  
  ```

## External Installations and Configurations

- [OpenCV Python with Gstreamer Backend](https://discuss.bluerobotics.com/t/opencv-python-with-gstreamer-backend/8842/1)
- [Guide to build OpenCV from source with GPU support (CUDA and cuDNN)](https://gist.github.com/minhhieutruong0705/8f0ec70c400420e0007c15c98510f133)
- [Compiling and installing ffmpeg with Decklink support](https://gist.github.com/afriza/879fed4ede539a5a6501e0f046f71463)
- [FFmpeg for DekTec](https://dektec.com/products/SDK/ffmpeg/linux/#)
- [Compile FFmpeg for Ubuntu](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu)
- [NVIDIA DeepStream SDK](https://developer.nvidia.com/deepstream-sdk)