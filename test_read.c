#include <omron_D6T8L09.h>


int Sampling = 3;
int16_t temp;

int main(){
 		
 		temp = D6T8L09_GetTemp(Sampling); 
 		printf("\n\nBody Temp!!!(4 celcius offset) : %4.1f \n\n\n", (temp/10.0)); 

}
