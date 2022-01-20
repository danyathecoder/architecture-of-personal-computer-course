#include <dos.h>
#include <ctype.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

int msCounter = 0;

void interrupt far (*oldInt70h)(void);
void interrupt far NewInt70Handler(void);

void LockClockUpdate();
void UnlockClockUpdate();
int BCDToInteger(int bcd);
unsigned char IntToBCD(int value);
void GetTime();
void SetTime();
void ShowValue(unsigned char regNum);
void CustomDelay();
void WaitClockIsFree();
void WaitForMSCounter(unsigned long delayPeriod);
void AllowClockInterrupts();
void EndOfInterruptForMasterInterruptController();
void EndOfInterruptForSlaveInterruptController();
void EnablePeriodicInterrupt();

// 00h: RTC — текущая секунда (00 – 59h или 00 – 3Bh) — формат
// выбирается регистром 0Bh, по умолчанию — BCD
// 01h: RTC — секунды будильника (00 – 59h или 00 – 3Bh или FFh (любая
// секунда))
// 02h: RTC — текущая минута (00 – 59h или 00 – 3Bh)
// 03h: RTC — минуты будильника (00 – 59h или 00 – 3Bh или FFh)
// 04h: RTC — текущий час:
// 0 – 23h/00 – 17h (24-часовой режим)
// 1 – 12h/01 – 1Ch (12-часовой режим до полудня)
// 81h – 92h/81 – 8Ch (12-часовой режим после полудня)
// 05h: RTC — часы будильника (то же или FFh, если любой час)
// 06h: RTC — текущий день недели (1 – 7, 1 — воскресенье)
// 07h: RTC — текущий день месяца (01 – 31h/01h – 1Fh)
// 08h: RTC — текущий месяц (01 – 12h/01 – 0Ch)
// 09h: RTC — текущий год (00 – 99h/00 – 63h)
// 0Ah: RTC — регистр состояния А
// бит 7: 1 — часы заняты (происходит обновление)
// биты 4 – 6: делитель фазы (010 — 32 768 KHz — по
// умолчанию)
// биты 3 – 0: выбор частоты периодического прерывания:
// 0000 — выключено
// 0011 — 122 микросекунды (минимум)
// 1111 — 500 миллисекунд
// 0110 — 976,562 микросекунды (1024 Hz)
// 0Bh: RTC — регистр состояния В
// бит 7: запрещено обновление часов (устанавливают перед
// записью новых значений в регистры даты и часов)
// бит 6: вызов периодического прерывания (IRQ8)
// бит 5: вызов прерывания при срабатывании будильника
// бит 4: вызов прерывания по окончании обновления времени
// бит 3: включена генерация прямоугольных импульсов
// бит 2: 1/0 — формат даты и времени двоичный/BCD
// бит 1: 1/0 — 24-часовой/12-часовой режим
// бит 0: автоматический переход на летнее время в апреле и
// октябре
// 0Ch только для чтения: RTC — регистр состояния С
// бит 7: произошло прерывание
// бит 6: разрешено периодическое прерывание
// бит 5: разрешено прерывание от будильника
// бит 4: разрешено прерывание по окончании обновления часов
// 0Dh только для чтения: регистр состояния D
// бит 7: питание RTC/CMOS есть
// 0Eh: результат работы POST при последнем старте компьютера:
// бит 7: RTC сбросились из-за отсутствия питания CMOS
// бит 6: неверная контрольная сумма CMOS-конфигурации
// бит 5: неверная конфигурация
// бит 4: размер памяти не совпадает с записанным в конфигурации
// бит 3: ошибка инициализации первого жесткого диска
// бит 2: RTC-время установлено неверно (например, 30 февраля)
// 0Fh: состояние, в котором находился компьютер перед последней
// перезагрузкой
// 00h — Ctr-Alt-Del
// 05h — INT 19h
// 0Ah, 0Bh, 0Ch — jmp, iret, retf на адрес, хранящийся в 0040h:0067h
// Другие значения указывают, что перезагрузка произошла в ходе POST
// или в других необычных условиях
// 10h: тип дисководов (биты 7 – 4 и 3 – 0 — типы первого и второго
// дисковода)
// 0000: отсутствует
// 0001: 360 Кб
// 0010: 1,2 Мб
// 0011: 720 Кб
// 0100: 1,44 Мб
// 0101: 2,88 Мб
// 12h: тип жестких дисков (биты 7 – 4 и 3 – 0 — типы первого и второго
// жестких дисков, 1111, если номер типа больше 15)
// 14h: байт состояния оборудования
// биты 7 – 6: число установленных жестких дисков минус один
// биты 5 – 4: тип монитора (00, 01, 10, 11 — EGA/VGA, 40x25 CGA, 80x25
// CGA, MDA)
// бит 3: монитор присутствует
// бит 2: клавиатура присутствует
// бит 1: FPU присутствует
// бит 0: дисковод присутствует
// 15h: младший байт размера базовой памяти в килобайтах (80h)
// 16h: старший байт размера базовой памяти в килобайтах (02h)
// 17h: младший байт размера дополнительной памяти (выше 1 Мб) в
// килобайтах
// 18h: старший байт размера дополнительной памяти (выше 1 Мб) в
// килобайтах
// 19h: тип первого жесткого диска, если больше 15
// lAh: тип второго жесткого диска, если больше 15
// 2Eh: старший байт контрольной суммы регистров 10h – 2Dh
// 2Fh: младший байт контрольной суммы регистров 10h – 2Dh
// 30h: младший байт найденной при POST дополнительной памяти в
// килобайтах
// 31h: старший байт найденной при POST дополнительной памяти в
// килобайтах
// 32h: первые две цифры года в BCD-формате(BCD - Binary Coded
// Decimal то есть 1998h для 1998-го года )

