#include "Shot.h"
#include "model.h"
#include "mouse.h"
#include "keylogger.h"
#include "Mouselogger.h"
#include "debug_ostream.h"
#include "sprite.h"
#include "texture.h"
#include "debug_text.h"
#include "direct3d.h"
using namespace DirectX;

static XMFLOAT3 g_position;
static XMFLOAT3 g_scale;

static constexpr float OFFSET_LENGTH = 5.f;

static XMFLOAT3 g_Front{ 0,0,1 };   // 単位方向
static MODEL* g_Model{};

static float g_Power = 0.0f;
static hal::DebugText* g_DebugText = nullptr;

const float cx = 300.0f;
const float cy = 600.0f;

const float charW = 32.0f;
const float charH = 32.0f;





static float g_AimTime = 0.0f;
static constexpr float SWING_SPEED = 3.0f;   // 揺れ速度
static constexpr float MAX_ANGLE = XM_PIDIV4; // ±45°
static float g_AimAngle = 0.0f;
static constexpr float BALL_RADIUS = -0.2f;
static bool g_IsCharging = false;
static int g_PowerBarTex = -1; // 白1pxテクスチャなど
static constexpr float MAX_POWER = 100.0f;
static float g_ChargeRatio = 0.0f;   // 0～1
static float g_ChargeDir = 1.0f;   // +1 or -1
static constexpr float CHARGE_SPEED = 1.5f; // 往復スピード
static float g_UIBlinkTime = 0.0f; // MAX時点滅用

float Shot_GetPower()
{
    return g_Power;
}
bool Shot_IsFired()
{
    return g_State == ShotState::Fired;
}

XMFLOAT3 Shot_GetShotVelocity()
{
    return {
        g_Front.x * g_Power,
        g_Front.y * g_Power,
        g_Front.z * g_Power
    };
}

void Shot_ResetPower()
{
    g_Power = 0.0f;
    g_AimTime = 0.0f;
    g_Front = { 0,0,1 };

    g_ChargeRatio = 0.0f;
    g_ChargeDir = 1.0f;

    g_IsCharging = true;   // ★ ここが重要
    g_State = ShotState::Charge;
}





void Shot_Initialize(const XMFLOAT3& position, const XMFLOAT3& front)
{
    g_position = position;
    XMStoreFloat3(&g_Front, XMVector3Normalize(XMLoadFloat3(&front)));
    g_Model = ModelLoad("rom\\Model\\yajirushi.fbx", 0.1f);
    g_PowerBarTex = Texture_Load(L"rom/white_8x8.png");
    g_IsCharging = true;
    
    //GetCursorPos(&g_PrevMousePos);
    // ===== DebugText 初期化 =====
    g_DebugText = new hal::DebugText(
        Direct3D_GetDevice(),          // ← あなたの環境の device
        Direct3D_GetContext(),         // ← context
        L"rom/consolab_ascii_512.png",      // フォントテクスチャ
        (float)Direct3D_GetBackBufferWidth(),
        (float)Direct3D_GetBackBufferHeight(),
        260.0f,               // 表示位置X
        520.0f                // 表示位置Y（円ゲージ中央付近）
    );
    Shot_SetPosition(g_position);
    g_scale = { 3,3,3 };
}

void Shot_Finalize()
{
    ModelRelease(g_Model);

}

