#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>

#define FRAME_DELIMITER 0x7e
#define ESCAPE_CHAR 0x7d
#define XOR_VALUE 0x20

struct data_s
{
    uint16_t tid;
    uint8_t oper;
    uint16_t addr;
    int16_t data;
} __attribute__((packed));

void send_serial_data(int fd, struct data_s *data);
void receive_serial_data(int fd, struct data_s *data);
void print_frame(uint8_t *frame, int frame_size);
uint16_t to_big_endian_16(uint16_t value);
int16_t to_big_endian_16_signed(int16_t value);
int wait_for_data(int fd, int timeout);

int main(int argc, char **argv)
{
    int fd;
    char buf[256];
    struct data_s *data = (struct data_s *)&buf;

    // Inicializando o gerador de números aleatórios
    srand(time(NULL));

    if (argc != 2)
    {
        printf("Usage: serial_host_data <port>\n");
        return -1;
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1)
    {
        printf("\nError opening port");
        return -1;
    }

    // Main loop
    while (1)
    {
        // Gerando um TID aleatório
        data->tid = (uint16_t)rand(); 

        // Solicitando input do usuário
        printf("Digite a operação (0 - READ, 1 - WRITE, ou 2 - CONFIGURE EDGE): ");
        scanf("%hhx", &data->oper);  // Entrada em hexadecimal
        printf("Digite o endereço (1001, 1002, 1003 e 1004 para leitura e escrita / 1101, 1102, 1103 e 1104 para configurar): ");
        scanf("%hx", &data->addr);   // Entrada em hexadecimal
        printf("Digite o dado em hexadecimal: ");
        scanf("%hx", &data->data);   // Entrada em hexadecimal

        // Print dos dados a serem enviados para facilitar o debug
        printf("\nDados a serem enviados:\n");
        printf("TID: 0x%04X\n", data->tid);
        printf("Operação: 0x%02X\n", data->oper);
        printf("Endereço: 0x%04X\n", data->addr);
        printf("Dado: 0x%04X\n", data->data);

        // Enviar dados
        send_serial_data(fd, data);
        usleep(1000000); // Pausa de 1 segundo

        if (data->oper == 0x00)
        {
            // Limpar o buffer e ler a resposta
            memset(buf, 0, sizeof(buf));
            
            // Espera por dados na porta serial com timeout de 5 segundos
            if (wait_for_data(fd, 5) > 0)
            {
                receive_serial_data(fd, data);
                print_frame(buf, 256);
            }
            else
            {
                printf("Timeout: Nenhum dado recebido\n");
            }
            // Print da resposta recebida
            printf("Resposta recebida:\n");
            printf("TID: 0x%04X\n", data->tid);
            printf("Operação: 0x%02X\n", data->oper);
            printf("Endereço: 0x%04X\n", data->addr);
            printf("Dado: 0x%04X\n", data->data);
            printf("\n------------------------\n");
        }
    }

    close(fd);
    return 0;
}

void send_serial_data(int fd, struct data_s *data)
{
    uint8_t frame[256];  // Buffer for the frame including delimiters and escape characters
    int frame_pos = 0;

    // Adicionar delimitador de início do quadro
    frame[frame_pos++] = FRAME_DELIMITER;

    // Convertendo os dados para Big-endian no momento do envio
    // uint16_t tid_be = to_big_endian_16(data->tid);
    // uint16_t addr_be = to_big_endian_16(data->addr);
    // int16_t data_be = to_big_endian_16_signed(data->data);
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
    print_frame(frame, frame_pos);

    // Enviar o quadro
    if (write(fd, frame, frame_pos) < 0)
    {
        printf("Erro ao enviar os dados: %s\n", strerror(errno));
    }
}

void receive_serial_data(int fd, struct data_s *data)
{
    uint8_t buf[512];
    uint8_t data_buf[sizeof(struct data_s)];
    int data_pos = 0;
    int escaped = 0;
    
    // Ler os dados da porta serial
    int bytes_read = read(fd, buf, sizeof(buf));
    if (bytes_read < 0)
    {
        printf("Erro ao ler os dados: %s\n", strerror(errno));
        return;
    }
    
    // Iterar pelos dados recebidos e decodificá-los
    for (int i = 0; i < bytes_read; i++)
    {
        if (buf[i] == FRAME_DELIMITER) {
            // Ignorar delimitadores de quadro
            continue;
        }
        else if (buf[i] == ESCAPE_CHAR) {
            // O próximo byte está escapado
            escaped = 1;
        }
        else
        {
            if (escaped) {
                data_buf[data_pos++] = buf[i] ^ XOR_VALUE;
                escaped = 0;
            }
            else {
                data_buf[data_pos++] = buf[i];
            }
        }
        
        // Parar quando a estrutura completa estiver preenchida
        if (data_pos >= sizeof(struct data_s))
            break;
    }
    
    // Copiar os dados decodificados para a estrutura
    memcpy(data, data_buf, sizeof(struct data_s));
}

void print_frame(uint8_t *frame, int frame_size)
{
    int count_delim = 0;
    printf("| ");
    for (int i = 0; i < frame_size; i++) {
        printf("0x%02X ", frame[i]);
        if (frame[i] == FRAME_DELIMITER) {
            if (count_delim < 2) {
                count_delim++;
            }
            else {
                break;
            }
        }
    }
    printf("|\n\n");
}

// Função para converter inteiros de 16 bits para big-endian
uint16_t to_big_endian_16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}

// Função para converter inteiros com sinal de 16 bits para big-endian
int16_t to_big_endian_16_signed(int16_t value)
{
    return (value >> 8) | (value << 8);
}

// Função para aguardar dados na porta serial
int wait_for_data(int fd, int timeout)
{
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return select(fd + 1, &readfds, NULL, NULL, &tv);
}