void main()
{
	char c, value;
	clrscr();
	printf("Press:\n'1' - Show time\n'2' - Set time\n'3' - Delay time\n'Esc' - quit\n\n");
	
	while(c != 27)
	{
		c = getch();
		switch(c)
		{
			case '1': GetTime();break;
			case '2': SetTime();break;
			case '3': CustomDelay();break;
			case 27: break;
		}
	}
}

void WaitClockIsFree() 
{
	do
	{
		outp(0x70, 0x0A);
	} while( inp(0x71) & 0x80 ); // Check for 1 in 7th bit
}

void GetTime()
{
	unsigned char value;

// 0
// счетчик секунд
// 1
// регистр секунд будильника
// 2
// счетчик минут
// 3
// регистр минут будильника
// 4
// счетчик часов
// 5
// регистр часов будильника
// 6
// счетчик дней недели (1 - воскресенье)
// 7
// счетчик дней месяца
// 8
// счетчик месяцев
// 9
// счетчик лет (последние две цифры текущего года)
	
	WaitClockIsFree();
	outp(0x70, 0x04); // Hours
	value = inp(0x71); 
	printf("%d:",BCDToInteger(value)); 
	
	WaitClockIsFree();
	outp(0x70, 0x02); // Minutes
	value = inp(0x71); 
	printf("%d:",BCDToInteger(value)); 
	
	WaitClockIsFree();
	outp(0x70, 0x00); // Seconds
	value = inp(0x71); 
	printf("%d   ",BCDToInteger(value)); 
	
	WaitClockIsFree();
	outp(0x70, 0x07); // Day of month
	value = inp(0x71); 
	printf("%d.",BCDToInteger(value)); 
	
	WaitClockIsFree();
	outp(0x70, 0x08); // Month
	value = inp(0x71); 
	printf("%d.",BCDToInteger(value)); 
	
	WaitClockIsFree();
	outp(0x70, 0x09); // Year
	value = inp(0x71); 
	printf("%d   ",BCDToInteger(value)); 
	
	WaitClockIsFree();
	outp(0x70, 0x06); // Day of week
	value = inp(0x71);
	
	switch(BCDToInteger(value))
	{
		case 1: printf("Sunday\n"); break;
		case 2: printf("Monday\n"); break;
		case 3: printf("Tuesday\n"); break;
		case 4: printf("Wednesday\n"); break;
		case 5: printf("Thursday\n"); break;
		case 6: printf("Friday\n"); break;
		case 7: printf("Saturday\n"); break;		
		default: printf("Undefined day of week\n"); break;
	}
}