void Shot_Update(double et)
{
    POINT cur;
    GetCursorPos(&cur);
    Shot_SetPosition(g_position);
   // float dx = float(cur.x - g_PrevMousePos.x);
    //float dy = float(cur.y - g_PrevMousePos.y);
    //g_PrevMousePos = cur;

    if (g_State == ShotState::Charge)
    {
        // Pキーでチャージ開始
        if (KeyLogger_IsTrigger(KK_P) && !g_IsCharging)
        {
            
            g_ChargeRatio = 0.0f;
            g_ChargeDir = 1.0f;
        }

        if (g_IsCharging)
        {
            // 往復更新
            g_ChargeRatio += g_ChargeDir * CHARGE_SPEED * (float)et;

            if (g_ChargeRatio >= 1.0f)
            {
                g_ChargeRatio = 1.0f;
                g_ChargeDir = -1.0f;
            }
            else if (g_ChargeRatio <= 0.0f)
            {
                g_ChargeRatio = 0.0f;
                g_ChargeDir = 1.0f;
            }

            // ★ ここで確定
            if (KeyLogger_IsTrigger(KK_P))
            {
                g_Power = Clamp(
                    g_ChargeRatio * MAX_POWER,
                    5.0f,
                    MAX_POWER
                );
                g_Power *= 1.5f;
                g_IsCharging = false;
                g_State = ShotState::Aim;
            }
        }

    }


    if (g_State == ShotState::Aim)
    {
        g_AimTime += (float)et;

        g_AimAngle = sinf(g_AimTime * SWING_SPEED) * MAX_ANGLE;

        XMMATRIX rotY = XMMatrixRotationY(g_AimAngle);
        XMVECTOR base = XMVectorSet(0, 0, 1, 0);

        XMVECTOR dir = XMVector3TransformNormal(base, rotY);
        XMStoreFloat3(&g_Front, XMVector3Normalize(dir));

        if (KeyLogger_IsTrigger(KK_O))
        {
            g_State = ShotState::Fired;
        }
    }

    bool isMax = (g_ChargeRatio >= 0.98f);

    if (isMax)
    {
        g_UIBlinkTime += (float)et;
    }
    else
    {
        g_UIBlinkTime = 0.0f;
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
   
    if (g_State != ShotState::Aim)
        return;

    XMMATRIX rot = XMMatrixRotationY(g_AimAngle);

    XMVECTOR offset =
        XMVector3TransformCoord(
            XMVectorSet(0, 0, OFFSET_LENGTH, 0),
            rot
        );

    XMFLOAT3 ofs;
    XMStoreFloat3(&ofs, offset);
    //hal::dout << g_position.z + ofs.z;
    XMMATRIX world =
        rot *
        XMMatrixScaling(g_scale.x, g_scale.y, g_scale.z) *
        XMMatrixTranslation(
            g_position.x + ofs.x,
            g_position.y - BALL_RADIUS,
            g_position.z + ofs.z
        );

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

float Shot_GetChargeRatio()
{
    return Clamp(g_ChargeRatio, 0.0f, 1.0f);
}



bool Shot_IsCharging()
{
    return g_IsCharging;
}

void Shot_DrawUI()
{
    if (g_State != ShotState::Charge &&
        g_State != ShotState::Aim)
        return;

    float ratio = Shot_GetChargeRatio();

    // ================= 円ゲージ =================
    const float cx = 300.0f;
    const float cy = 600.0f;
    const float radius = 200.0f;
    const float thickness = 50.0f;
    const int   SEGMENTS = 144;

    int drawCount = (int)(SEGMENTS * ratio);
    if (drawCount < 1 && ratio > 0.0f)
        drawCount = 1;

    XMFLOAT4 activeColor(
        1.0f,
        0.3f + 0.7f * ratio,
        0.1f,
        1.0f
    );

    for (int i = 0; i < SEGMENTS; ++i)
    {
        float t = (float)i / (float)SEGMENTS;
        float angle = t * XM_2PI - XM_PIDIV2;

        float x = cx + cosf(angle) * radius;
        float y = cy + sinf(angle) * radius;

        bool filled = (i < drawCount);

        Sprite_Draw(
            g_PowerBarTex,
            angle,
            x, y,
            thickness,
            thickness * 2.5f,
            0, 0, 8, 8,
            filled
            ? activeColor
            : XMFLOAT4(0.2f, 0.2f, 0.2f, 0.4f)
        );
    }

    // ================= 数値表示 =================
    if (!g_DebugText)
        return;

    int powerValue = (int)(ratio * MAX_POWER + 0.5f);

    char value[8];
    sprintf_s(value, "%3d", powerValue);

    constexpr float charW = 32.0f;
    constexpr float charH = 32.0f;

    float valueX = cx - charW * 5.f;
    float valueY = cy - charH * 0.3f;

    g_DebugText->Clear();
    g_DebugText->SetScale(3.f);
    g_DebugText->SetOffset(valueX, valueY);
    g_DebugText->SetText(value, { 1,1,1,1 });
    g_DebugText->Draw();
}


bool Shot_ConsumeFire()
{
    if (g_State == ShotState::Fired)
    {
        g_State = ShotState::Charge;
        g_IsCharging = true;
        return true;
    }
    return false;
}



