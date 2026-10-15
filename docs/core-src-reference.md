# Core/Src Firmware Reference

## Purpose

This document is a source-level reference for the hand-written and generated code under `Core/Src`.
It complements `docs/codebase-design.md` by going file-by-file through the runtime structure of the firmware and explaining how the major functions of the HVC work in the current implementation.

The codebase targets an STM32L432 and runs a FreeRTOS-based application that:

- samples analog and digital board inputs
- receives and decodes accumulator-module telemetry on the BMS CAN bus
- bridges and publishes telemetry on both CAN buses
- estimates SOC and current limits from pack data
- controls top-level outputs such as `BMS_Fault` and `PL_SIGNAL`
- enforces a top-level HVC state machine with fault handling

## Scope

This document covers everything in `Core/Src`:

- top-level startup and generated STM32 support files
- `Managers/`
- `Data/`
- `Drivers/`
- `config/ocv_lookup_table.c`

It does not try to fully re-document vendor libraries under `Drivers/` and `Middlewares/`, but it does explain how the `Core/Src` code uses them.

## Core/Src Layout

```text
Core/Src/
  config/
    ocv_lookup_table.c
  Data/
    acc.c
    can.c
    io.c
    moving_average.c
    soc.c
    state.c
  Drivers/
    bmb.c
    charging.c
    current_limit.c
    curr_sense.c
    debug.c
    mcp2515.c
    spi_can.c
    therm.c
    vsense.c
  Managers/
    acc_manager.c
    bms_can_manager.c
    io_manager.c
    lv_can_manager.c
    state_manager.c
  freertos.c
  main.c
  stm32l4xx_hal_msp.c
  stm32l4xx_hal_timebase_tim.c
  stm32l4xx_it.c
  syscalls.c
  sysmem.c
  system_stm32l4xx.c
```

The ownership split is consistent across the firmware:

- `Managers/` own task loops, scheduling, queue draining, event handling, and side effects.
- `Data/` own shared runtime state and synchronization.
- `Drivers/` own raw conversion and protocol decoding close to hardware or wire format.
- top-level `Core/Src/*.c` files handle boot, interrupts, HAL glue, and generated runtime support.

## Runtime Overview

At runtime the firmware is structured around seven threads:

| Task | Priority | Purpose |
| --- | --- | --- |
| `IO_ManagerTask` | realtime | Sample ADCs, evaluate floating inputs, drive `PL_SIGNAL`, publish IO telemetry |
| `BMS_CAN_ManagerTask` | high | Drain native CAN1 RX/TX queues, dispatch BMS messages to handlers |
| `State_ManagerTask` | high | Evaluate errors, update top-level HVC state, drive `BMS_Fault`, publish state |
| `Acc_ManagerTask` | high | Aggregate module data, integrate current for SOC, compute current limits |
| `LV_CAN_ManagerTask` | high | Poll MCP2515 for LV CAN RX/TX and handle LV-bus recovery |
| `SPI_IntCallbackTask` | normal | Wait on the LV CAN interrupt flag; currently acts as a wake placeholder |
| `LED_BlinkTask` | low | Blink the status LED |

The main system data flows are:

1. `main.c` initializes clocks, peripherals, managers, and tasks.
2. `IO_ManagerTask` continuously converts ADC readings into filtered current, voltage, and temperature values.
3. `BMS_CAN_ManagerTask` receives accumulator-module heartbeat, voltage-summary, and temperature-summary frames and writes them into `acc[]` shared state.
4. `Acc_ManagerTask` reads `acc[]` and current samples to compute pack summary, SOC, and current limits.
5. `State_ManagerTask` reads shared state and event flags to decide whether the HVC is in `PRE_INIT`, `RUNNING`, `CHARGING`, or `ERRORED`.
6. Application-originated telemetry is sent through `HVC_CAN_SendMessage()`, which broadcasts it to both CAN buses.

## Startup And Boot Sequence

### `main.c`

`main.c` is the application entry point and the top-level integration file.

Its startup sequence is:

