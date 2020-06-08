#include <omron_D6T8L09.h>


int Sampling = 3;

int main(){
    int16_t Max_temp = 0;
    int16_t MAX_Sampling_temp;

    for (int i=0;i<=Sampling;i++){
        printf("Sampling : %d \n", i); 
        MAX_Sampling_temp = D6T8L09_Read();
        if (MAX_Sampling_temp >  Max_temp){
              Max_temp = MAX_Sampling_temp;
        }
        if(MAX_Sampling_temp < 0){
              printf("\n\nError : %d \n\n\n",  MAX_Sampling_temp);
              break;
        }

    }

    printf("\n\nMAX Temp!!!(4 celcius offset) : %4.1f \n\n\n",  (Max_temp / 10.0)+4); 
        
}
