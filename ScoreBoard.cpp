// ScoreBoard.cpp
#include "ScoreBoard.h"
#include "debug_text.h"
#include "debug_ostream.h"
#include "sprite.h"
#include "texture.h"
#include "direct3d.h"

static int g_ScoreBoardTexId = -1;
static int g_NumberTexId = -1;
static constexpr int FRAME_COUNT = 4;
static constexpr float DIGIT_W = 64.0f;
static constexpr float DIGIT_H = 64.0f;
static constexpr int DIGIT_COLS = 5;
static constexpr float CELL_W = 200.0f;
static constexpr float CELL_H_NUM = 180.0f;   // 0〜9
static constexpr float CELL_H_SYM = 185.0f;   // X / -
constexpr float SCALE_THROW = 0.45f; // 上段（投球）
constexpr float SCALE_SUM = 0.55f; // 下段（合計）

static constexpr int SHEET_COLS = 5;
constexpr float startX = 0.0f;
constexpr float startY = 230.0f;
constexpr float frameW = 430.0f / FRAME_COUNT;
constexpr float throwSpacing = CELL_W * SCALE_THROW + 6.0f;
constexpr float lowerRowHeight = CELL_H_NUM * SCALE_THROW + 10.0f;

//constexpr float lowerRowHeight = 64.0f;

enum DigitSymbol
{
    DIGIT_0 = 0,
    DIGIT_1,
    DIGIT_2,
    DIGIT_3,
    DIGIT_4,
    DIGIT_5,
    DIGIT_6,
    DIGIT_7,
    DIGIT_8,
    DIGIT_9,
    SYMBOL_X,     // 10
    SYMBOL_SPARE, // 11
    SYMBOL_NONE,  // 12（空白）
};

struct Frame
{
    int throw1 = -1;
    int throw2 = -1;
    int frameScore = 0;
    
};

static Frame g_Frames[FRAME_COUNT];
static int g_CurrentFrame = 0;
static int g_CurrentThrow = 0;

void ScoreBoard_Initialize()
{
    g_ScoreBoardTexId = Texture_Load(L"rom\\Texture\\bord.jpg");
    g_NumberTexId     = Texture_Load(L"rom\\Texture\\Number.png");
    for (int i = 0; i < FRAME_COUNT; ++i)
    {
        g_Frames[i] = {};
    }
    g_CurrentFrame = 0;
    g_CurrentThrow = 0;
}

float GetSymbolSrcY(int symbol)
{
    if (symbol <= DIGIT_9)      return 0.0f;            // 0–9
    else if (symbol <= SYMBOL_SPARE) return CELL_H_NUM * 2; // X /
    else                         return CELL_H_NUM * 2; // 空白
}
float GetSymbolHeight(int symbol)
{
    if (symbol <= DIGIT_9) return CELL_H_NUM;
    else                  return CELL_H_SYM;
}


void DrawSymbol(
    int texId,
    int symbol,
    float x,
    float y,
    float scale
)
{
    if (symbol < 0 || symbol == SYMBOL_NONE) return;

    int col = symbol % SHEET_COLS;

    int row = 0;
    if (symbol >= DIGIT_0 && symbol <= DIGIT_4)
    {
        row = 0;
    }
    else if (symbol >= DIGIT_5 && symbol <= DIGIT_9)
    {
        row = 1;
    }
    else if (symbol == SYMBOL_X || symbol == SYMBOL_SPARE)
    {
        row = 2;
    }
    else
    {
        return;
    }

    float srcY = row * CELL_H_NUM;
    float srcH = (row == 2) ? CELL_H_SYM : CELL_H_NUM;

    Sprite_Draw(
        texId,
        x,
        y,
        CELL_W * scale,
        srcH * scale,
        col * CELL_W,
        srcY,
        CELL_W,
        srcH,
        { 0,0,0,1 }
    );
}


