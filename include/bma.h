/*
 *  STMicroelectronics kr3dm acceleration sensor driver
 *
 *  Copyright (C) 2010 Samsung Electronics Co.Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef __BMA_ACC_HEADER__
#define __BMA_ACC_HEADER__

#include <linux/types.h>
#include <linux/ioctl.h>




struct bma_acceldata{
	__s32 x;
	__s32 y;
	__s32 z;
};

/* dev info */
#define ACC_DEV_NAME "accelerometer_sensor"

/* bma ioctl command label */
#define BMA_IOCTL_BASE 'a'
#define BMA_IOCTL_SET_DELAY			_IOW(BMA_IOCTL_BASE, 0 , int64_t)
#define BMA_IOCTL_GET_DELAY			_IOR(BMA_IOCTL_BASE, 1 , int64_t)
#define BMA_IOCTL_READ_ACCEL_XYZ			_IOR(BMA_IOCTL_BASE, 8 ,struct bma_acceldata)
#define BMA_IOCTL_CALIBRATION				_IOR(BMA_IOCTL_BASE,9,struct bma_acceldata)

#endif
