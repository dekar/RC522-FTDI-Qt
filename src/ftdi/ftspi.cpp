#include "ftspi.h"
#include <ftdi.h>
#include <QDebug>

/*
  Class constructor.
*/

#define FTDI_CHECK(FUN, MSG) do {                  \
        if ((FUN) < 0)                                  \
        {                                               \
            snprintf(this->error,250,                   \
                    "%s: %s\n",                         \
                    MSG,                                \
                    ftdi_get_error_string(this->ftdi)); \
            this->ok = false;                           \
            return false;                               \
        }                                               \
    } while (0)


FtSpi::FtSpi()
{

    this->bufCount = 0;

    this->ftdi = new struct ftdi_context;
    ftdi_init(this->ftdi);

    this->ok = true;
    this->inited = false;

    this->vendor = 0;
    this->product = 0;
    this->interface = 0;

    this->options = 0;

    this->ioPort.a_SET_BITS_HIGHT = SET_BITS_HIGH;
    this->ioPort.hightDirection = 0;
    this->ioPort.hightState = 0;
    this->ioPort.a_SET_BITS_HIGHT = SET_BITS_LOW;
    this->ioPort.lowDirection = 0;
    this->ioPort.lowState = 0;

    this->x5 = false;
}
/*
  Class destructor.
*/
FtSpi::~FtSpi()
{
    if (this->inited)
        this->deInit();
}

bool FtSpi::setUsbSetting(int vendor,int product,int interface)
{
    if (this->inited)
        this->deInit();
    this->vendor = vendor;
    this->product = product;
    this->interface = interface;
    return true;
}

/*-----------------------------------------------------------
 *-----------------------------INIT / DEINIT-----------------
 *-----------------------------------------------------------
 */
/*
    Initing FT, MPSSE and SPI
*/
bool FtSpi::usbInit()
{
    if (!this->inited)
    {
        uint8_t buf[10];

//Настраиваем USB
        FTDI_CHECK(ftdi_set_interface(this->ftdi,(enum ftdi_interface)this->interface),"Selecting interface");
        FTDI_CHECK(ftdi_usb_open(this->ftdi, this->vendor, this->product),"Open USB");
        FTDI_CHECK(ftdi_usb_reset(this->ftdi), "RESET");
//Настраиваем драйвер ftdi
        FTDI_CHECK(ftdi_write_data_set_chunksize(this->ftdi, 4096), "SET CHUNK 4096");
        FTDI_CHECK(ftdi_read_data_set_chunksize(this->ftdi, 4096), "SET CHUNK 4096");
        FTDI_CHECK(ftdi_set_latency_timer(this->ftdi, 1), "SET LAT 1ms");
        FTDI_CHECK(ftdi_setflowctrl(this->ftdi, SIO_RTS_CTS_HS), "RTS/CTS");
//Настраиваем MPSSE
        FTDI_CHECK(ftdi_set_bitmode(this->ftdi, 0x00, BITMODE_RESET), "Reset FTDI");
        FTDI_CHECK(ftdi_set_bitmode(this->ftdi, 0x00, BITMODE_MPSSE), "SET SPI MODE");
        FTDI_CHECK(ftdi_usb_purge_buffers(this->ftdi), "PURGE");
            buf[0] = LOOPBACK_END;
        FTDI_CHECK(ftdi_write_data(this->ftdi, buf, 1), "SET NO LOOP");
//Устанавливаем GPIO порты
        FTDI_CHECK(this->flushPort(),"Can't flush GPIO ports on setup");

//Настраиваем SPI
        /*        ftdispi_setmode(this->ftdispi, 1, 0, 0, 0, 0, 0);
                ftdispi_setclock(this->ftdispi, this->getFrequency());
                */


//#error "spiInit() Not ready"
        this->inited = true;
        return true;
    }
    return false;

}
/*
*   Returned is SPI inited.
*/
bool FtSpi::isInited()
{
    return this->inited;
}
/*
    Deinit and free FT
*/
bool FtSpi::deInit()
{
    if(this->inited)
    {
        qDebug()<< "Turn FT off!";
        FTDI_CHECK(ftdi_set_bitmode(this->ftdi, 0xff, BITMODE_RESET), "Reset FTDI mode");
        FTDI_CHECK(ftdi_usb_reset(this->ftdi),"Reset FTDI");
        FTDI_CHECK(ftdi_usb_close(this->ftdi),"Close USB");

        qDebug()<< "Turn FT off OK!";
    }
    this->inited = false;
    return true;
}
/*------------------------------------------------------------------
 *-------------------Frequency--------------------------------------
 *------------------------------------------------------------------
 */

