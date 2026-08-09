# benchmark_hms (BMTH)

RTOS-agnostic C11 framework for measuring code sections in CPU cycles on
bare-metal targets.

The library performs no hardware access itself. All target-specific code is
either a *core layer* shipped with the package or a *port layer* supplied by
the integrating application.

The API is documented in the sources with Doxygen. This file covers purpose,
contents and configuration.

## Purpose

BMTH standardizes the measurement cycle so that all measurements in a project
are taken identically and remain comparable. It provides:

- One way to open and close a measured region.
- One way to determine the constant overhead of reading the counter and to
  subtract it from every sample.
- One way to accumulate iterations into a measurement series.
- Barriers and volatile time markers around every counter read, so the measured
  region is not reordered, hoisted or optimised away at any optimisation level.

A series records minimum, maximum, average, outlier count and, optionally, every
raw sample. Samples deviating from their predecessor are counted and reported as
jitter; acting on that is the caller's decision.

The precondition checks are separate from this. They assert that trace,
sampling, watchpoints, hardware breakpoints, DMA, ECC, caches and the second
core are quiet, and halt the CPU on violation.

Results and events are signalled out of band through three toggle hooks —
success, failure, event — since the code under measurement cannot print.

## Contents

| Path | Contents |
| --- | --- |
| `benchmark_tools_hms.h` | Public interface: measurement series type, timing macros, assertion macro, prototypes. |
| `benchmark_tools_hms.c` | Portable implementation: warm-up, overhead calibration, measurement series, hardware-influence check, signalling, optional global time store. |
| `cores/bmth_m33.c` | Cortex-M33 core layer: cycle counter control, SysTick gating, quiet-system assertions (DWT, ITM/SWO, ETM located via the CoreSight ROM table, FPB, DEMCR, MCXN947 SoC noise sources). |
| `port/bmth_port.h` | Interface to be implemented by the application. |
| `port/templates/bmth_config_template.h` | Template for the application's configuration header. |
| `CMakeLists.txt` | Defines the CMake INTERFACE library `bmth`. |
| `Kconfig` | Kconfig symbols for a Zephyr or other Kconfig-based build. |

## Layering

The package is incomplete on its own. The application supplies:

- **Configuration header** — a `bmth_config.h` on the include path, binding the
  counter to hardware and setting the options below. Derive it from the
  template under `port/templates/`.
- **Port layer** — hardware init and the three signal toggles declared in
  `port/bmth_port.h`. Counter enabling and SysTick gating are not part of it;
  they belong to the core layer.

In this repository both live in the application's `timing/` directory, with the
signals mapped to the three board LEDs.

## Configuration

### Kconfig options

| Symbol | Default | Effect |
| --- | --- | --- |
| `BMTH_ENABLE` | `n` | Enables the HMS Benchmark Tool. |
| `BMTH_FRDM_MCXN947` | `n` | Board and core selection. Depends on `BMTH_ENABLE`, compiles the Cortex-M33 core layer into the library. |

The application sources the package `Kconfig` from its top-level `Kconfig` and
selects the symbols in `prj.conf`.

A board symbol is mandatory in practice: without one, no core layer is compiled
and the link fails on the core-layer functions. A further target requires a core
layer under `cores/`, a Kconfig symbol, and the matching conditional in
`CMakeLists.txt`.

### Options in the configuration header

| Option | Meaning |
| --- | --- |
| `BMTH_GET_COUNTER` | Yields the current counter value. On Cortex-M the DWT cycle counter. |
| `BMTH_RESET_COUNTER` | Resets that counter to zero. |
| `BMTH_COUNTER_FREQUENCY` | Counter ticks per second, used for millisecond-to-tick conversion. Required by the library but absent from the template; must be added. This project derives it from the system core clock. |
| `BMTH_SIGNALING_EVNT_DURATION` | Duration of one signalling half-toggle in milliseconds. Default 1000. |
| `BMTH_GLOBAL_TIME_STORAGE` | Compiles the global accumulating time store in or out. Default: out. |

### Build integration

The package is a CMake INTERFACE library; its sources compile with the
consumer's flags. The application includes the package `CMakeLists.txt` and
links the `bmth` target. The library exports the package root and `port/` as
include directories. The application's include path must additionally resolve
the configuration header.

## Requirements and caveats

- **C11 or higher.** The build fails below that.
- **Assertions are always active.** The assertion macro is independent of the
  standard library, because a build-wide `NDEBUG` — defined by Zephyr's MCUX HAL
  glue whenever `CONFIG_ASSERT` is off — would compile every precondition check
  to nothing without a diagnostic. A failed check disables interrupts and spins
  forever.
- **Debugger influence.** The first calibration run performs a warm-up loop; a
  freshly attached debugger causes substantial jitter over the first few million
  cycles. Halting debug may remain attached. Intrusive debug facilities and armed
  hardware breakpoints are rejected by the preconditions.
- **Optimisation level.** Behaviour is consistent across optimisation levels.
  Placing the markers so that the compiler cannot hoist work out of the measured
  region remains the caller's responsibility.
- **Noise floor.** The hardware-influence check runs a series over an empty loop
  and characterises the platform, not the code under test. Its spread is the
  noise floor of all later series.

## Licence

BSD-3-Clause, © 2026 HMS Industrial Networks GmbH & Co. KG.
