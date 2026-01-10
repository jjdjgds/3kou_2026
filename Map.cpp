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

std::vector<Block> g_Blocks;

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
    // 20×20ブロック生成
    for (int x = 1; x <= 20; ++x) {
        for (int z = 1; z <= 20; ++z) {

            if (x == 1 && z == 1)
            {
                g_Blocks.push_back({ {float(x), 0.0f, float(z)}, Block::Normal });

            }
            else
            {
                g_Blocks.push_back({ {float(x), 1.0f, float(z)}, Block::Normal });

            }
            


          
        }
    }


    // 20×20ブロック生成
    for (int x = 22; x <= 30; ++x) {
        for (int z = 22; z <= 30; ++z) {
            g_Blocks.push_back({ {float(x), 5.f, float(z)}, Block::Normal });
        }
    }

    for (int x = 10; x <= 15; ++x) {
        for (int z = 10; z <= 15; ++z) {
            g_Blocks.push_back({ {float(x), 5.f, float(z)}, Block::Normal });
        }
    }
    g_Blocks.push_back({ {3, 3.f, 3}, Block::GOAL });
    g_Blocks.push_back({ {18, 5.f, 18}, Block::UFO });
    g_Blocks.push_back({ { 10,2,3 }, Block::Star });
   /// g_Blocks.push_back({ { 5,3,3 }, Block::StarCore });
    g_Blocks.push_back({{ 3,3,3 }, Block::Moon});

    g_MapTexId[0] = Texture_Load(L"rom\\grass.jpg");
    g_MapModels[0] = ModelLoad("rom\\Model\\kirby.fbx");
    g_MapModels[1] = ModelLoad("rom\\Model\\GoalPost.fbx");
    g_MapModels[2] = ModelLoad("rom\\Model\\UFO.fbx");
    g_MapModels[3] = ModelLoad("rom\\Model\\Star.fbx",2);
    g_MapModels[4] = ModelLoad("rom\\Model\\StarCore.fbx", 1);
    g_MapModels[5] = ModelLoad("rom\\Model\\Moon.fbx", 1);


    for (Block& block : g_Blocks) {
        switch (block.GetType())
        {
        case Block::Normal:
            block.SetAABB(AABB::Make(block.GetPosition(), { 1.0, 1.0, 1.0 }));
            break;
        case Block::GOAL:
            block.SetAABB(AABB::Make(block.GetPosition(), { 1.0, 1.0, 1.0 }));
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
    DebugDraw_Draw();
}

void Block::Draw() const
{
    XMMATRIX mtxworld = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

    switch (m_Type)
    {
    case Block::EMPTY:
        break;
    case Block::Normal:
        Cube_Draw(mtxworld, g_MapTexId[0]);
        break;
    case Block::Tree:
    case Block::Rock:
    case Block::WALL:
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
       // ModelDraw(g_MapModels[5], mtxworld);
		break;

    default:
        break;
    }
    // デバッグ描画：AABB を赤いラインで表示
    DebugDraw_AddAABB(m_Aabb, { 1.0f, 0.0f, 0.0f, 1.0f });
   
}
