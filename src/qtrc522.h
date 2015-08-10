#ifndef QTRC522_H
#define QTRC522_H

#include <QObject>
#include <QTimer>
#include "MFRC522.h"

class QtRC522 : public QObject
{
    Q_OBJECT
private:
    MFRC522 * rfid;
    QTimer *loopTimer;
public:
    explicit QtRC522(QObject *parent = 0);
    
signals:
    
public slots:
    void timeout();
};

#endif // QTRC522_H
