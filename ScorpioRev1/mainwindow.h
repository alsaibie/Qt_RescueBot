#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QGraphicsView>
#include <QtPlugin>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPixmap>
#include "graphicsitemscorpio.h"
#include <QTimer>
#include "RR_SDLJoystick.h"
#include <RR_QtSerial.h>
#include <RR_QtTelemetry.h>
#include <QPointer>
#include <math.h>

#define BASE_LATITUDE 3347.41417f  //N  ddmm.mmmm
#define BASE_LONGITUDE -08424.10717f //W dddmm.mmmm
//Nominal Target Location : Will be updated from Robot
#define TARGET_LATITUDE 3347.342f  //N  ddmm.mmmm
#define TARGET_LONGITUDE - 8424.003f //W dddmm.mmmm

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void gotJoystickMessage();
protected:
    void resizeEvent(QResizeEvent *);
private slots:
    void updateSerialStatusBar(bool);
    void updateGPSSerialStatusBar(bool);
    void printStuff(void);
    void testdebug(void);
    void updateGaugeIndicators(void); //Change to update Joystick Graphics
    void updateTrackerGraphics(void); //omit
    void updateTelemetryGraphics(void);
    void updateGPSData(void);
    void updateJoystickTelemetry(void);
    void setPrintFrequencyText(void);
    void updateGPSStatbar(void);
    void on_comboBox_currentTextChanged(const QString &arg1);
    void checkPulse(void);

private:
    Ui::MainWindow *ui;
    //Menu Configurations
    QActionGroup            *machineGroup;
    QActionGroup            *baudrateGroup;
    QActionGroup            *viewGroup;
    //Graphics Manipulation
    QPixmap                 rotatePixmap(QPixmap *pixmapOriginal,qint16 degree);
    QPixmap                 scalePixmap(QPixmap *pixmapOriginal, float scale);
    //Status Bar Items
    QLabel                  *gpsFooterStat, *connectedFooterStat, *connectedGPSFooterStat, *joystickFooterStat, *aliveFooterState;
    bool                    gpsFix, serialConnected, joystickConnected;


    //Indicator Items
    QPixmap                 *compassPixmap, *speedometerPixmap;
    QPixmap                 updateSpeedIndicator(quint8 percentSpeed);
    QTimer                  indicatorTimer;
    QGraphicsRectItem       *steerRailIndicator, *steerIndicator;
    void                    updateSteerIndicator(qint8 percentTurn);
    QGraphicsScene          *steerGraphicsScene;
    void                    updateLaunchStatus();
    //Tracker Items
    void                    resizeTrackerScreen();
    QGraphicsPixmapItem     *narrowGraphicsPixmap;
    GraphicsItemScorpio     *scorpio2DGraphicsItem;
    QGraphicsEllipseItem    *targetMarker;
    QGraphicsRectItem       *baseMarker;
    QGraphicsScene          *trackerScene;
    QGraphicsPolygonItem    *northArrowGraphicsItem;
    QTimer                  timerTracker;
    int                     baseUTMZone, robotUTMZone, targetUTMZone, waypoint1UTMZone;
    float                   baseEasting, baseNorthing;
    float                   robotEasting, robotNorthing;
    float                   targetEasting, targetNorthing;
    float                   waypoint1Easting, waypoint1Northing;
    quint32                 offset_easting, offset_northing;
    QPointF                 poBase, poRobotLastKnown;
    bool                    lastPoKnown;
    //Joystick Item
    RR_SDLJoystick          *myJoystick;
    RR_SDLJoystickData_t    *myJoystickData;
    QTimer                  timerJoystick;
    //Serial Items
    RR_QtSerial             *serialPort, *serialGPSPort;
    QByteArray              *sentBytes, *receivedBytes;
    QTimer                  timerSerial, timerGPSSerial;

    //Telemetry Items
    RR_QtTelemetry                  *telemetry, *gps;
    RR_QtTelemetryReceivedData_t    *receivedTelemetryData, *receivedGPSData;
    RR_QtTelemetrySentData_t        *sentTelemetryData, *sentGPSData;
    //Display Text
    QTimer                  timerSerialPrint;
    //Status Bar Update
    QTimer                  timerStatusBarUpdate;
    //Test and Debug Items
    QTimer                  timerTestDebug;
    //Pulse Check
    QTimer                  timerPulseCheck;
    quint16                 pulseSamplingTime;
    bool                    newPulse;
};

#endif // MAINWINDOW_H
