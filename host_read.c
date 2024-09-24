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
	char buf[256];
    struct data_s *data = (struct data_s *)&buf;

	if (argc != 2) {
		printf("Usage: serial_host_data <port>\n");
		
		return -1;
	}
	
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (fd == -1) {
		printf("\nError opening port");
		return -1;
	}	
    
    read(fd, buf, sizeof(struct data_s));
    
    printf("Running...\n");
	
    while (1) {
		
        memset(buf, 0, sizeof(buf));
        
        usleep(500000);
		
        receive_serial_data(fd, data);
        
        if (data->tid == 0x0)
            continue;

        if (data->tid == 0xffff) {
            printf("[IRQ] ");
        }
        printf("0x%04X 0x%02X 0x%04X 0x%04X \n", data->tid, data->oper, data->addr, data->data);
	}
	
    close(fd);
    
	return 0;
}
