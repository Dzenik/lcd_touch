#include <stdio.h>
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "touch.h"
#include "task.h"
#include "speaker.h"
#include "menu.h"

/*
7  6 - 4  3      2     1-0
s  A2-A0 MODE SER/DFR PD1-PD0
*/
#define TOUCH_MSR_Y  0x90   //读X轴坐标指令 addr:1
#define TOUCH_MSR_X  0xD0   //读Y轴坐标指令 addr:3


// 触摸硬件连接: (POWERAVR 红牛开发板)
// SPI    <==> SPI2
// TP_CS  <==> PB12
// TP_INT <==> PG7
#include "stm32f10x.h"

#define TP_CS_LOW()               GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define TP_CS_HIGH()              GPIO_SetBits(GPIOB,GPIO_Pin_12)
// 读取TP_INT引脚状态,0为有按下,1为释放
// 这里为了使用方便,TP_DOWN()返回1为有按下,0为释放.
#define TP_DOWN()                 (1-GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_7))

extern xQueueHandle xTPQueue;

uint8_t SPI_WriteByte(unsigned char data)
{
    //Wait until the transmit buffer is empty
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    // Send the byte
    SPI_I2S_SendData(SPI2, data);

    //Wait until a data is received
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    // Get the received data
    data = SPI_I2S_ReceiveData(SPI2);

    // Return the shifted data
    return data;
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the EXTI4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void EXTI_Configuration(void)
{
    /* PG7 touch INT */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO,ENABLE);

        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
    }/* PC4 touch INT */

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource7);

    /* Configure  EXTI  */
    {
        EXTI_InitTypeDef EXTI_InitStructure;

        /* Configure  EXTI  */
        EXTI_InitStructure.EXTI_Line    = EXTI_Line7;
        EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升

        /* enable */
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;

        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line7);
    }/* Configure  EXTI  */
}


void touch_init(void)
{
    /* SPI2 config */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        SPI_InitTypeDef SPI_InitStructure;

        /* Enable SPI2 Periph clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB
                               | RCC_APB2Periph_AFIO,ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

        /* Configure SPI2 pins: PB13-SCK, PB14-MISO and PB15-MOSI */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /*------------------------ SPI2 configuration ------------------------*/
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//SPI_Direction_1Line_Tx;
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;/* 72M/64=1.125M */
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial = 7;

        SPI_I2S_DeInit(SPI2);
        SPI_Init(SPI2, &SPI_InitStructure);

        /* Enable SPI_MASTER */
        SPI_Cmd(SPI2, ENABLE);
        SPI_CalculateCRC(SPI2, DISABLE);
    }

    // CS config : TP_CS <-> PB12
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        /* Enable SPI2 Periph clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 ;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        TP_CS_HIGH();
    }

    NVIC_Configuration();
    EXTI_Configuration();

    TP_CS_LOW();
    SPI_WriteByte( 1<<7 ); /* 打开中断 */
    TP_CS_HIGH();

//    LCD_write_english_string(0,20,"touch init",Green,Blue);
//    printf("touch init\r\n");
}

