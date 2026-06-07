#ifndef __MPU6050_H
#define __MPU6050_H
#include "sys.h"

void MPU6050_Init(void);
void MPU6050_ReadAccel(s16 *ax, s16 *ay, s16 *az);
void MPU6050_ReadGyro(s16 *gx, s16 *gy, s16 *gz);

#endif
