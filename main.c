#include <stm32f4xx_conf.h>
#include <hal.h>
#include <usart.h>
#include <libc.h>

struct data_s
{
    uint16_t tid;
    uint8_t oper;
    uint16_t addr;
    int16_t data;
} _attribute((packed)) _;

void send_serial_data(struct data_s *data)
{
    uint8_t *bytes = (uint8_t *)data;
    putchar(FRAME_DELIMITER);
    for (int i = 0; i < sizeof(struct data_s); i++)
    {
        if (bytes[i] == FRAME_DELIMITER || bytes[i] == ESCAPE_CHAR)
        {
            putchar(ESCAPE_CHAR);
            putchar(bytes[i] ^ XOR_VALUE);
        }
        else
        {
            putchar(bytes[i]);
        }
    }
    putchar(FRAME_DELIMITER);
}

void process_command(struct data_s *data);

void EXTI0_IRQHandler(void)
{
    char buffer_int[256];
    struct data_s *data_int = (struct data_s *)&buffer_int;

    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        memset(buffer_int, 0, sizeof(buffer_int));
        data_int->tid = 0xffff;
        data_int->oper = 0;
        data_int->addr = 0x0001;
        data_int->data = 0;

        // Simulate response for interrupt trigger
        process_command(data_int);

        for (int i = 0; i < sizeof(struct data_s); i++)
            putchar(buffer_int[i]);

        // Clear the EXTI line 0 pending bit
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

/*void configure_PA_irq(uint16_t line_opt, uint16_t edge_opt)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    char line, pinsrc, channel, edge;

    switch (line_opt)
    {
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

    ;; GPIOA Peripheral clock enable.
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    ;; Connect EXTI Line0 to PA0 pin
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    // ALTERAR PINSOURCE [LINE]
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, pinsrc); // ALTERADO

    ;; Configure EXTI Line0

    // ALTERAR LINE [LINE]
    EXTI_InitStructure.EXTI_Line = line; // ALTERADO
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

    // ALTERAR TRIGGER [EDGE]
    EXTI_InitStructure.EXTI_Trigger = edge; // ALTERADO
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Enable and set EXTI Line0 Interrupt to the lowest priority
    // ALTERAR CHANNEL EXTI [LINE]
    NVIC_InitStructure.NVIC_IRQChannel = channel; // ALTERADO
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}*/

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

/* Duvidas:
    Pull Up e Pull Down?
    Como usar o Handler?
    Podemos usar variaveis para setar Lines e Edges?


*/

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
                GPIO_SetBits(GPIOA, GPIO_Pin_1);
            else
                GPIO_ResetBits(GPIOA, GPIO_Pin_1);
            break;
        case 0x1002:
            if (data->data == 1)
                GPIO_SetBits(GPIOA, GPIO_Pin_2);
            else
                GPIO_ResetBits(GPIOA, GPIO_Pin_2);
            break;
        case 0x1003:
            if (data->data == 1)
                GPIO_SetBits(GPIOA, GPIO_Pin_3);
            else
                GPIO_ResetBits(GPIOA, GPIO_Pin_3);
            break;
        case 0x1004:
            if (data->data == 1)
                GPIO_SetBits(GPIOA, GPIO_Pin_4);
            else
                GPIO_ResetBits(GPIOA, GPIO_Pin_4);
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
    // Você precisará criar esta função para configurar interrupções com base no endereço e tipo de borda
    // Exemplo:
    switch (addr)
    {
    case 0x1101:
        configure_PA0_irq(edge_opt);
        break;
    case 0x1104:
        configure_PA1_irq(edge_opt);
        break;
    case 0x1107:
        configure_PA2_irq(edge_opt);
        break;
    case 0x110A:
        configure_PA3_irq(edge_opt);
        break;
    default:
        // Handle unknown address
        break;
    }
}

// ------------ MAIN [INICIO] -------------
void main(void)
{
    configure_output_pins();
    configure_input_pins();

    // Configure each pin to trigger an interrupt on state change
    configure_PA_irq(EXTI_Line0, EXTI_Trigger_Rising_Falling);

    while (1)
    {
        struct data_s data;
        if (usart_read_frame(&data, sizeof(data)))
        {
            process_command(&data);
        }
    }
}

// ------------ MAIN [FIM] -------------