# HVC Firmware Codebase Design

Revision notes

- 2026-04-28: Initial long-form documentation pass for the current repository layout and runtime architecture.

## 1. Purpose

This repository implements the firmware for the High Voltage Controller (HVC) in an FSAE electric vehicle. The HVC sits between battery-module telemetry, vehicle low-voltage networking, and safety-critical outputs such as the precharge logic signal and the BMS-fault line.

The codebase is trying to balance three concerns:

- deterministic safety behavior
- ongoing telemetry and state estimation
- enough modularity to unit test protocol and control logic on a host machine

At a high level, the firmware continuously:

- reads board-level analog and digital signals
- receives battery-module data from six accumulator modules
- calculates pack summary, SOC, and current limits
- evaluates top-level HVC state and error conditions
- publishes state and telemetry on one or both CAN buses

## 2. Scope Of This Document

This document focuses on the hand-written application layers:

- `Core/Src/Managers`
- `Core/Src/Data`
- `Core/Src/Drivers`
- `Core/Src/config`
- matching headers in `Core/Inc`
- test/build/tooling files that affect architecture or maintenance

It does not try to re-document STM32 HAL, FreeRTOS internals, CubeMX-generated boilerplate, or vendor middleware.

## 3. Architectural Model

The codebase follows a practical three-layer split:

1. `Managers`
   Own task loops, queue draining, event-flag handling, state transitions, and external side effects.
2. `Data`
   Own shared runtime state and lightweight synchronization primitives.
3. `Drivers`
   Own conversion or protocol-specific logic close to hardware or wire formats.

That split is the most important organizing idea in the repository.

### 3.1 Layer responsibilities

- Managers decide when work runs.
- Data modules decide where shared values live.
- Drivers decide how raw bytes or ADC readings become meaningful values.

This gives the code a clear default direction of dependency:

- `Managers -> Data`
- `Managers -> Drivers`
- `Drivers -> Data` only where a decoded message needs to populate shared structures

### 3.2 Main subsystem relationships

```text
Battery Module Boards
  -> BMS CAN (STM32 CAN1)
  -> BMS CAN Manager
  -> BMB/charging/debug decoders
  -> ACC shared data
  -> ACC Manager
  -> SOC + current limit outputs
  -> LV CAN and/or BMS CAN telemetry

Board GPIO / ADC / Comparator
  -> IO Manager
  -> IO shared data + current-sense sample queue
  -> State Manager and ACC Manager consumers

State Manager
  -> global state + error mask
  -> BMS fault logical value
  -> state messages on both CAN buses

LV CAN MCP2515 interrupt
  -> EXTI4 ISR
  -> SPI_IntCallback task
  -> LV CAN RX queue
  -> LV CAN dispatch handlers
```

## 4. Repository Structure

## 4.1 Hand-written application code

- `Core/Inc/Managers`, `Core/Src/Managers`
  - task-level orchestration
- `Core/Inc/Data`, `Core/Src/Data`
  - shared data containers and helper functions
- `Core/Inc/Drivers`, `Core/Src/Drivers`
  - protocol decoders, sensor conversion, MCP2515 SPI CAN driver
- `Core/Inc/Config`, `Core/Src/config`
  - IDs, constants, generated lookup tables

## 4.2 Generated or vendor-heavy areas

- `Core/Src/main.c`
  - mixed generated/user entry point
- `Core/Src/stm32l4xx_it.c`
  - mixed generated/user interrupts
- `Drivers/`, `Middlewares/`
  - HAL, CMSIS, FreeRTOS, MCP-related vendor code
- `HVC.ioc`, `HVC2.ioc`
  - CubeMX/board configuration inputs

## 4.3 Non-firmware support files

- `tests/`
  - host-side unit tests and stubs
- `tools/`
  - generators for CAN DBC and OCV tables
- `STM32Make.make`
  - current firmware build source of truth
- `dual_build.mk`
  - wrapper for bank A and bank B artifacts

## 5. Boot And Startup

