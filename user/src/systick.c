/*!
    \file  systick.c
    \brief the systick configuration file
 
    \version 2015-11-16, V1.0.0, demo for GD32F10x
    \version 2017-06-30, V2.0.0, demo for GD32F10x
    \version 2018-07-31, V2.1.0, demo for GD32F10x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f10x.h"

static uint32_t delay;
//static uint32_t g_localtime;

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SystickConfig(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if(SysTick_Config(SystemCoreClock / 1000U)){    //??????1ms????????????
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    //NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void Delay1ms(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DelayDecrement(void)
{
    if(delay){
        delay--;
    }
}



	//systick????????????		108MHz ??????8??????			//us???????????????
//59652323ns???59652ms???59s
void Delay1us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD??????
	uint8_t  fac_us= SystemCoreClock / 1000000U;
	ticks=nus*fac_us; 						//??????????????????
//	delay_osschedlock();					//??????OS?????????????????????us??????
	told=SysTick->VAL;        				//???????????????????????????
	while(1)
	{
		tnow=SysTick->VAL;
		if(tnow!=told)
		{
			if(tnow<told)tcnt+=told-tnow;	//??????????????????SYSTICK???????????????????????????????????????.
			else tcnt+=reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;			//????????????/????????????????????????,?????????.
		}
	}
}



void Led_Show_Work_ToggleOut(void);


/*!
    \brief      updates the system local time
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Systick_Int_Update(void)
{
    static uint16_t times = 0;
	times ++;
	
	DelayDecrement();
	
	if(times == 1)
	{
		Led_Show_Work_ToggleOut();
	}
	else if(times == 201)
	{
		Led_Show_Work_ToggleOut();
	}
	else if(times >= 401)
	{
		times = 0;
	}
}



void SysTick_Handler(void)
{
	Systick_Int_Update();
}