int FtSpi::setFrequency(int frequency)
{
    if (frequency > 6000000)
    {
        this->x5 = true;
        this->prescaler = (60000000/(2*frequency)) - 1;
    }
    else
    {
        this->prescaler = (12000000/(2*frequency)) - 1;
    }
    if (this->flushFreq())
        return this->getFrequency();
    return -1;
}
int FtSpi::getFrequency()
{
    if (this->x5)
        return (60000000 / (( 1 + this->prescaler ) * 2));
    else
        return (12000000 / (( 1 + this->prescaler ) * 2));
}


bool FtSpi::flushFreq()
{
    if (this->inited)
    {
        uint8_t  buf[3] = { 0, 0, 0 };
        if (0)// Если надо увеличить частоту.
        {
            buf[0] = TCK_X5;
            FTDI_CHECK(ftdi_write_data(this->ftdi, buf, 1), "SET CLK X5");
        }

        buf[0] = TCK_DIVISOR;
        buf[1] = (this->prescaler >> 0) & 0xFF;
        buf[2] = (this->prescaler >> 8) & 0xFF;
        FTDI_CHECK(ftdi_write_data(this->ftdi, buf, 3), "SET CLK DIV");
        return true;
    }
    return true;
}


/*------------------------------------------------------------------
 *-------------------Serial Commands--------------------------------
 *------------------------------------------------------------------
 */
int FtSpi::readWrite(unsigned char *buf,uint16_t size)
{
    uint8_t mbuf[size+3];
    mbuf[0] = this->options;
    mbuf[1] = (size-1)&0xff;
    mbuf[2] = (size-1) >> 8;
    memcpy(&mbuf[3],buf,size);
    uint16_t msize = size;
    //qDebug() << "Command : " << QByteArray((char*)mbuf,size+3).toHex();
    FTDI_CHECK(ftdi_write_data(this->ftdi,mbuf,size+3),"Write data to SPI");
    if(this->options& 1 <<5)
    {
/*        while(size >0)
        {
            size -= ftdi_read_data(this->ftdi,&buf[msize-size],size);
        }*/
        ftdi_read_data(this->ftdi,buf,size);
    }
    //qDebug() <<"Recive : "<< QByteArray((char*)buf,msize).toHex();
/*#define POROG 8000
    for (int i = 0;i< size;i+=POROG)
    {
        int newSize = (size - i)>POROG ? POROG : size - i;
        ftdispi_write_read(this->ftdispi, (buf+i) , newSize, (buf+i), newSize,0);
    }
    this->ok = true;
    return size;*/
    return -1;
}

bool FtSpi::setWriteOptions(bool ckpol, bool cpha, bool write, bool read)
{
/*    if (write || read)
    {
        if (ckpol)
        this->options = opt;
        return true;
    }*/
    this->setError("No write/read options set");
    return false;
}

unsigned char FtSpi::getWriteOptions(void)
{
    return this->options;
}

/*------------------------------------------------------------------
 *-------------------IO state---------------------------------------
 *------------------------------------------------------------------
 */
bool FtSpi::flushPort(void)
{
    if (this->inited)
    {
        uint8_t buf[6];
        uint8_t bufCount = 0;
            buf[bufCount++] = SET_BITS_HIGH;
            buf[bufCount++] = this->ioPort.hightState;
            buf[bufCount++] = this->ioPort.hightDirection;
            buf[bufCount++] = SET_BITS_LOW;
            buf[bufCount++] = (this->ioPort.lowState);
            buf[bufCount++] = (this->ioPort.lowDirection);
        FTDI_CHECK(ftdi_write_data(this->ftdi, buf, bufCount), "Set GPIO ports");
        return true;
    }
    return true;
}
uint8_t FtSpi::readIOHightBits()
{
    if (this->inited)
    {
        uint8_t buf[6];
        uint8_t bufCount = 0;
            buf[bufCount++] = GET_BITS_HIGH;
        FTDI_CHECK(ftdi_write_data(this->ftdi, buf, bufCount), "Read GPIO hight port");
        ftdi_read_data(this->ftdi,buf,1);
        return buf[0];
    }
    return this->ioPort.hightState;
}

