#include "Shot.h"
#include "model.h"
#include "mouse.h"
#include "keylogger.h"
#include "Mouselogger.h"

using namespace DirectX;

static XMFLOAT3 g_position;
static constexpr float OFFSET_LENGTH = 0.5f;

static XMFLOAT3 g_Front{ 0,0,1 };   // 単位方向
static MODEL* g_Model{};

static float g_Power = 0.0f;

// ===== マウススイング用 =====
static POINT g_PrevMousePos{};
static bool  g_IsSwinging = false;
static float g_SwingAccum = 0.0f;
// 定数
static constexpr float DIR_SMOOTH = 0.15f;

float Shot_GetPower()
{
    return g_Power;
}

void Shot_ResetPower()
{
    g_Power = 0.0f;
    g_SwingAccum = 0.0f;
    g_Front = { 0,0,1 };   // ★これ必須
    GetCursorPos(&g_PrevMousePos);
}

void Shot_Initialize(const XMFLOAT3& position, const XMFLOAT3& front)
{
    g_position = position;
    XMStoreFloat3(&g_Front, XMVector3Normalize(XMLoadFloat3(&front)));
    g_Model = ModelLoad("rom\\Model\\yajirushi.fbx", 0.1f);

    GetCursorPos(&g_PrevMousePos);
}

void Shot_Finalize()
{
    ModelRelease(g_Model);
}

void Shot_Update(double)
{
    POINT cur;
    GetCursorPos(&cur);

    float dx = float(cur.x - g_PrevMousePos.x);
    float dy = float(cur.y - g_PrevMousePos.y);
    g_PrevMousePos = cur;

    // ===== 左ボタン押下中：スイング =====
    if (MouseLogger_IsPressed(MouseKey::Left))
    {
        if (!g_IsSwinging)
        {
            g_IsSwinging = true;
            g_SwingAccum = 0.0f;
        }

        // スイング量（縦のみ）
        float swing = fabsf(dy);
        g_SwingAccum += swing;

        // ===== 向き更新（平滑化あり）=====
        float yaw = dx * 0.002f;
        float pitch = -dy * 0.002f;

        XMVECTOR currentFront = XMLoadFloat3(&g_Front);
        XMVECTOR targetFront = currentFront;

        // Yaw
        targetFront = XMVector3TransformNormal(
            targetFront,
            XMMatrixRotationY(yaw)
        );

        // Pitch
        XMVECTOR right = XMVector3Normalize(
            XMVector3Cross(XMVectorSet(0, 1, 0, 0), targetFront)
        );

        targetFront = XMVector3TransformNormal(
            targetFront,
            XMMatrixRotationAxis(right, pitch)
        );

        targetFront = XMVector3Normalize(targetFront);

        // ★ 平滑化（ここが本命）
        XMVECTOR smoothFront =
            XMVectorLerp(currentFront, targetFront, DIR_SMOOTH);

        smoothFront = XMVector3Normalize(smoothFront);
        XMStoreFloat3(&g_Front, smoothFront);
    }
    // ===== 離した瞬間：投擲確定 =====
    else if (g_IsSwinging)
    {
        g_Power = Clamp(g_SwingAccum * 0.05f, 5.0f, 50.0f);
        g_IsSwinging = false;
    }
}

float Clamp(float v, float minV, float maxV)
{
    if (v < minV) return minV;
    if (v > maxV) return maxV;
    return v;
}
void Shot_Draw()
{
    XMMATRIX rot = XMMatrixLookAtLH(
        XMVectorZero(),
        XMLoadFloat3(&g_Front),
        XMVectorSet(0, 1, 0, 0)
    );

    XMMATRIX world =
        XMMatrixTranslation(0, 0, OFFSET_LENGTH) *
        XMMatrixTranspose(rot) *
        XMMatrixTranslation(g_position.x, g_position.y, g_position.z);

    ModelDraw(g_Model, world);
}

const XMFLOAT3& Shot_GetVelocity()
{
    return g_Front;
}

void Shot_SetPosition(const XMFLOAT3& position)
{
    g_position = position;
}

