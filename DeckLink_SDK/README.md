# DeckLink SDK

C++ implementation using the Blackmagic Design DeckLink SDK to capture and output video and audio from the DeckLink Duo card.

### Installation
1. Install Blackmagic Desktop Video Drivers:
    - Download the latest Desktop Video software for Linux from the official Blackmagic Design website: [Desktop Video Downloads](https://www.blackmagicdesign.com/support/family/capture-and-playback)
    - Extract the archive and install the .deb package (for Ubuntu/Debian-based systems):
      ```bash
      tar -xf Blackmagic_Desktop_Video_Linux_*.tar
      cd Blackmagic_Desktop_Video_Linux_*/deb/x86_64/
      sudo dpkg -i desktopvideo_*.deb
      sudo apt-get install -f
      BlackmagicFirmwareUpdater status
      # if prompted to update, perform an update:
      BlackmagicFirmwareUpdater update 0
      # if prompted to shut down, reboot the device:
      sudo reboot
      ```
    - Restart your system to load the drivers.
    - Confirm installation by running `lspci | grep Blackmagic` or using the Blackmagic Desktop Video Setup utility.
    - Check that `libDeckLinkAPI.so` is installed in the system as well, run `ls -l /usr/lib | grep libDeckLink` for verification.
2. Obtain the DeckLink SDK:
    - Download the DeckLink SDK from the official developer page.
    - Unzip Desktop Video SDK
      ```bash
      unzip Blackmagic_DeckLink_SDK_*.zip
      ```
    - In `CMakeLists.txt`, update `DECKLINK_SDK_PATH` to match your extraction path.
3. Install Build Tools:
      ```bash
      sudo apt update
      sudo apt install cmake build-essential
      ```

### Project Structure
- `src/`: Contains source code files.
- `bin/`: Houses the compiled executable.
- `build/`: Temporary build directory (generated during compilation).
- `CMakeLists.txt`: Defines the build process.     
