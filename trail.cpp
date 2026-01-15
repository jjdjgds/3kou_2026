#include "trail.h"
#include <d3d11.h>
#include "direct3d.h"
#include "ShaderBillBoard.h"
using namespace DirectX;

static constexpr int POSITION_COUNT_MAX = 50;
static constexpr float POLIGON_WIDTH = 0.1f;
static ID3D11Buffer* g_pVertexBuffer = NULL;
static int g_Texture = -1;
static XMFLOAT3 g_PositionBuffer[POSITION_COUNT_MAX]{};
static XMFLOAT3 g_CameraPosition{};
static bool g_FadeEnabled = true;
static XMFLOAT4 g_StartColor = { 1.0f, 0.5f, 0.0f, 1.0f }; // オレンジ
static XMFLOAT4 g_EndColor = { 0.0f, 0.5f, 1.0f, 0.0f };   // 透明な青
static bool g_IsEnabled = true;

// 頂点構造体(カラー追加)
struct VertexTrail
{
	XMFLOAT3 position;
	XMFLOAT2 texcoord;
	XMFLOAT4 color;  // 追加
};

void Trail_Initialize()
{
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VertexTrail) * POSITION_COUNT_MAX * 2;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Direct3D_GetDevice()->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
}

void Trail_Finalize()
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void Trail_Draw()
{
	if (!g_IsEnabled) return;

	D3D11_MAPPED_SUBRESOURCE msr;
	Direct3D_GetContext()->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	VertexTrail* v = (VertexTrail*)msr.pData;

	for (int i = 0; i < POSITION_COUNT_MAX - 1; i++)
	{
		// カメラへのベクトル
		XMVECTOR vecEye = XMLoadFloat3(&g_PositionBuffer[i]) - XMLoadFloat3(&g_CameraPosition);
		// トレイル方向のベクトル
		XMVECTOR vecTrail = XMLoadFloat3(&g_PositionBuffer[i]) - XMLoadFloat3(&g_PositionBuffer[i + 1]);
		// 外積で幅方向を計算
		XMVECTOR vecCross = XMVector3Cross(vecEye, vecTrail);
		vecCross = XMVector3Normalize(vecCross);

		// フェード計算(古い位置ほど透明に)
		float t = (float)i / (POSITION_COUNT_MAX - 1);
		XMFLOAT4 color;
		if (g_FadeEnabled) {
			color.x = g_StartColor.x * (1 - t) + g_EndColor.x * t;
			color.y = g_StartColor.y * (1 - t) + g_EndColor.y * t;
			color.z = g_StartColor.z * (1 - t) + g_EndColor.z * t;
			color.w = g_StartColor.w * (1 - t) + g_EndColor.w * t;
		}
		else {
			color = g_StartColor;
		}

		// 頂点位置とカラー設定
		XMStoreFloat3(&v[i * 2].position, XMLoadFloat3(&g_PositionBuffer[i]) + vecCross * POLIGON_WIDTH);
		v[i * 2].texcoord = XMFLOAT2(t, 0.0f);
		v[i * 2].color = color;

		XMStoreFloat3(&v[i * 2 + 1].position, XMLoadFloat3(&g_PositionBuffer[i]) - vecCross * POLIGON_WIDTH);
		v[i * 2 + 1].texcoord = XMFLOAT2(t, 1.0f);
		v[i * 2 + 1].color = color;
	}

	// 最後の頂点
	v[POSITION_COUNT_MAX * 2 - 2].position = g_PositionBuffer[POSITION_COUNT_MAX - 1];
	v[POSITION_COUNT_MAX * 2 - 2].texcoord = XMFLOAT2(1.0f, 0.0f);
	v[POSITION_COUNT_MAX * 2 - 2].color = g_EndColor;

	v[POSITION_COUNT_MAX * 2 - 1].position = g_PositionBuffer[POSITION_COUNT_MAX - 1];
	v[POSITION_COUNT_MAX * 2 - 1].texcoord = XMFLOAT2(1.0f, 1.0f);
	v[POSITION_COUNT_MAX * 2 - 1].color = g_EndColor;

	Direct3D_GetContext()->Unmap(g_pVertexBuffer, 0);

	ShaderBillboard_Begin();
	ShaderBillboard_UV({ 0.0f, 0.0f }, { 1.f, 1.f });

	UINT stride = sizeof(VertexTrail);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	ShaderBillboard_SetWorldMatrix(XMMatrixIdentity());
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点カラーを使う場合はここでは白を設定
	ShaderBillboard_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	Direct3D_GetContext()->Draw(POSITION_COUNT_MAX * 2, 0);
}

void Trail_AddPosition(const DirectX::XMFLOAT3& position)
{
	for (int i = 0; i < POSITION_COUNT_MAX - 1; i++)
	{
		g_PositionBuffer[i] = g_PositionBuffer[i + 1];
	}
	g_PositionBuffer[POSITION_COUNT_MAX - 1] = position;
}

void Trail_SetCameraPosition(const DirectX::XMFLOAT3& position)
{
	g_CameraPosition = position;
}

void Trail_SetFadeEnabled(bool enabled)
{
	g_FadeEnabled = enabled;
}

void Trail_SetColor(const DirectX::XMFLOAT4& startColor, const DirectX::XMFLOAT4& endColor)
{
	g_StartColor = startColor;
	g_EndColor = endColor;
}

void Trail_SetEnabled(bool enabled)
{
	g_IsEnabled = enabled;
}

void Trail_Clear()
{
	for (int i = 0; i < POSITION_COUNT_MAX; i++)
	{
		g_PositionBuffer[i] = XMFLOAT3(0, 0, 0);
	}
}