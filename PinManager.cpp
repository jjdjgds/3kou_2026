#include "PinManager.h"
#include "bowlingBall.h"
#include "trail_explosion.h"
using namespace DirectX;
static constexpr float PIN_SPACING_X = 1.6f;   // 横間隔
static constexpr float PIN_SPACING_Z = 1.6f;   // 奥行間隔

void PinManager::Initialize()
{
    ResetPins();
}

void PinManager::Draw()
{
    for (auto& p : m_pins)
        p.Draw();
}


void PinManager::Update(float dt, BowlingBall& ball)
{
    // ① 個別更新
    for (auto& p : m_pins)
        p.Update(dt);

    // ② Ball vs Pin
    for (auto& p : m_pins)
    {
        if (p.IsDown()) continue;

        Hit hit = ball.GetAABB().IsHit(p.GetAABB());
        if (!hit.IsHit()) continue;

        ResolveBallPinHit(ball, p);
    }

    // ③ Pin vs Pin
    for (size_t i = 0; i < m_pins.size(); ++i)
    {
        for (size_t j = i + 1; j < m_pins.size(); ++j)
        {
            Hit hit = m_pins[i].GetAABB().IsHit(m_pins[j].GetAABB());
            if (!hit.IsHit()) continue;

            ResolvePinPin(m_pins[i], m_pins[j]);
        }
    }
   
}

void PinManager::AddPin(const XMFLOAT3& pos)
{
    Pins pin;
    pin.Initialize(pos);
    m_pins.emplace_back(pin);
}

void PinManager::ResolvePinPin(Pins& a, Pins& b)
{
    // 方向
    XMVECTOR dir =
        XMLoadFloat3(&b.GetPosition()) -
        XMLoadFloat3(&a.GetPosition());

    dir = XMVector3Normalize(dir);

    // 速度差
    XMFLOAT3 va = a.GetVelocity();
    XMFLOAT3 vb = b.GetVelocity();

    XMVECTOR relVel =
        XMLoadFloat3(&va) - XMLoadFloat3(&vb);

    float speed = XMVectorGetX(XMVector3Dot(relVel, dir));
    if (speed > 0.0f) return;

    float impulse = speed * 0.5f;

    XMFLOAT3 f;
    XMStoreFloat3(&f, dir * impulse);

    a.Hit({ -f.x, -f.y, -f.z }, a.GetPosition());
    b.Hit({ f.x,  f.y,  f.z }, b.GetPosition());
}
void PinManager::ResolveBallPinHit(BowlingBall& ball, Pins& pin)
{
    // 球 → ピン方向
    XMVECTOR dir =
        XMLoadFloat3(&pin.GetPosition()) -
        XMLoadFloat3(&ball.GetPosition());
    dir = XMVector3Normalize(dir);

    // ボール速度
    XMFLOAT3 ballVel = ball.GetVelocity();
    XMVECTOR v = XMLoadFloat3(&ballVel);

    // 正面衝突成分のみ
    float speed = XMVectorGetX(XMVector3Dot(v, dir));
    if (speed <= 0.0f) return;

    float impulsePower = speed * ball.GetMass() * 1.2f;
    XMVECTOR impulse = dir * impulsePower;

    // 少し上を叩く
    XMFLOAT3 hitPoint = pin.GetPosition();
    hitPoint.y += 1.0f;

    XMFLOAT3 impulseVec;
    XMStoreFloat3(&impulseVec, impulse);

    // ★ 上方向ボーナス
    impulseVec.y += impulsePower;

    pin.Hit(impulseVec, hitPoint);
    // 放射状爆発
    TrailExplosion_CreateRadial(pin.GetPosition(), 0.5f);

}

// 変更: RemoveDeadPins は消すのではなくオフスクリーン移動に変更
void PinManager::RemoveDeadPins()
{
    for (auto& p : m_pins)
    {
        if (p.IsDead())
        {
            // オフスクリーンに移動して描画しないようにする
            p.MoveOffscreen(100.0f);
        }
    }
}

// 既存の GetRemainingPinCount を「生きているピン数」を返す実装にしてある想定
int PinManager::GetRemainingPinCount() const
{
    int remaining = 0;
    for (const auto& p : m_pins)
    {
        if (!p.IsDead())
            ++remaining;
    }
    return remaining;
}

int PinManager::GetInitialPinCount() const
{
    return 10; // 今は固定でOK
}

void PinManager::SetupBowlingPins(const XMFLOAT3& headPinPos)
{
    m_pins.clear();

    static constexpr float DX = 1.6f;
    static constexpr float DZ = 1.6f;

    // 1段目（1本）
    AddPin(headPinPos);

    // 2段目（2本）
    AddPin({ headPinPos.x - DX * 0.5f, headPinPos.y, headPinPos.z + DZ });
    AddPin({ headPinPos.x + DX * 0.5f, headPinPos.y, headPinPos.z + DZ });

    // 3段目（3本）
    AddPin({ headPinPos.x - DX, headPinPos.y, headPinPos.z + DZ * 2 });
    AddPin({ headPinPos.x,       headPinPos.y, headPinPos.z + DZ * 2 });
    AddPin({ headPinPos.x + DX, headPinPos.y, headPinPos.z + DZ * 2 });

    // 4段目（4本）
    AddPin({ headPinPos.x - DX * 1.5f, headPinPos.y, headPinPos.z + DZ * 3 });
    AddPin({ headPinPos.x - DX * 0.5f, headPinPos.y, headPinPos.z + DZ * 3 });
    AddPin({ headPinPos.x + DX * 0.5f, headPinPos.y, headPinPos.z + DZ * 3 });
    AddPin({ headPinPos.x + DX * 1.5f, headPinPos.y, headPinPos.z + DZ * 3 });
}

void PinManager::ResetPins()
{
    // ヘッドピン基準位置
    SetupBowlingPins({ 5.f, 5.0f, 10.0f });
}

int PinManager::GetDownPinCount() const
{
    int down = 0;
    for (const auto& p : m_pins)
    {
        if (p.IsDown() || p.IsDead())
            down++;
    }
    return down;
}

// 新: 1フレーム（ボウリングのフレーム）終わりで全部破棄して再配置する関数
void PinManager::DestroyAndRecreatePins()
{
    m_pins.clear();
    SetupBowlingPins({ 5.f, 5.0f, 10.0f });
}