//void touch_show(void)
//{
//    uint16_t tmpx[10];
//    uint16_t tmpy[10];
//    uint16_t touch_x,touch_y;
//    unsigned int i;
//
//    LCD_write_english_string(0,100,"          ",White,Blue);
//    LCD_write_english_string(0,40,"touch down",White,Blue);
//
//    // 如果一直按下,就一直读取并显示原始坐标
//    while( TP_DOWN() )
//    {
//        for(i=0; i<10; i++)
//        {
//            TP_CS_LOW();
//            SPI_WriteByte(TOUCH_MSR_X);                            /* read X */
//            tmpx[i] = SPI_WriteByte(0x00)<<4;                      /* read MSB bit[11:8] */
//            tmpx[i] |= ((SPI_WriteByte(TOUCH_MSR_Y)>>4)&0x0F );    /* read LSB bit[7:0] and prepare read Y */
//            tmpy[i] = SPI_WriteByte(0x00)<<4;                      /* read MSB bit[11:8] */
//            tmpy[i] |= ((SPI_WriteByte(0x00)>>4)&0x0F );           /* read LSB bit[7:0] */
//            SPI_WriteByte( 1<<7 ); /* 再次打开中断 */
//            TP_CS_HIGH();
//        }
//
//        //去最高值与最低值,再取平均值
//        {
//            uint32_t min_x = 0xFFFF,min_y = 0xFFFF;
//            uint32_t max_x = 0,max_y = 0;
//            uint32_t total_x = 0;
//            uint32_t total_y = 0;
//            unsigned int i;
//
//            for(i=0; i<10; i++)
//            {
//                if( tmpx[i] < min_x )
//                {
//                    min_x = tmpx[i];
//                }
//                if( tmpx[i] > max_x )
//                {
//                    max_x = tmpx[i];
//                }
//                total_x += tmpx[i];
//
//                if( tmpy[i] < min_y )
//                {
//                    min_y = tmpy[i];
//                }
//                if( tmpy[i] > max_y )
//                {
//                    max_y = tmpy[i];
//                }
//                total_y += tmpy[i];
//            }
//            total_x = total_x - min_x - max_x;
//            total_y = total_y - min_y - max_y;
//            touch_x = total_x / 8;
//            touch_y = total_y / 8;
//        }//去最高值与最低值,再取平均值
//
//        //display
//        {
//            char x_str[20];
//            char y_str[20];
//            sprintf(x_str,"X: %04d",touch_x);
//            sprintf(y_str,"Y: %04d",touch_y);
//            LCD_write_english_string(0,60,x_str,Cyan,Blue);
//            LCD_write_english_string(0,80,y_str,Cyan,Blue);
//            printf("\rtouch down!");
//            printf("X:%04d Y:%04d         ",touch_x,touch_y);
//        }
//    }// 如果一直按下,就一直读取并显示原始坐标
//
//    // touch up
//    LCD_write_english_string(0,40,"           ",White,Blue);
//    LCD_write_english_string(0,100,"touch up  ",White,Blue);
//    printf("\rtouch up!                        ");
//}

/*********************************************/

void vTouchTask( void *pvParameters )
{
    printf("Touch start\r\n");

    touch_init();
    unsigned int x = 0, y = 0, beep = TOUCH_BEEP; // current x,y value
    uint16_t tmpx[10];
    uint16_t tmpy[10];
    unsigned int i;

    unsigned char valid = 0;
    for( ;; )
    {
        //measure x,y
        while( TP_DOWN() )
        {
            for(i=0; i<10; i++)
            {
                TP_CS_LOW();
                SPI_WriteByte(TOUCH_MSR_X);                            /* read X */
                tmpx[i] = SPI_WriteByte(0x00)<<4;                      /* read MSB bit[11:8] */
                tmpx[i] |= ((SPI_WriteByte(TOUCH_MSR_Y)>>4)&0x0F );    /* read LSB bit[7:0] and prepare read Y */
                tmpy[i] = SPI_WriteByte(0x00)<<4;                      /* read MSB bit[11:8] */
                tmpy[i] |= ((SPI_WriteByte(0x00)>>4)&0x0F );           /* read LSB bit[7:0] */
                SPI_WriteByte( 1<<7 ); /* 再次打开中断 */
                TP_CS_HIGH();
            }

            {
                uint32_t min_x = 0xFFFF,min_y = 0xFFFF;
                uint32_t max_x = 0,max_y = 0;
                uint32_t total_x = 0;
                uint32_t total_y = 0;
                unsigned int i;

                for(i=0; i<10; i++)
                {
                    if( tmpx[i] < min_x )
                    {
                        min_x = tmpx[i];
                    }
                    if( tmpx[i] > max_x )
                    {
                        max_x = tmpx[i];
                    }
                    total_x += tmpx[i];

                    if( tmpy[i] < min_y )
                    {
                        min_y = tmpy[i];
                    }
                    if( tmpy[i] > max_y )
                    {
                        max_y = tmpy[i];
                    }
                    total_y += tmpy[i];
                }
                total_x = total_x - min_x - max_x;
                total_y = total_y - min_y - max_y;
                x = total_x / 8;
                y = total_y / 8;
            }
        }

        //printf("x %d y %d\r\n", x, y);

        if (x >=0 && x < 320 && y >= 0 && y < 240)
        {
            if (!valid)
                menu_touch(x, y);
            valid = 1;
        }
        else if (valid)
        {
            menu_touch(-1, -1);
            valid = 0;
        }

        vTaskDelay( 10 );
    }
}
