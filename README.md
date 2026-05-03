# 📡 IoT Sensor Dashboard Simulator

A desktop telemetry dashboard built in **C++ with SDL2** that simulates a live ESP32 sensor stream — complete with real-time graphs, UART packet parsing, and a dark embedded-terminal UI.

> Built as a learning project to understand embedded telemetry architecture, sensor data pipelines, and SDL2 rendering.

---

## Preview

```
┌─────────────────────────────────────────────────────────────┐
│  ● ESP32 · IOT DASHBOARD              // LIVE · 350ms       │
├──────────────┬──────────────────────────────┬───────────────┤
│  TEMPERATURE │  ▁▂▃▄▅▆▇█ TEMP °C           │  IMU TILT     │
│  24.7 °C     │  ▁▂▃▄▅▆▇█ HUM  %            │   · (bubble)  │
│  HUMIDITY    │  ▁▂▃▄▅▆▇█ DIST cm           │               │
│  58.2 %      │                              │  UART PACKET  │
│  DISTANCE    │                              │  TEMP:24.7;.. │
│  120 cm      │                              │               │
│  IMU / Z     ├──────────────────────────────┴───────────────┤
│  -0.98 g     │  SERIAL CONSOLE                              │
├──────────────┤  00:01:23.450 [INFO] TEMP:24.7;HUM:58.2;... │
│  DISCONNECT  │  00:01:23.800 [ OK ] Device connected        │
│  FREEZE      │  00:01:24.150 [WARN] Distance spike detected  │
│  INJECT SPIKE│                                              │
└──────────────┴──────────────────────────────────────────────┘
```

---

## Features

- **Live sensor simulation** — temperature, humidity, distance, IMU (X/Y/Z)
- **Realistic behaviour** — slow value drift with occasional random noise spikes
- **UART packet engine** — data encoded/decoded using a real serial protocol format
- **Rolling line graphs** — 80-sample history per sensor at 350ms intervals
- **Colour-coded status cards** — green (OK) / yellow (WARN) based on thresholds
- **Serial console panel** — timestamped log of every packet and event
- **IMU tilt bubble** — live orientation visualiser
- **Interactive controls** — disconnect simulation, debug freeze mode, manual spike injection
- **Decoupled loop** — 60 FPS render rate, independent 350ms sensor update rate

---

## Architecture

```
iot-dashboard/
├── CMakeLists.txt
├── include/
│   ├── SensorData.h          # Central data struct shared by all modules
│   ├── SensorSimulator.h     # Sensor data generator
│   ├── Parser.h              # UART packet encode / decode
│   ├── DashboardUI.h         # SDL2 renderer
│   └── Logger.h              # Event log with circular buffer
└── src/
    ├── main.cpp              # Main loop + timing
    ├── SensorSimulator.cpp
    ├── Parser.cpp
    ├── DashboardUI.cpp
    └── Logger.cpp
```

### Data Flow

```
SensorSimulator::update(data)
        │
        ▼
Parser::encode(data) ──► "TEMP:25.3;HUM:60.1;DIST:120;IMU:0.10,0.02,-0.95"
        │
        ▼
Logger::log_packet(packet)
        │
        ▼
DashboardUI::render(data, logger)   ← runs every frame at 60 FPS
```

### UART Packet Format

```
TEMP:25.3;HUM:60.1;DIST:120;IMU:0.10,0.02,-0.95\n
```

| Field | Unit | Range |
|-------|------|-------|
| TEMP  | °C   | -10 to 80 |
| HUM   | %    | 0 to 100 |
| DIST  | cm   | 0 to 400 |
| IMU   | g    | -1.5 to 1.5 per axis |

---

## Dependencies

| Library | Purpose |
|---------|---------|
| SDL2 | Window, renderer, input |
| SDL2_ttf | Text rendering |
| CMake 3.16+ | Build system |
| C++17 | Language standard |

---

## Build & Run

### Install dependencies (Debian / Ubuntu)

```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev cmake build-essential
```

### Arch Linux

```bash
sudo pacman -S sdl2 sdl2_ttf cmake
```

### Build

```bash
git clone https://github.com/yourusername/iot-dashboard.git
cd iot-dashboard
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./IoTDashboard
```

---

## Sensor Thresholds

| Sensor | Warning Low | Warning High |
|--------|-------------|--------------|
| Temperature | < 15°C | > 45°C |
| Humidity | < 20% | > 80% |
| Distance | < 5cm | > 200cm |

Cards turn yellow and the status LED pulses when a sensor goes out of range.

---

## Controls

| Button | Action |
|--------|--------|
| `DISCONNECT` | Simulates UART loss — stops the data stream |
| `FREEZE` | Pauses sensor updates — debug snapshot mode |
| `INJECT SPIKE` | Manually triggers an out-of-range spike |

---

## Extending to Real Hardware

The simulator is designed to be swapped out for real serial input without changing anything else in the pipeline.

Replace `SensorSimulator` with a `SerialReader` class that opens `/dev/ttyUSB0` and reads lines from the ESP32:

```cpp
// SerialReader reads from real UART instead of generating fake data
// Parser::decode() stays identical — it just parses the same packet format
// Everything downstream (Logger, DashboardUI) is untouched
```

ESP32 firmware just needs to print the packet over Serial at 115200 baud:

```cpp
// Arduino / ESP32 firmware side
Serial.printf("TEMP:%.1f;HUM:%.1f;DIST:%.0f;IMU:%.2f,%.2f,%.2f\n",
              temp, hum, dist, imu_x, imu_y, imu_z);
```

---

## What This Project Teaches

- Embedded telemetry structure and data flow
- UART-style communication design and packet parsing
- Layered C++ architecture (data / logic / rendering)
- SDL2 rendering loop — similar to game engines and LVGL
- Debug mindset used in real ESP32 projects

---

## License

MIT
