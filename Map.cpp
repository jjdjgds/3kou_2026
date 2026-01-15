#include "Map.h"
#include "cube.h"
#include "texture.h"
#include <DirectXMath.h>
#include "model.h"
#include "DebugDraw.h"
using namespace DirectX;

constexpr int TEXTURE_MAX = 100;
static int g_MapTexId[TEXTURE_MAX]{};
static constexpr int MODEL_MAX = 8;
static MODEL* g_MapModels[MODEL_MAX]{};
constexpr int   LANE_COUNT = 5;     // レーン数

constexpr int LEFT_LANES = 2;
constexpr int RIGHT_LANES = 2;
constexpr float LANE_WIDTH = 12.0f;
constexpr float BASE_X = 0.0f;
std::vector<Block> g_Blocks;



const Block& MapGetBlock(int index)
{
    return g_Blocks[index];
}

int MapGetBlockCount()
{
    return static_cast<int>(g_Blocks.size());
}

const AABB& GetCollision(int index)
{
    return g_Blocks[index].GetAABB();
}

void Map_Initialize()
{
   
    //g_Blocks.push_back({ {4.0, 1.0f,10.0},{8.0,1.0,60.0}, Block::Wood });
    //g_Blocks.push_back({ {-0.50, 0.9f,-10.0},{1.0,1.0,60.0}, Block::Gutter});
    //g_Blocks.push_back({ {8.50, 0.9f,-10.0},{1.0,1.0,60.0}, Block::Gutter });
    //g_Blocks.push_back({ {3, 1.0f,32.0},{15.0,10.0,30.0}, Block::WALL });
    //g_Blocks.push_back({ {3, 1.0f,32.0},{15.0,10.0,30.0}, Block::Moon});

    //// ===== ガーター外側の進入禁止壁 =====
    //constexpr float wallHeight = 2.0f;
    //constexpr float wallThickness = 0.5f;
    //constexpr float wallLength = 60.0f;

    //constexpr float zPos = -10.0f;

    //// ガーター位置
    //constexpr float leftGutterX = -0.50f;
    //constexpr float rightGutterX = 8.50f;

    //// 壁はガーターの外側
    //constexpr float leftWallX = leftGutterX - 1.0f;
    //constexpr float rightWallX = rightGutterX + 1.0f;

    //// 左側進入禁止壁
    //g_Blocks.push_back({
    //    { leftWallX, wallHeight * 0.5f, zPos },
    //    { wallThickness, wallHeight, wallLength },
    //    Block::WALL2
    //    });

    //// 右側進入禁止壁
    //g_Blocks.push_back({
    //    { rightWallX, wallHeight * 0.5f, zPos },
    //    { wallThickness, wallHeight, wallLength },
    //    Block::WALL2
    //    });
    for (int lane = -LEFT_LANES; lane <= RIGHT_LANES; ++lane)
    {
        float xOffset = BASE_X + lane * LANE_WIDTH;

        // 床
        g_Blocks.push_back({
            { 4.0f + xOffset, 1.0f, 10.0f },
            { 8.0f, 1.0f, 60.0f },
            Block::Wood
            });

        // ガーター
        g_Blocks.push_back({
            { -0.50f + xOffset, 0.9f, -10.0f },
            { 1.0f, 1.0f, 60.0f },
            Block::Gutter
            });

        g_Blocks.push_back({
            { 8.50f + xOffset, 0.9f, -10.0f },
            { 1.0f, 1.0f, 60.0f },
            Block::Gutter
            });

        // 奥の壁
        g_Blocks.push_back({
            { 3.0f + xOffset, 1.0f, 32.0f },
            { 15.0f, 10.0f, 30.0f },
            Block::WALL
            });

        g_Blocks.push_back({
            { 3.0f + xOffset, 1.0f, 32.0f },
            { 15.0f, 10.0f, 30.0f },
            Block::Moon
            });

        // ===== ガーター外側の進入禁止壁 =====
        constexpr float wallHeight = 2.0f;
        constexpr float wallThickness = 0.5f;
        constexpr float wallLength = 60.0f;
        constexpr float zPos = -10.0f;

        // 左
        g_Blocks.push_back({
            { (-0.50f - 1.0f) + xOffset, wallHeight * 0.5f, zPos },
            { wallThickness, wallHeight, wallLength },
            Block::WALL2
            });

        // 右
        g_Blocks.push_back({
            { (8.50f + 1.0f) + xOffset, wallHeight * 0.5f, zPos },
            { wallThickness, wallHeight, wallLength },
            Block::WALL2
            });
    }

    //// ===== Audience 自動生成（左右）=====
    //constexpr float audienceScale = 10.0f;
    //constexpr float startZ = -20.0f;
    //constexpr float endZ = 32.0f;   // Wall くらいまで
    //constexpr float interval = 10.0f;

    //constexpr float leftX = -6.0f;
    //constexpr float rightX =16.0f;   // 反対側（Wall中心3 + 幅15/2 + 余白）

    //for (float z = startZ; z <= endZ; z += interval)
    //{
    //    // 左側
    //    g_Blocks.push_back({
    //        { leftX, 5.0f, z },
    //        { audienceScale, audienceScale, audienceScale },
    //        Block::Audience
    //        });

    //    // 右側
    //    g_Blocks.push_back({
    //        { rightX, 5.0f, z },
    //        { audienceScale, audienceScale, audienceScale },
    //        Block::Audience
    //        });
    //}


    g_MapTexId[0] = Texture_Load(L"rom\\Texture\\kankyaku.jpg");
    g_MapTexId[1] = Texture_Load(L"rom\\Texture\\jimen.jpg");
    g_MapTexId[2] = Texture_Load(L"rom\\Texture\\kuro.png");
    //sillber
    g_MapTexId[3] = Texture_Load(L"rom\\Texture\\sillber.jpg");


    g_MapModels[0] = ModelLoad("rom\\Model\\jimen.fbx");
    g_MapModels[1] = ModelLoad("rom\\Model\\GoalPost.fbx");
    g_MapModels[2] = ModelLoad("rom\\Model\\UFO.fbx");
    g_MapModels[3] = ModelLoad("rom\\Model\\Star.fbx",2);
    g_MapModels[4] = ModelLoad("rom\\Model\\StarCore.fbx", 1);
    g_MapModels[5] = ModelLoad("rom\\Model\\a.fbx", 0.01);


    for (Block& block : g_Blocks) {
        switch (block.GetType())
        {
        case Block::Normal:
            block.SetAABB(AABB::Make(block.GetPosition(), { 1.0, 1.0, 1.0 }));
            break;

        case Block::Wood:
            block.SetAABB(AABB::Make(block.GetPosition(), block.GetScale()));
            break;

        case Block::WALL:
        {
            XMFLOAT3 blockScale = block.GetScale();
            blockScale.z -= 5;
            block.SetAABB(AABB::Make(block.GetPosition(), blockScale));
        }
            break;
        case Block::Gutter:
        {
            XMFLOAT3 blockScale = block.GetScale();
            blockScale.y += 5;
            block.SetAABB(AABB::Make(block.GetPosition(), blockScale));
        }
            break;

        case Block::GOAL:
           // block.SetAABB(AABB::Make(block.GetPosition(), { 1.0, 1.0, 1.0 }));
            break;

        case Block::UFO:
            block.SetAABB(AABB::Make(block.GetPosition(), { 1.0, 1.0, 1.0 }));
            break;



        default:
            break;
        }
    }
}