Startup is centered in `Core/Src/main.c`.

## 5.1 Hardware bring-up sequence

The firmware starts in the usual STM32 pattern:

1. `HAL_Init()`
2. `SystemClock_Config()`
3. manual vector-table relocation with `SCB->VTOR = 0x08008000U`
4. peripheral initialization:
   - GPIO
   - `CAN1`
   - CRC
   - comparator
   - ADC
   - `SPI1`
5. `osKernelInitialize()`
6. manager initialization
7. thread creation
8. `osKernelStart()`

The explicit vector-table relocation matters because the repo also supports dual-bank application builds. New engineers should treat the bootloader/application-address contract as part of system design, not just build noise.

## 5.2 Manager initialization order

Current initialization order is:

1. `BMS_CAN_Manager_Init()`
2. `LV_CAN_Manager_Init()`
3. `Acc_Manager_Init()`
4. `IO_Manager_Init()`
5. `State_Manager_Init()`

That order establishes most shared primitives before the scheduler starts. It also means queue/mutex creation failures are treated as fatal startup errors through `Error_Handler()`.

## 5.3 Tasks and priorities

The application creates these threads:

- `IO_Manager` at realtime priority
- `BMS_CAN_Manager` at high priority
- `State_Manager` at high priority
- `Acc_Manager` at high priority
- `LV_CAN_Manager` at normal priority
- `SPI_IntCallback` at normal priority
- `LED_Blink` at low priority

The intent is straightforward:

- fast sensor/input work gets the highest urgency
- state and CAN processing stay responsive
- debug heartbeat stays lowest priority

## 6. Runtime Subsystems

## 6.1 State Manager

Files:

- `Core/Src/Managers/state_manager.c`
- `Core/Inc/Managers/state_manager.h`
- `Core/Src/Data/state.c`
- `Core/Inc/Data/state.h`

This is the top-level state machine for the HVC. It owns:

- current HVC state
- current error mask
- transition rules between `PRE_INIT`, `RUNNING`, `CHARGING`, and `ERRORED`
- publication of state frames on both CAN buses
- the logical `bms_fault` value that later drives the physical output pin

### State model

Defined states are:

- `PRE_INIT`
- `RUNNING`
- `CHARGING`
- `BALANCING`
- `ERRORED`

`BALANCING` exists in the enum but is not actively used by the current transition logic.

### Current transition logic

- `PRE_INIT -> RUNNING`
  - when IO, BMS CAN, and LV CAN managers are initialized and no active errors are present
- `RUNNING -> CHARGING`
  - when a charging-request event is observed
- `RUNNING -> ERRORED`
  - when any enabled error bit is set
- `CHARGING -> ERRORED`
  - on errors or if charging requests stop arriving for too many cycles
- `ERRORED -> RUNNING`
  - when all currently enabled errors clear

### Error aggregation

The manager checks several categories of faults:

- reference-board temperature over 60 C
- floating battery voltage input
- floating current-sense input
- per-module BMB-reported error state
- optional module-timeout check
- optional BMS CAN link error check
- optional LV CAN link error check

Important nuance: the module-timeout and both CAN-link checks are compile-time gated in `state_manager.h`, and the current defaults disable them:

- `CHECK_MODULE_TIMEOUT = 0`
- `CHECK_BMS_CAN_ERRORS = 0`
- `CHECK_LV_CAN_ERRORS = 0`

So the code retains the logic, but those fault categories are not currently promoted into the runtime error mask unless the macros are changed.

### Event-flag inputs

The state manager consumes:

- `charge_flag`
  - raised by the charging-request decoder when a charging heartbeat arrives on BMS CAN
- `floating_input_flag`
  - maintained by the IO manager for battery-voltage and current-sense floating detection

This is a good example of event flags being used for cross-manager coordination without direct task-to-task calls.

## 6.2 IO Manager

Files:

