/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "stm32f4xx_hal.h"
#include "board/Internal.h"
#include "core/src/general/ADC.h"

namespace core
{
    namespace adc
    {
        void startConversion()
        {
            /* Clear regular group conversion flag and overrun flag */
            /* (To ensure of no unknown state from potential previous ADC operations) */
            ADC1->SR = ~(ADC_FLAG_EOC | ADC_FLAG_OVR);

            /* Enable end of conversion interrupt for regular group */
            ADC1->CR1 |= (ADC_IT_EOC | ADC_IT_OVR);

            /* Enable the selected ADC software conversion for regular group */
            ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART;
        }

        void setChannel(uint32_t adcChannel)
        {
            /* Clear the old SQx bits for the selected rank */
            ADC1->SQR3 &= ~ADC_SQR3_RK(ADC_SQR3_SQ1, 1);

            /* Set the SQx bits for the selected rank */
            ADC1->SQR3 |= ADC_SQR3_RK(adcChannel, 1);
        }
    }    // namespace adc
}    // namespace core

extern "C" void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

extern "C" void HAL_MspDeInit(void)
{
    __HAL_RCC_SYSCFG_CLK_DISABLE();
    __HAL_RCC_PWR_CLK_DISABLE();
}

extern "C" void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_ENABLE();
    }
    else if (hadc->Instance == ADC2)
    {
        __HAL_RCC_ADC2_CLK_ENABLE();
    }
    else if (hadc->Instance == ADC3)
    {
        __HAL_RCC_ADC3_CLK_ENABLE();
    }
    else
    {
        return;
    }

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
}

extern "C" void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if (htim_base->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
    else if (htim_base->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
    else if (htim_base->Instance == TIM4)
    {
        __HAL_RCC_TIM4_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }
    else if (htim_base->Instance == TIM5)
    {
        __HAL_RCC_TIM5_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM5_IRQn);
    }
    else if (htim_base->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
    else if (htim_base->Instance == TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
    }
    else if (htim_base->Instance == TIM12)
    {
        __HAL_RCC_TIM12_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
    }
    else if (htim_base->Instance == TIM13)
    {
        __HAL_RCC_TIM13_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
    }
    else if (htim_base->Instance == TIM14)
    {
        __HAL_RCC_TIM14_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
    }
}

extern "C" void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    uint8_t channel = 0;

    if (Board::detail::map::uartChannel(huart->Instance, channel))
    {
        auto descriptor = Board::detail::map::uartDescriptor(channel);

        descriptor->enableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
            CORE_IO_CONFIG(descriptor->pins().at(i));

        if (descriptor->irqn() != 0)
        {
            HAL_NVIC_SetPriority(descriptor->irqn(), 0, 0);
            HAL_NVIC_EnableIRQ(descriptor->irqn());
        }
    }
}

extern "C" void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    uint8_t channel = 0;

    if (Board::detail::map::uartChannel(huart->Instance, channel))
    {
        auto descriptor = Board::detail::map::uartDescriptor(channel);

        descriptor->disableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
            HAL_GPIO_DeInit(descriptor->pins().at(i).port, descriptor->pins().at(i).index);

        HAL_NVIC_DisableIRQ(descriptor->irqn());
    }
}

extern "C" void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    uint8_t channel = 0;

    if (Board::detail::map::i2cChannel(hi2c->Instance, channel))
    {
        auto descriptor = Board::detail::map::i2cDescriptor(channel);

        descriptor->enableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
            CORE_IO_CONFIG(descriptor->pins().at(i));

        if (descriptor->irqn() != 0)
        {
            HAL_NVIC_SetPriority(descriptor->irqn(), 0, 0);
            HAL_NVIC_EnableIRQ(descriptor->irqn());
        }
    }
}

extern "C" void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    uint8_t channel = 0;

    if (Board::detail::map::i2cChannel(hi2c->Instance, channel))
    {
        auto descriptor = Board::detail::map::i2cDescriptor(channel);

        descriptor->disableClock();

        for (size_t i = 0; i < descriptor->pins().size(); i++)
            HAL_GPIO_DeInit(descriptor->pins().at(i).port, descriptor->pins().at(i).index);

        HAL_NVIC_DisableIRQ(descriptor->irqn());
    }
}

extern "C" void InitSystem(void)
{
    //set stack pointer
    __set_MSP(*reinterpret_cast<volatile uint32_t*>(FLASH_START_ADDR));

    //setup FPU
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10 and CP11 Full Access */
#endif

    //set the correct address of vector table
    SCB->VTOR = FLASH_START_ADDR;
}