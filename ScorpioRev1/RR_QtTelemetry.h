#ifndef RR_QTTELEMETRY_H
#define RR_QTTELEMETRY_H

#include <QObject>
#include <QPlainTextEdit>
#include <QThread>
class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};
typedef enum {LAUNCHING, ASCENDING, LANDING, LANDED, NAVIGATING, FINISHED} MainState_t;
typedef enum {CRUISING, OBSTACLED} NavigatingState_t;
typedef enum {AUTONOMOUS_SIMPLE, AUTONOMOUS_ADVANCED,MANUAL_PC, MANUAL_3CH} DriveMode_t;

struct RR_QtTelemetryReceivedData_t{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER SYSTEM
    struct Data_t
    {
        quint16 Year;
        quint8 Month, Day;
    }Date;
    struct Time_t
    {
        quint8 Hour, Minute, Seconds;
    }Time;

    quint8 LaunchState;
    quint8 NavigationalState;
    quint8  fix;
    float Latitude;
    float Longitude;
    float targetLatidude, targetLongitude;
    quint16 DistanceToTarget, DistanceTravelled;
    qint16 Heading;
    quint16 Bearing;
    quint16 Altitude, baseAltitude, maxAltitude;
    quint32 ellapsedm;
    quint8  isMoving;
    struct Motor_T
    {
        quint16 leftCurrent, rightCurrent;
        qint16	leftSpeed, rightSpeed;
    }Motors;

};

struct RR_QtTelemetrySentData_t{
    struct Joystick_t{
        struct Pad_t{
            qint16 X_Axis, Y_Axis;
        }Pad_Left, Pad_Right;
    }Joystick;

    quint8 buttom1=0;
    quint8 StartState=0;
    quint8 DriveMode=0;
};


class RR_QtTelemetry : public QObject
{
    Q_OBJECT
public:
    explicit RR_QtTelemetry(RR_QtTelemetryReceivedData_t *rec,
                            RR_QtTelemetrySentData_t        *sen, QObject *parent = 0);
    void printReceivedMessage(QPlainTextEdit *textEdit);
signals:
    void gotDecodedMessage(void);                    //Connected to UiTelemetryUpdates
    void gotEncodedBytes(QByteArray     *sentBytes); //Connect to serial sendbytes

public slots:
    void decodeBytes(QByteArray *bytes); //Connected from Serial gotBytes
    void sendMessage(void); //Connected from Serial Timer

private:
    RR_QtTelemetryReceivedData_t    *receivedData;
    RR_QtTelemetrySentData_t        *sentData;
    size_t sizeRECEIVED_MSG, sizeSENT_MSG;
    quint8 *rx_buffer;
    quint8 *addressRECEIVED_MSG, *addressSENT_MSG;
    int rx_array_ind;
};

#endif // RR_QTTELEMETRY_H
