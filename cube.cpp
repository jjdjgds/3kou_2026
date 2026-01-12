#include <d3d11.h>

#include "cube.h"

#include <DirectXMath.h>
#include"DirectXTex.h"
#include "debug_ostream.h"
#include "shader3d.h"
#include "direct3d.h"
#include "keyboard.h"
#include "keylogger.h"
#include "texture.h"
using namespace DirectX;

static constexpr int NUM_VERTEX = 4 * 6; // 頂点数
static constexpr int NUM_INDEX = 36; // 頂点数

//static constexpr int NUM_VERTEX = 100; // 頂点数
//static constexpr int NUM_VERTEX = 6*1; // 頂点数

//9923.png
static ID3D11Buffer* g_pIndexBuffer = nullptr;
static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
//2枚3枚使うときはこれを増やす
//一個につき一枚
//static ID3D11ShaderResourceView* g_pTexture = nullptr; // テクスチャビュー（必要なら追加）


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT3 normal;   // 法線
	XMFLOAT4 color;    // 頂点色
	XMFLOAT2 texcoord; // テクスチャ座標（必要なら追加）
	
};



float g_time = 0.0f;


static Vertex3d g_CubeVertex[] =
{
	// 前面 (Z+)
	{{-0.5f,  0.5f,  0.5f},{0.0f,0.0f,1.0f}, {1,1,1,1}, {0.000f, 0.000f}},
	{{-0.5f, -0.5f,  0.5f},{0.0f,0.0f,1.0f}, {1,1,1,1}, {0.000f, 0.500f}},
	{{ 0.5f, -0.5f,  0.5f},{0.0f,0.0f,1.0f}, {1,1,1,1}, {0.333f, 0.500f}},
	{{ 0.5f,  0.5f,  0.5f},{0.0f,0.0f,1.0f}, {1,1,1,1}, {0.333f, 0.000f}},

	// 背面 (Z-)
	{{ 0.5f,  0.5f, -0.5f},{0.0f,0.0f,-1.0f}, {1,1,1,1}, {1.000f, 0.500f}},
	{{ 0.5f, -0.5f, -0.5f},{0.0f,0.0f,-1.0f}, {1,1,1,1}, {1.000f, 1.000f}},
	{{-0.5f, -0.5f, -0.5f},{0.0f,0.0f,-1.0f}, {1,1,1,1}, {0.667f, 1.000f}},
	{{-0.5f,  0.5f, -0.5f},{0.0f,0.0f,-1.0f}, {1,1,1,1}, {0.667f, 0.500f}},

	// 左面 (X-)
	{{-0.5f,  0.5f, -0.5f},{-1.0f,0.0f,0.0f}, {1,1,1,1}, {0.000f, 0.500f}},
	{{-0.5f, -0.5f, -0.5f},{-1.0f,0.0f,0.0f}, {1,1,1,1}, {0.000f, 1.000f}},
	{{-0.5f, -0.5f,  0.5f},{-1.0f,0.0f,0.0f}, {1,1,1,1}, {0.333f, 1.000f}},
	{{-0.5f,  0.5f,  0.5f},{-1.0f,0.0f,0.0f}, {1,1,1,1}, {0.333f, 0.500f}},

	// 右面 (X+)
	{{ 0.5f,  0.5f,  0.5f},{1.0f,0.0f,0.0f}, {1,1,1,1}, {0.667f, 0.000f}},
	{{ 0.5f, -0.5f,  0.5f},{1.0f,0.0f,0.0f}, {1,1,1,1}, {0.667f, 0.500f}},
	{{ 0.5f, -0.5f, -0.5f},{1.0f,0.0f,0.0f}, {1,1,1,1}, {1.000f, 0.500f}},
	{{ 0.5f,  0.5f, -0.5f},{1.0f,0.0f,0.0f}, {1,1,1,1}, {1.000f, 0.000f}},

	// 上面 (Y+)
	{{-0.5f,  0.5f, -0.5f},{0.0f,1.0f,0.0f}, {1,1,1,1}, {0.333f, 0.000f}},
	{{-0.5f,  0.5f,  0.5f},{0.0f,1.0f,0.0f}, {1,1,1,1}, {0.333f, 0.500f}},
	{{ 0.5f,  0.5f,  0.5f},{0.0f,1.0f,0.0f}, {1,1,1,1}, {0.667f, 0.500f}},
	{{ 0.5f,  0.5f, -0.5f},{0.0f,1.0f,0.0f}, {1,1,1,1}, {0.667f, 0.000f}},

	// 下面 (Y-)
	{{-0.5f, -0.5f,  0.5f}, {0.0f,-1.0f,0.0f},{1,1,1,1}, {0.333f, 0.500f}},
	{{-0.5f, -0.5f, -0.5f}, {0.0f,-1.0f,0.0f},{1,1,1,1}, {0.333f, 1.000f}},
	{{ 0.5f, -0.5f, -0.5f}, {0.0f,-1.0f,0.0f},{1,1,1,1}, {0.667f, 1.000f}},
	{{ 0.5f, -0.5f,  0.5f}, {0.0f,-1.0f,0.0f},{1,1,1,1}, {0.667f, 0.500f}},
};



