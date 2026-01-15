#include "trail_explosion.h"
#include "trail.h"
#include <vector>
#include <cmath>

using namespace DirectX;

static constexpr int MAX_EXPLOSIONS = 50;
static constexpr int MAX_TRAILS_PER_EXPLOSION = 32;
static constexpr float PI = 3.14159265f;

// 個別トレイルデータ
struct ExplosionTrail
{
	XMFLOAT3 position;
	XMFLOAT3 velocity;
	XMFLOAT3 positionHistory[50];  // トレイル用位置履歴
	bool active;
};

// 爆発インスタンス
struct ExplosionInstance
{
	ExplosionTrail trails[MAX_TRAILS_PER_EXPLOSION];
	int trailCount;
	float timer;
	float duration;
	float maxDistance;
	XMFLOAT4 startColor;
	XMFLOAT4 endColor;
	bool useGravity;
	bool active;
};

static std::vector<ExplosionInstance> g_Explosions;
static XMFLOAT3 g_CameraPosition{};

// 乱数生成
static float RandFloat(float min, float max)
{
	return min + (max - min) * ((float)rand() / RAND_MAX);
}

void TrailExplosion_Initialize()
{
	g_Explosions.reserve(MAX_EXPLOSIONS);
	Trail_Initialize();
}

void TrailExplosion_Finalize()
{
	g_Explosions.clear();
	Trail_Finalize();
}

void TrailExplosion_Update(float deltaTime)
{
	for (auto& explosion : g_Explosions)
	{
		if (!explosion.active) continue;

		explosion.timer += deltaTime;

		// 持続時間経過でフェードアウト
		if (explosion.timer >= explosion.duration)
		{
			explosion.active = false;
			continue;
		}

		// 各トレイルを更新
		for (int i = 0; i < explosion.trailCount; i++)
		{
			ExplosionTrail& trail = explosion.trails[i];
			if (!trail.active) continue;

			// 位置更新
			trail.position.x += trail.velocity.x * deltaTime;
			trail.position.y += trail.velocity.y * deltaTime;
			trail.position.z += trail.velocity.z * deltaTime;

			// 重力適用
			if (explosion.useGravity)
			{
				trail.velocity.y -= 9.8f * deltaTime;
			}

			// 減速(空気抵抗)
			trail.velocity.x *= 0.98f;
			trail.velocity.y *= 0.98f;
			trail.velocity.z *= 0.98f;

			// 位置履歴を更新
			for (int j = 0; j < 49; j++)
			{
				trail.positionHistory[j] = trail.positionHistory[j + 1];
			}
			trail.positionHistory[49] = trail.position;


			

		}
	}

	// 非アクティブな爆発を削除
	g_Explosions.erase(
		std::remove_if(g_Explosions.begin(), g_Explosions.end(),
			[](const ExplosionInstance& e) { return !e.active; }),
		g_Explosions.end()
	);
}

void TrailExplosion_Draw()
{
	Trail_SetCameraPosition(g_CameraPosition);

	for (const auto& explosion : g_Explosions)
	{
		if (!explosion.active) continue;

		// フェード計算
		float fadeAmount = 1.0f;
		if (explosion.timer > explosion.duration * 0.7f)
		{
			fadeAmount = 1.0f - (explosion.timer - explosion.duration * 0.7f) / (explosion.duration * 0.3f);
		}

		// カラー設定(フェードアウト対応)
		XMFLOAT4 startColor = explosion.startColor;
		XMFLOAT4 endColor = explosion.endColor;
		startColor.w *= fadeAmount;
		endColor.w *= fadeAmount;

		Trail_SetColor(startColor, endColor);
		Trail_SetEnabled(true);
		Trail_SetFadeEnabled(true);

		// 各トレイルを描画
		for (int i = 0; i < explosion.trailCount; i++)
		{
			const ExplosionTrail& trail = explosion.trails[i];
			if (!trail.active) continue;

			// トレイルクリア
			Trail_Clear();
			Trail_SetColor(startColor, endColor); // ★毎回設定

			Trail_SetEnabled(true);
			Trail_SetFadeEnabled(true);
			// 位置履歴を追加
			for (int j = 0; j < 50; j++)
			{
				Trail_AddPosition(trail.positionHistory[j]);
			}

			// 描画
			Trail_Draw();
		}
	}
}

