/*
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>

#define LED_ON_TIME_MS  500
#define RGB_ON_TIME_MS  1000

#define RGB_NODE DT_NODELABEL(rgb_led)

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
};

static const struct device *const rgb_dev = DEVICE_DT_GET(RGB_NODE);

static const char *const rgb_names[] = {
	"Red", "Green", "Blue", "Yellow", "Magenta", "Cyan", "White",
};

static const uint8_t rgb_off[] = {0, 0, 0};

/* RGB color table: {red, green, blue} values 0-255 */
static const uint8_t rgb_colors[][3] = {
	{255,   0,   0}, /* Red */
	{  0, 255,   0}, /* Green */
	{  0,   0, 255}, /* Blue */
	{255, 255,   0}, /* Yellow */
	{255,   0, 255}, /* Magenta */
	{  0, 255, 255}, /* Cyan */
	{255, 255, 255}, /* White */
};

int main(void)
{
	int ret;

	printk("ADI-T1SUSB LED Test\n");

	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED%d not ready\n", i);
			return 0;
		}
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			printk("Failed to configure LED%d: %d\n", i, ret);
			return 0;
		}
	}

	if (!device_is_ready(rgb_dev)) {
		printk("RGB LED device not ready\n");
		return 0;
	}

	printk("All LEDs configured\n");

	while (1) {
		/* Cycle through single-color LEDs one at a time */
		printk("--- Single LED cycle ---\n");
		for (int i = 0; i < ARRAY_SIZE(leds); i++) {
			printk("LED%d on\n", i);
			gpio_pin_set_dt(&leds[i], 1);
			k_msleep(LED_ON_TIME_MS);
			gpio_pin_set_dt(&leds[i], 0);
		}

		/* Cycle through RGB colors using the multicolor LED driver */
		printk("--- RGB color cycle ---\n");
		for (int c = 0; c < ARRAY_SIZE(rgb_colors); c++) {
			printk("RGB: %s\n", rgb_names[c]);
			ret = led_set_color(rgb_dev, 0, 3, rgb_colors[c]);
			if (ret < 0) {
				printk("Failed to set RGB color: %d\n", ret);
				return 0;
			}
			k_msleep(RGB_ON_TIME_MS);
		}

		/* Turn off RGB before repeating */
		led_set_color(rgb_dev, 0, 3, rgb_off);
		k_msleep(LED_ON_TIME_MS);
	}

	return 0;
}
