#include "graphicsitemscorpio.h"

GraphicsItemScorpio::GraphicsItemScorpio()//(QObject *parent) :
    //QObject(parent)
{
    previousRotation=0;
}

QRectF GraphicsItemScorpio::boundingRect() const
{
    qreal adjust = 0.5;
    return QRectF(-18 - adjust, -22 - adjust,
                  36 + adjust, 60 + adjust);
}

QPainterPath GraphicsItemScorpio::shape() const
{
    QPainterPath path;
    path.addRect(-10, -20, 20, 40);
    return path;
}

void GraphicsItemScorpio::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Left Wheel
    painter->setOpacity(80);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setBrush(Qt::black);
    QRectF leftWRect(-6,-2.5, 6, 15);
    painter->drawRoundedRect(leftWRect,1,1);
    // Right Wheel
    painter->setBrush(Qt::black);
    QRectF rightWRect(10,-2.5, 6, 15);
    painter->drawRoundedRect(rightWRect,1,1);
    // Tail
    painter->setBrush(Qt::lightGray);
    QRectF tailRect(4,6, 2, 15);
    painter->drawRoundedRect(tailRect,1,1);
    // Tail Wheel
    painter->setBrush(Qt::black);
    QRectF tailWRect(3,15, 4, 8);
    painter->drawRoundedRect(tailWRect,1,1);
    // Body
    painter->setBrush(QColor(255, 0, 0, 127));
    QRectF bodyRect(0,0, 10, 10);
    painter->drawRoundedRect(bodyRect,1,1);
}

void GraphicsItemScorpio::updatePose(QPointF pos, qint16 angle)
{
    setPos(pos);
    setTransform(QTransform().translate(5,5).rotate(-previousRotation).translate(-5,-5),true);
    setTransform(QTransform().translate(5,5).rotate(angle).translate(-5,-5),true);
    previousRotation=angle;
}

void GraphicsItemScorpio::advanced()
{
    setTransform(QTransform().translate(5,5).rotate(45).translate(-5,-5),true);
}

