
#include "key.h"

void STM_EVAL_PBInit(void)
{
//  GPIO_InitTypeDef GPIO_InitStructure;

//  /* Enable the BUTTON Clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

//  /* Configure Button pin as input floating */
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);


}

uint32_t STM_EVAL_PBGetState(void)
{
  return 0;//GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
}
