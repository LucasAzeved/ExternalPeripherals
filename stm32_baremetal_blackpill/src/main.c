#include <stm32f4xx_conf.h>
#include <hal.h>
#include <usart.h>
#include <libc.h>

#define GPIO_READ 0x00
#define GPIO_WRITE 0x01
#define GPIO_CONFIGURE_EVENT 0x02

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

// ------------ UTILITARIOS [INICIO] -------------

void print_frame(uint8_t *frame, int frame_size)
{
    printf("| ");
    for (int i = 0; i < frame_size; i++)
    {
        printf("0x%02X ", frame[i]);
    }
    printf("|\n");
}

void send_serial_data(struct data_s *data)
{
    uint8_t frame[256];  // Buffer for the frame including delimiters and escape characters
    int frame_pos = 0;
    
    // Adicionar delimitador de início do quadro
    frame[frame_pos++] = FRAME_DELIMITER;
    
    uint16_t tid_be =  data->tid;
    uint16_t addr_be = data->addr;
    int16_t data_be =  data->data;
    
    // Realizar byte stuffing e adicionar os dados convertidos para o frame
    uint8_t *tid_ptr = (uint8_t *)&tid_be;
    uint8_t *addr_ptr = (uint8_t *)&addr_be;
    uint8_t *data_ptr = (uint8_t *)&data_be;
    
    // Enviar TID (2 bytes)
    for (int i = 0; i < 2; i++)
    {
        if (tid_ptr[i] == FRAME_DELIMITER || tid_ptr[i] == ESCAPE_CHAR)
        {
            frame[frame_pos++] = ESCAPE_CHAR;
            frame[frame_pos++] = tid_ptr[i] ^ XOR_VALUE;
        }
        else
        {
            frame[frame_pos++] = tid_ptr[i];
        }
    }
    
    // Enviar operação (1 byte)
    if (data->oper == FRAME_DELIMITER || data->oper == ESCAPE_CHAR)
    {
        frame[frame_pos++] = ESCAPE_CHAR;
        frame[frame_pos++] = data->oper ^ XOR_VALUE;
    }
    else
    {
        frame[frame_pos++] = data->oper;
    }
    
    // Enviar endereço (2 bytes)
    for (int i = 0; i < 2; i++)
    {
        if (addr_ptr[i] == FRAME_DELIMITER || addr_ptr[i] == ESCAPE_CHAR)
        {
            frame[frame_pos++] = ESCAPE_CHAR;
            frame[frame_pos++] = addr_ptr[i] ^ XOR_VALUE;
        }
        else
        {
            frame[frame_pos++] = addr_ptr[i];
        }
    }

    // Enviar dado (2 bytes)
    for (int i = 0; i < 2; i++)
    {
        if (data_ptr[i] == FRAME_DELIMITER || data_ptr[i] == ESCAPE_CHAR)
        {
            frame[frame_pos++] = ESCAPE_CHAR;
            frame[frame_pos++] = data_ptr[i] ^ XOR_VALUE;
        }
        else
        {
            frame[frame_pos++] = data_ptr[i];
        }
    }

    // Adicionar delimitador de fim do quadro
    frame[frame_pos++] = FRAME_DELIMITER;

    // Print do frame no formato especificado
    // print_frame(frame, frame_pos);

    // Enviar o quadro
    for (int i = 0; i < frame_pos; i++) {
        putchar(frame[i]);
    }
}

// ------------ UTILITARIOS [FIM] -------------

// ------------ FUNCOES THREAD [INICIO] -------------

void *taskAsyncIRQ(void *arg)
{
	char buf[256];
	struct data_s *data = (struct data_s *)&buf;
    
    if (flag_irq != 0xffff) {
        
        // printf("Antes: 0x%04x\n", flag_irq);
        memset(buf, 0, sizeof(buf));
        data->tid = 0xffff;
        data->oper = 0;
        data->addr = flag_irq;
        data->data = 0;
        // printf("[IRQ] addr: 0x%04X tid: 0x%04X \n", data->addr, data->tid);
        
        send_serial_data(buf);
        flag_irq = 0xffff;
        
        // printf("Depois: 0x%04x\n", flag_irq);
    }
	
	return 0;
}

