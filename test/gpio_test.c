#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    int gpio_pin;

    printf("Enter GPIO pin number:");
    scanf("%d",&gpio_pin);
    //export gpio to user
    FILE *export_file = fopen("/sys/class/gpio/export","w");
    if(export_file == NULL){
    	perror("Failed to open GPIO export file");
	return -1;
    }
    fprintf(export_file,"%d",gpio_pin);
    fclose(export_file);
    
    //get the direction_path
    char direction_path[50];
    snprintf(direction_path,sizeof(direction_path),"sys/class/gpio/gpio%d/value",gpio_pin);//get the format char
    //specify the pin direction
    FILE *direction_file = fopen(direction_path,"w");
    if (direction_file == NULL){
        perror("Failed to open GPUI direction file");
	return -1;
    }
    fprintf(direction_file,"out");
    fclose(direction_file);

    char value_path[50];
    snprintf(value_path,sizeof(value_path),"sys/class/gpio/gpio%d/value",gpio_pin);
    FILE *value_file = fopen(value_path,"w");
    if(value_file == NULL){
        perror("Failed to open GPIO value file");
	return -1;
    }
    
    for(int i=0; i<300; i++){
        fprintf(value_file,"1");
        fflush(value_file);
	sleep(1);

        fprintf(value_file,"0");
        fflush(value_file);
	sleep(1);

    }

    fclose(value_file);
    return 0;
}
