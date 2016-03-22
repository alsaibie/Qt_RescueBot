#ifndef RRQTSERIAL_H
#define RRQTSERIAL_H

#include <QObject>
#include <QPlainTextEdit>
#include <qextserialport.h>
#include <QPointer>
#include "qextserialenumerator.h"
class RR_QtSerial : public QObject
{
    Q_OBJECT
public:
    //explicit RR_QtSerial(QObject *parent = 0);
    explicit RR_QtSerial(quint32 baudRate, QString comPort,
                     QObject *parent = 0);
    void printReceivedRaw(QPlainTextEdit *textEdit);
    bool isConnected(void);
signals:
    void gotConnected(bool);
    void gotBytes(QByteArray *bytes);


public slots:
    void receiveBytes(void);
    void sendBytes(QByteArray *bytes);
    void switchSerial(bool);
private:
    void connectSerial(void);
    void disconnectSerial(void);


    QextSerialPort *port;
    int baudRates[8];
    int baudNdx;
    quint32 baud;
    bool hwFlow;
    bool openAtStart;
    QString com;
    QString device;
    QByteArray *sentBytes, *receivedBytes;

    QPlainTextEdit *textEdit;
    //TODO: malloc assignment but then every time it's received
};

#endif // RRQTSERIAL_H
