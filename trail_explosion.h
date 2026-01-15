/*==========================================================================
トレイルベース爆発エフェクト[trail_explosion.h]
												Author : hidetoshi muramatu
												Date   : 2025/01/15
---------------------------------------------------------------------------
既存のTrailシステムを使った軽量な爆発エフェクト
=========================================================================*/
#pragma once
#include <DirectXMath.h>

// 爆発の種類
enum class TrailExplosionType
{
	RADIAL,        // 放射状
	SPHERE,        // 球状
	CONE_UP,       // 円錐(上向き)
	RING,          // リング状
	SPIRAL,        // 螺旋
};

// 爆発設定
struct TrailExplosionSettings
{
	DirectX::XMFLOAT3 position;          // 爆発位置
	TrailExplosionType type;             // 爆発タイプ
	int trailCount;                      // トレイル数(8-32推奨)
	float speed;                         // 拡散速度
	float maxDistance;                   // 最大到達距離
	DirectX::XMFLOAT4 startColor;       // 開始カラー
	DirectX::XMFLOAT4 endColor;         // 終了カラー
	float duration;                      // 持続時間(秒)
	bool useGravity;                     // 重力を使うか
	float scale;                         // 全体スケール

	DirectX::XMFLOAT3 rotation; // ★追加（Euler角：ラジアン）
};

void TrailExplosion_Initialize();
void TrailExplosion_Finalize();
void TrailExplosion_Update(float deltaTime);
void TrailExplosion_Draw();

// 爆発を生成
void TrailExplosion_Create(const TrailExplosionSettings& settings);

// プリセット爆発
void TrailExplosion_CreateRadial(const DirectX::XMFLOAT3& position, float scale = 1.0f);
void TrailExplosion_CreateSphere(const DirectX::XMFLOAT3& position, float scale = 1.0f);
void TrailExplosion_CreateConeUp(const DirectX::XMFLOAT3& position, float scale = 1.0f);
void TrailExplosion_CreateRing(const DirectX::XMFLOAT3& position, float scale = 1.0f);
void TrailExplosion_CreateSpiral(const DirectX::XMFLOAT3& position, float scale = 1.0f);

// カメラ位置設定
void TrailExplosion_SetCameraPosition(const DirectX::XMFLOAT3& position);

// アクティブな爆発数を取得
int TrailExplosion_GetActiveCount();