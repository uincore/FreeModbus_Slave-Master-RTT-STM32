/*
 * File      : gpio.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-02-02     Armink      the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include "bsp.h"

#ifdef RT_USING_PIN

/* STM32 GPIO driver */
struct pin_index
{
    int index;
    uint32_t rcc;
    GPIO_TypeDef *gpio;
    uint32_t pin;
};

static const struct pin_index pins[] =
{
    { 0, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_14},		//Wakeup Input1
    { 1, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_15},		//Wakeup Input2
    { 2, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_12},  	//Power Control
    { 3, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_1},		//RS485 Control
    { 4, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_13},		//LED1
    { 5, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_14},		//LED2
    { 6, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_15},		//LED3

};

#define ITEM_NUM(items) sizeof(items)/sizeof(items[0])
const struct pin_index *get_pin(uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
    }
    else
    {
        index = RT_NULL;
    }

    return index;
};

void stm32_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    if (value == PIN_LOW)
    {
        GPIO_ResetBits(index->gpio, index->pin);
    }
    else
    {
        GPIO_SetBits(index->gpio, index->pin);
    }
}

int stm32_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return value;
    }

    if (GPIO_ReadInputDataBit(index->gpio, index->pin) == Bit_RESET)
    {
        value = PIN_LOW;
    }
    else
    {
        value = PIN_HIGH;
    }

    return value;
}

void stm32_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct pin_index *index;
    GPIO_InitTypeDef  GPIO_InitStructure;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    /* GPIO Periph clock enable */
    RCC_AHB1PeriphClockCmd(index->rcc, ENABLE);

    /* Configure GPIO_InitStructure */
    GPIO_InitStructure.GPIO_Pin     = index->pin;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;			// zzz
    GPIO_InitStructure.GPIO_PuPd 		= GPIO_PuPd_NOPULL;		// zzz

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			// zzz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    }
    else
    {
        /* input setting:default. */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    }
    GPIO_Init(index->gpio, &GPIO_InitStructure);
}

const static struct rt_pin_ops _stm32_pin_ops =
{
    stm32_pin_mode,
    stm32_pin_write,
    stm32_pin_read,
};

int stm32_hw_pin_init(void)
{
    rt_device_pin_register("pin", &_stm32_pin_ops, RT_NULL);
    return 0;
}
INIT_BOARD_EXPORT(stm32_hw_pin_init);

#endif
