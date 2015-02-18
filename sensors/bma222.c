/*
 * libakm: this is a free replacement for the non-free libakm that comes with
 * android devices such as the Nexus S.
 * Copyright (C) 2011  Paul Kocialkowski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <pthread.h>

#include "bma.h"

#include "akm.h"
#include "sensors/akm_sensors.h"

#ifdef TARGET_DEVICE_ANCORA
#define SENSOR_NAME	"bma222"
#endif

#ifdef TARGET_DEVICE_ANCORA_TMO
#define SENSOR_NAME	"bma222"
#endif

#ifdef TARGET_DEVICE_APACHE
#define SENSOR_NAME	"bma250"
#endif

/* 
 * Chip/sensor functions/structures definitions for the bma accelerometer 
 * chip. It uses the akm8975 publisher. 
 */

int bma_init(struct akm_chip_sensors *chip)
{
	int fd;

	if(chip->inited)
		goto exit;
	/* Open the device from which the data will be obtained. */
	fd=open(chip->device_name, O_RDWR);
	if(fd < 0)
	{
		ALOGE("Error while opening chip device: %s.\n", strerror(errno));
		return 1;
	}

	/* Add the file descriptor to the chip struct. */
	chip->fd=fd;
	chip->inited=1;

exit:
	/* Finally, start to get data from the chip. */
	chip->data_get(chip);

	return 0;
}

int bma_deinit(struct akm_chip_sensors *chip)
{
	if(!chip->inited)
		return 0;

	if(chip->fd < 0)
	{
		ALOGE("Error while closing chip device: negative fd.\n");
		return 1;
	}

	/* Close the descriptor that gives access to the data. */
	close(chip->fd);
	chip->fd=-1;

	chip->inited=0;

	return 0;
}

/* This is the threaded function to get the sensor data. */
void *bma_data_get_thread(void *chip_p)
{
	struct akm_chip_sensors *chip;
	struct bma_acceldata data;
	struct akm_publish_vector data_vector;

	int get_data;
	int i;

	chip=(struct akm_chip_sensors *) chip_p;

	/* If the chip isn't initialized, initialize it. */
	if(!chip->publisher->inited)
	{
		if(chip->publisher->init(chip) != 0)
		{
			ALOGE("publisher init failed, aborting\n");
			goto exit;
		}
	}

	do
	{
		get_data=0;

		/*
		 * If at least one of the sensors of the chip is enabled, don't
		 * exit the loop.
		 */
		for(i=0 ; i < chip->sensors_count ; i++)
			if(chip->sensors[i])
				if(chip->sensors[i]->enabled)
					get_data=1;

		if(!get_data)
			break;

		/* ioctl call to get the data from the device. */
		if(ioctl(chip->fd, BMA_IOCTL_READ_ACCEL_XYZ, &data) < 0)
		{
			ALOGE("ioctl failed, aborting: %s\n", strerror(errno));
			goto exit;
		}

		/* Publish the data we just obtained. */
		for(i=0 ; i < chip->sensors_count ; i++)
		{
			if(chip->sensors[i]->enabled)
			{
				switch(chip->sensors[i]->type)
				{
					case SENSOR_TYPE_ACCELEROMETER:
						data_vector.x=data.x * -1;
						data_vector.y=data.y;
						data_vector.z=data.z;

						chip->publisher->data_publish(chip, chip->sensors[i]->type, &data_vector);
					break;
				}
			}
		}
	} while(get_data);

exit:
	chip->data_lock=0;
	pthread_exit(NULL);
	return NULL;
}

/* This function just starts the real function to get data in a thread. */
void bma_data_get(struct akm_chip_sensors *chip)
{
	pthread_t thread;

	if(!chip->data_lock)
	{
		chip->data_lock=1;
		pthread_create(&thread, NULL, bma_data_get_thread, chip);
	}
}

int bma_set_delay(struct akm_sensor *sensor_info, uint64_t delay)
{
	int rc;

	rc=ioctl(sensor_info->chip->fd, BMA_IOCTL_SET_DELAY, &delay);

	if(rc < 0)
	{
		ALOGE("ioctl failed, aborting: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

/* This is the structure for the kr3dm chip. */
struct akm_chip_sensors bma = {
	.publisher=&akm8975_publisher,
	.sensors_count=1,
	.sensors={
		&bma_accelerometer
	},
	.data_get=bma_data_get,
	.data_lock=0,
	.init=bma_init,
	.inited=0,
	.fd=-1,
	.device_name="/dev/accelerometer",
};

/* This is the accelerometer sensor structure. */
struct akm_sensor bma_accelerometer = {
	.type=SENSOR_TYPE_ACCELEROMETER,
	.enabled=0,
	.enable=default_enable,
	.disable=default_disable,
	.set_delay=bma_set_delay,
	.chip=&bma,
};
