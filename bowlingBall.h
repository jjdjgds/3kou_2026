/*==========================================================================
ボーリングの球のクラス定義
	


													Author : hidetoshi muramatu
													Date   : 2026/01/10
---------------------------------------------------------------------------



==========================================================================*/

#pragma once
#include <DirectXMath.h>
#include "Collision.h"
class BowlingBall
{
public:
    BowlingBall();

    // 初期化
    void Init(const DirectX::XMFLOAT3& pos);

    // 更新
    void Update(float deltaTime);

    // 描画（中身はあとで）
    void Draw() ;

    // 操作用
    void AddForce(const DirectX::XMFLOAT3& force);

	DirectX::XMMATRIX GetWorldMatrix() const { return m_mtxWorld; }
    void Reset(const DirectX::XMFLOAT3& pos);
    bool IsStopped() const;

    // 取得
  //  const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
    float GetRadius() const { return m_radius; }
    const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
    const DirectX::XMFLOAT3& GetVelocity() const { return m_velocity; }
    float GetMass() const { return m_mass; }
    const AABB& GetAABB() { return m_Aabb; }
private:
    DirectX::XMFLOAT3 m_position{};
    DirectX::XMFLOAT3 m_velocity{};
    float m_gravity = -9.8f;
    bool  m_onGround = false;
    float m_pawer = 50.0f;
     AABB m_Aabb{};
    float m_radius = 0.3f;   // レーン基準の半径
    float m_mass = 6.8f;    // ボーリング球っぽい重さ(kg)
    DirectX::XMMATRIX m_mtxWorld{};
    bool m_gatterFlg = false;
};
