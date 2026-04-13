# JSN-SR04T Renode Plugin

This directory contains a C# plugin for Renode that provides a complete simulation of the JSN-SR04T waterproof ultrasonic distance sensor.

## Overview

| Item | Details |
|------|---------|
| **Language** | C# (.NET 8.0) |
| **Plugin Type** | Renode IExternal/IPlugin |
| **Target** | STM32H755 + JSN-SR04T |

## Prerequisites

### Required Software

1. **.NET 8.0 SDK**
   ```bash
   # macOS
   brew install dotnet-sdk
   
   # Linux
   wget https://dot.net/v1/dotnet-install.sh -O dotnet-install.sh
   chmod +x dotnet-install.sh
   ./dotnet-install.sh --channel 8.0
   
   # Windows
   # Download from: https://dotnet.microsoft.com/download
   ```

2. **Renode Source** (for building)
   ```bash
   git clone https://github.com/renode/renode.git
   cd renode
   ./build.sh
   ```

## Quick Start

### Option 1: Build from Source

```bash
# Clone and build Renode first
git clone https://github.com/renode/renode.git
cd renode
./build.sh
cd ..

# Build the plugin
cd JSN_SR04T_Plugin
./build.sh /path/to/renode

# Copy to Renode
cp bin/Release/net8.0/JSN_SR04T_Plugin.dll /path/to/renode/bin/
```

### Option 2: Use Python Helper (Recommended)

The Python helper (`../../python/sensor_helper.py`) provides the same functionality without requiring compilation:

```bash
# Run simulation with Python helper
cd ../../scripts
../../../../Renode.app/Contents/MacOS/renode sensor_test.resc
```

## Building

### Standard Build

```bash
cd JSN_SR04T_Plugin
dotnet build -c Release
```

### With Renode Integration

```bash
# Build with Renode references
./build.sh /path/to/renode
```

### Troubleshooting

#### "Renode not found" Error

The build script will try common locations. If it fails:

1. Clone and build Renode:
   ```bash
   git clone https://github.com/renode/renode.git
   cd renode
   ./build.sh
   cd ../JSN_SR04T_Plugin
   ```

2. Run with explicit path:
   ```bash
   ./build.sh ../../renode
   ```

#### "Reference assemblies not found"

Make sure you've built Renode from source. The plugin needs the compiled Renode DLLs.

## Using the Plugin

### Method 1: Load as Python Extension

This is the recommended method:

```python
# In Renode script
emulation LoadPythonExtension "path/to/sensor_helper.py"
```

### Method 2: Direct Plugin Load

```csharp
// After compiling and copying to Renode bin
// The plugin will be auto-discovered
```

## Plugin Architecture

### Main Class: `JSN_SR04T`

```csharp
public class JSN_SR04T : IExternal, IHasGPIO
```

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `Distance` | `int` | Current distance (cm) |
| `MinDistance` | `int` | Minimum (20cm) |
| `MaxDistance` | `int` | Maximum (450cm) |
| `Timeout` | `int` | Timeout in µs (default: 30000) |
| `TrigPin` | `IGPIOConnector` | TRIG pin |
| `EchoPin` | `IGPIOConnector` | ECHO pin |
| `State` | `SensorState` | Current state |

#### Methods

| Method | Description |
|--------|-------------|
| `SetDistance(int)` | Set simulated distance |
| `GetDistance()` | Get current distance |
| `Trigger()` | Trigger measurement |
| `Reset()` | Reset sensor |

#### States

```csharp
public enum SensorState
{
    Idle,           // Waiting for trigger
    WaitingTrigger, // Trigger pulse detected
    Measuring,      // Measuring distance
    ResultReady     // Measurement complete
}
```

## GPIO Connection

```
MCU (STM32H755)          JSN-SR04T Sensor
─────────────────────────────────────────
PD1 (TRIG)    ──────►   TRIG pin
PD0 (ECHO)    ◄──────   ECHO pin
```

## Timing Diagram

```
TRIG:  ────┐     ┌─────────────────────────┐
           │     │                         │
           └─────┘                         
           10µs    ←─── Processing ────→   100µs

ECHO:  ──────────────────────────────────┐
                                          │
                                          └────────
                                          
                     distance × 58µs
                     
Example: 100cm = 5800µs = 5.8ms pulse
```

## Integration with Platform

To use the plugin in a `.repl` file:

```repl
jsn_sr04t: Sensors.JSN_SR04T @ sysbus
    trigPin: gpioD.Pin1
    echoPin: gpioD.Pin0
```

## Testing

Run the Robot Framework tests:

```bash
cd ../../tests/robot
robot test_sensor_ultrasonic.robot
```

## Files

| File | Description |
|------|-------------|
| `JSN_SR04T.cs` | Main plugin source |
| `JSN_SR04T_Plugin.csproj` | Project file |
| `build.sh` | Build script |
| `README.md` | This file |

## License

MIT License - See LICENSE file for details.

## References

- [Renode Documentation](https://renode.io/)
- [JSN-SR04T Datasheet](https://makerguides.com/wp-content/uploads/2019/02/JSN-SR04T-Datasheet.pdf)
- [Renode Plugin Development](https://renode.readthedocs.io/en/latest/)
