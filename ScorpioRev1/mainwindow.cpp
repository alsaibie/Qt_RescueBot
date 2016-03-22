#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gpsconversion.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), sentBytes(NULL)
{
    ui->setupUi(this);

    //---- Menu Configuration ----
    machineGroup    = new QActionGroup(this);
    machineGroup->addAction(ui->actionScorpio);
    machineGroup->addAction(ui->actionQuadcopter);
    ui->actionScorpio->setChecked(true);
    viewGroup       =   new QActionGroup(this);
    viewGroup->addAction(ui->actionTracking);
    viewGroup->addAction(ui->actionTracking);
    viewGroup->addAction(ui->actionManual_Control);
    viewGroup->addAction(ui->actionCommand_Center);
    ui->actionCommand_Center->setChecked(true);
    //---- ----

    //---- Statusbar ----
    serialConnected=false;
    gpsFix=false;
    joystickConnected=false;
    gpsFooterStat       = new QLabel(this);
    connectedFooterStat = new QLabel(this);
    joystickFooterStat  = new QLabel(this);
    aliveFooterState    = new QLabel(this);
    connectedGPSFooterStat = new QLabel(this);
    ui->statusBar->addPermanentWidget(gpsFooterStat);
    ui->statusBar->addPermanentWidget(connectedFooterStat);
    ui->statusBar->addPermanentWidget(aliveFooterState);
    ui->statusBar->addPermanentWidget(joystickFooterStat);
    ui->statusBar->addPermanentWidget(connectedGPSFooterStat);
    //----  ----

    //---- Compass Graphics View ----
    compassPixmap = new QPixmap(":/img/CompassSquare150.png");
    ui->compassViewLabel->setPixmap(rotatePixmap(compassPixmap,0));
    //----  ----
    //---- Speedometer Graphics View ----
    ui->speedometerViewLabel->setPixmap(updateSpeedIndicator(0));
    //----  ----
    //---- Steer Graphics View ----
    ui->steerGraphicsView->setStyleSheet("background: transparent");
    steerGraphicsScene = new QGraphicsScene(this);
    QBrush  brush(QColor::fromRgb(59,174,227));
    steerRailIndicator = steerGraphicsScene->addRect(0,22.5,148,15,Qt::NoPen,brush);
    brush.setColor(Qt::red);
    QPen    pen(Qt::black);
    pen.setWidth(2);
    steerIndicator = steerGraphicsScene->addRect(67.5,5,15,50,Qt::NoPen,brush);
    updateSteerIndicator(-100);
    ui->steerGraphicsView->setScene(steerGraphicsScene);
    //----  ----

    //---- Tracker View ----
    ui->trackerGraphicsView->
            setBackgroundBrush(QBrush(QColor::fromRgb(237,200,90),Qt::Dense4Pattern));
    trackerScene = new QGraphicsScene(this);
    ui->trackerGraphicsView->setScene(trackerScene);
    QPolygonF NorthArrow;
    NorthArrow.append(QPointF(-5.,0)); NorthArrow.append(QPointF(0.,-20)); NorthArrow.append(QPointF(5.,0));
    NorthArrow.append(QPointF(0.,-5.)); NorthArrow.append(QPointF(-5.,0));
    brush.setColor(Qt::black);
    northArrowGraphicsItem = trackerScene->addPolygon(NorthArrow,Qt::NoPen,brush);
    northArrowGraphicsItem->setPos(800,10);
    northArrowGraphicsItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //Scorpio Graphics Item
    scorpio2DGraphicsItem = new GraphicsItemScorpio;
    trackerScene->addItem(scorpio2DGraphicsItem);
    scorpio2DGraphicsItem->setPos(500,150);
    wgs2utm(BASE_LATITUDE, BASE_LONGITUDE,&baseEasting,&baseNorthing, &baseUTMZone);
    lastPoKnown=false;
    //----  ----
    brush.setColor(QColor(255, 0, 0, 180));
    targetMarker    =   trackerScene->addEllipse(400,100,10,10,pen,brush);
    brush.setColor(QColor(255, 255, 0, 180));
    baseMarker      =   trackerScene->addRect(0,0,10,10,pen,brush);
    //----  ----
    connect(&timerTracker,SIGNAL(timeout()),this,SLOT(updateTrackerGraphics()));
    //TODO: another sig-slot for updating scene to zoom to fit
    timerTracker.start(100);
    //---- ---- ---- ----

    //---- Joystick Initialization ----
    myJoystick = new RR_SDLJoystick;
    myJoystickData = new RR_SDLJoystickData_t;
    QString *joystickInitError;

    if(!myJoystick->initJoystick(myJoystickData, joystickInitError)){
    ui->plainTextEdit->insertPlainText("Error Connecting Joystick");
    //TODO: Fix so that the insertPlainText takes the error code from Joystick Object.
    }
    else{
       ui->plainTextEdit->insertPlainText("Joystick Connected!");
       joystickConnected=true;
       connect(&timerJoystick,SIGNAL(timeout()),myJoystick ,SLOT(pollJoystick()));
       timerJoystick.start(150);
    }
    connect(myJoystick,SIGNAL(gotJoystick()),this,SLOT(updateGaugeIndicators()));
    connect(myJoystick,SIGNAL(gotJoystick()),this,SLOT(updateJoystickTelemetry()));
    //---- ----

    //---- Serial ----
    serialPort = new RR_QtSerial(57600,"COM26");

    connect(&timerSerial,SIGNAL(timeout()), serialPort,SLOT(receiveBytes()));
    connect(ui->actionConnect,SIGNAL(triggered(bool)),serialPort,SLOT(switchSerial(bool)));
    connect(serialPort,SIGNAL(gotConnected(bool)),this,SLOT(updateSerialStatusBar(bool)));
    connect(serialPort,SIGNAL(gotConnected(bool)),ui->actionConnect,SLOT(setChecked(bool)));
    timerSerial.start(150);
    //----- GPS ------
    serialGPSPort = new RR_QtSerial(57600,"COM19");
    connect(&timerGPSSerial, SIGNAL(timeout()),serialGPSPort, SLOT(receiveBytes()));
    connect(ui->actionConnect_GPS, SIGNAL(triggered(bool)), serialGPSPort, SLOT(switchSerial(bool)));
    connect(serialGPSPort,SIGNAL(gotConnected(bool)),this,SLOT(updateGPSSerialStatusBar(bool)));
    connect(serialGPSPort,SIGNAL(gotConnected(bool)),ui->actionConnect_GPS,SLOT(setChecked(bool)));
    timerGPSSerial.start(150);

    //-------
    //---- GPS Data Serialization ----
    receivedGPSData   = new RR_QtTelemetryReceivedData_t;
    sentGPSData       = new RR_QtTelemetrySentData_t;
    gps               = new RR_QtTelemetry(receivedGPSData,sentGPSData);
    receivedGPSData->fix=false;
    connect(serialGPSPort,SIGNAL(gotBytes(QByteArray*)),gps,SLOT(decodeBytes(QByteArray*)));
    connect(gps,SIGNAL(gotDecodedMessage()),this,SLOT(updateGPSData()));
    //----  ----
    //---- Telemetry ----
    receivedTelemetryData   = new RR_QtTelemetryReceivedData_t;
    sentTelemetryData       = new RR_QtTelemetrySentData_t;
    telemetry               = new RR_QtTelemetry(receivedTelemetryData,sentTelemetryData);
    receivedTelemetryData->fix=false;
    connect(this,SIGNAL(gotJoystickMessage()),telemetry,SLOT(sendMessage()));
    connect(telemetry,SIGNAL(gotEncodedBytes(QByteArray*)),serialPort,SLOT(sendBytes(QByteArray*)));
    connect(serialPort,SIGNAL(gotBytes(QByteArray*)),telemetry,SLOT(decodeBytes(QByteArray*)));
    connect(telemetry,SIGNAL(gotDecodedMessage()),this,SLOT(updateTelemetryGraphics()));
    //----  ----
    //---- Text Display ----
    connect(&timerSerialPrint,SIGNAL(timeout()),this, SLOT(printStuff()));
    timerSerialPrint.start(ui->printfreSlider->value());
    connect(ui->printfreSlider, SIGNAL(sliderMoved(int)),&timerSerialPrint,SLOT(start(int)));
    connect(ui->clearPushButton,SIGNAL(clicked()),ui->plainTextEdit,SLOT(clear()));
    connect(ui->printfreSlider,SIGNAL(sliderMoved(int)),this,SLOT(setPrintFrequencyText()));
    setPrintFrequencyText();
    //----  ----
    //---- Test and Debug ----
    connect(&timerTestDebug,SIGNAL(timeout()),this,SLOT(testdebug()));

    timerTestDebug.start(1000);
    //----  ----

    connect(&timerStatusBarUpdate, SIGNAL(timeout()), this, SLOT(updateGPSStatbar()));
    timerStatusBarUpdate.start(500);

    //Pulse Check
    connect(&timerPulseCheck,SIGNAL(timeout()), this, SLOT(checkPulse()));
    pulseSamplingTime = 5000;
    timerPulseCheck.start(pulseSamplingTime);
}

