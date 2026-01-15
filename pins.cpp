#include "pins.h"
#include "cube.h"
#include "DebugDraw.h"
#include "trail_explosion.h"
using namespace DirectX;

static constexpr float GRAVITY = -9.8f;
static constexpr float FLOOR_Y = 0.0f;
// 追加: X/Z 移動で倒れたと判定する閾値（調整可）
static constexpr float POSITION_MOVE_THRESHOLD = 0.15f;
// 少しでも回転しているとみなす角速度（調整用）
static constexpr float ANGULAR_VELOCITY_THRESHOLD = 0.3f;

Pins::Pins()
    : m_position{ 0,0,0 }
    , m_rotation{ 0,0,0 }
    , m_velocity{ 0,0,0 }
    , m_angularVelocity{ 0,0,0 }
    , m_isDown(false)
    , m_mass(9.8f)
{
}

void Pins::Initialize(const XMFLOAT3& pos)
{
    m_position = pos;
    m_rotation = { 0,0,0 };
    m_velocity = { 0,0,0 };
    m_scale = { 1,1,1 };
    m_angularVelocity = { 0,0,0 };
    m_isDown = false;
    m_isDead = false;
    m_isHit = false;
    m_aliveTime = 0.0f;
    m_lifeTimer = 0.0f;
    
    m_model = ModelLoad("rom\\Model\\Pin.fbx", 0.9);
        //m_model = ModelLoad("rom\\Model\\bowlingBall1.fbx", 0.9);
    m_baseHalf = { 1.5f, 3.f, 1.5f };

    m_Aabb = AABB::Make(m_position, m_baseHalf);

    // 生成時の基準位置を保存（X/Z の動きをここから判定する）
    m_spawnPosition = pos;
}

void Pins::Update(float deltaTime)
{
    if (m_isDead) {
        // 死亡扱いなら物理更新を止める（オフスクリーンに移動済み）
        // ただし AABB は位置に合わせて更新しておく
        XMFLOAT3 half =
        {
            m_baseHalf.x * m_scale.x,
            m_baseHalf.y * m_scale.y,
            m_baseHalf.z * m_scale.z
        };
        XMFLOAT3 aabbCenter = m_position;
        aabbCenter.y += half.y;
        m_Aabb.SetCenter(half);
        m_Aabb.Move(aabbCenter);
        return;
    }

    // 通常の物理更新
    m_velocity.y += GRAVITY * deltaTime;

    m_position.x += m_velocity.x * deltaTime;
    m_position.y += m_velocity.y * deltaTime;
    m_position.z += m_velocity.z * deltaTime;

    m_rotation.x += m_angularVelocity.x * deltaTime;
    m_rotation.y += m_angularVelocity.y * deltaTime;
    m_rotation.z += m_angularVelocity.z * deltaTime;

    if (m_position.y < FLOOR_Y)
    {
        m_position.y = FLOOR_Y;
        m_velocity.y *= -0.2f;
        m_velocity.x *= 0.9f;
        m_velocity.z *= 0.9f;

        m_angularVelocity.x *= 0.9f;
        m_angularVelocity.z *= 0.9f;
    }

    // 回転による倒れ判定（従来通り）
    if (fabsf(m_rotation.x) > XM_PIDIV4 || fabsf(m_rotation.z) > XM_PIDIV4)
    {
        m_isDown = true;
    }

    if (!m_isDown)
    {
        float angSpeed =
            fabsf(m_angularVelocity.x) +
            fabsf(m_angularVelocity.y) +
            fabsf(m_angularVelocity.z);

        if (angSpeed > ANGULAR_VELOCITY_THRESHOLD)
        {
            m_isDown = true;
        }
    }


    if (m_isDown)
    {
        m_aliveTime += deltaTime;
        if (m_aliveTime > 1.0f)
        {
            m_isDead = true;
        }
    }
    if (m_isHit)
    {
        m_lifeTimer += deltaTime;
        if (m_lifeTimer > 1.0f)
        {
            m_isDead = true;
        }
    }

    XMFLOAT3 half =
    {
        m_baseHalf.x * m_scale.x,
        m_baseHalf.y * m_scale.y,
        m_baseHalf.z * m_scale.z
    };

    XMFLOAT3 aabbCenter = m_position;
    aabbCenter.y += half.y;

    m_Aabb.SetCenter(half);
    m_Aabb.Move(aabbCenter);
}

void Pins::Draw()
{
    if (m_isDead) {
        // オフスクリーンに移動済みなら描画しない
        return;
    }

    float drawOffsetY = m_baseHalf.y * m_scale.y;

    XMMATRIX mtxworld =
        XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
        XMMatrixRotationX(m_rotation.x) *
        XMMatrixRotationY(m_rotation.y) *
        XMMatrixRotationZ(m_rotation.z) *
        XMMatrixTranslation(
            m_position.x,
            m_position.y + drawOffsetY,
            m_position.z
        );
    ModelDraw(m_model, mtxworld);
   // Cube_Draw(mtxworld, 0);
    DebugDraw_AddAABB(m_Aabb, { 1,1,0,1 });
}

void Pins::Hit(const XMFLOAT3& impulse, const XMFLOAT3& hitPoint)
{
    if (!m_isHit)
    {
        m_isHit = true;
        m_lifeTimer = 0.0f;
    }

    m_velocity.x += impulse.x / m_mass;
    m_velocity.y += impulse.y / m_mass;
    m_velocity.z += impulse.z / m_mass;

    XMVECTOR r = XMLoadFloat3(&hitPoint) - XMLoadFloat3(&m_position);
    XMVECTOR f = XMLoadFloat3(&impulse);

    XMVECTOR torque = XMVector3Cross(r, f);

    XMFLOAT3 t;
    XMStoreFloat3(&t, torque);

    m_angularVelocity.x += t.x * 3.f;
    m_angularVelocity.y += t.y * 3.f;
    m_angularVelocity.z += t.z * 3.f;


  

    
      
    

    

}

bool Pins::IsDown() const
{
    return m_isDown;
}

// 新: 指定X位置へ移動し死亡扱いにする
void Pins::MoveOffscreen(float x)
{
    m_position.x = x;
    m_velocity = { 0,0,0 };
    m_angularVelocity = { 0,0,0 };
    m_isDead = true;

    // AABB を位置に合わせる
    XMFLOAT3 half =
    {
        m_baseHalf.x * m_scale.x,
        m_baseHalf.y * m_scale.y,
        m_baseHalf.z * m_scale.z
    };
    XMFLOAT3 aabbCenter = m_position;
    aabbCenter.y += half.y;
    m_Aabb.SetCenter(half);
    m_Aabb.Move(aabbCenter);
}

// 新: 明示的に位置をセットする
void Pins::SetPosition(const XMFLOAT3& pos)
{
    m_position = pos;
    XMFLOAT3 half =
    {
        m_baseHalf.x * m_scale.x,
        m_baseHalf.y * m_scale.y,
        m_baseHalf.z * m_scale.z
    };
    XMFLOAT3 aabbCenter = m_position;
    aabbCenter.y += half.y;
    m_Aabb.SetCenter(half);
    m_Aabb.Move(aabbCenter);
}
