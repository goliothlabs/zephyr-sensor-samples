/*
 * Copyright (c) 2021 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(golioth_lightdb_stream, LOG_LEVEL_DBG);

#include <net/coap.h>
#include <net/golioth/system_client.h>
#include <net/golioth/wifi.h>

#include <drivers/sensor.h>
#include <stdlib.h>

static struct golioth_client *client = GOLIOTH_SYSTEM_CLIENT_GET();


static void fetch_and_display(const struct device *sensor, struct sensor_value *accel)
{
	static unsigned int count;
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		printk("ERROR: Update failed: %d\n", rc);
	} else {
		printk("#%u @ %u ms: %sx %f , y %f , z %f",
		       count, k_uptime_get_32(), overrun,
		       sensor_value_to_double(&accel[0]),
		       sensor_value_to_double(&accel[1]),
		       sensor_value_to_double(&accel[2]));
	}
	printk("\n");
	
}

void main(void)
{

	struct sensor_value accel[3];
	const struct device *sensor = DEVICE_DT_GET_ANY(st_lis3dh);

	char str_accel[64];
	// struct sensor_value temp;
	int err;

	LOG_DBG("Start Light DB LIS3DH accelerometer stream sample");

	if (IS_ENABLED(CONFIG_GOLIOTH_SAMPLE_WIFI)) {
		LOG_INF("Connecting to WiFi");
		wifi_connect();
	}

	golioth_system_client_start();

	while (true) {
		fetch_and_display(sensor,accel);
		//err = get_temperature(&temp);
		if (err) {
			k_sleep(K_SECONDS(1));
			continue;
		}

		snprintk(str_accel, sizeof(str_accel),
			 "%f", sensor_value_to_double(&accel[0]));
		str_accel[sizeof(str_accel) - 1] = '\0';

		LOG_DBG("Sending acceleration value x%s", log_strdup(str_accel));

		err = golioth_lightdb_set(client,
					  GOLIOTH_LIGHTDB_STREAM_PATH("accel_x"),
					  COAP_CONTENT_FORMAT_TEXT_PLAIN,
					  str_accel,
					  strlen(str_accel));
		if (err) {
			LOG_WRN("Failed to send acceleration value: %d", err);
		}

				snprintk(str_accel, sizeof(str_accel),
			 "%f", sensor_value_to_double(&accel[1]));
		str_accel[sizeof(str_accel) - 1] = '\0';

		LOG_DBG("Sending acceleration value y%s", log_strdup(str_accel));

		err = golioth_lightdb_set(client,
					  GOLIOTH_LIGHTDB_STREAM_PATH("accel_y"),
					  COAP_CONTENT_FORMAT_TEXT_PLAIN,
					  str_accel,
					  strlen(str_accel));
		if (err) {
			LOG_WRN("Failed to send acceleration value: %d", err);
		}

				snprintk(str_accel, sizeof(str_accel),
			 "%f", sensor_value_to_double(&accel[2]));
		str_accel[sizeof(str_accel) - 1] = '\0';

		LOG_DBG("Sending acceleration value z%s", log_strdup(str_accel));

		err = golioth_lightdb_set(client,
					  GOLIOTH_LIGHTDB_STREAM_PATH("accel_z"),
					  COAP_CONTENT_FORMAT_TEXT_PLAIN,
					  str_accel,
					  strlen(str_accel));
		if (err) {
			LOG_WRN("Failed to send acceleration value: %d", err);
		}

		k_sleep(K_SECONDS(5));
	}
}
