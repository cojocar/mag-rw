#include <stdio.h>
#include <stdlib.h>

int main(){
	unsigned char x;
	char i;
	while(1){
		if(scanf("%c",&x) < 1){
			exit(1);
		}
		for(i = 7 ; i >= 0 ; i--){
			if( x >= (1<<i)){
				x -= (1 << i);
				printf("1\n");
			}
			else{
				printf("0\n");
			}
		}
	}
	return 0;
}
