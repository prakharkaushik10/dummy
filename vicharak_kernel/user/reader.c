#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define DEVICE_NAME "/dev/prakhar"

struct data_packet {
int length;
char *data;
};

#define POP_DATA _IOR('p',3, struct data_packet)

int main(void) {
int fd = open(DEVICE_NAME, O_RDWR);
if (fd<0) {
perror("open");
return 1;
}
struct data_packet pkt;
pkt.length =10;
pkt.data = malloc(pkt.length);

if (ioctl(fd,POP_DATA,&pkt)){
perror("ioctl");
close(fd);
return 1;
}
write(1,pkt.data,pkt.length);
write(1,"\n",1);


free(pkt.data);
close(fd);
return 0;
}
