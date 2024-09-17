#include <stm32f4xx_conf.h>
#include <hal.h>
#include <usart.h>
#include <libc.h>

#define GPIO_READ 0
#define GPIO_WRITE 1
#define GPIO_CONFIGURE_EVENT 2

#define FRAME_DELIMITER 0x7e
#define ESCAPE_CHAR     0x7d
#define XOR_VALUE       0x20

#define MAX_TASKS	20

volatile uint16_t flag_irq = 0xffff;

struct data_s
{
    uint16_t tid;
    uint8_t oper;
    uint16_t addr;
    int16_t data;
} __attribute((packed))__;

// Viagem isso aqui, vamo usa o padrao dos exemplos
// void send_serial_data(struct data_s *data)
// {
//     uint8_t *buf = (uint8_t *)data;
//     putchar(FRAME_DELIMITER);
//     for (int i = 0; i < sizeof(struct data_s); i++)
//     {
//         if (buf[i] == FRAME_DELIMITER || buf[i] == ESCAPE_CHAR)
//         {
//             putchar(ESCAPE_CHAR);
//             putchar(buf[i] ^ XOR_VALUE);
//         }
//         else
//         {
//             putchar(buf[i]);
//         }
//     }
//     putchar(FRAME_DELIMITER);
// }

void send_serial_data(struct data_s *data) {
    uint8_t *buf = (uint8_t *)data;
    for (int i = 0; i < sizeof(struct data_s); i++)
        putchar(buf[i]);
}

void *taskAsyncIRQ(void *arg)
{
	// static int cont = 0;
	// printf("task 1, cont: %d\n", cont++);
	
    struct data_s *data;
    uint8_t *buf = (uint8_t *)&data;

    if (flag_irq != 0xffff) {
        memset(buf, 0, sizeof(buf));
        data->tid = 0xffff;
        data->oper = 0;
        data->addr = flag_irq;
        data->data = 0;
        send_serial_data(data);
        flag_irq = 0xffff;
        
    }
	
	return 0;
}

void *taskProcessCommand(void *arg)
{
	// static int cont = 0;
	// printf("task 2, cont: %d\n", cont++);

    struct data_s *data;
    uint8_t *buf = (uint8_t *)&data;
	
    if (kbhit()) {
        memset(buf, 0, sizeof(buf));
        for (int i = 0; i < sizeof(struct data_s) && kbhit(); i++)
            buf[i] = getchar();
        process_command(data);
    }
	
	return 0;
}

struct task_s {
	void *(*task)(void *);
	unsigned char priority;
	unsigned char pcounter;
};

void task_add(struct task_s *tasks, void *(task_ptr)(void *), unsigned char priority)
{
	struct task_s *ptask;
	int i;
	
	for (i = 0; i < MAX_TASKS; i++) {
		ptask = &tasks[i];
		if (!ptask->task) break;
	}
	
	ptask->task = task_ptr;
	ptask->priority = priority;
	ptask->pcounter = priority;
}

void task_schedule(struct task_s *tasks)
{
	struct task_s *ptask;
	int i;
	
	while (1) {
		for (i = 0; i < MAX_TASKS; i++) {
			ptask = &tasks[i];
			if (!ptask->task) break;
			
			if (!--ptask->pcounter) {
				ptask->pcounter = ptask->priority;
				ptask->task(0);
				return;
			}
		}
	}
}

// void process_command(struct data_s *data);

// void EXTI0_IRQHandler(void)
// {
//     char buffer_int[256];
//     struct data_s *data_int = (struct data_s *)&buffer_int;
    
//     if (EXTI_GetITStatus(EXTI_Line0) != RESET)
//     {
//         memset(buffer_int, 0, sizeof(buffer_int));
//         data_int->tid = 0xffff;
//         data_int->oper = 0;
//         data_int->addr = 0x0001;
//         data_int->data = 0;
        
//         // Simulate response for interrupt trigger
//         process_command(data_int);
        
//         for (int i = 0; i < sizeof(struct data_s); i++)
//             putchar(buffer_int[i]);
        