void TrailExplosion_Create(const TrailExplosionSettings& settings)
{
	if (g_Explosions.size() >= MAX_EXPLOSIONS) return;

	ExplosionInstance explosion;
	explosion.trailCount = settings.trailCount;
	if (explosion.trailCount > MAX_TRAILS_PER_EXPLOSION)
		explosion.trailCount = MAX_TRAILS_PER_EXPLOSION;

	explosion.timer = 0.0f;
	explosion.duration = settings.duration;
	explosion.maxDistance = settings.maxDistance;
	explosion.startColor = settings.startColor;
	explosion.endColor = settings.endColor;
	explosion.useGravity = settings.useGravity;
	explosion.active = true;

	// トレイル初期化
	for (int i = 0; i < explosion.trailCount; i++)
	{
		ExplosionTrail& trail = explosion.trails[i];
		trail.position = settings.position;

		// タイプ別の方向計算
		XMFLOAT3 direction;

		switch (settings.type)
		{
		case TrailExplosionType::RADIAL:
		{
			// XZ平面で放射状
			float angle = (360.0f / explosion.trailCount) * i * (PI / 180.0f);
			direction.x = cosf(angle);
			direction.y = RandFloat(-0.2f, 0.3f); // 少し上下にランダム
			direction.z = sinf(angle);
			break;
		}
		case TrailExplosionType::SPHERE:
		{
			// 球面上にランダム配置
			float theta = RandFloat(0, PI * 2);
			float phi = acosf(RandFloat(-1.0f, 1.0f));
			direction.x = sinf(phi) * cosf(theta);
			direction.y = sinf(phi) * sinf(theta);
			direction.z = cosf(phi);
			break;
		}
		case TrailExplosionType::CONE_UP:
		{
			// 上向き円錐
			float angle = (360.0f / explosion.trailCount) * i * (PI / 180.0f);
			float elevation = RandFloat(0.3f, 0.8f);
			direction.x = cosf(angle) * (1.0f - elevation);
			direction.y = elevation;
			direction.z = sinf(angle) * (1.0f - elevation);
			break;
		}
		case TrailExplosionType::RING:
		{
			// リング状(Y=0付近)
			float angle = (360.0f / explosion.trailCount) * i * (PI / 180.0f);
			direction.x = cosf(angle);
			direction.y = RandFloat(-0.05f, 0.05f);
			direction.z = sinf(angle);
			break;
		}
		case TrailExplosionType::SPIRAL:
		{
			// 螺旋状
			float angle = (720.0f / explosion.trailCount) * i * (PI / 180.0f);
			float height = (float)i / explosion.trailCount;
			direction.x = cosf(angle);
			direction.y = height * 2.0f - 1.0f;
			direction.z = sinf(angle);
			break;
		}
		}
		// ===== direction を回転させる =====
		{
			XMVECTOR dir = XMLoadFloat3(&direction);

			XMMATRIX rot =
				XMMatrixRotationRollPitchYaw(
					settings.rotation.x, // pitch (X)
					settings.rotation.y, // yaw   (Y)
					settings.rotation.z  // roll  (Z)
				);

			dir = XMVector3TransformNormal(dir, rot);
			dir = XMVector3Normalize(dir);

			XMStoreFloat3(&direction, dir);
		}

		// 速度設定
		float speedVariation = RandFloat(0.8f, 1.2f);
		trail.velocity.x = direction.x * settings.speed * speedVariation * settings.scale;
		trail.velocity.y = direction.y * settings.speed * speedVariation * settings.scale;
		trail.velocity.z = direction.z * settings.speed * speedVariation * settings.scale;

		// 位置履歴初期化
		for (int j = 0; j < 50; j++)
		{
			trail.positionHistory[j] = settings.position;
		}

		trail.active = true;
	}

	g_Explosions.push_back(explosion);
}

