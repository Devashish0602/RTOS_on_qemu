# riscv-rtos

A small preemptive real-time kernel written from scratch for RV32, running on QEMU's `virt` machine. No RTOS libraries, no HAL, no bootloader — the trap handler, context switch, scheduler and synchronisation primitives are all hand-written.

Built to understand scheduling and context switching at the instruction level rather than through an API.

## What it does

- Preemptive multitasking driven by the CLINT machine timer
- Priority-based scheduler with round-robin between equal priorities
- Full context save/restore in assembly (31 GPRs + `mepc` + `mstatus`)
- Cooperative yield via `ecall`
- Blocking `task_delay()` — sleeping tasks consume no CPU
- Counting semaphores with priority-aware wakeup
- Idle task using `wfi`

## Requirements

```
qemu-system-riscv32
riscv64-unknown-elf-gcc        # or any RV32-capable toolchain
gdb-multiarch                  # optional, for debugging
```

On Debian/Ubuntu:

```bash
sudo apt install qemu-system-misc gcc-riscv64-unknown-elf gdb-multiarch
```

## Build and run

```bash
make          # produces kernel.elf and kernel.dis
make run      # boot under QEMU
make debug    # boot frozen, gdb stub on :1234
```

Exit QEMU with **Ctrl-A** then **x**.

To debug, in a second terminal:

```bash
gdb-multiarch kernel.elf
(gdb) target remote :1234
```

## Target

QEMU `virt`, RV32IMAC. Addresses are taken from the board's own device tree rather than hardcoded assumptions:

```bash
qemu-system-riscv32 -machine virt -machine dumpdtb=virt.dtb
dtc -I dtb -O dts virt.dtb | less
```

| Region | Base | Notes |
| --- | --- | --- |
| CLINT | `0x0200_0000` | `mtimecmp` +0x4000, `mtime` +0xBFF8 |
| UART (16550) | `0x1000_0000` | THR +0, LSR +5 |
| RAM | `0x8000_0000` | image load address |

Timer frequency comes from `timebase-frequency` in the DTB.

## How it works

### Boot

`_start` masks interrupts, sets up the stack pointer, installs the trap vector in `mtvec`, zeroes `.bss`, and calls `main()`. `main()` creates the tasks and then jumps directly into the restore path via `start_first_task()` — it never becomes a task itself, so there is no startup special case in the scheduler.

### Trap entry

A single vector in `mtvec` handles everything. The entry point allocates a 128-byte frame on the *current task's own stack*, saves all 31 GPRs plus `mepc` and `mstatus`, then dispatches on `mcause`:

| `mcause` | Meaning | Path |
| --- | --- | --- |
| `0x8000_0007` | Machine timer | tick + reschedule |
| `0x0000_000B` | Ecall from M-mode | yield |
| anything else | Fault | report and halt |

Entry must be assembly: a trap has no caller, so the C ABI's caller-saved convention doesn't apply — every register belongs to the interrupted task and must be preserved.

### Context switch

The switch is one register assignment. Save and restore are shared code; only the middle differs.

```
save 32 words onto the current stack
sp  ──> context_switch(sp) ──> new sp
restore 32 words from the new stack
mret
```

Because `sp` already points into the running task's stack when the trap fires, the context lands on the correct stack for free. The TCB holds nothing but that pointer.

```c
typedef struct {
    uint32_t *sp;
    uint32_t  priority;
    uint32_t  state;
    uint32_t  wake_tick;
    void     *blocked_on;
} TCB;
```

### Frame layout

Fixed by the assembly, and shared by three pieces of code — the save path, the restore path, and the frame builder in `task_create()`.

```
sp + 124   mstatus      (MPP=3, MPIE=1)
sp + 120   mepc         (resume address)
sp + 116   x31
   ...
sp +   4   x3
sp +   0   x1           <- tcb[i].sp points here
```

`x0` is hardwired and `x2` (`sp`) lives in the TCB, so neither is stored.

### Starting a task that has never run

The restore path is unconditional — it reads 32 words and executes `mret` regardless of whether a real trap ever saved them. So `task_create()` fabricates a frame indistinguishable from a real one: registers zeroed, `mepc` set to the task function, and `mstatus` constructed with `MPP = 3` and `MPIE = 1` so that `mret` lands in machine mode with interrupts enabled.

`mret` then "returns" into a function that was never called.

### Scheduling

