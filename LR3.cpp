#include<conio.h>
#include "hexioctrl.h"
 
#include <iostream>
 
const int IDENTIFY_DEVICE = 0xEC;
const int IDENTIFY_PACKET_DEVICE = 0xA1;
 
const int dataRegister[2] = {0x1F0, 0x170};
const int DH_register[2] = {0x1F6, 0x176};
const int StateCommandRegister[2] = {0x1F7, 0x177};
const int altStateRegister[2] = {0x3F6, 0x376};
 
unsigned short data[256];
 
void WaitDeviceBusy(int channelNum);
bool getDeviceInfo(int devNum, int channelNum);
void showTable();
bool waitReady(int channelNum);
 
 
int main()
{
    ALLOW_IO_OPERATIONS;
    setlocale(LC_ALL, ".1251");
    for( int channel = 0; channel <= 1; channel++ )
        for( int device = 0; device <= 1; device++ )
        {               
            if(getDeviceInfo(device, channel))  
            {
                printf("Канал %d     Устройство %d\n", channel, device);
                showTable();
            }
        }
        system("pause");
        return 0;
}
 
 
bool waitReady(int channelNum)
{
    for (int i = 0; i < 1000; i++)
    {
        unsigned char state = _inp(altStateRegister[channelNum]);//регистр состояния
        if(state & (1 << 6)) return true; 
    }
    return false;
}
 
 
void  WaitDeviceBusy(int channelNum)
{
    unsigned char state;
    do state = _inp(altStateRegister[channelNum]); //регистр состояния
    while (state & (1 << 7));
}
 
bool  getDeviceInfo(int devNum, int channelNum)
{   
    const int commands[2] = {IDENTIFY_PACKET_DEVICE, IDENTIFY_DEVICE};
    for (int i = 0; i < 2; i++)
    {
        // Ожидаем обнуления бита BSY
        WaitDeviceBusy(channelNum);
 
        // Адресуем устройство
        unsigned char regData = (devNum << 4) + (7 << 5); //111X0000
        _outp(DH_register[channelNum], regData); // номер устройства в DH
 
        // Дожидаемся признака готовности, если устройство присутствует
        if(!waitReady(channelNum))  return false;       
 
        // Записываем код команды в регистр команд
        _outp(StateCommandRegister[channelNum], commands[i]); // в командный регистр
 
        WaitDeviceBusy(channelNum);
        
        // Проверка на ошибку (неподдерживаемая команда)
        unsigned char curState = _inp( StateCommandRegister[channelNum]);
        if(!(curState & (1 << 3)))  // Если DRQ = 0
        {
            if(i == 1) return false;
            continue;
        }
        else break;
    }
 
    // Получение конфигурационного блока
    for( int i = 0; i < 256; i++ )
        data[ i ] = _inpw( dataRegister[channelNum ] ); // из регистра данных
    
    return true;
}
 
