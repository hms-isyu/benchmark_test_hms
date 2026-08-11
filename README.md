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

## Rules for placing a measurement window

The counter read is not free, and its cost is only constant if the instructions
around it are. The compiler decides those, and its decision changes between
optimisation levels, compiler versions and unrelated edits to the same function.
The following rules keep every read site the same shape, so that the calibrated
overhead measured once applies everywhere.

1. **Time markers are `static`, never locals.** A `static` is observable outside
   the function, so the compiler must write it to memory and the sequence is
   always *load base → read counter → store*. A local may be held in a register
   with the store elided entirely, depending on register pressure elsewhere in
   the function, and the read then costs less. Statics are not cheaper; they are
   deterministic.

2. **Touch each marker exactly once inside a window.** A second access invites
   common-subexpression elimination and address reuse, and the sequence differs
   from the calibrated one.

3. **Reset the counter before the window opens, never inside it.** The reset is
   a store and would otherwise be measured.

4. **Read one counter per build.** `BMTH_GET_COUNTER` is deliberately singular.
   Capturing a second DWT counter in the same run places its read inside the
   first counter's window. Use two builds differing only in that macro.

5. **The counter base address and the marker address must be materialised before
   the window opens.** A file-scope variable's address is a link-time constant
   that the compiler loads from a literal pool with `ldr rX,[pc,#N]`, and where
   it places that load is not controllable from C. Verify in the disassembly
   that it lies ahead of the opening read.

6. **The calibration window must contain the same instructions as a measurement
   window minus the code under test.** That is what makes subtracting the
   overhead valid. Re-verify in the disassembly whenever either side is touched,
   or whenever the optimisation level changes. An instruction that leaves the
   calibration window silently shifts every reported result by its cost, and the
   error appears in the reference rather than in the code being measured.

7. **A window that spans a task switch cannot satisfy rule 5 on its closing
   side.** The closing function begins inside the window, so nothing can be
   hoisted ahead of it. Such windows carry an additional boundary cost that the
   ordinary calibration does not cover; it has to be counted from the listing
   and subtracted separately.

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
- **Optimisation level.** The calibrated overhead is a property of the build, not
  of the package, and must be re-measured whenever the flags change. The
  instructions the compiler places inside a window differ between optimisation
  levels, and a single additional flag is enough to move one: adding
  `-fno-schedule-insns2` to an `-O2` build moved an address load out of the
  calibration window and changed the overhead from 6 to 4 cycles, shifting every
  reported result by 2. See the rules above.
- **Noise floor.** The hardware-influence check runs a series over an empty loop
  and characterises the platform, not the code under test. Its spread is the
  noise floor of all later series.

## Licence

BSD-3-Clause, © 2026 HMS Industrial Networks GmbH & Co. KG.