1. disable interrupts and SysTick to cleanly transition from the bootloader
2. call `HAL_Init()`
3. configure the clock tree in `SystemClock_Config()`
4. initialize GPIO, CAN1, CRC, comparator, ADC1, and SPI1
5. initialize the RTOS kernel
6. initialize all manager subsystems
7. create all FreeRTOS tasks with static allocation
8. start the scheduler

Key implementation details:

- The comment notes that vector-table relocation is handled in `SystemInit()` rather than directly in `main()`.
- CAN filtering is configured in `MX_CAN1_Init()` before `HAL_CAN_Start()`, and the filter currently accepts all extended IDs.
- The BMS CAN peripheral is configured for robust behavior by enabling automatic bus-off management and clearing `NART` so transient failures can retry.
- GPIO initialization defines the firmware-controlled outputs `PL_SIGNAL`, `BMS_Fault`, and the LED, and it configures `LV_CAN_INT` as a falling-edge interrupt.
- `Error_Handler()` is intentionally firmware-specific: it disables interrupts, drives `BMS_Fault` low, and repeatedly sends a panic frame on both CAN buses containing subsystem initialization flags.

### `system_stm32l4xx.c`

This is the STM32 system startup file. It owns reset-time MCU setup and the system clock bookkeeping used by HAL. The application relies on it for vector-table relocation and early core configuration.

### `stm32l4xx_hal_msp.c`

This is HAL support-package glue generated by STM32 tools. It enables peripheral clocks and binds GPIO alternate functions for ADC, CAN1, SPI1, timer, and comparator support.

### `stm32l4xx_hal_timebase_tim.c`

This file provides the HAL time base using TIM6 instead of SysTick. That matters because FreeRTOS is running and the project avoids relying on SysTick for both RTOS scheduling and HAL timekeeping.

### `freertos.c`

This file is mostly an auto-generated placeholder. The real application tasks live in `Managers/` and `main.c`.

### `syscalls.c` and `sysmem.c`

These are the usual newlib support files for heap/syscall integration. They are part of the runtime environment but not part of application logic.

## Interrupt Model

### `stm32l4xx_it.c`

This file contains the interrupt handlers used by the firmware.

Important active interrupts:

- `EXTI4_IRQHandler()` handles the MCP2515 interrupt line and sets a thread flag for `SPI_IntCallbackTask`.
- `CAN1_TX_IRQHandler()`, `CAN1_RX0_IRQHandler()`, `CAN1_RX1_IRQHandler()`, and `CAN1_SCE_IRQHandler()` hand BMS CAN interrupts to HAL.
- `TIM6_DAC_IRQHandler()` services the HAL time base.
- `COMP_IRQHandler()` exists and forwards to HAL, but the actual comparator decision logic is performed by polling in `IO_ManagerTask`.

Notable nuance:

- The LV CAN interrupt path is intentionally light in ISR context, but the current code does not actually perform SPI work inside `SPI_IntCallbackTask`. The task waits on the flag and returns to waiting; the real LV CAN RX work is still done by polling inside `LV_CAN_ManagerTask`.

## Manager Layer

### `Managers/io_manager.c`

This is the highest-priority application task and the board-I/O control surface.

Responsibilities:

- calibrate ADC1 and start the comparator during initialization
- initialize all shared IO objects in `Data/io.c`
- read raw ADC channels for current sense, battery sense, inverter sense, thermistor, and internal Vref
- convert raw readings into engineering units using the driver layer
- maintain floating-input event flags
- drive `PL_SIGNAL` based on comparator state and top-level HVC state
- publish IO summary, current, voltage, and PL-signal reason messages
- push current samples into the accumulator manager queue for SOC integration

Scheduling model:

- The task runs every `IO_PRIORITY_UPDATE_FREQ_MS` which is `10 ms`.
- Every cycle it runs `_IO_HighPriority()` and `_IO_HandleCompEvent()`.
- It runs `_IO_LowPriority()` at a divided-down cadence of `IO_UPDATE_FREQ_MS`, which is `100 ms`.

How it works:

