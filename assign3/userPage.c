#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define IOCTL_GET_PFN _IOWR('p', 1, unsigned long)

unsigned long getphyaddr(int fd, unsigned long va){	
	if (ioctl(fd, IOCTL_GET_PFN, &va) == -1){
		perror("Failed to invoke ioctl to translate va\n");
	}
	return va;
}



int main(int argc, char *argv[]){
	int fd; 
	if( ( fd = open("/dev/page_walk", O_RDWR)) < 0) {
		printf("Error opening /dev/page_walk: %d\n", fd);
		return -1;
	}	
	unsigned long virt_addr;
	unsigned long phy_addr;
	char input[100];	
	char *endpoint;	
		
	printf("Enter virtual page number: ");
	scanf("%s", input);
	getchar(); 

	virt_addr = strtoul(input, &endpoint, 16); //hexadecimal		
	printf("virtual page number: %lX\n", virt_addr);
    phy_addr = getphyaddr(fd,virt_addr);
    printf("physical page number : %lx\n", virt_addr);
	close(fd);
	return 0;
}
