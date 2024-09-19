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
	// struct data_s s;
	// struct data_s *data = &s;
	// uint8_t *ptr = (uint8_t *)&s;
	
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

    
    while (1) {
        data->tid = 0x1010; data->oper = 0x00; data->addr = 0x1005; data->data = 0x0001;
		write(fd, buf, sizeof(struct data_s));
        usleep(1000000);
        memset(buf, 0, sizeof(buf));
        read(fd, buf, sizeof(struct data_s));
        printf("Status 0x%04X\n", data->data);
        // data->data = 0x0001;

        // scanf("%0d", &test);
	}
    
    close(fd);
	
    return 0;
}
