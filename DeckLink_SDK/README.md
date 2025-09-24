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
2. Obtain the Desktop Video SDK:
    - Download and Unzip the Desktop Video SDK from the official developer page.
      ```bash
      unzip Blackmagic_DeckLink_SDK_*.zip
      ```
    - In `CMakeLists.txt`, update `DECKLINK_SDK_PATH` to match your extraction path.
3. Install Build Tools:
   - GCC compiler and pkg-config dependencies:
      ```bash
      sudo apt install build-essential pkg-config cmake
      ```

### Project Structure
- `src/`: Contains source code files.
- `bin/`: Houses the compiled executable.
- `build/`: Temporary build directory (generated during compilation).
- `CMakeLists.txt`: Defines the build process.     

### Building the Project
```bash
cd /path/to/DeckLink_SDK
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

### Troubleshooting
- **Device Not Detected:** Confirm the card appears in `lspci` and add your user to the video group if access is denied: `sudo usermod -aG video $USER`.
- **API Failures:** Consult `HRESULT` error codes in the DeckLink SDK Manual for debugging.
- **Compilation Issues:** Ensure the SDK path is correct and headers like `DeckLinkAPI.h` are found.
- **Runtime Permissions:** Elevated privileges may be needed for hardware interaction in some setups.
- **Blackmagic Desktop Video Drivers Installation Fix:** If you encounter a dependency issue where the Blackmagic Desktop Video drivers need the `dkms` (Dynamic Kernel Module Support) package, ensure you install the missing dkms package along with its dependencies before proceeding with the installation of the Blackmagic Desktop Video drivers.

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
