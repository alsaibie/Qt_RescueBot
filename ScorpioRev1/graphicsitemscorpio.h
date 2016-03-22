#ifndef GRAPHICSITEMSCORPIO_H
#define GRAPHICSITEMSCORPIO_H

#include <QGraphicsItem>

#include <QtCore>
#include <QGraphicsView>
class GraphicsItemScorpio : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    //explicit GraphicsItemScorpio(QWidget *parent = 0);
    GraphicsItemScorpio();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);
    void updatePose(QPointF pos,qint16 angle);
protected:
public slots:

    void advanced();
private:
    qint16 previousRotation;
};

#endif // GRAPHICSITEMSCORPIO_H
