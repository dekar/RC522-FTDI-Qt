#include "qtrc522.h"
#include <QDebug>
#include <QByteArray>

QtRC522::QtRC522(QObject *parent) :
    QObject(parent)
{
    this->rfid = new MFRC522(5,7);
    this->rfid->PCD_Init();
    this->loopTimer = new QTimer(this);
    connect(this->loopTimer,SIGNAL(timeout()),SLOT(timeout()));
    loopTimer->setInterval(1000);
/*    this->rfid->PCD_PerformSelfTest();
    this->rfid->PCD_Reset();
    this->rfid->PCD_Init();*/
    loopTimer->start();
}

void QtRC522::timeout()
{
    if(this->rfid->PICC_IsNewCardPresent())
    {
        qDebug() << "New";
        this->rfid->PICC_ReadCardSerial();
        qDebug() <<"UID:"<< QByteArray((char*)this->rfid->uid.uidByte, this->rfid->uid.size).toHex();

        uint8_t status = this->rfid->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
            Serial.print("PCD_Authenticate() failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }


        uint8_t buf[256];
        uint8_t size = 16;
        for(int i=0;i<16;i++)
        {
            for (int j=0;j<64; j+= size)
            {
                this->rfid->MIFARE_Read(i*size +j,buf,&size);
                qDebug() <<"Blok "<< i<<" offset:"<< j <<QByteArray((char*)buf, size).toHex();
            }
        }
    }
}
