#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include<linux/i2c.h>  
#include<linux/i2c-dev.h> 
#define DEVICE_NAME "/dev/i2c-0"
#define SLAVE_GSENSOR_ADDR 0x0f//这是7位

int main()
{

	int fd = -1;//这是i2c
	int fd_sensor = -1;//这是Gsensor

	fd = open(DEVICE_NAME, O_RDWR);
	if(fd < 0)
	{		 
		perror("can't open i2c-0 device!");
		return -1;	
	}

	struct i2c_rdwr_ioctl_data work_queue;
	
	ioctl(fd,I2C_TIMEOUT,7);//超时时间    目前还不知道这是用来干嘛的
	ioctl(fd,I2C_RETRIES,1);//重复次数    通过内核分析，看到了IOCTL里的作用是client->adapter->retries = arg;
				//	     分析已经知道了在for (ret = 0, try = 0; try <= adap->retries; try++) {//mabey many times
				//				ret = adap->algo->master_xfer(adap, msgs, num);

	//nmsgs决定了有多少start信号  
	//一个msgs对应一个start信号，(补充：根据我分析内核I2C相关代码，已经知道了为什么说一个msgs对应一个start信号了 
	//在nmsg个信号结束后总线会产生一个stop  
	//下面因为在操作时序中用到2个start信号(字节读操作中)
	work_queue.nmsgs = 2;
	work_queue.msgs = (struct i2c_msg *)malloc(work_queue.nmsgs * sizeof(struct i2c_msg));
	if(!work_queue.msgs)  
	{  
		perror("malloc msgs error");  
		close(fd);  
		return -1;  
	}  
	//从i2c里读出数据
	//len是指buf的长度
	//buf在使用前必须先分配内存，buf=(unsigned char *)malloc(len);
	//如果写，buf[0]是写的地址，buf[1]之后都是写的数据了；如果读，第一遍写地址时buf[0]是地址，第二遍读数据时存放读的数据

	printf("开始读取数据\n");
	work_queue.nmsgs = 2;//根据KXTE9-2050 Gsensor的手册，不管是读1个数据到S5PV210还是读多个数据，都需要两个S信号
	//第一次S信号	
	//先设定一下地址  
	(work_queue.msgs[0]).len = 1;  
	(work_queue.msgs[0]).flags = 0;//write  
	(work_queue.msgs[0]).addr = SLAVE_GSENSOR_ADDR;  
	(work_queue.msgs[0]).buf = (unsigned char *)malloc(1); ;//根据Gsensor手册规则，这里只需要一个buf用来存Gsensor的寄存器地址     	
	(work_queue.msgs[0]).buf[0] = 0x0C;//0x0c寄存器地址，根据芯片手册，该寄存器是只读寄存器，在CTCbit没被设置的时候，返回0x55,设置后返回0xAA

	//第二次S信号
	(work_queue.msgs[1]).len = 1;  
	(work_queue.msgs[1]).flags = I2C_M_RD;//read,也就是1了  
	(work_queue.msgs[1]).addr = SLAVE_GSENSOR_ADDR;  
	(work_queue.msgs[1]).buf = (unsigned char *)malloc(1);
	(work_queue.msgs[1]).buf[0] = 0;//初始化读缓冲	

	int ret = ioctl(fd, I2C_RDWR, (unsigned long)&work_queue);	

	if(ret < 0)
	{
		perror("ioctl error");
		close(fd);
		return -1;
	}

	printf("Read out data: %x\n", (work_queue.msgs[1]).buf[0]);

	close(fd);
	//close(fd_sensor);
	return 0;
}
