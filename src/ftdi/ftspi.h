#ifndef FTSPI_H
#define FTSPI_H
#include <ftdi.h>
#include <stdint.h>
//#include "ftdispi.h"
#include <stdio.h>

#ifndef __libftdi_h__
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */

#endif // __libftdi_h__

#define TCK_X5          0x8a

#define FT_IO_SET 1
#define FT_IO_RESET 2
#define FT_IO_TOGGLE 3




class FtSpi
{
public:
    struct IoPort
    {
        uint8_t a_SET_BITS_LOW;
        uint8_t lowDirection;
        uint8_t lowState;
        uint8_t a_SET_BITS_HIGHT;
        uint8_t hightDirection;
        uint8_t hightState;
    };
private:
    struct IoPort ioPort;
    struct ftdi_context *ftdi;
    //struct ftdispi_context *ftdispi;
    bool inited;
    bool ok;
    char error[255];
    int vendor;
    int product;
    int prescaler;

    bool x5;
    int interface;

    unsigned char buf[16];
    unsigned char bufCount;

    void addBuf (unsigned char c);
    bool writeBuf (void);
    bool flushPort(void);

public:
    uint8_t options;
    FtSpi();
    ~FtSpi();
    bool setUsbSetting(int vendor,int product,int interface);

    int setFrequency(int frequency);
    int getFrequency();
    bool flushFreq(void);

//IO block
    bool setIOPort(IoPort port);
    bool setIOBits(uint8_t lowState,uint8_t hightState);
    bool setIODirection(uint8_t lowDirection,uint8_t hightDirection);
    bool setIOLowBits(uint8_t lowState);
    bool setIOHightBits(uint8_t hightState);
    bool setIOLowDirection(uint8_t lowDirection);
    bool setIOHightDirection(uint8_t hightDirection);
    bool setIOLowBit(uint8_t bitNum,uint8_t state);
    bool setIOHightBit(uint8_t bitNum,uint8_t state);
    uint8_t readIOHightBits();
    uint8_t readIOLowBits();
    uint8_t getIOLowDirection();
    uint8_t getIOHightDirection();
    uint8_t getIOLowState();
    uint8_t getIOHightState();

    FtSpi::IoPort getIOBits(void);
/*    bool setPort(void);
    bool getPort(void);*/


    bool usbInit();
    bool isInited();
    bool reInit();
    bool deInit();

    bool setWriteOptions(bool ckpol,bool cpha,bool write, bool read);
    unsigned char getWriteOptions(void);


    int readWrite(unsigned char *buf,uint16_t size);

    bool isOK(void);
    /*bool ft2232_spi_send_command(unsigned int writecnt, unsigned int readcnt,
                const unsigned char *writearr, unsigned char *readarr);*/

    char* getError(void);
private:
    void setError(char* buf);

};

#endif // FTSPI_H
