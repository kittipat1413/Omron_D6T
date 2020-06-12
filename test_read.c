#include <omron_D6T8L09.h>


int Sampling = 3;
float temp;

int main(){
 		
 		temp = D6T8L09_GetTemp(Sampling); 
 		printf("\n\nBody Temp : %4.1f \n\n\n", temp); 

}
