#include "game.h"
#include "game_bg.h"
#include "grid.h"
#include "game_score.h"
#include "lighth.h"
#include "keylogger.h"
#include "Map.h"
#include "debug.h"
#include "camera.h"
#include "gameeffect.h"
#include "texture.h"
#include "player.h"

#include "direct3d.h"
#include "scene.h"
#include "Timer.h"
#include "Ball.h"
#include "sprite_anim.h"
//#include "cube.h"
#include "model.h";
#include "Shot.h"
#include "BillBord.h"
#include "sprite_anim.h"
#include "ShotCamera.h"
#include "camera_fixed.h"
#include "trail.h"
#include "bowlingBall.h"
#include "DebugDraw.h"
#include "Mouselogger.h"
#include "PinManager.h"
using namespace DirectX;


static Camera* g_a[2];
static bool g_Start = false;
static int g_BgmId = -1; // BGMのID
static bool g_NextChlick = false;
static AnimPattern* g_pTestAnim = nullptr;
static float g_SelectedX = 0.0f;
static float g_SelectedY = 0.0f;
static float g_LimitTime = 0.0f;
static int Cube01tex = -1;
static AnimPatternPlayer* g_pAnimPlayer{ nullptr };
//ポインタ Newすれば作ったタイミングで生成される
//deleteすれば消える
static DebugCamera* g_pDebugCamera{}; 
static FixedCamera* g_FixedCameras[3];
static int g_FixedCameraIndex{0};


static MODEL* g_pKirby;
static MODEL* g_pPenis;
static AnimPattern* g_pTestAnim2 = nullptr;
static AnimPatternPlayer* g_pAnimPlayer2{ nullptr };
static ShotCamera* g_pShotCamera{ nullptr };

static int g_texid = -1;
static int g_texid2 = -1;

static AABB g_GoalAABB;
static bool g_IsGoal = false;
BowlingBall g_BowlingBall;


PinManager g_Pinmanager;

void Game_Initialize()
{
	g_GoalAABB = AABB::Make({ float(1.0f), 0.9f, float(1.0f) }, { 1,1,1 });
	DebugDraw_Initialize();
	g_pDebugCamera = new DebugCamera({ 0.0f,1.0f,-5.0f }, { 0.0f,0.0f,0.0f });
	g_pShotCamera = new ShotCamera({ 0.0f,0.0f,50.0f }, {2.0f,1.0f,2.0f},2.0f);
	g_FixedCameras[0] = new FixedCamera({ -5.0f, 10.0f, 10.0f }, AABB::Make({ 1.0f,5.00f,1.0f }, {5.0f,5.0f,5.0f}));
	g_FixedCameras[1] = new FixedCamera({5.0f, 10.0f, 10.0f}, AABB::Make({ 6.0f,6.0f,6.0f }, { 20.0f,10.0f,20.0f }));
	g_BowlingBall.Init({ 3,5,2 });
//	Cube01tex = Texture_Load(L"rom\\saikoro_image.png");
	Billboard_Initialize();
	Gulid_Initialize(10,10,1);
	Map_Initialize();
	Light_Initialize();
	
//	Ball_Initialize({3,5,2});
	Shot_Initialize({ 3,5,2 },g_pDebugCamera->GetFront());
	Score_Initialize(100,100,2);
	Trail_Initialize();
	//df.fbx
	//g_pPenis = ModelLoad("rom\\Model\\yajirusi.fbx",0.1);
	g_texid = Texture_Load(L"rom\\Texture\\gra_effect_lightA.png");
	g_pKirby = ModelLoad("rom\\Model\\kirby.fbx",0.1);
	g_texid2 = Texture_Load(L"rom\\bomb3.png");
	g_Pinmanager.Initialize();


	g_pTestAnim = new AnimPattern(g_texid, 10, 5, 0.1, { 0,0 }, { 140,200});
	g_pAnimPlayer = new AnimPatternPlayer(g_pTestAnim);


	/*g_pTestAnim2 = new AnimPattern(g_texid2, 10, 5, 0.1, { 0,0 }, { 140,200 }, false);
	g_pAnimPlayer2 = new AnimPatternPlayer(g_pTestAnim2);*/

}
static double g_time{ 0.0 };
void Game_Update(double elapsed_time)
{
	g_time += elapsed_time;
	g_pDebugCamera->Update(elapsed_time);
	g_BowlingBall.Update(elapsed_time);
	// ココを追加
	//Cube_Update(elapsed_time);
	Shot_Update(elapsed_time);
	//Ball_Update(elapsed_time);
	Trail_AddPosition(Ball_GetPosition());
	Billboard_Update(elapsed_time);
	for (int i = 0; i < 2; i++)
	{
		g_FixedCameras[i]->SetTarget(Ball_GetPosition());
		if (g_FixedCameras[i]->IsOverlap())
		{
			g_FixedCameraIndex = i;
			break;
		}
	}
	g_FixedCameras[g_FixedCameraIndex]->SetMatrix();
	Billboard_SetViewMatrix(g_pDebugCamera->GetViewMatrix());

	//Trail_SetCameraPosition(g_FixedCameras[g_FixedCameraIndex]->GetPosition());
	//if (Ball_IsStationary())
	//{
	//	Shot_SetPosition(Ball_GetPosition());

	//	if (/*!MouseLogger_IsPressed(MouseKey::Left) &&*/ Shot_GetPower() > 0.0f)
	//	{
	//		XMFLOAT3 dir = Shot_GetVelocity();
	//		float power = Shot_GetPower();

	//		g_BowlingBall.AddForce({
	//			dir.x * power,
	//			0.0f,
	//			dir.z * power
	//			});

	//		Shot_ResetPower();
	//	}
	//}

	Score_Update();
	/*for (auto& a : g_pins)
	{
		a.Update(elapsed_time);
		if (!a.IsDown())
		{
			Hit hit = g_BowlingBall.GetAABB().IsHit(a.GetAABB());
			if (hit.IsHit())
			{
				ResolveBallPinHit(g_BowlingBall, a);
			}
		}
	}*/

	if (Shot_IsFired())
	{
		g_BowlingBall.AddForce(Shot_GetShotVelocity());
		Shot_ResetPower();   // 状態初期化
	}

	g_Pinmanager.Update(elapsed_time, g_BowlingBall);

	Hit hit = g_GoalAABB.IsHit(Ball_GetAABB());
	//上向き法線がTrueだったらのを取ればまぁいいよね
	if (hit.IsHit()) {

		//ゴール演出

		g_IsGoal = true;

	}

	g_pAnimPlayer->Update(elapsed_time);

	if(g_pAnimPlayer2)
	{
		g_pAnimPlayer2->Update(elapsed_time);
		if (g_pAnimPlayer2->IsAnimStopeed())
		{
			delete g_pAnimPlayer2;
			delete g_pTestAnim2;
			g_pAnimPlayer2 = nullptr;
		}
	}


	
	
}

