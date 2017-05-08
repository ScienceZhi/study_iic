#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include<linux/i2c.h>  
#include<linux/i2c-dev.h> 
#define DEVICE_NAME "/dev/i2c-0"
#define SLAVE_GSENSOR_ADDR 0x0f//这是7位
//以下所有代码都很粗糙和不规范，请谅解
static int iic_read(int fd, char buff[], int addr, int count)  
{  
    int res;  
    char sendbuffer1[1];  
    sendbuffer1[0]=addr;  
    //sendbuffer1[1]=addr;  
    write(fd,sendbuffer1,1);        
        res=read(fd,buff,count);  
        printf("read %d byte at 0x%x\n", res, addr);  
        return res;  
}
static int iic_write(int fd, char buf, int addr, int count)  
{  
        int res;  
        int i,n;  
        char sendbuffer[2];  
        //memcpy(sendbuffer+2, buff, count);  
   	sendbuffer[0]=addr;//寄存器地址
	sendbuffer[1]=buf; //写入该基础的值
        res=write(fd,sendbuffer,count+1);  
        printf("write %d byte at 0x%x\n", res, addr);  
}  

int main()
{

	int fd = -1;//这是i2c
	char buf[3] = {0};
	int ret = -1;
	int regaddr = 0x0c;

	fd = open(DEVICE_NAME, O_RDWR);
	if(fd < 0)
	{		 
		perror("can't open i2c-0 device!");
		return -1;	
	}


	ioctl(fd, I2C_TENBIT, 0);
	ioctl(fd, I2C_SLAVE, SLAVE_GSENSOR_ADDR);

	regaddr = 0x1D;
	buf[0] = 0x16;
	ret=iic_write(fd,buf[0],regaddr,1);

	buf[0] = 0;
	buf[1] = 0;
	regaddr = 0x0c;
	ret =iic_read(fd,buf,regaddr,1); 
	printf("%d bytes read:",ret);  
	int i;        
	for(i=0;i<ret;i++){  
        	printf("%x ",buf[i]);  
        }  
        printf("\n");  	


	
	close(fd);
	return 0;	
}