- `_IO_HighPriority()` reads the analog inputs, computes `vref`, converts battery and inverter voltages, converts both current-sense channels, filters them with moving averages, updates shared IO state, and pushes a timestamped current sample to `Acc_CurrSenseQueue`.
- `_IO_CalculateVREF()` reads `ADC_CHANNEL_VREFINT` and rejects implausible values so a bad sample does not corrupt scaling.
- `_IO_UpdateFloatingInputFlags()` treats low battery voltage as a floating battery input and rail-stuck current-sense ADC values as floating current-sense input. It then sets or clears bits in `floating_input_flag`.
- `_IO_HandleCompEvent()` debounces the comparator and decides whether `PL_SIGNAL` should follow the comparator or be forced open/closed. It forces `PL_SIGNAL` open on `ERRORED` or floating battery input, forces it closed during `CHARGING`, and otherwise mirrors the debounced comparator output.
- `_IO_LowPriority()` samples slow-changing inputs like SDC, IMD, and reference temperature, then sends `CAN_ID_IO_SUMMARY`, `CAN_ID_IO_CURRENT`, and `CAN_ID_IO_VSENSE`.

Important caveats:

- Comparator trigger interrupts are configured, but the actual logic is polled, not interrupt-driven.
- Floating current-sense input forces reported currents to zero before they are handed to the SOC path.
- Battery voltage is scaled with `INVERSE_PRECHARGE_FACTOR`, so the published pack voltage is intentionally corrected from the sensed node to the actual input voltage.

### `Managers/bms_can_manager.c`

This task owns the native STM32 CAN1 bus that carries BMB traffic.

Responsibilities:

- create and own BMS CAN RX/TX queues
- start CAN1 and activate RX/error notifications
- receive messages from the HAL callbacks and queue them for task-context processing
- transmit queued outgoing messages with bounded retries
- dispatch incoming BMS messages through a static decoder/handler table
- track bus-off and error-passive events and recover the bus with backoff

Receive path:

1. A CAN RX interrupt fires.
2. HAL callback code reads the frame from FIFO 0 or FIFO 1.
3. `BMS_CAN_QueueRXMessage()` converts it into `CAN_Message_t` and pushes it onto the RX queue.
4. `BMS_CAN_ManagerTask()` drains the queue and passes each message into `BMS_CAN_ProcessRXMessage()`.
5. `BMS_CAN_ProcessRXMessage()` runs every entry in `BMS_DispatchRegister` and invokes the first matching decode/handle pair.

Transmit path:

1. Application code calls `BMS_CAN_SendMessage()` or `BMS_CAN_SendMessageWithTimeout()`.
2. The message is copied into the TX queue.
3. `BMS_CAN_ManagerTask()` drains the queue and sends each frame via `HAL_CAN_AddTxMessage()` with retry handling.

Current dispatch table from `Core/Inc/Managers/bms_can_manager.h`:

- cell temperature summary
- ambient temperatures
- voltage summary
- BMS heartbeat
- reset command
- charging heartbeat

Important caveats:

- `BMS_ECHO_MSGS` is enabled, so every received BMS frame is echoed onto the LV CAN bus.
- The code declares raw per-cell voltage decode handlers in `bmb.h`, but the current implementation only wires voltage-summary messages, not individual cell-voltage frames.
- Bus-link error checking exists and timestamps recent failures, but whether those errors influence HVC state depends on compile-time checks in `state_manager.h`.

### `Managers/lv_can_manager.c`

This task owns the low-voltage CAN path via an MCP2515 on SPI1.

Responsibilities:

- initialize the MCP2515 abstraction through `CANSPI_Initialize()`
- own LV CAN RX/TX queues
- poll MCP2515 for received frames
- queue and transmit outbound LV frames
- recover from bus-off or error-passive conditions with exponential backoff
- dispatch a small set of received LV messages

Current dispatch table from `Core/Inc/Managers/lv_can_manager.h`:

- reset command
- BMB passthrough frames

How it works:

