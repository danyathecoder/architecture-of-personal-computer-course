#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>

void initFrequencyDivider (int frequency) {
	short value;
	value = 1193180 / frequency;  // Вычисляем значение для регистра счетчика таймера			
    outp(0x42, (char)value); // Загружаем сначала младший,	
    outp(0x42, (char)(value >> 8)); // затем старший байты
}

void readStatusWords() {
	unsigned char temp;
	char *str;
	str=(char*)calloc(9, sizeof(char));

	outp(0x43, 0xe2); // 0 канал	
	temp = inp(0x40);	
	itoa(temp, str, 2);
    printf("0 channel status: %s\n", str);

	
	outp(0x43, 0xe4); // 1 канал

	temp=inp(0x41); 
	itoa(temp, str, 2);
	printf("1 channel status: %s\n", str);
	
	outp(0x43, 0xe8); // 2 канал
	temp = inp(0x42);
	itoa(temp, str, 2);
    printf("2 channel status: %s\n", str);
	
	free(str);
}

int main() {
	char port61;
	int i;
    int melody[9] = {659, 622, 659, 622, 659, 493, 622, 523, 440}; //fur elise
//     int melody[25] = { 
// 4186,4699,4186,5588,5588,4186,4186,4699,4186,7040,6645,4186,4186,7902,7040,5588,5274,4699,
// 7902,7902,7040,5588,6272,5920}; //happy birthday

	// settings 10110110b, channal 2, operation 4, mode 3 , format 0
	outp(0x43, 0xB6); 
	
    for(i = 0; i < 9; i++){
        			 
	initFrequencyDivider(melody[i]); // set sound frequence

	// turns on speaker and channels
	port61 = inp(0x61);		 
	port61 = port61 | 3;  
	outp(0x61, port61);		

	readStatusWords();


	delay(200); // задержка 
	
	// turn off system dinamic
	port61 = port61 & 0xFFFC; 
	outp(0x61, port61);

    }
	
	return 0;
}

