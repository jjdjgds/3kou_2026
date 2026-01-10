#include "MouseLogger.h"
#include <cstring>

static Mouse_State gCurrentState;
static Mouse_State gPrevState;

static int gWheelDelta = 0;
static int gPrevWheelDelta = 0;

// 追加：前フレーム座標
static int gPrevX = 0;
static int gPrevY = 0;
void MouseLogger_Initialize()
{
    memset(&gCurrentState, 0, sizeof(Mouse_State));
    memset(&gPrevState, 0, sizeof(Mouse_State));

    gPrevX = 0;
    gPrevY = 0;
}

void MouseLogger_Update()
{
    // 前フレームを保持
    memcpy(&gPrevState, &gCurrentState, sizeof(Mouse_State));
    gPrevWheelDelta = gWheelDelta;

    gPrevX = gCurrentState.x;
    gPrevY = gCurrentState.y;

    // 現在の状態を取得（mouse.cpp）
    Mouse_GetState(&gCurrentState);

    // ホイール量
    gWheelDelta = gCurrentState.scrollWheelValue;
}

/* -------------------------------------------------------
   ボタン押下状態チェック
------------------------------------------------------- */

static bool getButton(const Mouse_State& s, MouseKey key)
{
    switch (key)
    {
    case MouseKey::Left:   return s.leftButton;
    case MouseKey::Right:  return s.rightButton;
    case MouseKey::Middle: return s.middleButton;
    case MouseKey::X1:     return s.xButton1;
    case MouseKey::X2:     return s.xButton2;
    case MouseKey::WheelUp:
        return (s.scrollWheelValue > 0);
    case MouseKey::WheelDown:
        return (s.scrollWheelValue < 0);
    }
    return false;
}

/*-------------------------------------------------------
   Public API
-------------------------------------------------------*/

bool MouseLogger_IsPressed(MouseKey key)
{
    return getButton(gCurrentState, key);
}

bool MouseLogger_IsTrigger(MouseKey key)
{
    return (getButton(gCurrentState, key) && !getButton(gPrevState, key));
}

bool MouseLogger_IsRelease(MouseKey key)
{
    return (!getButton(gCurrentState, key) && getButton(gPrevState, key));
}

/* -------------------------------------------------------
   相対移動量の取得
------------------------------------------------------- */

int MouseLogger_GetRelativeX()
{
    return gCurrentState.x - gPrevX;
}

int MouseLogger_GetRelativeY()
{
    return gCurrentState.y - gPrevY;
}

/* -------------------------------------------------------
   絶対座標
------------------------------------------------------- */

int MouseLogger_GetX()
{
    return gCurrentState.x;
}

int MouseLogger_GetY()
{
    return gCurrentState.y;
}