void TrailExplosion_CreateRadial(const XMFLOAT3& position, float scale)
{
	TrailExplosionSettings settings;
	settings.position = position;
	settings.position.y += 5;
	settings.type = TrailExplosionType::RADIAL;
	settings.trailCount = 8;
	settings.speed = 100.0f;
	settings.maxDistance = 10.0f;
	settings.startColor = { 1.0f, 0.5f, 0.0f, 1.0f }; // オレンジ
	settings.endColor = { 1.0f, 0.0f, 1.0f, 0.0f };   // 透明な赤
	settings.duration = 0.4f;
	settings.useGravity = true;
	settings.scale = scale;
	settings.rotation = { -XM_PIDIV2, 0.0f, 0.0f }; // X軸 -90度

	TrailExplosion_Create(settings);
}

void TrailExplosion_CreateSphere(const XMFLOAT3& position, float scale)
{
	TrailExplosionSettings settings;
	settings.position = position;
	settings.type = TrailExplosionType::SPHERE;
	settings.trailCount = 24;
	settings.speed = 10.0f;
	settings.maxDistance = 25.0f;
	settings.startColor = { 1.0f, 0.8f, 0.2f, 1.0f }; // 黄色
	settings.endColor = { 1.0f, 0.3f, 0.0f, 0.0f };   // 透明なオレンジ
	settings.duration = 2.0f;
	settings.useGravity = true;
	settings.scale = scale;

	TrailExplosion_Create(settings);
}

void TrailExplosion_CreateConeUp(const XMFLOAT3& position, float scale)
{
	TrailExplosionSettings settings;
	settings.position = position;
	settings.type = TrailExplosionType::CONE_UP;
	settings.trailCount = 20;
	settings.speed = 30.0f;
	settings.maxDistance = 30.0f;
	settings.startColor = { 0.8f, 0.0f, 1.0f, 1.0f }; // 紫
	settings.endColor = { 1.0f, 0.5f, 1.0f, 0.0f };   // 透明なピンク
	settings.duration = 2.5f;
	settings.useGravity = true;
	settings.scale = scale;

	TrailExplosion_Create(settings);
}

void TrailExplosion_CreateRing(const XMFLOAT3& position, float scale)
{
	TrailExplosionSettings settings;
	settings.position = position;
	settings.type = TrailExplosionType::RING;
	settings.trailCount = 24;
	settings.speed = 15.0f;
	settings.maxDistance = 35.0f;
	settings.startColor = { 0.5f, 0.5f, 1.0f, 1.0f }; // 青白
	settings.endColor = { 0.8f, 0.8f, 1.0f, 0.0f };   // 透明な白
	settings.duration = 1.2f;
	settings.useGravity = false;
	settings.scale = scale;

	TrailExplosion_Create(settings);
}

void TrailExplosion_CreateSpiral(const XMFLOAT3& position, float scale)
{
	TrailExplosionSettings settings;
	settings.position = position;
	settings.type = TrailExplosionType::SPIRAL;
	settings.trailCount = 32;
	settings.speed = 7.0f;
	settings.maxDistance = 20.0f;
	settings.startColor = { 0.0f, 1.0f, 0.5f, 1.0f }; // 明るい緑
	settings.endColor = { 0.0f, 0.5f, 0.0f, 0.0f };   // 透明な暗い緑
	settings.duration = 3.0f;
	settings.useGravity = false;
	settings.scale = scale;

	TrailExplosion_Create(settings);
}

void TrailExplosion_SetCameraPosition(const XMFLOAT3& position)
{
	g_CameraPosition = position;
}

int TrailExplosion_GetActiveCount()
{
	return static_cast<int>(g_Explosions.size());
}