#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define I2C_DEVICE_PATH "/dev/i2c-3"
#define AS5600_ADDR 0x36
#define LOW_DATA_ADDR 0x0D
#define HIGH_DATA_ADDR 0x0C

#define _2PI 6.28318530718
int main() {
    uint8_t data = 0x00;
	uint8_t * data_p=&data;
    uint8_t result =0x00;
	uint16_t result16=0x0000;
	int     result16_int=0;
	uint8_t * result_p = &result;
    const char *i2c_device = I2C_DEVICE_PATH;  
    int i2c_file;
	float rad;

    if ((i2c_file = open(i2c_device, O_RDWR)) < 0) {
        perror("Failed to open I2C device");
        return -1;
    }
    
    ioctl(i2c_file, I2C_TENBIT, 0);
    ioctl(i2c_file, I2C_RETRIES, 5);//retry times
    	
	ioctl(i2c_file,I2C_SLAVE,AS5600_ADDR);
	while(1){
	data = LOW_DATA_ADDR;
	write(i2c_file,data_p,1);
	read(i2c_file,result_p,1);
	result16=result;
	printf("%x,",result);

	data = HIGH_DATA_ADDR;
	write(i2c_file,data_p,1);
	read(i2c_file,result_p,1);
	result16=result16 | (result << 8);
	
	result16_int=result16;
	rad=((float)result16_int/(float)4096)*_2PI;

	printf("%x,",result);
	printf("%x,",result16);
	printf("%d,",result16);
	printf("%f\n",rad);

	}
    close(i2c_file);


    return 0;
}
