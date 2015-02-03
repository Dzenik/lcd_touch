#include <stdio.h>
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*app includes. */
#include "touch.h"
#include "menu.h"
#include "speaker.h"



#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/*-----------------------------------------------------------*/

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK ( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/*-----------------------------------------------------------*/

/**
 * Configure the hardware for the App.
 */
static void prvSetupHardware( void );

/*-----------------------------------------------------------*/

xTaskHandle xLCDTaskHandle,
    xTouchTaskHandle,
    xAdcTaskHandle ,
    xBeepTaskHandle,
    xTimerSetupHandle,
    xDS1820Handle;

void USART_Configuration()
{
    /* USARTx configured as follow:
            - BaudRate = 115200 baud
            - Word Length = 8 Bits
            - One Stop Bit
            - No parity
            - Hardware flow control disabled (RTS and CTS signals)
            - Receive and transmit enabled
    */
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    /* Configure USART1 Tx (PA9) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Rx (PA10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);
}

/**
* @brief Retargets the C library printf function to the USART.
* @param None
* @retval None
*/
PUTCHAR_PROTOTYPE
{
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    //发送数据
    USART_SendData(USART1, (uint8_t) ch);

    return ch;
}
//以上代码用以实现printf输出

#include "ili_lcd_general.h"
#include "lcd_api.h"

extern void fsmc_init(void);
extern void lcd_gram_test(void);
extern void lcd_data_bus_test(void);

int main(void)
{
    prvSetupHardware();// set up peripherals etc

    USART_Configuration();
    printf("\r\n\r\nsystem startup!");

    //打开FSMC时钟并初始化所有FSMC的IO
    fsmc_init();

    //LCD初始化
    lcd_Initializtion();

    //填充屏幕为蓝色
    lcd_clear(Blue);

    xTaskCreate( vTouchTask,
                 ( signed portCHAR * ) "touch",
                 configMINIMAL_STACK_SIZE +1000,
                 NULL,
                 tskIDLE_PRIORITY+2,
                 &xTouchTaskHandle );

    // 在屏上面显示LCD控制器的型号
    {
        char device_str[20];
        uint16_t deviceid = lcd_getdeviceid();

        sprintf(device_str,"LCD control: %04X",deviceid);
        LCD_write_english_string(0,0,device_str,Cyan,Blue);
    }

    {
        extern void touch_init(void);
        touch_init();
    }


    while(1);
}

static void prvSetupHardware( void )
{
//   SystemInit(); // 此函数已在start.S里调用过了.
    /* Start with the clocks in their expected state. */
    RCC_DeInit();

    /* Enable HSE (high speed external clock). */
    RCC_HSEConfig( RCC_HSE_ON );

    /* Wait till HSE is ready. */
    while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    /* 2 wait states required on the flash. */
    *( ( unsigned portLONG * ) 0x40022000 ) = 0x02;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	/* PCLK2 = HCLK */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

	/* PCLK1 = HCLK/2 */
	RCC_PCLK1Config( RCC_HCLK_Div2 );

	/* PLLCLK = 8MHz * 9 = 72 MHz. */
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

	/* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

	/* Wait till PLL is used as system clock source. */
	while( RCC_GetSYSCLKSource() != 0x08 )
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA |
                                RCC_APB2Periph_GPIOB |
                                RCC_APB2Periph_GPIOC |
                                RCC_APB2Periph_GPIOD |
                                RCC_APB2Periph_GPIOE |
                                RCC_APB2Periph_GPIOF |
                                RCC_APB2Periph_AFIO,
                                ENABLE );

	/* SPI2 Periph clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );
        RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );

	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}
/*-----------------------------------------------------------*/
// STACK OVERFLOW HOOK -  TURNS ON LED
/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOF, &GPIO_InitStructure );
    GPIO_WriteBit( GPIOF, GPIO_Pin_6, 1 );

    for( ;; );
}
