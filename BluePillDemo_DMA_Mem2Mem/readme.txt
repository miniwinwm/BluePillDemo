This example shows memory to memory DMA transfer in polling and interrupt mode. The Device
Configuration Tool is used to set up a memorytomemory DMA transfer on Channel 1. This then 
allows the polling method to be used. The Device Configuration Tool does not seem able to
set up memorytomemory DMA completely in interrupt mode. Some additional code needs to be
added. In main() the interrupts need configuring with the NVIC with this code...

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  
In stm32f1xx_it.c an interrupt handler needs to be added like this...

extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

void DMA1_Channel1_IRQHandler(void)
{
    /* Check the interrupt and clear flag */
    HAL_DMA_IRQHandler(&hdma_memtomem_dma1_channel1);
}

After that the normal HAL API can be used.

No schematic for this example.