void *taskProcessCommand(void *arg)
{
    char buf[256];
	struct data_s *data = (struct data_s *)&buf;
    
    uint8_t data_buf[sizeof(struct data_s)];
    int data_pos = 0;
    int escaped = 0;
    int count_buf = 0;
	
    if (kbhit()) {
        memset(buf, 0, sizeof(buf));
        // for (int i = 0; i < sizeof(struct data_s) && kbhit(); i++) {
        for (int i = 0; i < 256; i++) {
            buf[i] = getchar();
            count_buf++;
            // Iterar pelos dados recebidos e decodificá-los
            if (buf[i] == FRAME_DELIMITER)
            {
                continue;
            }
            else if (buf[i] == ESCAPE_CHAR)
            {
                // O próximo byte está escapado
                escaped = 1;
            }
            else
            {
                if (escaped)
                {
                    data_buf[data_pos++] = buf[i] ^ XOR_VALUE;
                    escaped = 0;
                }
                else
                {
                    data_buf[data_pos++] = buf[i];
                }
            }
            
            // Parar quando a estrutura completa estiver preenchida
            if (data_pos >= sizeof(struct data_s)) {
                buf[i+1] = getchar();
                count_buf++;
                break;
            }
        }

        // print_frame(buf, count_buf);
        // Copiar os dados decodificados para a estrutura
        memcpy(data, data_buf, sizeof(struct data_s));
        process_command(data);
        // printf("TID: 0x%04X OP: 0x%02X ADDR: 0x%04X DATA: 0x%04X \n", data->tid, data->oper, data->addr, data->data);

    }
	
	return 0;
}

void process_command(struct data_s *data)
{
    switch (data->oper)
    {
    case GPIO_READ: // LEITURA
        switch (data->addr) {
            case 0x1001:
                data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
                break;
            case 0x1002:
                data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
                break;
            case 0x1003:
                data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2);
                break;
            case 0x1004:
                data->data = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3);
                break;
            default:
                data->data = 0xffff; // Unknown address
                break;
        }
        
        send_serial_data(data);
        break;
    
    case GPIO_WRITE: // ESCRITA
        switch (data->addr)
        {
        case 0x1001:
            if (data->data == 0x0001)
                GPIO_SetBits(GPIOB, GPIO_Pin_6);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_6);
            break;
        case 0x1002:
            if (data->data == 0x0001)
                GPIO_SetBits(GPIOB, GPIO_Pin_7);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            break;
        case 0x1003:
            if (data->data == 0x0001)
                GPIO_SetBits(GPIOB, GPIO_Pin_8);
            else
                GPIO_ResetBits(GPIOB, GPIO_Pin_8);
            break;
        case 0x1004:
            if (data->data == 0x0001)
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

// ------------ FUNCOES THREAD [FIM] -------------

// ------------ ESTRUTURA CORROTINAS [INICIO] -------------

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

// ------------ ESTRUTURA CORROTINAS [FIM] -------------


// ------------ INTERRUPCOES [INICIO] -------------

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
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    /* Configure EXTI Line0 */

    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
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

// ------------ INTERRUPCOES [FIM] -------------

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

// ------------ MAIN [INICIO] -------------

void main(void)
{
    struct data_s *data;
    uint8_t *buf = (uint8_t *)&data;
    
    struct task_s tasks[MAX_TASKS] = { 0 };
	struct task_s *ptasks = tasks;
    
    configure_output_pins();
    configure_input_pins();
    
    // 0: Rising
    // 1: Falling
    // 2: Rising Falling
    
    uart_init(USART_PORT, 115200, 0);
    
    task_add(ptasks, taskAsyncIRQ, 10);
	task_add(ptasks, taskProcessCommand, 20);
    
    while (1)
    {
        task_schedule(ptasks);
        
    }
}

// ------------ MAIN [FIM] -------------