#ifndef __I2C_H
#define __I2C_H
#include "sys.h"

void I2C2_Init(void);
void I2C2_Start(I2C_TypeDef *I2Cx, u8 addr, u8 direction);
void I2C2_Stop(I2C_TypeDef *I2Cx);
void I2C2_WriteByte(I2C_TypeDef *I2Cx, u8 data);
u8   I2C2_ReadByte(I2C_TypeDef *I2Cx, u8 ack);
u8   I2C2_WriteReg(I2C_TypeDef *I2Cx, u8 dev_addr, u8 reg, u8 data);
u8   I2C2_ReadReg(I2C_TypeDef *I2Cx, u8 dev_addr, u8 reg, u8 *buf, u8 len);

#endif