- `LV_CAN_ManagerTask()` runs every `1 ms`.
- It first polls the MCP2515 directly with `CANSPI_Receive()`.
- It then drains its software RX queue, although there is currently no separate producer path filling that queue in the firmware.
- It drains the TX queue and transmits through `CANSPI_Transmit()`.
- It checks MCP2515 error state every cycle and reinitializes the interface if bus-off is detected.

Important caveats:

- The LV interrupt thread exists, but actual LV RX remains poll-driven in the manager task.
- The software RX queue is effectively unused in the present code path.
- LV CAN traffic is intentionally minimal; it mainly supports reset and forwarding BMB passthrough frames back into the BMS bus.

### `Managers/acc_manager.c`

This task turns per-module telemetry into pack-level telemetry.

Responsibilities:

- initialize the six-module `acc[]` data model and summary storage
- own the current-sense sample queue used for SOC integration
- initialize the SOC subsystem
- compute pack min/max cell voltage and temperature across all modules
- estimate starting SOC from OCV once all modules have reported valid data
- integrate current over time through `soc.c`
- compute current limits and publish them
- publish SOC and accumulator summary telemetry

How it works:

- `Acc_ManagerTask()` first drains `Acc_CurrSenseQueueHandle` and passes each sample to `Soc_UpdateDeltaFromCurrSample()`.
- It then calls `Acc_CalculateSummary()` to derive pack min/max voltage and temperature from the six module records.
- Once all six modules have reported valid heartbeat, voltage, and temperature information, it performs a one-time starting-capacity estimate with `SOC_UpdateStartingCapacityFromVolt(summary.volt_min_mV)`.
- After that, it reads an SOC snapshot, calculates current limits from the OCV estimate, and publishes three messages:
  - `CAN_ID_CURRENT_LIMIT`
  - `CAN_ID_SOC`
  - `CAN_ID_ACC_SUMMARY`

Important caveats:

- Pack summary validity requires all of the needed fields to be non-zero for a module. Partial module data is ignored.
- Starting SOC is estimated once from pack minimum cell voltage and then maintained by coulomb counting.
- Current-limit calculation is currently based only on cell-voltage headroom; temperature and imbalance are not yet in the pipeline.

### `Managers/state_manager.c`

This task is the top-level behavioral state machine of the HVC.

Responsibilities:

- initialize the shared state mutex and event flags
- evaluate error conditions from IO, accumulator modules, and CAN link state
- handle transitions between `PRE_INIT`, `RUNNING`, `CHARGING`, and `ERRORED`
- drive the logical `bms_fault` shared value
- publish the top-level state and error bitmask on CAN

State model:

- `PRE_INIT`
- `RUNNING`
- `CHARGING`
- `BALANCING`
- `ERRORED`

Notes on current usage:

- `BALANCING` exists in the enum but is not actively used in the transition logic.
- The task runs every `100 ms`.

Transition behavior:

- `PRE_INIT -> RUNNING` when IO, BMS CAN, and LV CAN are initialized and no enabled error bits are set
- `PRE_INIT -> ERRORED` when initialization is complete but enabled errors are already present
- `RUNNING -> CHARGING` when a charging-request event is seen
- `RUNNING -> ERRORED` when any enabled error bit is set
- `CHARGING -> ERRORED` when an error appears or charging requests stop for more than `MAX_CYCLES_WITHOUT_CHARGE_REQUEST`
- `ERRORED -> RUNNING` when all enabled errors clear

Error sources:

- reference-board overtemperature
- battery-input floating detection
- current-sense floating detection
- per-module BMB error status
- optional module timeout detection
- optional BMS CAN error detection
- optional LV CAN error detection

Compile-time fault gates in `Core/Inc/Managers/state_manager.h`:

- `CHECK_REF_OVERTEMP = 1`
- `CHECK_BATT_FLOATING = 1`
- `CHECK_CURR_SENSE_FLOATING = 1`
- `CHECK_BMB_ERRORS = 1`
- `CHECK_MODULE_TIMEOUT = 0`
- `CHECK_BMS_CAN_ERRORS = 0`
- `CHECK_LV_CAN_ERRORS = 0`

That means module timeout and CAN-link failures are implemented but are not currently promoted into the active HVC error mask by default.