Linear scan, starting one past the current task so that equal priorities rotate:

```c
for (int n = 1; n <= NTASKS; n++) {
    int i = (current + n) % NTASKS;
    if (tcb[i].state != TASK_READY) continue;
    if (best < 0 || tcb[i].priority > tcb[best].priority) best = i;
}
```

Strict `>` combined with the rotated start gives round-robin within a priority level and strict preemption across levels. The idle task is always READY, so the scan can never fail.

O(n) is fine at this task count; a priority bitmap with a leading-zero count would make it O(1).

### Timer acknowledgement

The CLINT machine timer has no pending bit and no clear register — the interrupt is asserted continuously while `mtime >= mtimecmp`. Rescheduling the compare value *is* the acknowledgement.

Both registers are 64-bit and accessed through 32-bit loads and stores, which needs care in both directions:

- **Reading `mtime`:** read high, read low, read high again; retry if the high half changed, since the low half can wrap between the two loads.
- **Writing `mtimecmp`:** park the low half at `0xFFFFFFFF` first, then write high, then the real low. Otherwise an intermediate value can transiently fall below `mtime` and fire a spurious interrupt.

### Yield

`task_yield()` issues `ecall`, producing cause 11. Because this is a synchronous exception, `mepc` points *at* the `ecall` — so the yield path advances the saved `mepc` in the frame by 4 before switching. Fixing the live CSR would not work, since it is about to be overwritten by the incoming task's value.

### Blocking delay

```c
void task_delay(uint32_t n)
{
    tcb[current].wake_tick = ticks + n;
    tcb[current].state     = TASK_BLOCKED;
    task_yield();
}
```

The timer path increments `ticks` and wakes any blocked task whose deadline has passed. A yield does not advance time. A sleeping task is skipped entirely by the scheduler, so it costs nothing.

### Semaphores

Counting semaphores, with waiters identified by a `blocked_on` pointer in the TCB rather than a separate queue.

```c
sem_t s;
sem_init(&s, 0);

sem_wait(&s);   /* blocks while count == 0 */
sem_post(&s);   /* wakes the highest-priority waiter */
```

- `sem_wait` — if the count is positive, decrement and continue. Otherwise record what the task is blocked on, mark it BLOCKED, and yield.
- `sem_post` — if any task is waiting on this semaphore, wake the highest-priority one and hand the token over directly without touching the count. If none is waiting, increment.

Both run inside a critical section, because a timer interrupt landing between the read and the write of `count` would let two tasks both observe the same token. Masking is save-and-restore rather than unconditional enable, so the operations nest correctly:

```c
uint32_t s = irq_save();      /* csrrc mstatus, MIE — clear and return old */
/* ... */
irq_restore(s);
```

Interrupts are restored *before* the `ecall` in `sem_wait`, so a task does not resume with interrupts still masked.

## API

```c
void task_create(int id, void (*fn)(void), uint32_t priority);
void task_yield(void);
void task_delay(uint32_t ticks);

void sem_init(sem_t *s, int32_t initial);
void sem_wait(sem_t *s);
void sem_post(sem_t *s);

uint32_t irq_save(void);
void     irq_restore(uint32_t state);
```

## Layout

```
src/
  start.S      reset vector, trap entry, context save/restore
  kernel.c     TCB, task creation, scheduler, context_switch
  sem.c        semaphores and critical sections
  timer.c      CLINT access, tick arming
  uart.c       16550 output
  trap.c       fault reporting
inc/
kernel.ld      linker script
Makefile
```

## Status

Working: boot and UART, trap handling, periodic tick, context switch, priority scheduler, cooperative yield, blocking delay, semaphores.

Planned: mutex with ownership tracking, message queues, and a measured priority-inversion experiment — construct the inversion, measure the blocking time, implement priority inheritance, and measure the improvement.

Other things worth adding: stack-overflow detection via guard patterns, context-switch latency benchmarking with the cycle counter, an O(1) bitmap scheduler, and PMP-based stack isolation.

## Notes

Addresses are read from the device tree rather than assumed. Register-level behaviour follows the RISC-V privileged specification; the CLINT register offsets follow the SiFive layout that QEMU and most RISC-V SoCs implement.

## References

- RISC-V Privileged Architecture Specification
- RISC-V Unprivileged ISA Specification
- QEMU `hw/riscv/virt.c` — authoritative memory map for the target board
