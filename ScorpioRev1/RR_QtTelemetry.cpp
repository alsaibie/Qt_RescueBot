#include "RR_QtTelemetry.h"
#include <QDebug>



RR_QtTelemetry::RR_QtTelemetry(RR_QtTelemetryReceivedData_t *rec,
        RR_QtTelemetrySentData_t        *sen, QObject *parent) :
    QObject(parent)
{
    sentData=sen;
    receivedData=rec;
    sizeRECEIVED_MSG=sizeof(*receivedData);
    sizeSENT_MSG=sizeof(*sentData);
    addressRECEIVED_MSG=(quint8*)receivedData;
    addressSENT_MSG=(quint8*)sentData;
    rx_buffer = (quint8*) malloc(sizeRECEIVED_MSG);

}

void RR_QtTelemetry::printReceivedMessage(QPlainTextEdit *textEdit)
{
    quint8 h=receivedData->Time.Hour;
    quint8 m=receivedData->Time.Minute;
    quint8 s=receivedData->Time.Seconds;
    textEdit->insertPlainText(
                QString("Time: %1:%2:%3\n").arg(h).arg(m).arg(s));
    quint16 y=receivedData->Date.Year;
    quint8  mo=receivedData->Date.Month;
    quint8  d=receivedData->Date.Day;
    textEdit->insertPlainText(
                QString("Date: %1/%2/%3\n ").arg(y).arg(mo).arg(d));
    textEdit->insertPlainText(
                QString("Launch State %1, ").arg(receivedData->LaunchState));
    textEdit->insertPlainText(
                QString("Navigational State %1\n").arg(receivedData->NavigationalState));
    float lt=receivedData->Latitude;
    float ln=receivedData->Longitude;
    textEdit->insertPlainText(
                QString("Latitude: %1%2, Longitude: %3%4\n").arg(lt).arg("N").arg(ln).arg("W"));
    textEdit->insertPlainText(
                QString("Heading %1, Bearing %2\n").arg(receivedData->Heading).arg(receivedData->Bearing));
    textEdit->insertPlainText(
                QString("Distance to Target %1\n").arg(receivedData->DistanceToTarget));
    textEdit->insertPlainText(
                QString("Altitude %1m\n").arg(receivedData->Altitude));
    textEdit->insertPlainText("Printing Received Message\n");
    textEdit->moveCursor(QTextCursor::End);
}

void RR_QtTelemetry::decodeBytes(QByteArray *bytes)
{

    if (bytes->isEmpty()) return;

    char *data=bytes->data();
    Sleeper::msleep(1);
    while((quint8)*data!= 0x06)
    {
        data++;
        if((quint8)*data==NULL) return;
    }

    if ((quint8)(*++data)==0x85)
    {

        if((quint8)(*++data)==sizeRECEIVED_MSG)
        {

           rx_array_ind=0;
           data++; //Need to skip one address for some reason. Padding possibly
           while(rx_array_ind<=sizeRECEIVED_MSG)
           {
              rx_buffer[rx_array_ind++]=*data++;
           }
           //Checksum verification
           quint8 calc_CS=sizeRECEIVED_MSG;
           for (int i=0; i<sizeRECEIVED_MSG;i++)
           {
               calc_CS^=rx_buffer[i];
           }
           if(calc_CS==rx_buffer[rx_array_ind-1])
           {
               qDebug("\nChecksum  Good :) \n");
               memcpy(addressRECEIVED_MSG,rx_buffer,sizeRECEIVED_MSG);
               emit gotDecodedMessage();
           }
           else
           {
                qDebug("\nChecksum No Good :( \n");
           }
           //---------------------
        }
        else {qDebug()<<"Size of Data Don't Match";}
    }
    else
    {
        qDebug()<<"No Matching Header";
    }
}

void RR_QtTelemetry::sendMessage()
{
    QByteArray bytes;
    qint8 CS = sizeSENT_MSG;
    //qDebug()<<CS;
    bytes.append(0x06);
    bytes.append(0x85);
    bytes.append(CS);
    for (int i=0; i<sizeSENT_MSG; i++){
    CS^=*(addressSENT_MSG+i);
    bytes.append(*(addressSENT_MSG+i));
    }
    bytes.append(CS);
    emit gotEncodedBytes(&bytes);
}