void showTable()
{
    printf("\nНаименование модели устройства: ");
    for(int i = 27; i <= 46; i++)
        printf("%c%c", data[i] >> 8, data[i] & 0x00FF );
 
    printf("\nСерийный номер: ");
    for( int i = 10; i <= 19; i++ )
        printf("%c%c", data[i] >> 8, data[i] & 0x00FF );
 
    printf("\nВерсия прошивки: ");
    for( int i = 23; i <= 26; i++ )
        printf("%c%c", data[i] >> 8, data[i] & 0x00FF );
 
    printf("\nТип обмена данными: ");
    // 15-й бит 0-го слова
    if(data[0] & (1 << 15)) printf("ATAPI\n");
    else printf("ATA\n");
 
    // Если ATA
    if(!(data[0] & (1 << 15))){
        printf("Размер: %.0lf байт\n", (long double)(((unsigned long *)data)[30]) * 512); 
    }
 
    printf("\n\nPIO:\n");
    printf(" Поддерживаются:\n" );
    printf("  [%s%s", (data[64] & 1) ? "+" : "-" ,"] PIO 3\n" );
    printf("  [%s%s", (data[64] & 2) ? "+" : "-" ,"] PIO 4\n" );
 
    printf("\nMultiword DMA:\n");
    printf(" Поддерживаются:\n" );
    printf("  [%s%s", (data[63] & 1) ? "+" : "-" ,"] MWDMA 0\n" );
    printf("  [%s%s", (data[63] & 2) ? "+" : "-" ,"] MWDMA 1\n" );
    printf("  [%s%s", (data[63] & 4) ? "+" : "-" ,"] MWDMA 2\n" );
 
    printf("\nUltra DMA:\n" );
    printf(" Поддерживаются:\n" );
    printf("  [%s%s", (data[88] & 1) ? "+" : "-" ,"] UDMA Mode 0\n" );
    printf("  [%s%s", (data[88] & (1 << 1)) ? "+" : "-" ,"] UDMA Mode 1\n" );
    printf("  [%s%s", (data[88] & (1 << 2)) ? "+" : "-" ,"] UDMA Mode 2\n" );
    printf("  [%s%s", (data[88] & (1 << 3)) ? "+" : "-" ,"] UDMA Mode 3\n" );
    printf("  [%s%s", (data[88] & (1 << 4)) ? "+" : "-" ,"] UDMA Mode 4\n" );
    printf("  [%s%s", (data[88] & (1 << 5)) ? "+" : "-" ,"] UDMA Mode 5\n" );
 
    printf("\nВерсии ATA:\n" );
    printf("  [%s%s", (data[80] & (1 << 1)) ? "+" : "-" ,"] ATA 1\n" );
    printf("  [%s%s", (data[80] & (1 << 2)) ? "+" : "-" ,"] ATA 2\n" );
    printf("  [%s%s", (data[80] & (1 << 3)) ? "+" : "-" ,"] ATA 3\n" );
    printf("  [%s%s", (data[80] & (1 << 4)) ? "+" : "-" ,"] ATA 4\n" );
    printf("  [%s%s", (data[80] & (1 << 5)) ? "+" : "-" ,"] ATA 5\n" );
    printf("  [%s%s", (data[80] & (1 << 6)) ? "+" : "-" ,"] ATA 6\n" );
    printf("  [%s%s", (data[80] & (1 << 7)) ? "+" : "-" ,"] ATA 7\n" );
    
    if (data[82] != 0x0000 && data[82] != 0xFFFF)
    {
        printf("\nПоддерживаемый набор комманд:\n");
        printf("  [%s%s", (data[82] & (1 << 0)) ? "+" : "-" ,"] команды SMART\n" );
        printf("  [%s%s", (data[82] & (1 << 1)) ? "+" : "-" ,"] шифрование и ограничение доступа\n" );
        printf("  [%s%s", (data[82] & (1 << 2)) ? "+" : "-" ,"] съёмный носитель\n" );
        printf("  [%s%s", (data[82] & (1 << 3)) ? "+" : "-" ,"] управление питанием\n" );
        printf("  [%s%s", (data[82] & (1 << 4)) ? "+" : "-" ,"] команды ATAPI\n" );
        printf("  [%s%s", (data[82] & (1 << 5)) ? "+" : "-" ,"] функция отложенной записи (Write Cache)\n" );
        printf("  [%s%s", (data[82] & (1 << 6)) ? "+" : "-" ,"] функция предвыборки (Look-Ahead)\n" );
        printf("  [%s%s", (data[82] & (1 << 7)) ? "+" : "-" ,"] функция Release Interrupt\n" );
        printf("  [%s%s", (data[82] & (1 << 8)) ? "+" : "-" ,"] функция Service Interrupt\n" );
        printf("  [%s%s", (data[82] & (1 << 9)) ? "+" : "-" ,"] функция Device Reset\n" );
        printf("  [%s%s", (data[82] & (1 << 10)) ? "+" : "-" ,"] функция Host Protected Area \n" );
        printf("  [%s%s", (data[82] & (1 << 12)) ? "+" : "-" ,"] функция Write Buffer\n" );
        printf("  [%s%s", (data[82] & (1 << 13)) ? "+" : "-" ,"] функция Read Buffer\n" );
        printf("  [%s%s", (data[82] & (1 << 14)) ? "+" : "-" ,"] функция NOP\n" );
    }
    
    printf("_____________________________________________________________\n\n\n");
}
