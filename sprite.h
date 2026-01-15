/*==========================================================================

スプライト描画[sprite.h]

												Author : hidetoshi muramatu
												Date   : 2025/06/06
---------------------------------------------------------------------------






=========================================================================*/

#ifndef SPRITE_H
#define SPRITE_H
#include <DirectXMath.h>
//#include <DirectXColors.h>
#include <d3d11.h>

void Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Sprite_Finalize(void);
void Sprite_Draw(
	int texid,
	float x, float y,
	float width, float height,
	int tx, int ty,
	int tw, int th,
	DirectX::XMFLOAT4 color = {1.0f,1.0f,1.0f,1.0f}
);

void Sprite_Draw(
	int texid,
	float x, float y,
	float w, float h,
	DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f }
);

void Sprite_Draw(int texid, float x, float y, float w, float h, float scrollX, float scrollY, DirectX::XMFLOAT4 color);

void Sprite_Draw(
	int texid,
	float angle, // 回転角度（ラジアン単位）
	float x, float y,
	float width, float height,
	int tx, int ty,
	int tw, int th,
	DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f }
);

void Sprite_DrawCenter(
	int texid,
	float cx, float cy,
	float w, float h,
	DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f }
);






#endif