## Data Layer

### `Data/io.c`

This file owns the shared IO objects used by the rest of the firmware.

Objects provided:

- digital values: `sdc`, `imd`, `bms_fault`
- raw analog values: `cs_low_raw`, `cs_high_raw`, `therm`, `batt_raw`, `inv_raw`
- converted values: `ref_temp`, `cs_low`, `cs_high`, `batt`, `inv`

What it does:

- creates per-object mutexes
- stores values and timestamps
- provides thread-safe getters and setters
- initializes moving-average state for current and voltage channels

This layer is intentionally simple. It does not decide how values are produced; it only provides synchronized storage.

### `Data/acc.c`

This file owns the shared accumulator-module model.

What is stored per module:

- last heartbeat timestamp
- module error flag
- BMS1 voltage summary
- BMS2 voltage summary
- cell temperature summary
- ambient temperatures

Global shared objects:

- `acc[6]` for the six physical modules
- `acc_summary` for pack-level min/max values

What it does:

- provides thread-safe getters and setters for each module record
- calculates the pack summary across all valid modules
- tracks whether `acc_summary` is valid yet

Important behavior:

- `Acc_CalculateSummary()` skips any module whose heartbeat, voltage summary, or temperature summary has not arrived yet.
- Summary data only becomes readable through `Acc_GetSummary()` after at least one valid summary has been calculated.

### `Data/soc.c`

This file owns state-of-charge estimation.

What it does:

- tracks a mutex-protected `SOC_Snapshot_t`
- integrates current over time in ampere-seconds
- chooses between low-range and high-range current-sense channels based on magnitude
- ignores very small currents inside a deadband
- estimates initial capacity from the OCV lookup table
- performs reverse lookup from SOC back to OCV for the current-limit logic

How the estimator works:

1. `Soc_Init()` resets runtime state and creates the mutex.
2. `Soc_UpdateDeltaFromCurrSample()` receives timestamped current samples from the accumulator manager.
3. It selects `cs_high` when the low-range channel magnitude crosses the switch threshold.
4. It integrates current over `dt_ms`, preserving remainder error in `soc_delta_remainder_mAms`.
5. Once starting capacity has been set, it updates `soc_capacity_As` and `soc_pctx100`.

Important caveats:

- The first current sample only establishes a timestamp and does not change SOC.
- Starting capacity is based on OCV and then held fixed until explicitly reset.
- Coulomb counting can drift over time if there is no later re-synchronization to a known pack-rest condition.

### `Data/state.c`

This file owns the mutex-protected top-level state and error mask.

It provides:

- `State_InitState()`
- `State_GetState()` / `State_SetState()`
- `State_GetErrorMask()` / `State_SetErrorMask()`

It also defines `charge_flag`, which is set by the charging decoder and consumed by the state manager.

### `Data/can.c`

This file provides the common application-level CAN send helper.

`HVC_CAN_SendMessage()` broadcasts every application-originated frame to both buses by calling:

- `BMS_CAN_SendMessage()`
- `LV_CAN_SendMessage()`

It also owns the shared statistics reset helper.

Design implication:

- The HVC treats most of its own telemetry as dual-published by default.
- If either bus send path fails, the function returns `HAL_ERROR` even if the other bus succeeded.

### `Data/moving_average.c`

This file implements a simple exponential moving average filter used by the IO layer.

- `MovingAverage_Init()` converts a nominal window size into a smoothing weight.
- `MovingAverage_Update()` applies the exponential filter and returns the filtered integer value.

## Driver Layer

### `Drivers/bmb.c`

This is the BMS-message decode layer for accumulator-module traffic.

Implemented decode paths:

- cell temperature summary
- ambient temperatures
- BMS heartbeat
- BMS1 voltage summary
- BMS2 voltage summary

What the handlers do:

- write cell temperature summary into the correct `acc[module]`
- write ambient temperatures into the correct `acc[module]`
- write heartbeat timestamp and module error status into the correct `acc[module]`
- write BMS1 or BMS2 voltage summary into the correct `acc[module]`

