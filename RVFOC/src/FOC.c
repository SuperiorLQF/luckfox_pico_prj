#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
// utility defines
#define _2_SQRT3 1.15470053838
#define _SQRT3 1.73205080757
#define _1_SQRT3 0.57735026919
#define _SQRT3_2 0.86602540378
#define _SQRT2 1.41421356237
#define _120_D2R 2.09439510239
#define _PI 3.14159265359
#define _PI_2 1.57079632679
#define _PI_3 1.0471975512
#define _2PI 6.28318530718
#define _3PI_2 4.71238898038
#define _PI_6 0.52359877559

#define Uq_threshold 0
#define voltage_power_supply 12
float _normalizeAngle(float angle){
  float a = fmod(angle, (float)(2*_PI));   //取余运算可以用于归一化，列出特殊值例子算便知
  return a >= 0 ? a : (a + 2*_PI);  
  //三目运算符。格式：condition ? expr1 : expr2 
  //其中，condition 是要求值的条件表达式，如果条件成立，则返回 expr1 的值，否则返回 expr2 的值。可以将三目运算符视为 if-else 语句的简化形式。
  //fmod 函数的余数的符号与除数相同。因此，当 angle 的值为负数时，余数的符号将与 _2PI 的符号相反。也就是说，如果 angle 的值小于 0 且 _2PI 的值为正数，则 fmod(angle, _2PI) 的余数将为负数。
  //例如，当 angle 的值为 -PI/2，_2PI 的值为 2PI 时，fmod(angle, _2PI) 将返回一个负数。在这种情况下，可以通过将负数的余数加上 _2PI 来将角度归一化到 [0, 2PI] 的范围内，以确保角度的值始终为正数。
}

void UartSend1(uint8_t data){
    char serial_port[]="/dev/ttyS3";

    int serial_fd;
    serial_fd = open(serial_port, O_RDWR | O_NOCTTY);
    if (serial_fd == -1) {
        perror("Failed to open serial port");
    }
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(serial_fd, &tty) != 0) {
        perror("Error from tcgetattr");
    }

    cfsetospeed(&tty, B921600);
    cfsetispeed(&tty, B921600);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
    }

    char *tx_buffer = &data;
    ssize_t bytes_written = write(serial_fd, tx_buffer,1);
    if (bytes_written < 0) {
        perror("Error writing to serial port");
        close(serial_fd);
    }

    close(serial_fd);
	

}
void setPWM(uint8_t Ta_u,uint8_t Tb_u,uint8_t Tc_u){
	UartSend1(Ta_u);
	UartSend1(Tb_u);
	UartSend1(Tc_u);
}
//SVPWM输出，输入是Uq,Ud和当前角度
  void setPhaseVoltage(float Uq,float Ud,float angle_el){
		
	int sector;
	float angle_meg;
    float Ualpha,Ubeta,Uref;
    float T1,T2,T0;
    float Ta,Tb,Tc;	
		//Sensor_update(&S0);//更新传感器数值

	

   //转子落后磁场90度		
	  if(Uq>Uq_threshold)//40us防止Uq在0位波动导致转矩方向急转
		{
		angle_meg=_normalizeAngle(angle_el+_PI_2);//顺时针是角度减小
		angle_el = _normalizeAngle(angle_el+_PI_2);	//!!!	
		}	
		else if(Uq<-Uq_threshold)
		{
		angle_meg=_normalizeAngle(angle_el-_PI_2);
		angle_el = _normalizeAngle(angle_el-_PI_2);
		}
		else{
		angle_meg=angle_el;
		angle_el = _normalizeAngle(angle_el);
		}

			
    //park反变换600us,现在使用快速算法降到了180us
    Ualpha = Ud * cos(angle_el)-Uq * sin(angle_el);
    Ubeta  = Ud * sin(angle_el)+Uq * cos(angle_el);

	
    //计算归一化合成电压矢量Uref，相当于其他资料的Uref/Udc//这段一直到switch需要190us
    Uref = sqrt(Ualpha*Ualpha + Ubeta*Ubeta) / voltage_power_supply;  
    if(Uref>_1_SQRT3) Uref=_1_SQRT3;//这里为何是3分之根号三？
    if(Uref<-_1_SQRT3) Uref=-_1_SQRT3;

    //计算扇区位置
    sector = (int)(angle_el / _PI_3) +1;

    //计算相邻电压矢量和0矢量作用时间,以1为单位
    T1= _SQRT3*sin(sector*_PI_3 - angle_el) * Uref;
    T2= _SQRT3*sin(angle_el-(sector-1.0)*_PI_3) * Uref;
    T0= 1- T1 - T2;//0矢量作用时间
    //
    //计算各相桥臂开通时间,七段式递增
	
    switch(sector)//18us
    {
        case 1:
            Ta = T1 + T2 + T0/2;
            Tb = T2 + T0/2;
            Tc = T0/2;
						break;
        case 2:
            Ta = T1 + T0/2;
            Tb = T1 + T2 + T0/2;
            Tc = T0/2;  
						break;				
        case 3:
            Ta = T0/2;
            Tb = T1 + T2 + T0/2;
            Tc = T2 + T0/2; 
						break;
        case 4:
            Ta = T0/2;
            Tb = T1 + T0/2;
            Tc = T1 + T2 + T0/2; 
						break;
        case 5:
            Ta = T2 + T0/2;
            Tb = T0/2;
            Tc = T1 + T2 + T0/2; 
						break;
        case 6:
            Ta = T1 + T2 + T0/2; 
            Tb = T0/2;
            Tc = T1 + T0/2; 
						break;
        default:
					printf("error\r\n");
            Ta = 0; 
            Tb = 0;
            Tc = 0; 
    }

    //输出PWM，设置占空比
	uint8_t Ta_u,Tb_u,Tc_u;
	Ta_u=(Ta==0)?0x00:(uint8_t)(Ta*256);
    Tb_u=(Tb==0)?0x00:(uint8_t)(Tb*256);
	Tc_u=(Tc==0)?0x00:(uint8_t)(Tc*256);
        setPWM(Ta_u,Tb_u,Tc_u);
		printf("%f,%f,%f--",Ta,Tb,Tc);
		printf("%d,%d,%d\n",Ta_u,Tb_u,Tc_u);
	

}
int main(){
	setPhaseVoltage(0,2,0);//010
}
