/******************************************************************************
 * 三路超声波避障小车 — 主程序
 * MCU: STM32F103C8T6  主频: 72MHz
 *
 * =========================== 用法说明 ===========================
 *
 * 【调速度】打开 USER/speed.h，改数字 → 编译烧录 → 生效，不用碰其他文件。
 *   PWM范围: 0~899，低于100可能扭矩不够无法起步。
 *   常见调法:
 *    小车太慢      → SPEED_GO_L/R    150→200   (加大)
 *    转弯转不动    → SPEED_TURN_L/R  150→180   (加大)
 *    后退没力      → SPEED_BACK_L/R  140→160   (加大)
 *    死角撞墙      → THR_HEAD_NEAR    20→25    (提早刹车)
 *    太敏感老转弯  → THR_HEAD_NEAR    20→15    (减小探测距离)
 *    靠太近才躲    → THR_LEFT_NEAR    12→8     (减小侧向阈值)
 *    转弯时间不够  → DELAY_TURN       50→80    (延长转弯)
 *
 * 【蜂鸣器】3个函数，任意位置调用即可：
 *     Buzzer_Beep();     // 响一声(100ms)，最常用
 *     Buzzer_On();       // 长鸣，需配合 Buzzer_Off() 使用
 *     Buzzer_Off();      // 关闭
 *   引脚: PB12，低电平触发
 *
 * 【启用陀螺仪 MPU6050】取消下面两行注释：
 *     #include "i2c.h"
 *     #include "mpu6050.h"
 *   并在 main() 中取消 I2C2_Init() 和 MPU6050_Init() 注释
 *
 * 【启用 OLED 屏幕】取消 #include "oled.h" 并在 main() 中取消
 *   OLED_Init() 注释。注意：OLED 与右侧超声波共用 PB8/PB9，
 *   显示时会影响该路测距，建议仅在调试阶段用。
 *
 * 【串口调试】PA9(TX) PA10(RX), 波特率 9600。
 *   printf 直接输出距离和 PWM 值，可用串口助手查看。
 *
 * =========================== 引脚分配 ===========================
 *
 *  功能          引脚        备注
 *  ─────────────────────────────────────
 *  电机A PWM     PA0         TIM2_CH1, 80KHz
 *  电机B PWM     PA1         TIM2_CH2
 *  电机A方向     PA6/PA7     A1/A2
 *  电机B方向     PB0/PB1     B1/B2
 *  正前超声Trig  PA11        推挽输出
 *  正前超声Echo  PA12        EXTI12
 *  右侧超声Trig  PB8         推挽输出(与OLED分时复用)
 *  右侧超声Echo  PB9         EXTI9
 *  左侧超声Trig  PC13        推挽输出
 *  左侧超声Echo  PB3         EXTI3 (需禁JTAG)
 *  编码器1       PA2/PA3     EXTI2脉冲 / 方向
 *  编码器2       PB5/PB6     EXTI5脉冲 / 方向
 *  MPU6050       PB10/PB11   I2C2_SCL/SDA
 *  OLED          PB8/PB9     软件I2C (与右侧超声分时)
 *  蜂鸣器        PB12        低电平响
 *  串口调试      PA9/PA10    USART1, 9600
 *  SWD调试       PA13/PA14   保留
 *
 * =========================== 中断说明 ===========================
 *
 *  EXTI2    → 编码器1脉冲计数
 *  EXTI3    → 左侧超声波Echo
 *  EXTI9_5  → 右侧超声Echo + 编码器2脉冲
 *  EXTI15_10 → 正前方超声Echo
 *  TIM2     → 电机PWM输出
 *  TIM4     → 超声波计时(1MHz自由运行)
 *  USART1   → 串口接收
 *
 ******************************************************************************/

#include "delay.h"
#include "sys.h"
#include "pwm.h"
#include "usart.h"
#include "gpio.h"
#include "read.h"
#include "timer.h"
#include "ai.h"
#include "buzzer.h"
#include "speed.h"
//#include "i2c.h"
//#include "mpu6050.h"
//#include "oled.h"

/*
 * 出口检测 — 波谷法 (超声45度对称安装，适合薄墙出口)
 *
 * 原理: 持续追踪两侧距离的近期最小值(波谷)。
 *       当两侧当前值都回升到 >EXIT_SIDE_HIGH 且
 *       之前的波谷都曾 <EXIT_SIDE_LOW → 刚穿过窄口。
 *
 * 调试: 每10个主循环打印一次 min/cur 值，串口可观察。
 *       调阈值到 speed.h 的"出口检测阈值"区。
 */
static u8 IsExitPassing(float head, float left, float right)
{
	static float minL = 999;        // 左距离近期最小值
	static float minR = 999;        // 右距离近期最小值
	static u8    cooldown = 0;      // 触发后冷却
	static u8    tick = 0;          // 打印周期

	if (cooldown > 0) {
		cooldown--;
		return 0;
	}

	// 异常数据或前方有障碍 → 重置波谷
	if (head < EXIT_HEAD_MIN || left == 99 || right == 99) {
		minL = 999;
		minR = 999;
		return 0;
	}

	// 更新波谷
	if (left  < minL) minL = left;
	if (right < minR) minR = right;

	// 调试打印 (每10个周期)
	tick++;
	if (tick >= 10) {
		printf("[EXIT] min:L=%.0f R=%.0f  cur:L=%.0f R=%.0f\n\r",
		       minL, minR, left, right);
		tick = 0;
	}

	// 触发: 之前两侧都很低 + 现在两侧都回升
	if (minL < EXIT_SIDE_LOW && minR < EXIT_SIDE_LOW &&
	    left  > EXIT_SIDE_HIGH && right > EXIT_SIDE_HIGH)
	{
		minL = 999;
		minR = 999;
		cooldown = EXIT_COOLDOWN;
		return 1;
	}

	return 0;
}

int main(void)
{
	float zuo, qian, you;
	delay_init();
	gpio_Init();
	Buzzer_Init();

	TIM2_PWM_Init(899, 0);			// PWM频率=72000/(899+1)=80KHz

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	TIM4_FreeRun_Init(0xFFFF, 71);	// 1MHz自由计数器，超声波计时
	Ultrasonic_EXTI_Init();			// 超声波EXTI中断配置
	Encoder_Init();				// 编码器EXTI配置

	uart_init(9600);
	Buzzer_Beep();
	// I2C2_Init();			// 硬件I2C (MPU6050) — 待外设确认后启用
	// MPU6050_Init();			// 陀螺仪初始化
	// OLED_Init();			// OLED显示屏初始化

	while (1)
	{
		qian = read_qian();
		you  = read_you();
		zuo  = read_zuo();

		printf("\n\rfront=%.1f cm  right=%.1f cm  left=%.1f cm\n\r", qian, you, zuo);
		printf("PWM: L=%d  R=%d\n\r", TIM2->CCR1, TIM2->CCR2);

		calculate(qian, zuo, you);

		if (IsExitPassing(qian, zuo, you))
		{
			printf("=== EXIT ===\n\r");
			Buzzer_Beep();
		}
	}
}