Important behavior:

- Module identity is extracted from bits `15:12` of the extended CAN ID.
- Voltage summary handlers update only pack summary fields, not raw per-cell arrays.
- `bmb.h` still declares raw cell-voltage decode functions, but that path is not implemented in the current source file.

### `Drivers/charging.c`

This file implements the charging-request decoder.

- It matches a single CAN ID: `CAN_ID_CHARGING_REQUEST`.
- The handler sets `CHARGING_EVENT` in `charge_flag`.

In practice this means charging is controlled as a heartbeat-style request: the state manager must keep seeing the event regularly or it exits charging by faulting.

### `Drivers/debug.c`

This file implements small but powerful debug/control hooks.

Implemented behaviors:

- reset command on the BMS bus
- reset command on the LV bus
- LV-side passthrough of BMB frames into the native BMS bus

What they do:

- both reset handlers call `NVIC_SystemReset()` immediately
- BMB passthrough reconstructs the 8-byte frame from the MCP2515 message structure and sends it onto the BMS bus at critical priority with zero queue wait

This is mainly useful for diagnostics and remote interaction with the battery-module network from the LV side.

### `Drivers/current_limit.c`

This file calculates pack current limits from cell-voltage headroom.

What it does:

- starts from loose limits (`UINT32_MAX`)
- runs a pipeline of limit functions that can only tighten the limits
- currently uses only one stage: cell-voltage-based limiting

Current logic:

- positive current limit is based on distance from minimum safe cell voltage
- negative current limit is based on distance from maximum safe cell voltage
- both are converted by Ohm's law using internal-resistance assumptions and pack scaling constants from `cells.h`
- a safety factor is subtracted from the negative limit

The pipeline shape makes it straightforward to add thermal or imbalance constraints later.

### `Drivers/curr_sense.c`

This file converts raw ADC readings into current for two sensor ranges.

Provided conversions:

- `Curr_CalculateCurrentSenseHigh()` for the high-current channel
- `Curr_CalculateCurrentSenseLow()` for the low-current channel

Both functions encode fixed calibration/math derived from the current-sensor transfer function and the board divider ratio. They return current in milliamps.

### `Drivers/vsense.c`

This file converts an ADC sample into a sensed voltage in millivolts using the configured divider ratio.

It is intentionally simple and is used by the IO manager after ADC sampling.

### `Drivers/therm.c`

This file converts the thermistor ADC reading into a board reference temperature.

What it does:

- converts ADC code to voltage
- infers thermistor resistance from the divider
- applies the B-parameter equation
- clamps the result to a reasonable temperature range
- returns `-127 C` when the thermistor looks disconnected

That value is later used by the state manager for the reference-board overtemperature check.

### `Drivers/mcp2515.c`

This is the low-level MCP2515 register-access driver. It handles SPI transactions, mode switching, register reads/writes, and controller configuration details for the LV CAN path.

This file is infrastructure, not application logic.

### `Drivers/spi_can.c`

This file sits above `mcp2515.c` and provides a CAN-message abstraction for the LV CAN manager.

What it does:

- initialize MCP2515 CAN settings and masks
- transmit LV CAN frames
- receive LV CAN frames from MCP2515 RX buffers
- report bus-off and passive-error status

This is the LV-CAN equivalent of a hardware abstraction layer.

## Generated Configuration Data

### `config/ocv_lookup_table.c`

This file provides the open-circuit-voltage lookup table used by `soc.c`.

The accumulator manager uses it in two ways:

- one-time initial SOC estimate from minimum cell voltage
- reverse lookup from SOC to approximate OCV for current-limit estimation

It should be treated as generated data rather than hand-maintained business logic.

## End-To-End Functional Flows

## 1. Board sensing and telemetry

1. `IO_ManagerTask` samples ADC channels for battery voltage, inverter voltage, both current sensors, thermistor, and Vref.
2. Driver functions convert raw ADC readings into physical units.
3. The IO data layer stores filtered values with timestamps.
4. Floating-input logic updates event flags.
5. IO telemetry is published on both CAN buses through `HVC_CAN_SendMessage()`.

