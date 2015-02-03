#include "stm32f10x.h"

void fsmc_init(void)
/* FSMC GPIO configure */
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF
                           | RCC_APB2Periph_GPIOG, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /*
    FSMC_D0 ~ FSMC_D3
    PD14 FSMC_D0   PD15 FSMC_D1   PD0  FSMC_D2   PD1  FSMC_D3
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD,&GPIO_InitStructure);

    /*
    FSMC_D4 ~ FSMC_D12
    PE7 ~ PE15  FSMC_D4 ~ FSMC_D12
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
                                  | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE,&GPIO_InitStructure);

    /* FSMC_D13 ~ FSMC_D15   PD8 ~ PD10 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOD,&GPIO_InitStructure);

    /*
    FSMC_A0 ~ FSMC_A5   FSMC_A6 ~ FSMC_A9
    PF0     ~ PF5       PF12    ~ PF15
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3
                                  | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOF,&GPIO_InitStructure);

    /* FSMC_A10 ~ FSMC_A15  PG0 ~ PG5 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOG,&GPIO_InitStructure);

    /* FSMC_A16 ~ FSMC_A18  PD11 ~ PD13 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOD,&GPIO_InitStructure);

    /* RD-PD4 WR-PD5 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOD,&GPIO_InitStructure);

    /* NBL0-PE0 NBL1-PE1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOE,&GPIO_InitStructure);

    /* NE1/NCE2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOD,&GPIO_InitStructure);
    /* NE2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOG,&GPIO_InitStructure);
    /* NE3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOG,&GPIO_InitStructure);
    /* NE4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_Init(GPIOG,&GPIO_InitStructure);
}
/* FSMC GPIO configure */
