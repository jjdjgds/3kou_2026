#include "DebugDraw.h"
#include "direct3d.h"
#include "shader3d.h"
#include <cstring>

using namespace DirectX;

struct DDVertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

static ID3D11Buffer* g_pVB = nullptr;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static std::vector<DDVertex> g_vertices;
static int g_maxVertices = 0;

void DebugDraw_Initialize(int maxLines)
{
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetContext();
	g_maxVertices = maxLines * 2;
	g_vertices.clear();
	g_vertices.reserve(g_maxVertices);

	// バッファ作成（動的、書き換え）
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DDVertex) * g_maxVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (g_pVB) { g_pVB->Release(); g_pVB = nullptr; }
	g_pDevice->CreateBuffer(&bd, nullptr, &g_pVB);
}

void DebugDraw_Finalize()
{
	if (g_pVB) { g_pVB->Release(); g_pVB = nullptr; }
	g_vertices.clear();
	g_maxVertices = 0;
}

void DebugDraw_AddLine(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT4& color)
{
	if ((int)g_vertices.size() + 2 > g_maxVertices) return; // 容量オーバーは無視
	g_vertices.push_back(DDVertex{ a, color });
	g_vertices.push_back(DDVertex{ b, color });
}

void DebugDraw_AddAABB(const AABB& aabb, const XMFLOAT4& color)
{
	// 8頂点を計算
	const XMFLOAT3& mn = aabb.GetMin();
	const XMFLOAT3& mx = aabb.GetMax();

	XMFLOAT3 v[8] = {
		{ mn.x, mn.y, mn.z },
		{ mx.x, mn.y, mn.z },
		{ mx.x, mx.y, mn.z },
		{ mn.x, mx.y, mn.z },
		{ mn.x, mn.y, mx.z },
		{ mx.x, mn.y, mx.z },
		{ mx.x, mx.y, mx.z },
		{ mn.x, mx.y, mx.z }
	};

	// エッジ 12 本（各エッジをラインで追加）
	const int edges[12][2] = {
		{0,1},{1,2},{2,3},{3,0}, // 下面
		{4,5},{5,6},{6,7},{7,4}, // 上面
		{0,4},{1,5},{2,6},{3,7}  // 縦エッジ
	};

	for (int i = 0; i < 12; ++i) {
		DebugDraw_AddLine(v[edges[i][0]], v[edges[i][1]], color);
	}
}

void DebugDraw_Draw()
{
	if (g_vertices.empty() || !g_pVB || !g_pContext) return;

	// バッファへ書き込み
	D3D11_MAPPED_SUBRESOURCE ms{};
	if (SUCCEEDED(g_pContext->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
		std::memcpy(ms.pData, g_vertices.data(), sizeof(DDVertex) * g_vertices.size());
		g_pContext->Unmap(g_pVB, 0);
	}

	// 描画セットアップ
	Shader3d_Begin();
	UINT stride = sizeof(DDVertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);
	Shader3d_SetWorldMatrix(XMMatrixIdentity());
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	g_pContext->Draw((UINT)g_vertices.size(), 0);

	// 次フレームへクリア
	g_vertices.clear();
}