# Temporal Loom

A desk-side PC monitor for the **M5Stack Fire**, inspired by the Temporal Loom from *Loki*.
Your computer's CPU, GPU, RAM and VRAM become a spinning loom of amber threads, and an alarm fires when something runs too hot.

It has two parts:

- **Firmware** (`firmware/`) runs on the M5Stack and draws the screens.
- **Companion** (`pc/`) runs on your computer, reads the sensors and sends them over USB.

## Requirements

- M5Stack Fire (M5Stack Basic works too) and a USB-C data cable
- Arduino IDE 2
- Python 3.8 or newer on Windows, macOS or Linux

## 1. Flash the M5Stack

1. In Arduino IDE, open **File > Preferences** and add this board manager URL:
   `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`
2. In **Boards Manager**, install **M5Stack**.
3. In **Library Manager**, install **M5Unified** and **ArduinoJson** (v7).
4. Open `firmware/temporal_loom/temporal_loom.ino`.
5. Select **Tools > Board > M5Stack > M5Fire** (or **M5Core** for the Basic), set **PSRAM** to **Enabled**, and pick the port.
6. Click **Upload**. The screen shows **TIMELINE SEVERED** until the companion connects.

Use the ESP32 core that the M5Stack board package installs. Espressif's ESP32 core 4.x is not yet supported by M5Unified.

## 2. Run the companion

```
cd pc
pip install -r requirements.txt
python temporal_loom.py
```

On macOS and Linux, use `python3` and `pip3`. The loom starts spinning within a second.

| Option | What it does |
|---|---|
| `--sensors` | Show the detected sensors and one reading |
| `--dry-run` | Print stats without an M5Stack |
| `--list-ports` | List serial ports |
| `--port COM5` | Use a specific port |
| `--gpu-index 1` | Watch the second NVIDIA GPU |

Only one program can use the serial port at a time, so close the Arduino Serial Monitor while the companion runs.

### Platform notes

- **Windows:** for CPU temperature and AMD/Intel GPUs, run [LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor) as administrator and enable **Options > Remote Web Server > Run**.
- **Linux:** add yourself to the serial group with `sudo usermod -aG dialout $USER` and log in again. If CPU temperature shows `--`, install `lm-sensors`.
- **macOS:** if no port appears, install the CP210x or CH9102 USB driver from M5Stack.

### CPU temperature on macOS

macOS needs a small helper tool to report CPU temperature. Without it, the temperature shows `--` and everything else still works.

On Apple Silicon (M1 and newer), build and install [smctemp](https://github.com/narugit/smctemp):

```
git clone https://github.com/narugit/smctemp
cd smctemp
sudo make install
```

On Intel Macs, install `osx-cpu-temp` with Homebrew:

```
brew install osx-cpu-temp
```

Run `python3 temporal_loom.py --sensors` to confirm it shows **CPU temperature: smctemp command** (or **osx-cpu-temp command**).

Any value a computer can't report shows as `--` or 0%, and everything else keeps working.

## Using the M5Stack

| Button | Action |
|---|---|
| A | Previous screen |
| B (click) | Mute, unmute, or silence the alarm |
| B (hold) | Turn the LED bars on or off |
| C | Next screen |

The screens are **LOOM**, **HISTORY** (last 40 seconds) and **DETAIL**.
The alarm fires when CPU load is over 85%, CPU temperature over 85°C, GPU load over 90%, GPU temperature over 83°C, or RAM usage over 90% for 3 seconds.
You can change these limits, brightness, volume and LED options in `firmware/temporal_loom/src/config/settings.h`.

## Project layout

```
firmware/temporal_loom/src/
  config/      user settings
  core/        stats store, PC link, alarm, animation timing
  display/     canvas, theme, layout, effects and screens
  hardware/    LED bars
  input/       buttons and navigation
pc/loom_companion/
  cli/         command line and console output
  core/        snapshot and collector
  sources/     CPU, GPU and system sensors per platform
  transport/   serial port and message format
```

## Running the tests

```
cd pc
python -m unittest discover -s tests -t .
```

## License

MIT. See [LICENSE](LICENSE).

*Temporal Loom is a fan project and is not affiliated with Marvel or Disney.*
