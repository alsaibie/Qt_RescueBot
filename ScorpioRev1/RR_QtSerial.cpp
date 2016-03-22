#include "RR_QtSerial.h"
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
RR_QtSerial::RR_QtSerial(quint32 baudRate, QString comPort,
                         QObject *parent) :
    QObject(parent), port(0), receivedBytes(NULL), sentBytes(NULL)
{
    baud=baudRate;
    com=comPort;
    hwFlow = false;
    baudRates[0] = BAUD300;
    baudRates[1] = BAUD2400;
    baudRates[2] = BAUD4800;
    baudRates[3] = BAUD9600;
    baudRates[4] = BAUD19200;
    baudRates[5] = BAUD38400;
    baudRates[6] = BAUD57600;
    baudRates[7] = BAUD115200;

 //Find Serial Ports
/*
#ifdef _TTY_POSIX_
    QDir dir("/dev");
    QStringList filterList("ttyS*");
    filterList += QStringList("ttyUSB*");
    filterList += QStringList("ttyACM*");
    dir.setNameFilters(filterList);
    QStringList devices = dir.entryList(QDir::System);
#elif defined(_TTY_WIN_)
    QList< QextPortInfo > list = QextSerialEnumerator::getPorts();
    QStringList devices;
    for (int i=0;i<list.count();i++)
    {
        QString s = list.at(i).portName;
        char t[20];
        for(int j=0;j<s.length();j++)
        {
            if (!isprint((int)s.at(j).toAscii()))
            {
                s.resize(j);
                break;
            }
            else
                t[j] = s.at(j).toAscii();
        }
        devices.append(s);
    }
#endif

    for(int i=0;i<devices.count();i++)
    {
        if (!devices.at(i).contains("ttys"))
            dlgUi.listPorts->addItem(devices.at(i));
        if (devices.at(i) == deviceName)
            selectedDevice = devices.at(i);
    }
    if (!selectedDevice.isEmpty())
    {
        for (int i=0;i<dlgUi.listPorts->count();i++)
            if (selectedDevice == dlgUi.listPorts->item(i)->text())
                dlgUi.listPorts->setCurrentRow(i);
    }
*/
    //---- Settings ---- TODO: Import from configuration & use QSetting
    //hwFlow = false;
    //baudNdx = 6;
    //deviceName = "COM19";
    emit gotConnected(false);

}


void RR_QtSerial::switchSerial(bool on)
{
    if(on){connectSerial();}
    else{disconnectSerial();}
}

void RR_QtSerial::receiveBytes()
{
    if (!port) {
        return;
    }
    if(!port->isOpen()){emit gotConnected(false);}
    free(receivedBytes);
    receivedBytes = new QByteArray(port->readAll());
    if (receivedBytes->isEmpty()) return;
    emit gotBytes(receivedBytes);
}

void RR_QtSerial::sendBytes(QByteArray *bytes)
{

    if (!port){
        return;
    }
    if(bytes==NULL) return;
    if(bytes->isEmpty()) return;
    port->write(*(bytes));
}

void RR_QtSerial::connectSerial()
{
    if (!port){
        // Open it
        // Start comm
#ifdef _TTY_POSIX_
        device = QString("/dev/");
#endif
        device += com;
        port = new QextSerialPort(device, QextSerialPort::Polling);
        port->setBaudRate((BaudRateType)baud);
        port->setDataBits(DATA_8);
        port->setParity(PAR_NONE);
        port->setStopBits(STOP_1);
        port->setFlowControl(hwFlow ? FLOW_HARDWARE : FLOW_OFF);
        port->setTimeout(100);

        if (!port->open(QIODevice::ReadWrite)){
            delete port;
            port = NULL;
            QString s("Cannot open port ");
            s += device;
            QMessageBox::critical(NULL,"error",s);
            emit gotConnected(false);
        }
        else{
            emit gotConnected(port->isOpen());
        }
    }
}

void RR_QtSerial::disconnectSerial()
{
    if (port){
        //Close it
        delete port;
        port = NULL;
        emit gotConnected(false);
    }
}


void RR_QtSerial::printReceivedRaw(QPlainTextEdit *textEdit)
{
    if(!receivedBytes->isNull()){
        receivedBytes->replace("\r", "");
        if (receivedBytes->contains(8))
        {
            // Must parse backspace commands manually
            for (int i=0;i<receivedBytes->count();i++)
            {
                char ch = receivedBytes->at(i);
                if (ch == 8)
                {
                    // Backspace
                    QString s = textEdit->toPlainText();
                    s.chop(1);
                    textEdit->setPlainText(s);
                }
                else
                {
                    // Add char to edit
                    QString s(ch);
                    textEdit->insertPlainText(s);
                }
                textEdit->moveCursor(QTextCursor::End);
            }
        }
        else
        {
            textEdit->moveCursor(QTextCursor::End);
            textEdit->insertPlainText(*(receivedBytes));
        }
        textEdit->ensureCursorVisible();
    }
}

bool RR_QtSerial::isConnected()
{
    return port;
}