- `Core/Src/Managers/io_manager.c`
- `Core/Inc/Managers/io_manager.h`
- `Core/Src/Data/io.c`
- `Core/Inc/Data/io.h`
- `Core/Src/Drivers/curr_sense.c`
- `Core/Src/Drivers/therm.c`
- `Core/Src/Drivers/vsense.c`

This manager is the firmware's hardware-observation layer. It samples board inputs, filters them, updates shared data, drives certain safety outputs, and publishes IO telemetry.

### Cadence

The task runs every `10 ms` and internally divides work into two lanes:

- high-priority lane every `10 ms`
- low-priority lane every `100 ms`

That keeps fast analog work and output maintenance responsive while reducing the cost of slower telemetry packaging.

### High-priority responsibilities

- write the physical `BMS_Fault` output based on the logical shared value
- sample battery voltage, inverter voltage, and both current channels
- compute ADC reference voltage from `VREFINT`
- convert raw ADC readings into engineering units
- apply moving-average filtering
- detect floating battery and current-sense inputs
- push current samples into the ACC-owned queue for SOC integration
- publish `CAN_ID_IO_VSENSE` on LV CAN

### Low-priority responsibilities

- sample `SDC`, `IMD`, and board thermistor inputs
- update shared digital and temperature data
- publish `CAN_ID_IO_SUMMARY` on LV CAN
- publish `CAN_ID_IO_CURRENT` on both LV CAN and BMS CAN

### Comparator path and precharge signal

`COMP_IRQHandler()` does not touch GPIO directly. It raises `IO_COMP_EVENT`, and the IO manager consumes that event in task context.

When handling the comparator event, the manager decides the `PL_SIGNAL` output using:

- current HVC state
- battery-input floating status
- comparator output level

The important safety rules encoded there are:

- force `PL_SIGNAL` low if the HVC is errored
- force `PL_SIGNAL` low if the battery input appears floating
- force `PL_SIGNAL` high while charging
- otherwise follow the comparator result

This is one of the clearest places where safety behavior is intentionally separated from telemetry packaging.

## 6.3 BMS CAN Manager

Files:

- `Core/Src/Managers/bms_can_manager.c`
- `Core/Inc/Managers/bms_can_manager.h`
- `Core/Src/Drivers/bmb.c`
- `Core/Src/Drivers/charging.c`
- `Core/Src/Drivers/debug.c`

This manager owns the STM32 native CAN interface used for battery-module traffic.

### Responsibilities

- create BMS CAN RX/TX queues
- start `CAN1`
- enable RX and error interrupts
- move interrupt-side RX into a task-side queue
- queue outgoing messages by priority
- decode incoming battery-module and control messages through a dispatch table
- track bus-off/error-passive states and perform recovery with exponential backoff

### Dispatch-table pattern

Incoming messages are matched against a register of decoder/handler pairs:

- cell temperature summary
- ambient temperatures
- voltage summary
- BMS heartbeat
- reset command
- charging heartbeat

This is a strong local pattern because it keeps frame matching and side effects paired, while still letting the manager stay generic.

### Inbound battery-module data flow

Typical BMB telemetry path:

1. CAN RX interrupt fires
2. frame is copied into `BMS_CAN_RxQueue`
3. `BMS_CAN_ManagerTask` drains the queue
4. `bmb.c` decodes module-indexed payloads
5. `acc[]` shared module records are updated
6. `acc_manager` later consumes those records to compute higher-level pack outputs

That decoupling is central to the current architecture.

### Control-message path

The same bus also carries:

- reset commands
- charging requests

Those are handled through the same dispatch mechanism rather than a special side channel.

## 6.4 LV CAN Manager

Files:

- `Core/Src/Managers/lv_can_manager.c`
- `Core/Inc/Managers/lv_can_manager.h`
- `Core/Src/Drivers/spi_can.c`
- `Core/Inc/Drivers/spi_can.h`
- `Core/Src/Drivers/mcp2515.c`

This manager owns the low-voltage CAN path implemented through an MCP2515 attached to `SPI1`.

### Responsibilities