void Map_Finalize()
{
    for (int i = 0; i < MODEL_MAX; i++)
    {
        if (g_MapModels[i]) {
            ModelRelease(g_MapModels[i]);
            g_MapModels[i] = nullptr;
        }
    }
}

void Map_Draw()
{
    for (auto& block : g_Blocks)
    {
        block.Draw();
    }
   // DebugDraw_Draw();
}

void Block::Draw() const
{
    XMMATRIX pos = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX scale = XMMatrixScaling(
        m_Scale.x,
        m_Scale.y,
        m_Scale.z
    );

    XMMATRIX mtxworld = scale*pos ;

    switch (m_Type)
    {
    case Block::EMPTY:
        break;
    case Block::Normal:
        Cube_Draw(mtxworld, g_MapTexId[0]);
        break;

    case Block::WALL2:
        Cube_Draw(mtxworld, g_MapTexId[3]);
        break;

    case Block::Audience:
        Cube_Draw(mtxworld, g_MapTexId[0]);
        break;
    case Block::Wood:
        Cube_Draw(mtxworld, g_MapTexId[1]);
        //ModelDraw(g_MapModels[0],mtxworld);
        break;
    case Block::WALL:
    case Block::Gutter:

        Cube_Draw(mtxworld, g_MapTexId[2]);
        break;

    case Block::Tree:
    case Block::Rock:
   
    case Block::START:
    case Block::GOAL:
        ModelDraw(g_MapModels[1], mtxworld);
        break;
    case Block::Sand:
    case Block::TYPE_MAX:
    case Block::UFO:
        ModelDraw(g_MapModels[2], mtxworld);
        break;

    case Block::Star:
        ModelDraw(g_MapModels[3], mtxworld);
        break;
    case Block::StarCore:
        ModelDraw(g_MapModels[4], mtxworld);

        break;
	case Block::Moon:
        Cube_Draw(mtxworld, g_MapTexId[0]);
		break;

    default:
        break;
    }
    // デバッグ描画：AABB を赤いラインで表示
    DebugDraw_AddAABB(m_Aabb, { 1.0f, 0.0f, 0.0f, 1.0f });
   
}
