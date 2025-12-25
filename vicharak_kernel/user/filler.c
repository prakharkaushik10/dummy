#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#define DEVICE_NAME "/dev/prakhar"
#define PUSH_DATA _IOW('p',2,struct data_packet)

struct data_packet {
int length;char *data;
};
int main(void){
int fd = open(DEVICE_NAME, O_RDWR);
if (fd<0){
perror("open");
return 1;
}
struct data_packet pkt;
pkt.length=3;
pkt.data = malloc(3);
memcpy(pkt.data,"xyz",3);
if (ioctl(fd,PUSH_DATA,&pkt)<0){
perror("ioctl");
}
free(pkt.data);
close(fd);
return 0;
}
