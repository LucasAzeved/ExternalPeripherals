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
} __attribute((packed))__;

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

void receive_serial_data(int fd, struct data_s *data)
{
    uint8_t buf[256];
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
        if (buf[i] == FRAME_DELIMITER)
        {
            // Ignorar delimitadores de quadro
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
        if (data_pos >= sizeof(struct data_s))
            break;
    }
    
    // Copiar os dados decodificados para a estrutura
    memcpy(data, data_buf, sizeof(struct data_s));
}

int main(int argc, char **argv)
{
	int fd;
	struct data_s s;
	struct data_s *data = &s;
	uint8_t *ptr = (uint8_t *)&s;
	
	if (argc != 2) {
		printf("Usage: serial_host_data <port>\n");
		
		return -1;
	}
	
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (fd == -1) {
		printf("\nError opening port");
		return -1;
	}	
    
    read(fd, ptr, sizeof(struct data_s));
    
    printf("Running...\n");
	
    while (1) {
		usleep(50000);
		// read(fd, ptr, sizeof(struct data_s));
        receive_serial_data(fd, data);
        // printf("0x%04X\n", data->tid);
        if (data->tid == 0xffff) {
            printf("0x%04X 0x%02X 0x%04X 0x%04X \n", data->tid, data->oper, data->addr, data->data);
        }
	}
	
    close(fd);
    
	return 0;
}