void SetTime()
{
	int hours, minutes, seconds, weekDay, monthDay, month, year;	
	
	printf("Enter hours: "); 
	scanf("%d", &hours);
	printf("Enter minutes: "); 
	scanf("%d", &minutes);
	printf("Enter seconds: "); 
	scanf("%d", &seconds);
	printf("Enter week day number: "); 
	scanf("%d", &weekDay);
	printf("Enter day of month: "); 
	scanf("%d", &monthDay);
	printf("Enter mounth: "); 
	scanf("%d", &month);
	printf("Enter year: "); 
	scanf("%d", &year);
	
	LockClockUpdate();
	
	outp(0x70, 0x04);
	outp(0x71, IntToBCD(hours));	
	
	outp(0x70, 0x02);
	outp(0x71, IntToBCD(minutes));	
	
	outp(0x70, 0x00);
	outp(0x71, IntToBCD(seconds));	
	
	outp(0x70, 0x06);
	outp(0x71, IntToBCD(weekDay));	
	
	outp(0x70, 0x07);
	outp(0x71, IntToBCD(monthDay));	
	
	outp(0x70, 0x08);
	outp(0x71, IntToBCD(month));	
	
	outp(0x70, 0x09);
	outp(0x71, IntToBCD(year));

	UnlockClockUpdate();
}


void LockClockUpdate()
{
	unsigned char value;
	WaitClockIsFree();

	outp(0x70,0x0B);
	value = inp(0x71);
	value|=0x80;       // 1 to 7th bit
	outp(0x70, 0x0B);
	outp(0x71, value);
}

void UnlockClockUpdate()
{
	unsigned char value;
	WaitClockIsFree(); 
	outp(0x70,0x0B);
	value = inp(0x71); 
	value-=0x80;        // 0 to 7th bit
	outp(0x70, 0x0B);
	outp(0x71, value);
}

void interrupt far NewInt70Handler(void)
{
	msCounter++;

	outp(0x70,0x0C); // Neccessary code for interrupt
	inp(0x71);       // controller initiate interrupt again
	
	EndOfInterruptForMasterInterruptController();
	EndOfInterruptForSlaveInterruptController();	
}

void EndOfInterruptForMasterInterruptController()
{
	outp(0x20,0x20);
}

void EndOfInterruptForSlaveInterruptController()
{
	outp(0xA0,0x20);
}

void CustomDelay()
{
	unsigned long delayPeriod;
	
	disable();    // cli
	oldInt70h = getvect(0x70);
	setvect(0x70, NewInt70Handler);
	enable();     // sti

	printf("Delay in ms: ");
	scanf("%ld", &delayPeriod);

	AllowClockInterrupts();
	
	EnablePeriodicInterrupt();

	WaitForMSCounter(delayPeriod);
	
	printf("\nEnd delay\n");
	
	setvect(0x70, oldInt70h);
	
    UnlockClockUpdate();
}

void EnablePeriodicInterrupt()
{
	unsigned char value;
	
	outp(0x70, 0x0B); 
	value = inp(0x0B);

	outp(0x70, 0x0B);
	outp(0x71, value|0x40); // 1 to 6th bit
}

void WaitForMSCounter(unsigned long delayPeriod)
{
	msCounter = 0;
	while(msCounter != delayPeriod);
}

void AllowClockInterrupts()
{
	unsigned char value;
	value = inp(0xA1);
	outp(0xA1,value & 0xFE);// 0 to 0th bit
}

int BCDToInteger (int bcd)
{
	return bcd % 16 + bcd / 16 * 10;
}

unsigned char IntToBCD (int value)
{
	return (unsigned char)((value/10)<<4)|(value%10);
}