- initialize the MCP2515 CAN controller
- create LV CAN RX/TX queues
- transmit queued messages through `CANSPI_Transmit`
- drain received MCP2515 frames into the RX queue
- track LV-side bus-off and passive-error conditions
- reinitialize the MCP2515 after bus-off with backoff

### Interrupt handling model

The LV CAN interrupt line is attached to EXTI4.

The ISR does only one thing of architectural importance:

- set a thread flag on `SPI_IntCallbackHandle`

The real SPI drain work happens in `SPI_IntCallbackTask`, which:

- waits on that thread flag
- checks bus error status
- drains all pending RX messages from the MCP2515
- pushes them into the LV RX queue

That keeps SPI transactions out of interrupt context and is a good fit for RTOS-based firmware.

### Current LV dispatch scope

LV inbound dispatch is intentionally small right now. The current dispatch table only handles reset commands. Most application-originated traffic on LV CAN is outbound telemetry.

## 6.5 ACC Manager

Files:

- `Core/Src/Managers/acc_manager.c`
- `Core/Inc/Managers/acc_manager.h`
- `Core/Src/Data/acc.c`
- `Core/Inc/Data/acc.h`
- `Core/Src/Data/soc.c`
- `Core/Inc/Data/soc.h`
- `Core/Src/Drivers/current_limit.c`

This manager owns accumulator-wide derived values rather than raw battery-module traffic.

### Inputs

- per-module BMB summary data already stored in `acc[]`
- current-sense samples pushed by the IO manager
- OCV lookup table data

### Responsibilities

- drain queued current samples and integrate SOC delta
- calculate pack-wide min/max voltage and temperature summary
- initialize starting SOC from OCV once valid module coverage is available
- derive current limits from estimated OCV
- publish:
  - `CAN_ID_SOC` on LV CAN
  - `CAN_ID_ACC_SUMMARY` on LV CAN
  - `CAN_ID_CURRENT_LIMIT` on both LV CAN and BMS CAN

### Shared accumulator model

Each of the six modules has its own mutex-protected `Acc_Module_t`, storing:

- heartbeat freshness and error flag
- BMS1 voltage summary
- BMS2 voltage summary
- cell temperature summary
- ambient temperatures

The ACC manager does not own those structures outright; it consumes them after the CAN manager populates them.

## 7. Shared Data Ownership

## 7.1 IO data

`io.c` exposes global singleton objects such as:

- `sdc`, `imd`, `bms_fault`
- `cs_low_raw`, `cs_high_raw`, `therm`, `batt_raw`, `inv_raw`
- `ref_temp`
- `cs_low`, `cs_high`
- `batt`, `inv`

Each is a small struct with a mutex, value, and timestamp. This is the codebase's main pattern for shared board-level state.

## 7.2 State data

`state.c` owns:

- `bms_state`
- `bms_errors`
- `charge_flag`

Conceptually, both state and error-mask access are shared-state services. In practice, `bms_state` gets its mutex initialized, while `bms_errors` is treated as lockable but does not currently have an initialization path. That is worth keeping in mind when evolving the state layer.

## 7.3 ACC data

`acc.c` owns:

- six module records through `acc[NUM_ACC_MODULES]`
- `acc_summary`

This gives the rest of the firmware a central in-memory model of pack health and battery-module telemetry.

## 7.4 SOC data

`soc.c` keeps:

- integrated delta in A*s
- last timestamp
- starting capacity derived from OCV
- a mutex-protected SOC snapshot

The module is intentionally small and stateless from the outside: other code only pushes current samples, sets starting capacity, or requests snapshots.

## 8. CAN Design

## 8.1 Two-bus model

The HVC uses two independent CAN paths:

- native STM32 CAN for BMS traffic
- MCP2515-over-SPI CAN for LV traffic

This is reflected in the separate managers, separate queue sets, and different interrupt/recovery mechanisms.

## 8.2 HVC-originated CAN messages

Current HVC-owned message IDs include:

