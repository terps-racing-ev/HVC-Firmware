# HVC Firmware

Firmware for the High Voltage Controller (HVC) used in an FSAE electric vehicle. The target is an STM32L432 running STM32 HAL plus FreeRTOS/CMSIS-RTOS v2.

This repository is organized around a small set of hand-written subsystems:

- `Managers` own timing, queues, event flags, and side effects.
- `Data` owns shared runtime state behind getters/setters and mutexes.
- `Drivers` convert raw hardware/protocol details into usable values.
- `Config` holds CAN IDs, cell constants, and the generated OCV lookup table.

The HVC talks to two CAN domains:

- `BMS CAN` on the STM32's native `CAN1` peripheral for battery-module traffic.
- `LV CAN` through an MCP2515 on `SPI1` for low-voltage vehicle-network traffic.

For the full architecture walkthrough, see [docs/codebase-design.md](docs/codebase-design.md).

## At A Glance

- Safety-facing logic lives primarily in `state_manager` and `io_manager`.
- Pack telemetry from six battery module boards flows through `bms_can_manager` into accumulator data structures.
- SOC and current-limit estimation live in `acc_manager` and `soc`.
- The firmware publishes state, IO, SOC, summary, voltage-sense, and current-limit messages over CAN.
- The repository includes host-side unit tests and helper scripts for generated artifacts such as `hvc.dbc` and the OCV table.

## Repository Layout

- `Core/Inc`, `Core/Src`: application code, split into `Config`, `Data`, `Drivers`, and `Managers`
- `Drivers/`, `Middlewares/`: STM32 HAL, CMSIS, FreeRTOS, and vendor code
- `docs/`: long-form project documentation and the local FSAE rules PDF
- `tools/`: helper scripts for generating `hvc.dbc` and `Core/Src/config/ocv_lookup_table.c`
- `tests/`, `tests.mk`, `Dockerfile.test`: host-side unit test workflow
- `STM32Make.make`: current firmware build source of truth
- `dual_build.mk`: dual-bank application build wrapper
- `openocd.cfg`: OpenOCD target configuration

## Runtime Shape

`main.c` performs hardware init, creates RTOS primitives for each manager, and starts these application threads:

- `IO_Manager`: realtime sensor sampling, filtering, output pin updates
- `BMS_CAN_Manager`: native CAN queueing, dispatch, and recovery
- `LV_CAN_Manager`: MCP2515 TX/RX queueing and error handling
- `SPI_IntCallback`: deferred LV-CAN interrupt drain task
- `Acc_Manager`: accumulator summary, SOC, and current-limit publishing
- `State_Manager`: top-level state machine and fault aggregation
- `LED_Blink`: board heartbeat/debug task

## Build And Flash

### Recommended firmware build paths

- VS Code STM32 extension tasks:
  - `Build STM`
  - `Build Clean STM`
  - `Flash STM`
- Dual-bank wrapper:

```bash
make -f dual_build.mk
```

- Direct STM32Make flow:

```bash
make -f STM32Make.make
```

`STM32Make.make` reflects the current hand-written source layout. The root `Makefile` is an older generated file and should be treated as legacy unless it is regenerated to match the present tree.

### Flashing

The repo ships with `openocd.cfg` for ST-Link plus STM32L4 targets. The STM32 VS Code extension can use that config directly, or you can invoke OpenOCD yourself.

## Host Tests

Host tests are intentionally separate from the firmware build:

```bash
make -f tests.mk test
```

Run one module:

```bash
make -f tests.mk test state_manager
```

Containerized path:

```bash
docker build -f Dockerfile.test -t hvc-tests .
docker run --rm -v ${PWD}:/work -w /work hvc-tests
```

More detail is in `tests/README.md` and `tests/testing_guide.md`.

## Generated Artifacts And Tooling

- `python tools/generate_io_dbc.py` regenerates `hvc.dbc` from `can_id.h` and `state.h`
- `python tools/generate_ocv_table.py` regenerates the OCV lookup table from source CSV data
- `requirements.txt` lists Python dependencies used by the tooling notebooks/scripts

## Documentation

- Architecture and subsystem guide: [docs/codebase-design.md](docs/codebase-design.md)
- Local rules reference: `docs/FSAE_Rules_2026_V1.pdf`
