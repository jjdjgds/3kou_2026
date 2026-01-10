
/*==========================================================================



												Author : hidetoshi muramatu
												Date   : 2025/
---------------------------------------------------------------------------






=========================================================================*/

#pragma once
#include "DirectXMath.h"
#include "mouse.h"

void Shot_Initialize(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3&   front);
void Shot_Finalize();
void Shot_Update(double et);
void Shot_Draw();

const DirectX::XMFLOAT3& Shot_GetVelocity();
void Shot_SetPosition(const DirectX::XMFLOAT3& position);
float Shot_GetPower();
void Shot_ResetPower();
float Clamp(float v, float minV, float maxV);