Outputs involved:

- `CAN_ID_IO_SUMMARY`
- `CAN_ID_IO_CURRENT`
- `CAN_ID_IO_VSENSE`
- `CAN_ID_PL_SIGNAL`

## 2. Accumulator-module telemetry ingest

1. BMS module frames arrive on CAN1.
2. HAL callbacks queue them into the BMS RX queue.
3. `BMS_CAN_ManagerTask` drains the queue and runs the dispatch table.
4. `bmb.c` decoders identify heartbeat, voltage-summary, and temperature-summary frames.
5. Handlers store decoded data into `acc[module]`.

This is how the rest of the firmware learns pack-wide cell voltage, temperature, and module health.

## 3. SOC and current-limit estimation

1. `IO_ManagerTask` pushes filtered current samples to the accumulator queue.
2. `Acc_ManagerTask` drains the queue and updates SOC integration.
3. Once all module summaries are valid, the accumulator manager estimates initial capacity from OCV.
4. It reads the SOC snapshot and converts SOC back into OCV.
5. `current_limit.c` turns OCV headroom into positive and negative current limits.
6. The task publishes `CAN_ID_SOC`, `CAN_ID_ACC_SUMMARY`, and `CAN_ID_CURRENT_LIMIT`.

## 4. Charging control

1. A charging-request frame arrives on the BMS bus.
2. `charging.c` sets `CHARGING_EVENT`.
3. `State_ManagerTask` consumes the event on its next cycle.
4. If there are no active enabled errors, the state machine moves from `RUNNING` to `CHARGING`.
5. While charging, `IO_ManagerTask` forces `PL_SIGNAL` asserted regardless of the comparator.
6. If charging requests stop arriving for too long, the state manager enters `ERRORED`.

## 5. Fault handling and top-level outputs

1. `State_ManagerTask` checks enabled error sources every `100 ms`.
2. It updates the global error mask.
3. It decides the top-level HVC state.
4. It drives logical `bms_fault` false in `ERRORED`, true otherwise.
5. `IO_ManagerTask` mirrors that logical value to the physical `BMS_Fault` output pin.
6. `IO_ManagerTask` also decides whether `PL_SIGNAL` is allowed to follow the comparator.

This split is important:

- `State_ManagerTask` decides whether the system is healthy.
- `IO_ManagerTask` owns the physical board outputs and enforces the precharge/output rules.

## 6. CAN publication model

Application-originated messages are normally broadcast on both CAN buses through `HVC_CAN_SendMessage()`.

This includes:

- state
- IO telemetry
- accumulator summary
- SOC
- current limits
- PL-signal reason

Separately:

- all received BMS frames are echoed onto LV CAN because `BMS_ECHO_MSGS` is enabled
- selected LV frames can be forwarded back to the BMS bus through the debug passthrough handler

## Notable Implementation Realities

These details are worth knowing before changing behavior:

- The source tree uses a clean `Managers -> Data/Drivers` dependency direction for most application logic.
- `BALANCING` is defined but inactive in the current state machine.
- Module timeout and CAN-link fault checks exist but are compile-time disabled by default.
- LV CAN RX is still effectively polled even though there is an interrupt thread scaffold.
- Raw per-cell voltage decode hooks are declared but not currently implemented in `bmb.c`.
- `Error_Handler()` is not generic HAL boilerplate; it is an application panic path that repeatedly broadcasts subsystem status.

## Practical Reading Order

For a new developer trying to understand the firmware quickly, the best order is:

1. `main.c`
2. `Managers/state_manager.c`
3. `Managers/io_manager.c`
4. `Managers/bms_can_manager.c`
5. `Managers/acc_manager.c`
6. `Data/acc.c`, `Data/state.c`, `Data/io.c`, `Data/soc.c`
7. `Drivers/bmb.c`, `Drivers/charging.c`, `Drivers/current_limit.c`, `Drivers/debug.c`
8. `Managers/lv_can_manager.c` and the MCP2515 driver files

That order follows the real control flow from boot to sensing to state to telemetry.