//         // Clear the EXTI line 0 pending bit
//         EXTI_ClearITPendingBit(EXTI_Line0);
//     }
// }

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        flag_irq = 0x0010;
        
        // Clear the EXTI line 0 pending bit
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        flag_irq = 0x0011;
        
        // Clear the EXTI line 0 pending bit
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        flag_irq = 0x0012;
        
        // Clear the EXTI line 0 pending bit
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        flag_irq = 0x0013;
        
        // Clear the EXTI line 0 pending bit
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void configure_PA0_irq(uint16_t edge_opt)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    char edge;

    switch (edge_opt)
    {
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
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0); // ALTERADO

    /* Configure EXTI Line0 */

    // ALTERAR LINE [LINE]
    EXTI_InitStructure.EXTI_Line = EXTI_Line0; // ALTERADO
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    // ALTERAR TRIGGER [EDGE]
    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    // ALTERAR CHANNEL EXTI [LINE]
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; // ALTERADO
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void configure_PA1_irq(uint16_t edge_opt)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    char edge;

    switch (edge_opt)
    {
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
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1); // ALTERADO

    /* Configure EXTI Line0 */

    // ALTERAR LINE [LINE]
    EXTI_InitStructure.EXTI_Line = EXTI_Line1; // ALTERADO
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    // ALTERAR TRIGGER [EDGE]
    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    // ALTERAR CHANNEL EXTI [LINE]
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn; // ALTERADO
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void configure_PA2_irq(uint16_t edge_opt)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    char edge;

    switch (edge_opt)
    {
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
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2); // ALTERADO

    /* Configure EXTI Line0 */

    // ALTERAR LINE [LINE]
    EXTI_InitStructure.EXTI_Line = EXTI_Line2; // ALTERADO
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    // ALTERAR TRIGGER [EDGE]
    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    // ALTERAR CHANNEL EXTI [LINE]
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; // ALTERADO
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void configure_PA3_irq(uint16_t edge_opt)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    char edge;

    switch (edge_opt)
    {
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
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource3); // ALTERADO

    /* Configure EXTI Line0 */

    // ALTERAR LINE [LINE]
    EXTI_InitStructure.EXTI_Line = EXTI_Line3; // ALTERADO
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    // ALTERAR TRIGGER [EDGE]
    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    // ALTERAR CHANNEL EXTI [LINE]
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn; // ALTERADO
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
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void configure_input_pins()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* GPIOA Peripheral clock enable. */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* configure board key as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// ------------ CONFIGURA MODO DOS PINOS [FIM] -------------

void process_command(struct data_s *data)
{
    switch (data->oper)
    {
    case GPIO_READ: // LEITURA
        switch (data->addr)
        {
        case 0x1001:
            data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
            break;
        case 0x1002:
            data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2);
            break;
        case 0x1003:
            data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
            break;
        case 0x1004:
            data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4);
            break;
        default:
            data->data = 0xFFFF; // Unknown address
            break;
        }
        break;

    case GPIO_WRITE: // ESCRITA
        switch (data->addr)
        {
        case 0x1001:
            if (data->data == 1)
                GPIO_SetBits(GPIOB, GPIO_Pin_6);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            break;
        case 0x1002:
            if (data->data == 1)
                GPIO_SetBits(GPIOB, GPIO_Pin_7);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            break;
        case 0x1003:
            if (data->data == 1)
                GPIO_SetBits(GPIOB, GPIO_Pin_8);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_8);
            break;
        case 0x1004:
            if (data->data == 1)
                GPIO_SetBits(GPIOB, GPIO_Pin_9);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_9);
            break;
        default:
            data->data = 0xFFFF; // Unknown address
            break;
        }
        break;

    case GPIO_CONFIGURE_EVENT: // Configuração de eventos de gatilho
        configure_gpio_irq(data->addr, data->data);
        break;

    default:
        data->data = 0xFFFF; // Unknown operation
        break;
    }
}

void configure_gpio_irq(uint16_t addr, uint16_t edge_opt)
{
    switch (addr)
    {
    case 0x1101:
        configure_PA0_irq(edge_opt);
        break;
    case 0x1102:
        configure_PA1_irq(edge_opt);
        break;
    case 0x1103:
        configure_PA2_irq(edge_opt);
        break;
    case 0x1104:
        configure_PA3_irq(edge_opt);
        break;
    default:
        // Handle unknown address
        break;
    }
}

// ------------ MAIN [INICIO] -------------

/* Duvidas:
    Pull Up e Pull Down?
    Resistores pros botões
    Corrotinas

*/

void main(void)
{
    struct data_s *data;
    uint8_t *buf = (uint8_t *)&data;

    struct task_s tasks[MAX_TASKS] = { 0 };
	struct task_s *ptasks = tasks;
    
    
    configure_output_pins();
    configure_input_pins();
    
    uart_init(USART_PORT, 115200, 0);

    task_add(ptasks, taskAsyncIRQ, 10);
	task_add(ptasks, taskProcessCommand, 20);
    
    while (1)
    {
        task_schedule(ptasks);


        // if (flag_irq != 0xffff) {
		// 	memset(buf, 0, sizeof(buf));
        //     data->tid = 0xffff;
        //     data->oper = 0;
        //     data->addr = flag_irq;
        //     data->data = 0;
        //     send_serial_data(data);
        //     flag_irq = 0xffff;
            
        // }
		// if (kbhit()) {
		// 	memset(buf, 0, sizeof(buf));
			
		// 	for (int i = 0; i < sizeof(struct data_s) && kbhit(); i++)
		// 		buf[i] = getchar();
            
        //     process_command(data);

		// }
    }
}

// ------------ MAIN [FIM] -------------