# GPIO Interrupt Logger — Linux Kernel Module

Linux kernel module for BeagleBone Black that registers a hardware
interrupt on a configurable GPIO pin and logs every event to the
kernel log with microsecond-resolution timestamps.

Written to explore kernel-space development: interrupt registration,
GPIO subsystem APIs, and kernel time APIs on ARM embedded Linux.

## Features

- Interrupt-driven — no polling; handler registered via `request_irq()`
- Microsecond timestamps using `ktime_get()`, measured from module load
- Configurable at load time via module parameters (no recompile):
  GPIO pin and edge trigger (rising / falling / both)
- Event counter and pin-state readback in every log line
- Clean resource management: `gpio_free()` / `free_irq()` on unload,
  error paths with proper cleanup

## Build

Cross-compile against your BeagleBone kernel headers, or build
natively on the board:
