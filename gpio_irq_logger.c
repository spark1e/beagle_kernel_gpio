/*
 * gpio_irq_logger.c
 *
 * Linux kernel module for BeagleBone Black — GPIO interrupt logger
 * with microsecond timestamps.
 *
 * Registers an interrupt on a configurable GPIO pin and logs each
 * event to dmesg with a ktime_get() timestamp.
 *
 * Usage:
 *   insmod gpio_irq_logger.ko gpio_pin=60 trigger=0
 *   dmesg | grep gpio_irq
 *   rmmod gpio_irq_logger
 *
 * Module parameters:
 *   gpio_pin  — GPIO number (default 60 = P9.12 on BBB)
 *   trigger   — 0 = rising edge, 1 = falling edge, 2 = both (default 0)
 *
 * Created on: Mar 14, 2026
 * Author: Nikita Volkov (https://github.com/spark1e)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>

#define MODULE_TAG "gpio_irq: "

/* ============================================================
 * MODULE PARAMETERS
 * ============================================================
 *
 * Common BeagleBone Black GPIO mappings (active by default):
 *   GPIO 60  = P9.12
 *   GPIO 48  = P9.15
 *   GPIO 49  = P9.23
 *   GPIO 115 = P9.27
 *   GPIO 66  = P8.07
 *   GPIO 67  = P8.08
 *   GPIO 69  = P8.09
 */
static unsigned int gpio_pin = 60;
module_param(gpio_pin, uint, 0644);
MODULE_PARM_DESC(gpio_pin, "GPIO pin number (default 60 = P9.12)");

static unsigned int trigger = 0;
module_param(trigger, uint, 0644);
MODULE_PARM_DESC(trigger, "Edge trigger: 0=rising, 1=falling, 2=both (default 0)");

/* ============================================================
 * STATE
 * ============================================================ */
static unsigned int irq_number;
static unsigned long event_count;
static ktime_t load_time;

/* ============================================================
 * INTERRUPT HANDLER
 * ============================================================ */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	ktime_t now;
	s64 elapsed_us;

	event_count++;
	now = ktime_get();
	elapsed_us = ktime_to_us(ktime_sub(now, load_time));

	pr_info(MODULE_TAG "event #%lu | GPIO %u = %d | t = %lld us\n",
		event_count, gpio_pin, gpio_get_value(gpio_pin), elapsed_us);

	return IRQ_HANDLED;
}

/* ============================================================
 * MODULE INIT
 * ============================================================ */
static int __init gpio_irq_logger_init(void)
{
	int result;
	unsigned long irq_flags;

	pr_info(MODULE_TAG "loading — GPIO %u, trigger %u\n", gpio_pin, trigger);

	/* Request the GPIO pin */
	if (!gpio_is_valid(gpio_pin)) {
		pr_err(MODULE_TAG "invalid GPIO %u\n", gpio_pin);
		return -ENODEV;
	}

	result = gpio_request(gpio_pin, "gpio_irq_logger");
	if (result) {
		pr_err(MODULE_TAG "gpio_request(%u) failed: %d\n", gpio_pin, result);
		return result;
	}

	/* Configure as input */
	result = gpio_direction_input(gpio_pin);
	if (result) {
		pr_err(MODULE_TAG "gpio_direction_input(%u) failed: %d\n",
		       gpio_pin, result);
		goto err_free_gpio;
	}

	/* Map GPIO to IRQ number */
	irq_number = gpio_to_irq(gpio_pin);
	if (irq_number < 0) {
		pr_err(MODULE_TAG "gpio_to_irq(%u) failed: %d\n",
		       gpio_pin, irq_number);
		result = irq_number;
		goto err_free_gpio;
	}

	/* Select edge trigger */
	switch (trigger) {
	case 1:
		irq_flags = IRQF_TRIGGER_FALLING;
		break;
	case 2:
		irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
		break;
	default:
		irq_flags = IRQF_TRIGGER_RISING;
		break;
	}

	/* Register the interrupt handler */
	result = request_irq(irq_number, gpio_irq_handler, irq_flags,
			     "gpio_irq_logger", NULL);
	if (result) {
		pr_err(MODULE_TAG "request_irq(%u) failed: %d\n",
		       irq_number, result);
		goto err_free_gpio;
	}

	/* Record load time as reference for timestamps */
	event_count = 0;
	load_time = ktime_get();

	pr_info(MODULE_TAG "ready — IRQ %u, GPIO %u, pin state = %d\n",
		irq_number, gpio_pin, gpio_get_value(gpio_pin));

	return 0;

err_free_gpio:
	gpio_free(gpio_pin);
	return result;
}

/* ============================================================
 * MODULE EXIT
 * ============================================================ */
static void __exit gpio_irq_logger_exit(void)
{
	free_irq(irq_number, NULL);
	gpio_free(gpio_pin);

	pr_info(MODULE_TAG "unloaded — %lu events captured on GPIO %u\n",
		event_count, gpio_pin);
}

module_init(gpio_irq_logger_init);
module_exit(gpio_irq_logger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikita Volkov");
MODULE_DESCRIPTION("GPIO interrupt logger with microsecond timestamps for BeagleBone Black");
MODULE_VERSION("1.0");
