#ifndef RR_SDLJOYSTICK_H
#define RR_SDLJOYSTICK_H
#include <QtCore>
#include <QApplication>
#include <QString>
#include <QObject>
#define SDL_MAIN_HANDLED // Not to define SDL main since qt already provides it
#include <SDL2/SDL.h>
static const qint16 JOYSTICK_THRESHOLD =1000;

struct RR_SDLJoystickData_t{
    struct Pad_t{
        qint16 X_Axis, Y_Axis;

    }Pad_Left, Pad_Right;
    quint8 buttom1;
};

class RR_SDLJoystick : public QObject
{
    Q_OBJECT
public:
    explicit RR_SDLJoystick(QObject *parent = 0);
    //RR_SDLJoystick();
    bool initJoystick(RR_SDLJoystickData_t  *data, QString *errorReason);
    ~RR_SDLJoystick();
signals:
    void gotJoystick();
public slots:
    void pollJoystick();
private:
    RR_SDLJoystickData_t *RR_SDLJoystickData;
    SDL_Joystick* joystick;
    bool isJoystickOpen;
};

#endif // RR_SDLJOYSTICK_H
