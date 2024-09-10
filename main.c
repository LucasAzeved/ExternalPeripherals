#include <stm32f4xx_conf.h>
#include <hal.h>
#include <usart.h>
#include <libc.h>

struct data_s {
	uint16_t tid;
	uint8_t oper;
	uint16_t addr;
	int16_t data;
} __attribute((packed))__;


void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		if (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
			GPIO_ToggleBits(GPIOC, GPIO_Pin_13);
		
		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

void configure_PA_irq(uint16_t line_opt, uint16_t edge_opt)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	

    char line, pinsrc, channel, edge;

    switch (line_opt) {
        case 0:
            line = EXTI_Line0;
            pinsrc = EXTI_PinSource0;
            channel = EXTI0_IRQn;
            break;
        case 1:
            line = EXTI_Line1;
            pinsrc = EXTI_PinSource1;
            channel = EXTI1_IRQn;
            break;
        case 2:
            line = EXTI_Line2;
            pinsrc = EXTI_PinSource2;
            channel = EXTI2_IRQn;
            break;
        case 3:
            line = EXTI_Line3;
            pinsrc = EXTI_PinSource3;
            channel = EXTI3_IRQn;
            break;
    }

    switch (edge_opt) {
        case 0:
            edge = EXTI_Trigger_Rising;
            break;
        case 1:
            edge = EXTI_Trigger_Falling;
            break;
        case 2:
            edge = EXTI_Trigger_Rising_Falling;
            break;
    }

	/* GPIOA Peripheral clock enable. */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* Connect EXTI Line0 to PA0 pin */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    // ALTERAR PINSOURCE [LINE]
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, pinsrc); // ALTERADO
	
	/* Configure EXTI Line0 */
    
    // ALTERAR LINE [LINE]
	EXTI_InitStructure.EXTI_Line = line; // ALTERADO
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	
    // ALTERAR TRIGGER [EDGE]
    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
    // ALTERAR CHANNEL EXTI [LINE]
	NVIC_InitStructure.NVIC_IRQChannel = channel; // ALTERADO
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


// ------------ CONFIGURA MODO DOS PINOS [INICIO] -------------

void configure_output_pins()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOD Peripheral clock enable. */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	/* configure board LEDs as outputs */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void configure_input_pins()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOA Peripheral clock enable. */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* configure board key as input */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// ------------ CONFIGURA MODO DOS PINOS [FIM] -------------

/* Duvidas:
    Pull Up e Pull Down?
    Como usar o Handler?
    Podemos usar variaveis para setar Lines e Edges?


*/ 


// ------------ MAIN [INICIO] -------------
int main(void)
{
    char buf[256];
    struct data_s *data = (struct data_s *)&buf;

    uint16_t d_tid;
	uint8_t d_oper;
	uint16_t d_addr;
	int16_t d_data;

    uart_init(USART_PORT, 115200, 0);

	configure_leds();
	configure_input_pins();
	configure_PA_irq(0, 0);

	while (1) {
		if (kbhit()) {
			memset(buf, 0, sizeof(buf));
			
			// recebe
            for (int i = 0; i < sizeof(struct data_s) && kbhit(); i++)
				buf[i] = getchar();
			
            d_tid = data->tid;
            d_oper = data->oper;
            d_addr = data->addr;
            d_data = data->data;

            switch (d_oper) {
                case 0: // LEITURA
                    switch (d_addr)
                    {
                        case 0x0001:
                            /* code */
                            break;
                        default:
                            break;
                    }
                    break;
                case 1: // ESCRITA
                    switch (d_addr)
                    {
                        case 0x0001:
                            /* code */
                            break;
                        
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }


            // envia
			// for (int i = 0; i < sizeof(struct data_s); i++)
			// 	putchar(buf[i]);
		}
	}
	
	return 0;
}

// ------------ MAIN [FIM] -------------