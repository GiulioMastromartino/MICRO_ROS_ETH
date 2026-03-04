# JSN-SR04T Ultrasonic Sensor Simulation

This directory contains the simulation stack for the STM32H755 CM4 firmware and
JSN-SR04T (AJ-SR04M compatible) ultrasonic sensor behavior.

## What Is Implemented

- Complete sensor state machine in Python (`idle`, `triggered`, `measuring`, `timeout`, `ready`)
- Distance clamping and timeout handling
- Echo pulse-width computation (`distance_cm * 58`)
- Deterministic Renode script outputs via `RESULT key=value`
- Robot tests that assert real script outputs (no placeholder keywords)

## Layout

```
simulation/
|-- config/
|   `-- sensor_config.yaml
|-- platform/
|   `-- stm32h755_with_sensors.repl
|-- python/
|   `-- sensor_helper.py
|-- scripts/
|   |-- run_simulation.resc
|   `-- sensor_test.resc
|-- src/
|   |-- JSN_SR04T.cs
|   `-- JSN_SR04T_Plugin/
`-- tests/
    `-- robot/
        |-- common.robot
        `-- test_sensor_ultrasonic.robot
```

## Prerequisites

- Renode executable available either:
  - via `RENODE_PATH` env var, or
  - at `../Renode.app/Contents/MacOS/renode` relative to `simulation/`
- Robot Framework installed for automated tests (`robot` command)
- CM4 ELF at `../CM4/build/Polispace_Stm_CM4.elf` for integration run

## Run

From `simulation/`:

```bash
$RENODE_PATH scripts/sensor_test.resc
$RENODE_PATH scripts/run_simulation.resc
```

## Test

From `simulation/`:

```bash
robot tests/robot/test_sensor_ultrasonic.robot
```

The test suite parses `RESULT key=value` lines produced by Renode scripts.

## Notes

- `sensor_test.resc` validates the sensor model deterministically.
- `run_simulation.resc` loads platform + firmware + helper for integration execution.
