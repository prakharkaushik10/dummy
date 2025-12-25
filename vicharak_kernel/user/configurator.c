#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/prakhar"

#define SET_SIZE_OF_QUEUE _IOW('p',1,int)
int main(void){
int fd;
int queue_size = 10;
fd = open(DEVICE_NAME,O_RDWR);
if (fd<0){
perror("Failed to open");
return 1;
}
if (ioctl(fd,SET_SIZE_OF_QUEUE,&queue_size)<0){
perror("IOCTL failed");
close(fd);
return 1;
}
printf("SET_SIZE_OF_QUEUE ioctl sent succesfully");
close(fd);
return 0;
}