uint8_t FtSpi::readIOLowBits()
{
    if (this->inited)
    {
        uint8_t buf[6];
        uint8_t bufCount = 0;
            buf[bufCount++] = GET_BITS_LOW;
        FTDI_CHECK(ftdi_write_data(this->ftdi, buf, bufCount), "Read GPIO low port");
        ftdi_read_data(this->ftdi,buf,1);
        return buf[0];
    }
    return this->ioPort.lowState;
}

bool FtSpi::setIOPort(IoPort port)
{
    this->ioPort = port;
    return flushPort();
}

bool FtSpi::setIOBits(uint8_t lowState,uint8_t hightState)
{
    this->ioPort.lowState = lowState;
    this->ioPort.hightState = hightState;
    return flushPort();
}

bool FtSpi::setIODirection(uint8_t lowDirection,uint8_t hightDirection)
{
    this->ioPort.lowDirection = lowDirection;
    this->ioPort.hightDirection = hightDirection;
    return flushPort();
}

bool FtSpi::setIOLowBits(uint8_t lowState)
{
    this->ioPort.lowState = lowState;
    return flushPort();
}

bool FtSpi::setIOHightBits(uint8_t hightState)
{
    this->ioPort.hightState = hightState;
    return flushPort();
}

bool FtSpi::setIOLowDirection(uint8_t lowDirection)
{
    this->ioPort.lowDirection = lowDirection;
    return flushPort();
}
bool FtSpi::setIOHightDirection(uint8_t hightDirection)
{
    this->ioPort.hightDirection = hightDirection;
    return flushPort();
}

bool FtSpi::setIOLowBit(uint8_t bitNum,uint8_t state)
{
    switch (state)
    {
    case FT_IO_SET:
        this->ioPort.lowState |= (1 << bitNum);
        break;
    case FT_IO_RESET:
        this->ioPort.lowState &= ~(1 << bitNum);
        break;
    case FT_IO_TOGGLE:
        this->ioPort.lowState ^= (1 << bitNum);
        break;
    default:
        return false;
    }
    return flushPort();
}
bool FtSpi::setIOHightBit(uint8_t bitNum,uint8_t state)
{
    switch (state)
    {
    case FT_IO_SET:
        this->ioPort.hightState |= (1 << bitNum);
        break;
    case FT_IO_RESET:
        this->ioPort.hightState &= ~(1 << bitNum);
        break;
    case FT_IO_TOGGLE:
        this->ioPort.hightState ^= (1 << bitNum);
        break;
    default:
        return false;
    }
    return flushPort();
}


FtSpi::IoPort FtSpi::getIOBits(void)
{
    return this->ioPort;
}

uint8_t FtSpi::getIOLowDirection()
{
    return this->ioPort.lowDirection;
}
uint8_t FtSpi::getIOHightDirection()
{
    return this->ioPort.hightDirection;
}

uint8_t FtSpi::getIOLowState()
{
    return this->ioPort.lowState;
}

uint8_t FtSpi::getIOHightState()
{
    return this->ioPort.hightState;
}



/*
-------------------------------------------------------------
-----------------------Error functions-----------------------
-------------------------------------------------------------
*/
char* FtSpi::getError()
{
    return this->error;
}

bool FtSpi::isOK()
{
    return this->ok;
}

void FtSpi::setError(char *buf)
{
    this->ok = false;
    int i = 0;
    while (buf[i]>0)
    {
        this->error[i] = buf[i];
        i++;
    }
}

/*
 *----------------------------------------------------
 *----------------------FTDI Buff functions-----------
 *----------------------------------------------------
 */

void FtSpi::addBuf (unsigned char c)
{
    this->buf[this->bufCount] = c;
    this->bufCount++;
}

bool FtSpi::writeBuf (void)
{
    if (ftdi_write_data(this->ftdi, this->buf, this->bufCount)!= this->bufCount)
    {
        this->bufCount = 0;
        return false;
    }
    this->bufCount = 0;
    return true;
}
