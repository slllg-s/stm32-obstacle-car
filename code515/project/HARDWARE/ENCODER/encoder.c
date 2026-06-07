#include "encoder.h"
#include "timer.h"

s32 Encoder_GetCount1(void)
{
	return encoder1_count;
}

s32 Encoder_GetCount2(void)
{
	return encoder2_count;
}

void Encoder_ResetCount1(void)
{
	encoder1_count = 0;
}

void Encoder_ResetCount2(void)
{
	encoder2_count = 0;
}
