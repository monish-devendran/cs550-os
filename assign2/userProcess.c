#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define dev "/dev/process_list"
#define BUFFER_LENGTH 256000            
static char receive[BUFFER_LENGTH];

int main()
{
    //code goes here
    int fd;
	
	fd = open(dev, O_RDONLY);

	if(fd < 0){
		printf("Error in opening the device");
		exit;
	}
    printf("Reading from the device...\n");
	
	int ret = read(fd, receive, BUFFER_LENGTH);
	
    if (ret < 0){
		perror("Failed to read the message from the device.");
		return errno;
	}

	printf("OUTPUT PROCESS LIST FROM CHARACTER DEVICE %s", receive);



    return 0;
}