// Score_AddThrow の堅牢化:
// - pins を 0..10 にクランプ
// - 2投目がフレーム合計で 10 を超えないように調整（物理的に発生するときの保険）
// - 不整合が起きた場合、デバッグ出力で警告する
void Score_AddThrow(int pins)
{
    // デバッグログ追加：Score_AddThrow に渡される値を確実に出力する
    hal::dout << "Score_AddThrow called: pins=" << pins
              << " currentFrame=" << g_CurrentFrame
              << " currentThrow=" << g_CurrentThrow << std::endl;

    if (g_CurrentFrame >= FRAME_COUNT)
        return;

    // 基本的なクランプ
    if (pins < 0) pins = 0;
    if (pins > 10) {
        hal::dout << "  Warning: pins clamped from " << pins << " to 10" << std::endl;
        pins = 10;
    }

    Frame& f = g_Frames[g_CurrentFrame];

    // 1投目
    if (g_CurrentThrow == 0)
    {
        f.throw1 = pins;

        // デバッグ：割り当て後の状態
        hal::dout << "  Assigned throw1=" << f.throw1 << " (frame " << g_CurrentFrame << ")" << std::endl;

        // ストライクなら次フレーム（既存挙動）
        if (pins == 10)
        {
            // 明示的に throw2 は空のままにしておく
            f.throw2 = -1;
            g_CurrentFrame++;
            g_CurrentThrow = 0;
        }
        else
        {
            g_CurrentThrow = 1;
        }
    }
    // 2投目
    else
    {
        // 保険：1投目が既に記録されている前提で、合計が10を超えないようにする
        if (f.throw1 < 0) {
            // 想定外の状態だが保護する
            hal::dout << "  Warning: throw1 was not set for frame " << g_CurrentFrame << std::endl;
            f.throw1 = 0;
        }

        int maxSecond = 10 - f.throw1;
        if (pins > maxSecond) {
            hal::dout << "  Warning: second throw pins (" << pins << ") exceed frame limit, clamped to " << maxSecond << std::endl;
            pins = maxSecond;
        }

        f.throw2 = pins;

        hal::dout << "  Assigned throw2=" << f.throw2 << " (frame " << g_CurrentFrame << ")" << std::endl;

        g_CurrentFrame++;
        g_CurrentThrow = 0;
    }
}

void Score_Recalculate()
{
}

void ScoreBoard_Draw()
{
    // デバッグ：現在保持しているフレーム情報を毎フレーム出力（調査用）
    {
        std::ostringstream oss;
        oss << "ScoreBoard frames:";
        for (int i = 0; i < FRAME_COUNT; ++i) {
            oss << " [" << i << "](" << g_Frames[i].throw1 << "," << g_Frames[i].throw2 << ")";
        }
        hal::dout << oss.str() << std::endl;
    }

    Sprite_Draw(
        g_ScoreBoardTexId,
        0.0f,
        230.0f, 145.0f,
        430.0f, 145.0f,
        0.0f, 0.0f,
        430.0f, 145.0f);

    constexpr float startX = 0.0f;
    constexpr float startY = 100.0f;
    constexpr float frameW = 430.0f / FRAME_COUNT;
    constexpr float throwSpacing = 11.0f;
    constexpr float lowerRowHeight = 64.0f;
    constexpr float THROW_SCALE = 0.3f;
    float throwDigitW = CELL_W * THROW_SCALE;

    for (int i = 0; i < FRAME_COUNT; ++i)
    {
        const Frame& f = g_Frames[i];
        float frameX = startX + frameW * i;

        // --- 1投目 ---
        if (f.throw1 >= 0)
        {
            int symbol =
                (f.throw1 == 10) ? SYMBOL_X :
                f.throw1;



            DrawSymbol(
                g_NumberTexId,
                symbol,
                frameX + 16.0f,
                startY,
                0.3f
            );



        }

        // --- 2投目 ---
        if (f.throw2 >= 0)
        {
            int symbol =
                (f.throw1 < 10 && f.throw1 + f.throw2 == 10)
                ? SYMBOL_SPARE
                : f.throw2;

            DrawSymbol(
                g_NumberTexId,
                symbol,
                frameX + 5.0f + throwDigitW + 6.0f,
                startY,
                0.3f
            );
        }

        // --- フレーム合計 ---
        if (f.throw1 >= 0)
        {
            int sum = f.throw1 + (f.throw2 >= 0 ? f.throw2 : 0);

            constexpr float SUM_SCALE = 0.32f;
            float digitW = CELL_W * SUM_SCALE;

            // 10の位：10未満は表示しない（現在は先頭に不要な 0 が出ていたため）
            if (sum >= 10)
            {
                DrawSymbol(
                    g_NumberTexId,
                    sum / 10,
                    frameX + 24.0f,
                    startY + lowerRowHeight,
                    SUM_SCALE
                );
            }

            // 1の位（常に表示）
            DrawSymbol(
                g_NumberTexId,
                sum % 10,
                frameX + 24.0f + (sum >= 10 ? (digitW - 30) : 0.0f),
                startY + lowerRowHeight,
                SUM_SCALE
            );
        }

    }
}
