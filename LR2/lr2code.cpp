#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/io.h>
#include <errno.h>
//#include "./ids_parser.c"
#define PCI_MAX_BUS 255
#define PCI_MAX_SLOT 31
#define PCI_MAX_FUNC 7

#define CONFIG_ADDR 0xcf8
#define CONFIG_DATA 0xcfc

char class_code[15][100] =
    {
        "Device was built before Class Code definitions were finalized",
        "Mass storage controller",
        "Network controller",
        "Display controller",
        "Multimedia device",
        "Memory controller",
        "Bridge device",
        "Simple communication controllers",
        "Base system peripherals",
        "Input devices",
        "Docking stations",
        "Processors",
        "Serial bus controllers",
        "Wireless controller",
        "Intelligent I/O controllers",
        "Satellite communication controllers",
        "Encryption/Decryption controllers",
        "Data acquisition and signal processing controllers",
        "Processing accelerators",
        "Non-Essential Instrumentation",
        "FEh Reserved",
        "Device does not fit in any defined classes"};

int getDevName(u_int16_t code, char *tmp)
{
    int i = 0;
    memset(tmp, 0, 255);
    if (code == 0xFF)
    {
        return 0;
    }
    else if (code >= 14 && code <= 0xFE)
    {
        printf(" 14h - FEh are reserved\n");
    }
    else
    {
        printf(" %s\n", class_code[code]);
        return 0;
    }
    return 1;
}

u_int32_t readDevice(u_int8_t bus, u_int8_t slot, u_int8_t func)
{
    u_int32_t address;
    u_int32_t lbus = (u_int32_t)bus;
    u_int32_t lslot = (u_int32_t)slot;
    u_int32_t lfunc = (u_int32_t)func;
    u_int32_t data = 0;

    address = (u_int32_t)((lbus << 16) | (lslot << 11) |
                          (lfunc << 8) | ((u_int32_t)0x80000000));

    outl(address, CONFIG_ADDR);
    data = inl(CONFIG_DATA);
    if (((data & 0xFFFF) != 0xFFFF) && (data != 0))
    {
        printf("%04d|\t%02d |\t", bus, slot);
        printf("%04x  |  %04x    |", (data & 0xFFFF), (data & 0xFFFF0000) >> 16);
    }
}

u_int8_t readClassCode(u_int8_t bus, u_int8_t slot)
{

    u_int32_t address;
    u_int32_t lbus = (u_int32_t)bus;
    u_int32_t lslot = (u_int32_t)slot;
    u_int32_t lfunc = 0;
    u_int32_t offset = 8;
    u_int32_t code = 0;

    address = (u_int32_t)((lbus << 16) | (lslot << 11) |
                          (lfunc << 8) | offset & 0xFC | ((u_int32_t)0x80000000));
    outl(address, CONFIG_ADDR);
    code = inl(CONFIG_DATA);
    if (((code & 0xFFFF) != 0xFFFF) && (code != 0))
    {
        return (code & 0xFF000000) >> 24;
    }
}

int main()
{
    u_int16_t bus;
    u_int16_t slot;
    u_int16_t func;

    char *tmp = malloc(255);

    if (iopl(3) < 0)
    {
        printf("iopl set error\n%d", errno);
        return -1;
    }
    printf("bus | slot | vendorID | deviceID | class\n");

    for (bus = 0; bus <= PCI_MAX_BUS; bus++)
    {
        for (slot = 0; slot <= PCI_MAX_SLOT; slot++)
        {
            readDevice((u_int8_t)bus, (u_int8_t)slot, 0);
            u_int16_t code = 0;
            getDevName(readClassCode((u_int8_t)bus, (u_int8_t)slot), tmp);
        }
    }

    if (iopl(0) < 0)
    {
        printf("iopl set error\n");
        return -1;
    }
    free(tmp);
    return 0;
}
