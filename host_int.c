#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>


struct data_s
{
    uint16_t tid;
    uint8_t oper;
    uint16_t addr;
    int16_t data;
} __attribute((packed))__;

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
	
	while (1) {
		// write(fd, ptr, sizeof(struct data_s));
		usleep(50000);
		read(fd, ptr, sizeof(struct data_s));
        printf("0x%04X\n", data->tid);
        if (data->tid == 0xffff) {
            printf("0x%04X 0x%02X 0x%04X 0x%04X \n", data->tid, data->oper, data->addr, data->data);
        }
	}
	
    close(fd);
    
	return 0;
}
