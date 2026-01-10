#include "BowlingBall.h"
#include "model.h"
#include "direct3d.h"
#include "shader3d.h"
#include "DebugDraw.h" // 追加
#include "Map.h"
using namespace DirectX;
static AABB g_Aabb{};
static MODEL* g_pBall{};


BowlingBall::BowlingBall()
{
}

void BowlingBall::Init(const XMFLOAT3& pos)
{
    m_position = pos;
    m_velocity = { 0.0f, 0.0f ,0.0f};
    g_pBall = ModelLoad("rom\\Model\\ball.fbx", 0.1);
    g_Aabb = g_pBall->Aabb;
    g_Aabb = AABB::Make(m_position, { 0.3f,0.3f,0.3f });
}

void BowlingBall::Update(float deltaTime)
{
    // ===== 重力 =====
    m_velocity.y += m_gravity * deltaTime;

    // ===== 移動 =====
    m_position.x += m_velocity.x * deltaTime;
    m_position.y += m_velocity.y * deltaTime;
    m_position.z += m_velocity.z * deltaTime;

    g_Aabb.Move(m_position);

    m_onGround = false;

    // ===== Map 衝突 =====
    for (int i = 0; i < MapGetBlockCount(); ++i)
    {
        const AABB& block = GetCollision(i);
        Hit hit = g_Aabb.IsHit(block);

        if (!hit.IsHit()) continue;

        // 各軸の貫入深度を再計算（球側=AABB g_Aabb, ブロック=block）
        float depth_x =
            std::min(g_Aabb.GetMax().x, block.GetMax().x) - std::max(g_Aabb.GetMin().x, block.GetMin().x);
        float depth_y =
            std::min(g_Aabb.GetMax().y, block.GetMax().y) - std::max(g_Aabb.GetMin().y, block.GetMin().y);
        float depth_z =
            std::min(g_Aabb.GetMax().z, block.GetMax().z) - std::max(g_Aabb.GetMin().z, block.GetMin().z);

        // 最も浅い軸を選ぶ
        int shallow_axis = 0; // 0:x, 1:y, 2:z
        if (depth_x < depth_y) {
            shallow_axis = (depth_x < depth_z) ? 0 : 2;
        }
        else {
            shallow_axis = (depth_y < depth_z) ? 1 : 2;
        }

        const XMFLOAT3& ballCenter = g_Aabb.GetCenter();
        const XMFLOAT3& blockCenter = block.GetCenter();
        const XMFLOAT3& ballHalf = g_Aabb.GetHalfSize();

        // 軸ごとに押し戻す（半幅を使う）
        if (shallow_axis == 1) // Y
        {
            if (ballCenter.y > blockCenter.y)
            {
                // ボールはブロックの上にいる → ブロックの上面に乗せる
                m_position.y = block.GetMax().y + ballHalf.y;
                m_velocity.y = 0.0f;
                m_onGround = true;
            }
            else
            {
                // ボールはブロックの下にいる（押し上げられている）→ 下側へスナップ
                m_position.y = block.GetMin().y - ballHalf.y;
                m_velocity.y = 0.0f;
            }
        }
        else if (shallow_axis == 0) // X
        {
            if (ballCenter.x > blockCenter.x)
            {
                m_position.x = block.GetMax().x + ballHalf.x;
            }
            else
            {
                m_position.x = block.GetMin().x - ballHalf.x;
            }
            m_velocity.x = 0.0f;
        }
        else // Z
        {
            if (ballCenter.z > blockCenter.z)
            {
                m_position.z = block.GetMax().z + ballHalf.z;
            }
            else
            {
                m_position.z = block.GetMin().z - ballHalf.z;
            }
            m_velocity.z = 0.0f;
        }

        // 補正後に AABB を移動して次の判定に反映
        g_Aabb.Move(m_position);
    }

    // ===== 摩擦 =====
    if (m_onGround)
    {
        m_velocity.x *= 0.98f;
        m_velocity.z *= 0.98f;
    }
}



void BowlingBall::AddForce(const XMFLOAT3& force)
{
    
    m_velocity.x += (force.x) / m_mass;
    m_velocity.y += (force.y) / m_mass;
    m_velocity.z += (force.z) / m_mass;

}

void BowlingBall::Draw() 
{
    Shader3d_Begin();
    XMMATRIX mtxWorld = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

    // ここはDirect3D側で円 or モデル描画
    ModelDraw(g_pBall, mtxWorld);

    // デバッグ描画：AABB を赤いラインで表示
    DebugDraw_AddAABB(g_Aabb, {1.0f, 0.0f, 0.0f, 1.0f});
    DebugDraw_Draw();
}
