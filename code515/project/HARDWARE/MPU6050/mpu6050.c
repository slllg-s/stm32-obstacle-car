#include "mpu6050.h"
#include "i2c.h"
#include "delay.h"

#define MPU6050_ADDR  0xD0

#define SMPLRT_DIV   0x19
#define CONFIG       0x1A
#define GYRO_CONFIG  0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1   0x6B
#define WHO_AM_I     0x75
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H  0x43

void MPU6050_Init(void)
{
	u8 id;
	delay_ms(100);

	I2C2_ReadReg(I2C2, MPU6050_ADDR, WHO_AM_I, &id, 1);
	if (id != 0x68) return;

	I2C2_WriteReg(I2C2, MPU6050_ADDR, PWR_MGMT_1, 0x00);
	delay_ms(10);
	I2C2_WriteReg(I2C2, MPU6050_ADDR, GYRO_CONFIG, 0x18);
	I2C2_WriteReg(I2C2, MPU6050_ADDR, ACCEL_CONFIG, 0x00);
	I2C2_WriteReg(I2C2, MPU6050_ADDR, SMPLRT_DIV, 0x07);
	I2C2_WriteReg(I2C2, MPU6050_ADDR, CONFIG, 0x06);
}

void MPU6050_ReadAccel(s16 *ax, s16 *ay, s16 *az)
{
	u8 buf[6];
	I2C2_ReadReg(I2C2, MPU6050_ADDR, ACCEL_XOUT_H, buf, 6);
	*ax = (s16)((buf[0] << 8) | buf[1]);
	*ay = (s16)((buf[2] << 8) | buf[3]);
	*az = (s16)((buf[4] << 8) | buf[5]);
}

void MPU6050_ReadGyro(s16 *gx, s16 *gy, s16 *gz)
{
	u8 buf[6];
	I2C2_ReadReg(I2C2, MPU6050_ADDR, GYRO_XOUT_H, buf, 6);
	*gx = (s16)((buf[0] << 8) | buf[1]);
	*gy = (s16)((buf[2] << 8) | buf[3]);
	*gz = (s16)((buf[4] << 8) | buf[5]);
}
