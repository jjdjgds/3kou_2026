#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "Collision.h" // AABB ‚ª’è‹`‚³‚ê‚Ä‚¢‚éƒwƒbƒ_

void DebugDraw_Initialize(int maxLines = 4096);
void DebugDraw_Finalize();
void DebugDraw_AddLine(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT4& color = { 1.0f,1.0f,1.0f,1.0f });
void DebugDraw_AddAABB(const AABB& aabb, const DirectX::XMFLOAT4& color = { 1.0f,0.0f,0.0f,1.0f });
void DebugDraw_Draw();