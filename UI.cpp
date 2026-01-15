#include "UI.h"
#include "sprite.h"
#include "texture.h"
#include <algorithm>

using namespace DirectX;

namespace UI
{
    // テクスチャID
    static int g_StrikeTex = -1;
    static int g_SpareTex = -1;
    static int g_GutterTex = -1;

    // 現在の通知状態
    static NotificationType g_CurrentNotification = NotificationType::NONE;

    // アニメーション用変数
    static float g_AnimTimer = 0.0f;           // アニメーション経過時間
    static float g_DisplayDuration = 2.0f;     // 表示時間（秒）
    static float g_FadeInDuration = 0.5f;      // フェードイン時間（秒）
    static float g_FadeOutDuration = 0.5f;     // フェードアウト時間（秒）

    // イージング関数（EaseOutCubic）
    float EaseOutCubic(float t)
    {
        return 1.0f - powf(1.0f - t, 3.0f);
    }

    // イージング関数（EaseInCubic）
    float EaseInCubic(float t)
    {
        return t * t * t;
    }

    // イージング関数（EaseOutBack - オーバーシュート効果）
    float EaseOutBack(float t)
    {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
    }

    void Initialize()
    {
        g_StrikeTex = Texture_Load(L"rom\\Texture\\Strike.png");
        g_SpareTex = Texture_Load(L"rom\\Texture\\Spare.png");
        g_GutterTex = Texture_Load(L"rom\\Texture\\Gutter.png");

        g_CurrentNotification = NotificationType::NONE;
        g_AnimTimer = 0.0f;
    }

    void Update(double dt)
    {
        // 通知が表示中の場合
        if (g_CurrentNotification != NotificationType::NONE)
        {
            g_AnimTimer += static_cast<float>(dt);

            // 全体の表示時間を超えたら非表示に
            float totalDuration = g_FadeInDuration + g_DisplayDuration + g_FadeOutDuration;
            if (g_AnimTimer >= totalDuration)
            {
                g_CurrentNotification = NotificationType::NONE;
                g_AnimTimer = 0.0f;
            }
        }
    }

    void Draw()
    {
        // 通知が無い場合は何も描画しない
        if (g_CurrentNotification == NotificationType::NONE)
            return;

        // 使用するテクスチャを選択
        int texId = -1;
        switch (g_CurrentNotification)
        {
        case NotificationType::STRIKE:
            texId = g_StrikeTex;
            break;
        case NotificationType::SPARE:
            texId = g_SpareTex;
            break;
        case NotificationType::GUTTER:
            texId = g_GutterTex;
            break;
        default:
            return;
        }

        if (texId == -1)
            return;

        // アニメーション段階を判定
        float alpha = 1.0f;
        float scale = 1.0f;
        float totalDuration = g_FadeInDuration + g_DisplayDuration + g_FadeOutDuration;

        if (g_AnimTimer < g_FadeInDuration)
        {
            // フェードイン段階
            float t = g_AnimTimer / g_FadeInDuration;
            alpha = EaseOutCubic(t);
            scale = EaseOutBack(t); // オーバーシュートでポップな感じに
        }
        else if (g_AnimTimer < g_FadeInDuration + g_DisplayDuration)
        {
            // 表示段階（完全表示）
            alpha = 1.0f;
            scale = 1.0f;
        }
        else
        {
            // フェードアウト段階
            float t = (g_AnimTimer - g_FadeInDuration - g_DisplayDuration) / g_FadeOutDuration;
            alpha = 1.0f - EaseInCubic(t);
            scale = 1.0f + EaseInCubic(t) * 0.2f; // 少し拡大しながらフェードアウト
        }

        // 画面中央に描画
        const float baseWidth = 512.0f;   // 画像の基本幅
        const float baseHeight = 256.0f;  // 画像の基本高さ
        const float screenCenterX = 960.0f; // 画面中央X（解像度に応じて調整）
        const float screenCenterY = 360.0f; // 画面中央Y（解像度に応じて調整）

        float width = baseWidth * scale;
        float height = baseHeight * scale;

        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, alpha };

        // 中央揃えで描画
        Sprite_DrawCenter(
            texId,
            screenCenterX,
            screenCenterY,
            width,
            height,
            color
        );
    }

    void Finalize()
    {
        // テクスチャの解放は Texture システムが行うため、ここでは何もしない
        g_CurrentNotification = NotificationType::NONE;
        g_AnimTimer = 0.0f;
    }

    void ShowNotification(NotificationType type)
    {
        // 新しい通知を表示
        g_CurrentNotification = type;
        g_AnimTimer = 0.0f;
    }
}