void MainWindow::resizeEvent(QResizeEvent *){
resizeTrackerScreen();
}

void MainWindow::resizeTrackerScreen(){
    trackerScene->removeItem(northArrowGraphicsItem);
    QRectF rect = trackerScene->itemsBoundingRect();
    trackerScene->setSceneRect(rect);
    ui->trackerGraphicsView->fitInView(rect ,Qt::KeepAspectRatio);
    trackerScene->addItem(northArrowGraphicsItem);
    northArrowGraphicsItem->setPos(rect.width()-5,10);
    ui->trackerGraphicsView->scale(.95,.95);
}

QPixmap MainWindow::rotatePixmap(QPixmap *pixmapOriginal,qint16 degree)
{
    QPixmap rotatedPixmap(pixmapOriginal->size());
    rotatedPixmap.fill(Qt::transparent);
    QPainter p(&rotatedPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.translate(rotatedPixmap.size().width() / 2, rotatedPixmap.size().height() / 2);
    p.rotate(degree);
    p.translate(-rotatedPixmap.size().width() / 2, -rotatedPixmap.size().height() / 2);
    p.drawPixmap(0, 0, *compassPixmap);
    p.end();
    return rotatedPixmap;
}

QPixmap MainWindow::scalePixmap(QPixmap *pixmapOriginal, float scale)
{
    QSize   scaledsize = QSize(pixmapOriginal->size());
    scaledsize.setHeight(scaledsize.height()*scale);
    scaledsize.setWidth(scaledsize.width()*scale);
    QPixmap scaledPixmap(scaledsize);
    scaledPixmap.fill(Qt::transparent);
    QPainter p(&scaledPixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.scale(scale,scale);
    p.drawPixmap(0, 0, *pixmapOriginal);
    p.end();
    return scaledPixmap;
}

void MainWindow::updateGaugeIndicators()
{
    quint16 speed = ((myJoystickData->Pad_Left.Y_Axis-32767.0)/(65534))*-100;
    ui->speedometerViewLabel->setPixmap(updateSpeedIndicator((quint8)speed));
    quint8  steer = ((myJoystickData->Pad_Right.X_Axis)/(32767.0))*100;
    updateSteerIndicator(steer);
}

QPixmap MainWindow::updateSpeedIndicator(quint8 percentSpeed)
{
    qint8 value;
    if(percentSpeed<=0)             {value =0;}
    else if (percentSpeed >=100)    {value =18;}
    else                            {value=(qint8)(((float)percentSpeed/100.00)*18.00);}
    QString filename= QString(":/img/SegGauge/Gauge%1-18.png").arg(value);
    QPixmap pixmap(filename);
    return pixmap;
}

void MainWindow::updateSteerIndicator(qint8 percentTurn)
{
    qreal xposi=0;
    if(percentTurn>0){
        if(percentTurn>100? 100: percentTurn)
        xposi = percentTurn*62.5/100;
    }
    else if(percentTurn<0){
       if(percentTurn<-100? -100 : percentTurn)
       xposi = percentTurn*65.0/100;
    }
    steerIndicator->setPos(xposi,0);
}

void MainWindow::updateTrackerGraphics()
{


}

void MainWindow::updateTelemetryGraphics()
{
    newPulse=true;
    ui->labelElapsedTime->setText( QString("%1:%2").arg(int(receivedTelemetryData->ellapsedm/60)).arg(
                                       int(receivedTelemetryData->ellapsedm%60)));
    if(receivedTelemetryData->fix){
    wgs2utm(receivedTelemetryData->Latitude,
            receivedTelemetryData->Longitude,
            &robotEasting, &robotNorthing, &robotUTMZone);
    }
    wgs2utm(receivedTelemetryData->targetLatidude,
            receivedTelemetryData->targetLongitude,
            &targetEasting, &targetNorthing, &targetUTMZone);
    //Offset UTM to fit graphics view using min easting and max northing
    //Since grid starts top left and UTM starts bottom left
    offset_easting = qMin(baseEasting,targetEasting);
    if(receivedTelemetryData->fix){offset_easting = qMin((float)offset_easting,robotEasting);}
    offset_easting-=10;
    offset_northing=qMax(baseNorthing,targetNorthing);
    if(receivedTelemetryData->fix){offset_northing=qMax((float)offset_northing,robotNorthing);}
    offset_northing+=10;

    qint16 h = receivedTelemetryData->Heading;
    ui->compassViewLabel->setPixmap(rotatePixmap(compassPixmap,(-h)));
    ui->headingLcdNumber->display(h);
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    int scale=1;
    int scaleR=1;
    QPointF poTarget= QPointF(targetEasting/scaleR-offset_easting,
                        offset_northing-targetNorthing/scale);
    targetMarker->setPos(poTarget);
    QPointF poBase= QPointF(baseEasting/scaleR-offset_easting,
                        offset_northing-baseNorthing/scale);
    qDebug()<<"Target Easting/Northing: "<<targetEasting<<"  "<<targetNorthing;
    qDebug()<<"Target Mapping: "<<targetEasting/scaleR-offset_easting<<"  "<<offset_northing-targetNorthing/scale;
    qDebug()<<"Base Easting/Northing: "<<baseEasting<<"  "<<baseNorthing;
    qDebug()<<"Target Mapping: "<<baseEasting/scaleR-offset_easting<<"  "<<offset_northing-baseNorthing/scale;
    if(!lastPoKnown)
    {
        poRobotLastKnown = poBase;
    }

    if(receivedTelemetryData->fix){
        poRobotLastKnown = QPointF(robotEasting/scaleR-offset_easting,
                            offset_northing-robotNorthing/scale);
        lastPoKnown=true;
        ui->distanceTravelledLcdNumber->display(receivedTelemetryData->DistanceTravelled);
        ui->distanceToTargetLcdNumber->display(receivedTelemetryData->DistanceToTarget);
        ui->bearingLcdNumber->display(receivedTelemetryData->Bearing);
        qDebug()<<"Robot Easting/Northing: "<<robotEasting<<"  "<<robotNorthing;
        qDebug()<<"Robot Mapping: "<<robotEasting/scale-offset_easting<<"  "<<offset_northing-robotNorthing/scale;
        ui->labelRobotLocation->setText(QString("%1 %2, %3 %4").arg(float(int(receivedTelemetryData->Latitude/100))).arg(
                                           QString::number(fmod(fabs(receivedTelemetryData->Latitude), 100.0),'f',4)).arg(
                                           float(int(receivedTelemetryData->Longitude/100))).arg(
                                           QString::number(fmod(fabs(receivedTelemetryData->Longitude), 100.0),'f',4)));
        if(receivedTelemetryData->isMoving == 1){
            ui->labelMovingStatus->setText("Moving");
        }
        else{
            ui->labelMovingStatus->setText("Not Moving");
        }
    }


    baseMarker->setPos(poBase);
    scorpio2DGraphicsItem->updatePose(poRobotLastKnown,-h);
    updateLaunchStatus();
    ui->labelBaseAlt->setText( QString("%1").arg(receivedTelemetryData->baseAltitude));
    ui->labelMaxAlt->setText(  QString("%1").arg(receivedTelemetryData->maxAltitude));
    ui->labelAltitude->setText(  QString("%1").arg(receivedTelemetryData->Altitude));
    ui->labelleftcurrent->setText(  QString("%1").arg(receivedTelemetryData->Motors.leftCurrent));
    ui->labelrightcurrent->setText(  QString("%1").arg(receivedTelemetryData->Motors.rightCurrent));
    ui->labelleftspeed->setText(  QString("%1").arg(receivedTelemetryData->Motors.leftSpeed));
    ui->labelrightspeed->setText(  QString("%1").arg(receivedTelemetryData->Motors.rightSpeed));
}

void MainWindow::updateGPSData()
{
     //   qDebug()<<"receivedGPSData->fix";
    if(receivedGPSData->fix){
        ui->labelBaseGPSFix->setText("Fix");
        ui->labelBaseGPSFix->setStyleSheet("QLabel { background-color : green; padding: 2 2 2 0px; border-radius: 3px; }");

        ui->labelBaseLocation->setText(QString("%1, %2").arg(receivedGPSData->Latitude).arg(
                                           receivedGPSData->Longitude));
            ui->labelBaseLocation->setText(QString("%1 %2, %3 %4").arg(float(int(receivedGPSData->Latitude/100))).arg(
                                               QString::number(fmod(fabs(receivedGPSData->Latitude), 100.0),'f',4)).arg(
                                               float(int(receivedGPSData->Longitude/100))).arg(
                                               QString::number(fmod(fabs(receivedGPSData->Longitude), 100.0),'f',4)));
        if(receivedTelemetryData->fix){
            quint16 distance, bearing;
            getDistance_Bearing(&receivedTelemetryData->Latitude, &receivedTelemetryData->Longitude,
                                &receivedGPSData->Latitude, &receivedGPSData->Longitude, distance, bearing );
            ui->labelDist2Robot->setText(QString("%1").arg(distance));
            ui->labelRobotBearing->setText(QString("%1").arg(bearing));
        }
    }
    else{
        ui->labelBaseGPSFix->setText("No Fix");
        ui->labelBaseGPSFix->setStyleSheet("QLabel { background-color : red; padding: 2 2 2 0px; border-radius: 3px; }");
    }

}

void MainWindow::updateLaunchStatus(){
    qint8 lstate = receivedTelemetryData->LaunchState;
    switch(lstate){
    case LAUNCHING:
        ui->labelLaunchState->setText("LAUNCHING");
        break;
    case ASCENDING:
        ui->labelLaunchState->setText("ASCENDING");
        break;
    case LANDING:
        ui->labelLaunchState->setText("LANDING");
        break;
    case LANDED:
        ui->labelLaunchState->setText("LANDED");
        break;
    case NAVIGATING:
        ui->labelLaunchState->setText("NAVIGATING");
        break;
    case FINISHED:
        ui->labelLaunchState->setText("FINISHED");
        break;
    }
}

void MainWindow::updateJoystickTelemetry()
{
    memcpy(&(sentTelemetryData->Joystick),myJoystickData,sizeof(*myJoystickData));
   // sentTelemetryData->buttom1=myJoystickData->buttom1;
    //sentTelemetryData->Joystick.Pad_Left.X_Axis=50;
    //qDebug()<<sentTelemetryData->Joystick.Pad_Left.X_Axis;
    //qDebug()<<sentTelemetryData->Joystick.Pad_Left.Y_Axis;
    //qDebug()<<sentTelemetryData->Joystick.Pad_Right.X_Axis;
   // qDebug()<<sentTelemetryData->Joystick.Pad_Right.Y_Axis;
    emit gotJoystickMessage();
}

void MainWindow::setPrintFrequencyText()
{
    ui->printFrequencyLabel->setText(
                QString("Print Frequency (%1ms)").arg(ui->printfreSlider->value()));
}

void MainWindow::printStuff()
{
    //Incoming Serial Stuff
    if(serialPort->isConnected()){
        if(ui->printCheckBox->isChecked()){

            if(!ui->rawprintCheckBox->isChecked()){
                telemetry->printReceivedMessage(ui->plainTextEdit);
            }
            else{
                serialPort->printReceivedRaw(ui->plainTextEdit);
            }
        }
    }
}

void MainWindow::testdebug()
{

}

void MainWindow::updateGPSStatbar()
{
    if(serialPort->isConnected()){
        if(!receivedTelemetryData->fix){
        gpsFooterStat->setText("No GPS Fix");
            gpsFooterStat->setStyleSheet
            ("QLabel { background-color : red; padding: 2 2 2 0px; border-radius: 3px; }");
        }
        else {
            gpsFooterStat->setText("GPS Fixed");
            gpsFooterStat->setStyleSheet
            ("QLabel { background-color : green; padding: 2 2 2 0px; border-radius: 3px; }");
        }
    }
    else{
        gpsFooterStat->setText("");
    }


    if(!joystickConnected){
        joystickFooterStat->setText("No Joystick");
        joystickFooterStat->setStyleSheet
        ("QLabel { background-color : red; padding: 2 2 2 0px; border-radius: 3px; }");
    }
    else {
        joystickFooterStat->setText("Joystick");
        joystickFooterStat->setStyleSheet
        ("QLabel { background-color : green; padding: 2 2 2 0px; border-radius: 3px; }");
    }
}

void MainWindow::updateSerialStatusBar(bool serialConnected)
{
    if(!serialPort->isConnected()){
        connectedFooterStat->setText("Radio Disconnected");
        connectedFooterStat->setStyleSheet
        ("QLabel { background-color : red; padding: 2 2 2 0px; border-radius: 3px; }");
    }
    else{
        connectedFooterStat->setText("Radio Connected");
        connectedFooterStat->setStyleSheet
        ("QLabel { background-color : green; padding: 2 2 2 0px; border-radius: 3px; }");
    }

}

void MainWindow::updateGPSSerialStatusBar(bool)
{
    if(!serialGPSPort->isConnected()){
        connectedGPSFooterStat->setText("GPS Disconnected");
        connectedGPSFooterStat->setStyleSheet
        ("QLabel { background-color : red; padding: 2 2 2 0px; border-radius: 3px; }");
    }
    else{
        connectedGPSFooterStat->setText("GPS Connected");
        connectedGPSFooterStat->setStyleSheet
        ("QLabel { background-color : green; padding: 2 2 2 0px; border-radius: 3px; }");
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    if(arg1=="Autonomous - Simple"){
        sentTelemetryData->DriveMode=AUTONOMOUS_SIMPLE;
    }
    else if(arg1=="Autonomous - Advanced"){
        sentTelemetryData->DriveMode=AUTONOMOUS_ADVANCED;
    }
    else if(arg1=="Manual - PC Joystick"){
        sentTelemetryData->DriveMode=MANUAL_PC;
    }
    else if(arg1=="Manual - 3Ch Transmitter"){
        sentTelemetryData->DriveMode=MANUAL_3CH;
    }
}

void MainWindow::checkPulse()
{
    if(newPulse)
    {
        newPulse=false;
        aliveFooterState->setText("Alive");
        aliveFooterState->setStyleSheet
        ("QLabel { background-color : green; padding: 2 2 2 0px; border-radius: 3px; }");
    }
    else{
        aliveFooterState->setText("No Pulse");
        aliveFooterState->setStyleSheet
        ("QLabel { background-color : red; padding: 2 2 2 0px; border-radius: 3px; }");
    }
}