void Game_Draw()
{

	Direct3D_SetDepthTest(true);
	//g_pShotCamera->SetMatrix();
	
	//Direct3D_SetDepthWriteDisable();
	//g_pAnimPlayer->BillboardDraw({ 3.0, 2.0f, 2.0f }, { 0.7,1 }, {0.5,0.5});


		//XMMATRIX mtx = XMMatrixScaling(100.0f, 3000.0f, 10000.0f) * XMMatrixTranslation(4.5f, 0.5f, 4.5f);
		//Cube_Draw(mtx,0);
	
	g_pDebugCamera->SetMatrix();   // ★これが最重要
	g_BowlingBall.Draw();
	g_Pinmanager.Draw();

	Gulid_Draw();
	Light_SetAmnient({0.5f,0.5f,0.6f,1.0f});
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, { -1.0f,-1.5f,1.0f });
	Light_SetDiffuse({ 0.5f,0.5f,0.5f }, direction);

	//ModelDraw(g_pKirby, XMMatrixIdentity());
	/*Ball_Draw();
	if (Ball_IsStationary()) {
		Shot_Draw();
	}
	Trail_Draw();*/
	//ModelDraw(g_pPenis, XMMatrixIdentity());
	
		Shot_Draw();
	

	Billboard_SetViewMatrix(g_pDebugCamera->GetViewMatrix());



	//Billboard_Draw(g_texid, g_BowlingBall.GetWorldMatrix(),
	//	{ 140.0f,200.0f }, { 0.0f,0.0f }); // world を Identity にするか、任意のワールド行列を渡す
	//
	Map_Draw();
	DebugDraw_Draw();
	//2D描画はここに
	Direct3D_SetDepthTest(false);
	//Score_Draw();
	/*Billboard_Draw(
		g_texid,
		g_BowlingBall.GetWorldMatrix(),
		{ 140.0f,200.0f },
		{ 140.0f,200.0f },
		{ 14.f,20.f },
		{ 1.0,0.5,0.1,1.0 },
		{ 0,0 }
	);*/

	//Billboard_Draw(g_texid, XMMatrixTranslation(3.0, 2.0f, 2.0f), 
	//	{ 140.0f,200.0f }, { 14.0f,20.0f }); // world を Identity にするか、任意のワールド行列を渡す
	//g_pAnimPlayer->BillboardDraw({ 3.0, 2.0f, 2.0f }, { 0.14,0.2 });
	if(g_IsGoal)
	{
		XMFLOAT3 pos = g_GoalAABB.GetCenter();
		pos.y += 5.0f;
		if (g_pAnimPlayer2)
		{
			g_pAnimPlayer2->BillboardDraw({ pos }, { 1.4,2.0 }, { 0,0 });
		}
	}


}

void Game_Finalize()
{
	//delete g_pTestAnim2;
	delete g_pTestAnim;
	delete g_pDebugCamera;
	delete g_pShotCamera;
	ModelRelease(g_pKirby);
	DebugDraw_Finalize();
	//ModelRelease(g_pPenis);
	Ball_Finalize();
	Map_Finalize();
	Gulid_Finalize();
	Light_Finalize();
	Shot_Finalize();
	Trail_Finalize();
}