static unsigned short g_CubeIndex[] =
{
	// Front
	0,1,2, 0,2,3,
	// Back
	4,5,6, 4,6,7,
	// Left
	8,9,10, 8,10,11,
	// Right
	12,13,14, 12,14,15,
	// Top
	16,17,18, 16,18,19,
	// Bottom
	20,21,22, 20,22,23,
};

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	//Cube01tex = Texture_Load(L"rom\\saikoro_image.png");


	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Polygon_Initialize() : 与えられたデバイスかコンテキストが不正です";
		return;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sd;
	sd.pSysMem = g_CubeVertex;

	//頂点バッファへ流し込むデータの設計

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);


	
	
	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	
	sd.pSysMem = g_CubeIndex;
	g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);


}

void Cube_Finalize()
{
	SAFE_RELEASE(g_pIndexBuffer);
	SAFE_RELEASE(g_pVertexBuffer);

}
static XMVECTOR position{};
void Cube_Update(double elapsedTime)
{

}

//void Cube_Draw()
//{
//
//	Shader3d_Begin();
//	UINT stride = sizeof(Vertex3d);
//	UINT offset = 0;
//	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
//
//	const float SCREEN_WIDTH =  (float)Direct3D_GetBackBufferWidth();
//	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
//
//	XMMATRIX scalingMatrix = XMMatrixScaling
//	(camera.Scaling.x, camera.Scaling.y, camera.Scaling.z);
//
//	XMMATRIX translationMatrix = XMMatrixTranslation
//	(camera.Position.x, camera.Position.y, camera.Position.z);
//
//	XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw
//	(XMConvertToRadians(camera.Rotation.x),
//		XMConvertToRadians(camera.Rotation.y),
//		XMConvertToRadians(camera.Rotation.z));
//
//	Shader3d_SetWorldMatrix(scalingMatrix * RotationMatrix * translationMatrix);
//
//
//	//Shader3d_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
//	XMVECTOR eye{ camera.posx, camera.posy, camera.posz }; // -5.0 は不要
//	//camera.posz = 3.0f;
//	XMVECTOR target{ camera.posx, camera.posy, camera.posz - 1.0f };
//
//
//	XMVECTOR up{ 0.0f,1.0,0.0f };
//	XMMATRIX mtxView = XMMatrixLookAtLH(eye, target, up);
//	Shader3d_SetViewMatrix(mtxView);
//	float w = static_cast<int>(Direct3D_GetBackBufferWidth());
//	float h = static_cast<int>(Direct3D_GetBackBufferHeight());
//	float aspect = (float)w / (float)h;
//
//	XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(1.0f, aspect, 0.1f, 100.0f);
//	Shader3d_SetProjectionMatrix(mtxProj);
//
//	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	//Tecture_SetTexture(texid);
//	g_pContext->Draw(NUM_VERTEX, 0);
//}
void Cube_Draw(const XMMATRIX& mtxWorld,int texid)
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();

	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	//頂点Indexを描画パイプラインに設定
	
	g_pContext->IASetIndexBuffer(g_pIndexBuffer,DXGI_FORMAT_R16_UINT,0);

	//ワールド変換座標の設定

	//パースペクティブ変換行列の設定

	// 頂点シェーダーにworld変換行列を設定
	//平行移動
	XMMATRIX mtxT = XMMatrixTranslation(0.5f, 0.5f, 0.5f);
	//XMMATRIX mtxT = XMMatrixTranslationFromVector(position);

	//回転
	XMMATRIX mtxR = XMMatrixRotationY(g_time);

	//拡大
	float s = sinf(g_time) + 1.0;
	XMMATRIX mtxS = XMMatrixScaling(s, s, s);

	Shader3d_SetWorldMatrix(mtxWorld); //合成したい順番で掛け算する


	static float angle = 0.0f;
	angle += 0.01f;
	float radius = 5.0f;

	//ビュー変換行列の設定
	//XMVECTOR eye = {
	//sinf(angle) * radius,  // X座標
	//sinf(angle) * radius* 0.5,                  // 高さ（固定）
	//cosf(angle) * radius   // Z座標
	//};
	//XMVECTOR eye{ 2.0f, 3.0f, -10.0f }; //カメラの位置
	//XMVECTOR target{ 0.0f, 0.0f, 0.0f }; //注視点の位置
	//XMVECTOR up{ 0.0f, 1.0f, 0.0f }; //カメラの上方向
	//XMMATRIX mtxView = XMMatrixLookAtLH(eye, target, up);
	//Shader3d_SetViewMatrix(mtxView);
	//// 画面サイズの取得
	//float w = static_cast<float>(Direct3D_GetBackBufferWidth());
	//float h = static_cast<float>(Direct3D_GetBackBufferHeight());
	//// 頂点シェーダーにプロジェクション変換行列を設定
	//XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(1.0f, w / h, 0.1f, 1000.0f);
	//Shader3d_SetProjectionMatrix(mtxProj);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Tecture_SetTexture(texid); // テクスチャの設定
	Shader3d_SetMaterialDiffuse({ 1.0,1.0,1.0 ,1.0 });

	// ポリゴン描画命令発行
	//g_pContext->Draw(NUM_VERTEX, 0);
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);

}

