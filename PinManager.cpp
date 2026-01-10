#include "PinManager.h"
#include "bowlingBall.h"
using namespace DirectX;

void PinManager::Initialize()
{
    m_pins.clear();

    // ★ ボウリングの基本配置例（簡略）
    AddPin({ 4.f, 5.0f, 10.0f });
    AddPin({ 3.f, 5.0f, 13.0f });
    AddPin({ 0.8f, 0.0f, 1.0f });
    AddPin({ -1.6f, 0.0f, 2.0f });
    AddPin({ 0.0f, 0.0f, 2.0f });
    AddPin({ 1.6f, 0.0f, 2.0f });
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
    impulseVec.y += impulsePower * 1.5f;

    pin.Hit(impulseVec, hitPoint);

}
