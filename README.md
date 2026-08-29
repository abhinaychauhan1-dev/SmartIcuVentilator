# Smart ICU Ventilator

Smart ICU Ventilator is a desktop ventilator monitoring and simulation interface built with C++20, Qt Quick, Qt Graphs, and Qt Quick 3D. It generates simulated respiratory telemetry and presents it through an ICU-style dashboard.

> [!WARNING]
> This project is a software simulation and UI demonstration. It is not a medical device and must not be used for diagnosis, treatment, patient monitoring, or control of clinical equipment.

## Features

- Real-time pressure, flow, and volume waveforms
- Pressure-volume and flow-volume respiratory loops
- Animated 3D lung visualization
- Calculated breath metrics, including peak and mean pressure, PEEP, tidal volume, minute volume, respiratory rate, and FiO2
- Adjustable PEEP, tidal volume, rate, inspiratory time, FiO2, compliance, and resistance
- Volume-control and pressure-control simulation modes
- High-pressure, low-tidal-volume, and low-PEEP alarms with temporary audio silencing
- Lock-free telemetry transfer between the simulation worker and UI

## Requirements

- CMake 3.21 or newer
- A C++20-compatible compiler
- Qt 6.8 or newer with these modules:
  - Qt Core
  - Qt Quick
  - Qt Quick 3D
  - Qt Graphs
  - Qt Widgets

The project is currently configured and developed with Qt 6.8.3 and MinGW 64-bit on Windows.

## Build and Run

### Qt Creator

1. Open `CMakeLists.txt` in Qt Creator.
2. Select a Qt 6.8 or newer desktop kit containing the required modules.
3. Configure the project, then build and run the `SmartIcuVentilator` target.

### Command Line

Set `CMAKE_PREFIX_PATH` to the Qt installation matching your compiler. For example, with Qt 6.8.3 and MinGW on Windows:

```powershell
cmake -S . -B build/command-line -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/mingw_64
cmake --build build/command-line
./build/command-line/SmartIcuVentilator.exe
```

For multi-config generators, pass a configuration when building and run the executable from the corresponding configuration directory:

```powershell
cmake --build build/command-line --config Debug
```

## Project Structure

```text
.
|-- main.cpp                 Application bootstrap and QML integration
|-- qml/
|   |-- Main.qml             Main dashboard window and navigation
|   |-- components/          Reusable controls, tiles, keypad, and alarm banner
|   `-- views/               Waveform, respiratory-loop, and 3D-lung views
`-- src/
    |-- application/         Alarm evaluation, breath metrics, and parameter routing
    |-- domain/              Simulation engine, configuration, and units
    `-- viewmodels/          Dashboard and plotting data exposed to QML
```

The `SimulationEngine` runs on a worker thread and publishes telemetry through a single-producer/single-consumer ring buffer. The view models consume that data, derive display metrics and alarm state, and expose the results to the QML interface.

## Ventilator Parameters

The simulation starts in volume-control mode with these defaults:

| Parameter | Default |
| --- | ---: |
| PEEP | 5 cmH2O |
| Tidal volume | 0.5 L |
| Respiratory rate | 12 breaths/min |
| Inspiratory time | 1.0 s |
| FiO2 | 40% |
| Compliance | 0.05 L/cmH2O |
| Resistance | 10 cmH2O/(L/s) |

Alarm thresholds are defined in `src/domain/VentilatorConfig.h`.