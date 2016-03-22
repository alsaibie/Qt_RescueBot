#include "RR_SDLJoystick.h"
#include <math.h>

RR_SDLJoystick::RR_SDLJoystick(QObject *parent) :
    QObject(parent), isJoystickOpen(false), joystick(NULL)
{
}

bool RR_SDLJoystick::initJoystick(RR_SDLJoystickData_t  *data, QString *errorReason)
{
    RR_SDLJoystickData=data;
    if(SDL_Init( SDL_INIT_JOYSTICK ) < 0){
       errorReason = new QString("SDL could not initialize! SDL Error: 1\n");
       errorReason->arg(SDL_GetError());
       return false;
    }
    else{
        if( SDL_NumJoysticks() < 1 ){
            errorReason = new QString( "Warning: No joysticks connected!\n" );
            return false;
        }
        else{
            //Load joystick
            joystick = SDL_JoystickOpen( 0 );
            if( joystick == NULL ){
                errorReason = new QString( "Warning: Unable to open game controller! SDL Error: %1\n");
                errorReason->arg(SDL_GetError());
                return false;
            }
        }
    }
    errorReason = new QString("Success");
    isJoystickOpen=true;
    return true;
}

void RR_SDLJoystick::pollJoystick()
{
    if(isJoystickOpen){
        SDL_JoystickUpdate();
        qint16 axis0 = SDL_JoystickGetAxis(joystick, 0);
        qint16 axis1 = SDL_JoystickGetAxis(joystick, 1);
        qint16 axis2 = SDL_JoystickGetAxis(joystick, 2);
        qint16 axis3 = SDL_JoystickGetAxis(joystick, 3);
        RR_SDLJoystickData->Pad_Left.X_Axis = (abs(axis0)>JOYSTICK_THRESHOLD? axis0 :0);
        RR_SDLJoystickData->Pad_Left.Y_Axis = (abs(axis1)>JOYSTICK_THRESHOLD? axis1 :0);
        RR_SDLJoystickData->Pad_Right.X_Axis = (abs(axis2)>JOYSTICK_THRESHOLD? axis2 :0);
        RR_SDLJoystickData->Pad_Right.Y_Axis = (abs(axis3)>JOYSTICK_THRESHOLD? axis3 :0);
        RR_SDLJoystickData->buttom1=SDL_JoystickGetButton(joystick,1);
        emit gotJoystick();
  }
}


RR_SDLJoystick::~RR_SDLJoystick()
{
    SDL_Quit();
}
