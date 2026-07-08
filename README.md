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

```
make
```

## Usage

```
# Load: GPIO 60 (P9.12), rising edge
sudo insmod gpio_irq_logger.ko gpio_pin=60 trigger=0

# Watch events
dmesg -w | grep gpio_irq

# Unload
sudo rmmod gpio_irq_logger
```

### Module parameters

| Param      | Default | Description                             |
|------------|---------|-----------------------------------------|
| `gpio_pin` | 60      | GPIO number (60 = P9.12 on BBB)         |
| `trigger`  | 0       | 0 = rising, 1 = falling, 2 = both edges |

### Sample output

```
gpio_irq: ready — IRQ 236, GPIO 60, pin state = 0
gpio_irq: event #1 | GPIO 60 = 1 | t = 2145332 us
gpio_irq: event #2 | GPIO 60 = 0 | t = 2145719 us
gpio_irq: unloaded — 2 events captured on GPIO 60
```

## How it works

1. `gpio_request()` + `gpio_direction_input()` — claim the pin
2. `gpio_to_irq()` — map GPIO to its IRQ line
3. `request_irq()` — register the handler with edge-trigger flags
4. Handler: `ktime_get()` → elapsed µs since load → `pr_info()` to dmesg

## Hardware

BeagleBone Black (AM335x). Common header pins: GPIO 60 = P9.12,
GPIO 48 = P9.15, GPIO 66 = P8.07.