- `CAN_ID_IO_SUMMARY`
- `CAN_ID_STATE`
- `CAN_ID_ERRORED_PANIC`
- `CAN_ID_IO_CURRENT`
- `CAN_ID_SOC`
- `CAN_ID_ACC_SUMMARY`
- `CAN_ID_RESET`
- `CAN_ID_IO_VSENSE`
- `CAN_ID_CURRENT_LIMIT`
- `CAN_ID_CHARGING_REQUEST`

The helper script `tools/generate_io_dbc.py` parses `can_id.h` and `state.h` to generate `hvc.dbc` for the HVC-originated telemetry set.

## 8.3 Battery-module CAN decoding

Battery-module frame families are described in `Core/Inc/Config/can_id.h`. The decoders in `bmb.c` currently focus on summarized battery-module data rather than reconstructing every raw cell-voltage frame into a large internal table.

That keeps the runtime model compact:

- min/max/avg temperatures
- min/max/avg voltages
- ambient temperatures
- heartbeat/error state

## 8.4 Cross-bus mirroring and debug coupling

Two compile-time switches in `Core/Inc/Data/can.h` are architecturally important:

- `BMS_ECHO_MSGS`
- `DEBUG`

With the current defaults:

- inbound BMS messages are echoed onto LV CAN
- outbound LV CAN messages are also mirrored onto BMS CAN

This effectively creates a debug-oriented bridge between buses. It can be useful for visibility, but it also means the two CAN domains are not fully isolated in the current build configuration. Anyone changing message ownership or bus semantics should account for that coupling first.

## 9. Interrupt And Concurrency Model

The firmware uses three different synchronization styles:

- mutexes for shared state containers
- message queues for decoupling producers from manager tasks
- event flags and thread flags for lightweight signaling

### Current concurrency patterns

- native BMS CAN RX:
  - ISR copies frame into a queue
  - manager task processes it later
- LV CAN RX:
  - ISR sets a thread flag
  - dedicated callback task performs SPI reads
  - manager task later processes queued frames
- comparator safety event:
  - ISR sets `IO_COMP_EVENT`
  - IO manager decides output behavior in thread context
- current samples:
  - IO manager pushes samples into ACC queue
  - ACC manager drains and integrates them

That is the core real-time shape of the firmware.

## 10. Build, Flash, And Operational Tooling

## 10.1 Firmware build files

There are two top-level firmware makefiles, but they do not play the same role:

- `STM32Make.make`
  - current source-of-truth build file for the present repo layout
- `Makefile`
  - older generated file that still references previous source locations such as `Core/Src/CANSPI.c` and `Core/Src/can_manager.c`

For onboarding and automation, new contributors should use `STM32Make.make` or the VS Code STM32 tasks rather than assuming the root `Makefile` is authoritative.

## 10.2 Dual-bank build flow

`dual_build.mk` wraps `STM32Make.make` to produce:

- bank A artifacts with `VECT_TAB_OFFSET=0x8000`
- bank B artifacts with `VECT_TAB_OFFSET=0x22000`

That is consistent with the repository's bootloader/application-bank concerns, but it also means vector-table assumptions should be reviewed any time boot strategy changes.

## 10.3 Flash path

`openocd.cfg` is the local OpenOCD target file for ST-Link and STM32L4x.

## 10.4 Test strategy

Host tests are built with:

- Unity
- CMock
- repo-local stubs
- `tests.mk`

The tests are intentionally separate from the embedded toolchain and can run in a Docker container defined by `Dockerfile.test`. The GitHub workflow uses that containerized path for CI.

## 10.5 Tool scripts

`tools/` contains two especially important generators:

- `generate_io_dbc.py`
  - builds `hvc.dbc` directly from source headers
- `generate_ocv_table.py`
  - regenerates the compiled OCV table from CSV battery data

These scripts reduce documentation drift by turning source headers and calibration data into checked-in artifacts.

## 11. Typical Data Flows

## 11.1 Battery telemetry to pack summary

```text
BMB frame
-> BMS CAN interrupt
-> BMS RX queue
-> bmb decoder
-> acc[module]
-> Acc_CalculateSummary()
-> ACC summary CAN message
```

## 11.2 Current measurement to SOC/current limit

```text
ADC current samples
-> IO manager conversion + filtering
-> Acc_CurrSenseQueue_Push()
-> Acc manager drains samples
-> Soc_UpdateDeltaFromCurrSample()
-> OCV-backed current-limit calculation
-> SOC/current-limit CAN messages
```

## 11.3 Charging request to state transition

```text
Charging request CAN frame
-> charging decoder on BMS bus
-> charge_flag event
-> State manager sees CHARGING_EVENT
-> RUNNING -> CHARGING transition
```

## 11.4 Safety fault to physical outputs

```text
temperature / floating input / module error
-> State manager error mask
-> logical bms_fault value
-> IO manager writes BMS_Fault output pin

comparator interrupt + state context
-> IO manager decides PL_SIGNAL
```

## 12. Strong Design Patterns Already Present

- Clear split between orchestration (`Managers`) and state storage (`Data`)
- Queue-based separation between interrupt context and task context
- Dispatch-table decoding for CAN message families
- Shared, mutex-protected singleton data objects instead of uncontrolled globals
- Dedicated host-test harness for manager and driver logic
- Small helper scripts that derive checked-in artifacts from source data

These are the patterns worth preserving when the codebase grows.

## 13. Known Maintenance Notes

These are not rewrite requests; they are the main caveats a maintainer should understand before making architectural changes.

- The root `Makefile` is older than the current source tree. Prefer `STM32Make.make` and `dual_build.mk`.
- `freertos.c` is currently just generated scaffolding; application tasks are created in `main.c`.
- `BALANCING` exists as a state enum but is not yet part of the active state-machine transitions.
- CAN-link fault tracking exists in both CAN managers, but those error classes are disabled in the state manager by default.
- `CAN_ID_ERRORED_PANIC` and `CAN_ID_CHARGING_REQUEST` are still marked as TODO IDs in `can_id.h`.
- The OCV table is generated and explicitly notes that it does not account for voltage droop.
- The current build configuration mirrors traffic between CAN domains for debug visibility, so the two buses are not fully isolated at the application layer.

## 14. Extension Guide

## 14.1 Adding a new HVC-originated telemetry message

1. Add the CAN ID in `Core/Inc/Config/can_id.h`.
2. Decide which manager owns the data and transmission cadence.
3. Add a packing helper close to that manager.
4. Queue the message through `LV_CAN_SendMessage()` and/or `BMS_CAN_SendMessage()`.
5. Regenerate `hvc.dbc` with `tools/generate_io_dbc.py` if the message belongs to the generated HVC telemetry set.
6. Add or update host tests if the packing logic is non-trivial.

## 14.2 Adding a new inbound BMB message

1. Add or confirm the ID pattern in `can_id.h`.
2. Add a decoder and handler pair in `bmb.c`.
3. Register the pair in `BMS_DispatchRegister`.
4. Store the result in `acc[]` or another shared data module.
5. Extend higher-level summary logic only if the rest of the system actually needs the new value.

## 14.3 Adding a new safety fault

1. Add an error bit in `state.h`.
2. Decide which manager detects the condition.
3. Expose that condition through shared data or event flags.
4. Add the check to `_State_CheckErrors()`.
5. Regenerate `hvc.dbc` so the state message documentation stays aligned.

## 15. Bottom Line

The most useful mental model for this repository is:

- `IO manager` observes the local board.
- `BMS CAN manager` ingests battery-module traffic.
- `ACC manager` turns module data and current samples into pack-level estimates.
- `State manager` decides whether the HVC is safe and what top-level mode it is in.
- `LV CAN manager` publishes and receives low-voltage network traffic through the MCP2515 path.

If a new engineer starts from that model and then drills down into the specific manager, data, and driver files, the rest of the codebase becomes much easier to reason about.
