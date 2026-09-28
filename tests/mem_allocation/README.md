# Mem Allocation Test

The mem allocation test, tests the performance of mem allocation as a function of the arena state and the request size.

## Terminology

| Name | Meaning |
| --- | --- |
| allocator | The Zephyr allocation API one test case forwards its calls to. A fixed-size allocator serves pieces of one granularity regardless of the request; the other allocators carve each allocation from the arena at the requested size. |
| arena | The memory one test case serves every allocation from. |
| granularity, piece | The size the arena is divided into. A fixed-size allocator cuts the arena into pieces of this size, and a piece is one of them; the other allocators ignore the granularity. |
| step, s | The sweep parameter. At step s the request and the granularity are both 2^s bytes. |
| requested space, c(s) | The total size requested across a workload, held fixed across steps. c(s) is the count of requests of step s that sum to it. |
| used space | The bytes the allocator reports in use, usable memory and bookkeeping together. |
| fixed arena cost | The bytes of bookkeeping the arena holds whatever the number of live allocations. |
| live | An allocation that has been returned and not yet freed. |
| hole | Free space between two live allocations. |
| clog | Allocate at the request size until the allocator refuses, keeping every allocation live in allocation order. A clog that passes a fixed bound is reported unbounded. |
| gap | One of five holes built for the fit-policy workloads, in allocation order: a first decoy gap, the large gap, the exact gap of the probe size, then a second and a third decoy gap. A live separator allocation lies before the first gap, between each two, and after the last. |
| fit policy | The classification the three fit-policy workloads yield together, from the address each returns relative to the gaps built for it. One of a fixed set of named outcomes. |

## Scenarios

At step s every request is 2^s bytes.

| Workload | Trigger | Path measured |
| --- | --- | --- |
| W1 | Arena created, nothing live. | One allocation. |
| W2 | Arena created, three allocations made, the second freed: one hole of exactly the request between two live allocations. | One allocation. |
| W3 | Arena created, one allocation made, the only live allocation. | Free of that allocation. |
| W4 | Arena created, nothing live. | A loop of up to c(s) allocations, stopped at the first refusal. The used space and the fixed arena cost are read after the window, with the allocations still live. |
| W4_FIXED_SIZE | Arena created, nothing live. | One call allocating c(s) pieces at once. The used space is read after the window. Not applicable where the allocator does not support allocating a count in one call. |
| W5 | Arena clogged, at least 2 · c(s) allocations, else reported too short. | c(s) frees of the odd-indexed allocations, ascending, one window each; the indices on both sides of each freed one stay live. |
| W6 | Arena clogged, at least 2 · c(s) + 1 allocations, indices 0, 2, .., 2 · c(s) already freed. | The same frees as W5, with the indices on both sides of each freed one already freed. |
| W7_A | Arena created at the size of the separator allocation as granularity, the gaps built, then freed in allocation order. | One allocation of the probe size. Not applicable, for all three fit-policy workloads, where the allocator is fixed-size. |
| W7_B | As W7_A, with the exact gap freed before the large gap. | The same call, with no window. |
| W7_C | As W7_A, then one probe allocation made and freed before the measured one. | The same call, with no window. |
| W8 | The arena destroyed. | One arena creation at granularity 2^s. |

## Measurement principle

1. One window holds one operation of the allocator, or one loop of them where the workload is a
   loop. Every setup operation and every read of the allocator's bookkeeping runs outside it.
2. Every step of a workload, and every run of the fit-policy workloads, starts from a newly
   created arena.
3. At one step every workload uses the same request and the same granularity.
4. The requested space is the same at every step and in every test case.
5. A workload whose precondition the allocator cannot meet is not measured, and its status
   records the reason.
6. One build runs the whole sweep, step ascending. No build-time value selects one step of it.
7. The test cases differ in the allocator alone: same harness, same workloads, same sweep,
   same marker placement.
8. Nothing is subtracted from a recorded value. No compensation series is recorded.

## Configuration

A test case is linked by setting its CONFIG_BENCHMARK_TEST_MEM_* symbol to y in prj.conf. The
symbols belong to one Kconfig choice, so one build links one test case. The options below are
set in prj.conf and shared by every test case.

The harness drives every workload through the calls mem_allocator.h declares:
mem_allocator_alloc(), mem_allocator_alloc_n(), mem_allocator_free() and
mem_allocator_create_arena(). Each test case implements them by forwarding each call to its
allocator, and every window sits around one of these calls or a loop of them.

Bookkeeping, identical in every test case. The sweep runs step s = 3..10, 8 steps. The requested
space is 1024 bytes at every step; the arena is 8192 bytes. W4 and W4_FIXED_SIZE record a
failure unless the used space read after the window falls between the requested space and the
arena size. The separator allocation is 16 bytes, a decoy gap 32 bytes, the large gap 320
bytes, and the exact gap and the probe 128 bytes each.

Before any workload, the harness clogs a freshly created arena at the smallest step three
times: once after creating it, once after freeing every allocation of the first clog, and once
after creating it again over an already clogged arena. If the second clog reaches the count of
the first, freeing is known to reclaim space; if the third does, arena creation is known to
empty the arena. The status a workload records when it is not measured:

| Condition | Workloads | Status |
| --- | --- | --- |
| a probe clog is unbounded | all | MEM_STATUS_UNBOUNDED |
| the first probe clog serves nothing | all | MEM_STATUS_NOT_APPLICABLE |
| arena creation is not known to empty the arena | all | MEM_STATUS_NO_CREATE |
| freeing is not known to reclaim space | W5, W6, W7_A, W7_B, W7_C | MEM_STATUS_NO_RECLAIM |
| the clog of a W5 or W6 step is unbounded | that step | MEM_STATUS_UNBOUNDED |
| a setup allocation of W2 or W3, the measured W1 allocation, or the gap layout, cannot be built | that step or fit-policy run | MEM_STATUS_SETUP_FAILED |

The workloads run in the order W1, W2, W3, W4, W4_FIXED_SIZE, W8, W5, W6, W7_A, W7_B, W7_C.
