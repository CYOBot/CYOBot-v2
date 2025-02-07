# Introdution
This is the official release of CYOBot OS for CYOBot v2.0
The repo includes source code for onboard peripherals, including:
* Source code for robot kinematics (walking, rotating, PCA9685 control)
* Source code for NeoPixel LED Ring and LED matrix: individual LED control, text scrolling, bitmap display, etc
* Source code for micro SD card mounting
* Source code for speaker and microphone recording and replaying
* Network management system, including AP mode and STA mode, DNS
* * Portal for coding interface and WiFi setup

# Environment setup
Create a conda environment with **esptool** and **rshell** to interface with the robot following our [Wiki](https://cyobot.notion.site/Establish-USB-connection-b7c086ff2b9b4edba9dc345fd7659f00), or follow the instruction below:
1. Install miniconda to your device
2. Create new conda env with the necessary tools by running the following commands
```bash
conda create --name cyobot python=3.8
conda activate cyobot
pip install esptool rshell
```

# Installation
## Windows
For Windows, **esptool** and **rshell** work on top of a Conda environment. An `install.ps1` file is included to help with the process.

## Ubuntu
The `micropython.bin` is the MicroPython release that works with CYOBot v2.0, together with `partition-table.bin` and `bootloader.bin`.

To burn the CYOBrain, install `esptool.py` to your computer, and run the following commands:

```bash
esptool.py --port <PORT> erase_flash
```

If you run into `permission denied` problem, run the following command:
```bash
chmod 666 <PORT>
```

Then
```bash
esptool.py -p <PORT> -b 460800 --before default_reset --after hard_reset --chip esp32s3 write_flash --flash_mode dio --flash_size 4MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 micropython.bin
```
Example if you are running this on Ubuntu

```bash
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py -p /dev/ttyUSB0 -b 460800 --before default_reset --after hard_reset --chip esp32s3 write_flash --flash_mode dio --flash_size 4MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 micropython.bin
```

Then, copy content in the folder `/sd` to the micro SD card to be plugged into CYOBrain.

Next, copy all the source code from `/pyboard` folder to root directory of CYOBrain by install [rshell](https://github.com/dhylands/rshell), connect to the board running MicroPython, and call the following command:

```bash
# go to pyboard directory in this folder (not to confuse this with the root directory, /pyboard)
cd pyboard

# copy everything from the pyboard folder to the root directory of CYOBrain
rsync . /pyboard
```

Connecting to board and copying all files to board can be shorten to a single command:
```bash
rshell -p <PORT> -b 115200 rsync . /pyboard/
```

Finally, hard reset your CYOBrain by either pressing the reset button at the top of the CYOBrain, or disconnect power source (unplug USB cable/switch off the board) and reconnect. If you see a blinking LED on the LED display, the board is now running OS with everything else setup. Connect your computer to the board via WiFi (find the access point name `CYOBot`), open browser and either type in `portal.cyobot.com` or `192.168.4.1` to load the portal and start coding.

# Troubleshoot
## WebREPL is not on
The file `webrepl_cfg.py` with password information of WebREPL, and the following code in `boot.py` should be sufficient to turn on and configure WebREPL at startup:
```python
import webrepl
webrepl.start()
```

However, if WebREPL is not turned on, go through the following steps to turn on `webrepl` to be loaded on startup. Open `REPL` by typing into a terminal running `rshell` connected to CYOBrain
```bash
repl
```
Then turn on `webrepl` with:
```python
import webrepl_setup
```
Press `E` to enable webrepl on startup, and set `cyobot` as the password.

**Note**: The portal will attempt to connect to CYOBrain via webrepl with the password `cyobot`, so if you set the password to be anything else, the connection will not be successfully established.