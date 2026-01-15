#include "BowlingBall.h"
#include "model.h"
#include "direct3d.h"
#include "shader3d.h"
#include "DebugDraw.h" // 追加
#include "Map.h"
#include "Shot.h"
#include "AuraEffect.h"
#include "texture.h"
#include "ParticleEffect.h"
#include "lighth.h" // 追加：ライト初期化／設定用
using namespace DirectX;
static MODEL* g_pBall{};
static int g_Texid = -1;

BowlingBall::BowlingBall()
{
}

void BowlingBall::Init(const XMFLOAT3& pos)
{
    m_position = pos;
    m_velocity = { 0.0f, 0.0f ,0.0f};
    g_pBall = ModelLoad("rom\\Model\\ball.fbx", 0.1);
    m_Aabb = g_pBall->Aabb;
    m_Aabb = AABB::Make(GetPosition(), {0.3f,0.3f,0.3f});
    g_Texid = Texture_Load(L"rom\\Texture\\gra_effect_lightA.png");
    AuraEffect_Initialize();
    ParticleEffect_Initialize();

    // ライト初期化 — 全体的に暗い問題をまず緩和するために ambient を上げる
    Light_Initialize();

    // アンビエントを少し強めに（0.25〜0.4 を試してみてください）
    Light_SetAmnient(XMFLOAT4(0.30f, 0.30f, 0.30f, 1.0f));

    // ディフューズ（方向光）を設定：色と方向
    // 方向はワールド空間で「上から斜め下」に向かうベクトルを与える（正規化は内部で行われている想定）
    Light_SetDiffuse(XMFLOAT3(1.0f, 1.0f, 0.95f), XMFLOAT3(-0.5f, -1.0f, 0.5f));
}

void BowlingBall::Update(float deltaTime)
{
    // ===== 重力 =====
    m_velocity.y += m_gravity * deltaTime;

    // ===== 移動 =====
    m_position.x += m_velocity.x * deltaTime;
    m_position.y += m_velocity.y * deltaTime;
    m_position.z += m_velocity.z * deltaTime;

    m_Aabb.Move(m_position);

    m_onGround = false;

   

    // ===== Map 衝突 =====
    for (int i = 0; i < MapGetBlockCount(); ++i)
    {

        const Block& block = MapGetBlock(i);
        const AABB& blockAabb = GetCollision(i);


        Hit hit = m_Aabb.IsHit(blockAabb);
        
          


        if (!hit.IsHit()) continue;

        // 各軸の貫入深度を再計算（球側=AABB g_Aabb, ブロック=block）
        float depth_x =
            std::min(m_Aabb.GetMax().x, blockAabb.GetMax().x) - std::max(m_Aabb.GetMin().x, blockAabb.GetMin().x);
        float depth_y =
            std::min(m_Aabb.GetMax().y, blockAabb.GetMax().y) - std::max(m_Aabb.GetMin().y, blockAabb.GetMin().y);
        float depth_z =
            std::min(m_Aabb.GetMax().z, blockAabb.GetMax().z) - std::max(m_Aabb.GetMin().z, blockAabb.GetMin().z);

        // 最も浅い軸を選ぶ
        int shallow_axis = 0; // 0:x, 1:y, 2:z
        if (depth_x < depth_y) {
            shallow_axis = (depth_x < depth_z) ? 0 : 2;
        }
        else {
            shallow_axis = (depth_y < depth_z) ? 1 : 2;
        }

        const XMFLOAT3& ballCenter = m_Aabb.GetCenter();
        const XMFLOAT3& blockCenter = blockAabb.GetCenter();
        const XMFLOAT3& ballHalf = m_Aabb.GetHalfSize();

        // 軸ごとに押し戻す（半幅を使う）
        if (shallow_axis == 1) // Y
        {
            if (ballCenter.y > blockCenter.y)
            {
                // ボールはブロックの上にいる → ブロックの上面に乗せる
                m_position.y = blockAabb.GetMax().y + ballHalf.y;
                m_velocity.y = 0.0f;
                m_onGround = true;
            }
            else
            {
                // ボールはブロックの下にいる（押し上げられている）→ 下側へスナップ
                m_position.y = blockAabb.GetMin().y - ballHalf.y;
                m_velocity.y = 0.0f;
            }
        }
        else if (shallow_axis == 0) // X
        {
            if (ballCenter.x > blockCenter.x)
            {
                m_position.x = blockAabb.GetMax().x + ballHalf.x;
            }
            else
            {
                m_position.x = blockAabb.GetMin().x - ballHalf.x;
            }
            m_velocity.x = 0.0f;
        }
        else // Z
        {
            if (ballCenter.z > blockCenter.z)
            {
                m_position.z = blockAabb.GetMax().z + ballHalf.z;
            }
            else
            {
                m_position.z = blockAabb.GetMin().z - ballHalf.z;
            }
            m_velocity.z = 0.0f;
        }

        // 補正後に AABB を移動して次の判定に反映
        m_Aabb.Move(m_position);
        if (hit.IsHit())
        {
            if (block.GetType() == Block::Type::Gutter)
            {

                m_velocity.x = 0;
                m_velocity.y = 0;
                m_velocity.z -=0.001;

            }

            continue;

        }
    }
  


    // ===== 摩擦 =====
    if (m_onGround)
    {
        m_velocity.x *= 0.98f;

        (m_gatterFlg) ? m_velocity.z *= 0.98f : m_velocity.z *= 1.0f;
    }

   
    Shot_SetPosition(m_position);

}



void BowlingBall::AddForce(const XMFLOAT3& force)
{
    
    m_velocity.x += (force.x) / m_mass;
    m_velocity.y += (force.y) / m_mass;
    m_velocity.z += (force.z) / m_mass;

}

void BowlingBall::Reset(const XMFLOAT3& pos)
{
    m_position = pos;
    m_velocity = { 0,0,0 };
   
}

bool BowlingBall::IsStopped() const
{
    const float STOP_EPS = 0.05f;
    return
        fabs(m_velocity.x) < STOP_EPS &&
        fabs(m_velocity.y) < STOP_EPS &&
        fabs(m_velocity.z) < STOP_EPS &&
        m_onGround;
}



void BowlingBall::Draw()
{
    Shader3d_Begin();
    Direct3D_SetDepthWriteEnable();
    XMMATRIX mtxWorld = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    m_mtxWorld = mtxWorld;

    // モデル描画(通常の深度テスト)
    ModelDraw(g_pBall, mtxWorld);

    // ★パーティクルエフェクト描画前に深度書き込みを無効化
    //Direct3D_SetDepthWriteDisable();
  
    ParticleEffectParams fire = ParticleEffect_PresetFire();
    ParticleEffect_Draw(fire, mtxWorld);
  
    // ★深度書き込みを元に戻す
   // Direct3D_SetDepthWriteEnable();

    // デバッグ描画
    DebugDraw_AddAABB(m_Aabb, { 1.0f, 0.0f, 0.0f, 1.0f });
}
