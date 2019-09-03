
void I2C_HW_Init(void)
{
    LL_RCC_ClocksTypeDef rcc_clocks;
    LL_GPIO_InitTypeDef GPIO_Initstruct;
	/* 1.初始化对应IO. */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    GPIO_Initstruct.Pin        = LL_GPIO_PIN_6 | LL_GPIO_PIN_9;
    GPIO_Initstruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_Initstruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_Initstruct.Pull       = LL_GPIO_PULL_UP;
    GPIO_Initstruct.Alternate	 = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOB, &amp;GPIO_Initstruct);
    RCC-&gt;APB1RSTR |= (RCC_APB1RSTR_I2C1RST);
    RCC-&gt;APB1RSTR &amp;= ~(RCC_APB1RSTR_I2C1RST);
	/* 2.开启I2C时钟. */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
	/* 3.先禁止I2C外设,这样才能设置寄存器. */
    LL_I2C_Disable(I2C1);
	/* 4.设置I2C速度为100kHz. */
    LL_RCC_GetSystemClocksFreq(&amp;rcc_clocks);
    LL_I2C_ConfigSpeed(I2C1, rcc_clocks.PCLK1_Frequency, 100000, LL_I2C_DUTYCYCLE_2);
	/* 5.使能I2C外设. */
    LL_I2C_Enable(I2C1);
}
void I2C_HW_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	/* 1.保证I2C外设不在使用中. */
    while(LL_I2C_IsActiveFlag_BUSY(I2C1))
    {
    }
	/* 2.发送START信号 */
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1))
    {
    }
	/* 2.写器件地址 */
    LL_I2C_TransmitData8(I2C1, Addr);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
    {
    }
    LL_I2C_ClearFlag_ADDR(I2C1);
	/* 3.地址位已经置位,通常TXE也会完成,为了谨慎,再查一下. */
    while(!LL_I2C_IsActiveFlag_TXE(I2C1))
    {
    }
	/* 4.发送器件寄存器地址. */
    LL_I2C_TransmitData8(I2C1, Reg);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1))
    {
    }
	/* 5.写入寄存器内容 */
    LL_I2C_TransmitData8(I2C1, Value);
    while(!LL_I2C_IsActiveFlag_BTF(I2C1))
    {
    }
	/* 6.传送结束条件. */
    LL_I2C_GenerateStopCondition(I2C1);
}
uint8_t I2C_HW_Read(uint8_t Addr, uint8_t Reg)
{
    uint8_t Value = 0x00;
	/* 1.保证I2C外设不在使用中. */
    while(LL_I2C_IsActiveFlag_BUSY(I2C1))
    {
    }
	/* 2.发送START信号 */
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1))
    {
    }
	/* 2.写器件地址 */
    LL_I2C_TransmitData8(I2C1, Addr);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
    {
    }
    LL_I2C_ClearFlag_ADDR(I2C1);
	/* 3.地址位已经置位,通常TXE也会完成,为了谨慎,再查一下. */
    while(!LL_I2C_IsActiveFlag_TXE(I2C1))
    {
    }
	/* 4.发送器件寄存器地址. */
    LL_I2C_TransmitData8(I2C1, Reg);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1))
    {
    }
	/* 5.提供RESTART信号. */
    LL_I2C_GenerateStopCondition(I2C1);
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1))
    {
    }
	/* 6.重新发送地址,并附带读标记. */
    LL_I2C_TransmitData8(I2C1, Addr | 0x01);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
    {
    }
	/* 7.标记读完就STOP. */
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
    LL_I2C_ClearFlag_ADDR(I2C1);
    LL_I2C_GenerateStopCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_RXNE(I2C1))
    {
    }
	/* 8.数据已读取. */
    Value = LL_I2C_ReceiveData8(I2C1);
    return Value;
}
