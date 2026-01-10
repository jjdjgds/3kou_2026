
/*==========================================================================
É}ÉEÉXì¸óÕÇÃä«óù[Mouselogger.h]



                                                    Author : hidetoshi muramatu
                                                    Date   : 2025/
---------------------------------------------------------------------------



==========================================================================*/
#pragma once
#include "mouse.h"

enum class MouseKey
{
    Left,
    Right,
    Middle,
    X1,
    X2,
    WheelUp,
    WheelDown
};
void MouseLogger_Initialize();
void MouseLogger_Update();

bool MouseLogger_IsPressed(MouseKey key);
bool MouseLogger_IsTrigger(MouseKey key);
bool MouseLogger_IsRelease(MouseKey key);

int MouseLogger_GetRelativeX();
int MouseLogger_GetRelativeY();
int MouseLogger_GetX();
int MouseLogger_GetY();
