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
#include "AuraEffect.h"
#include "ParticleEffect.h"
#include "ScoreBoard.h"

#include "debug_ostream.h" // 追加：ログ出力用

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
static int g_ShotCount = 0;
static constexpr int MAX_SHOT = 2;



static const XMFLOAT3 BALL_START_POS = { 4,5,-38 };


static bool g_BallInPlay = false;

static AABB g_GoalAABB;
static bool g_IsGoal = false;
BowlingBall g_BowlingBall;
static int g_PrevDownPinCount = 0;
static float g_PinSettleTimer = 0.0f;
// 余裕を増やして誤判定を減らす
static constexpr float PIN_SETTLE_TIME = 1.5f; // 1秒 -> 1.5秒に変更
static bool g_WaitingPinSettle = false;
static int g_FrameCount = 0;
static constexpr int MAX_FRAME = 4;
static int g_PrevDownPinsThisThrow = 0;
// 追加: 投球時に残っていたピン数（Remove/削除を考慮した確実な差分算出に使用）
static int g_PrevRemainingPinsThisThrow = 0;


PinManager g_Pinmanager;
float GameClamp(float v, float minV, float maxV);
void Game_Initialize()
{
	g_GoalAABB = AABB::Make({ float(1.0f), 0.9f, float(1.0f) }, { 1,1,1 });
	DebugDraw_Initialize();
	g_pDebugCamera = new DebugCamera({ 0.0f,1.0f,-5.0f }, { 0.0f,0.0f,0.0f });
	g_pShotCamera = new ShotCamera({ 0.0f,0.0f,50.0f }, {2.0f,1.0f,2.0f},2.0f);
	g_FixedCameras[0] = new FixedCamera({ -5.0f, 10.0f, 10.0f }, AABB::Make({ 1.0f,5.00f,1.0f }, {5.0f,5.0f,5.0f}));
	g_FixedCameras[1] = new FixedCamera({5.0f, 10.0f, 10.0f}, AABB::Make({ 6.0f,6.0f,6.0f }, { 20.0f,10.0f,20.0f }));
	g_BowlingBall.Init(BALL_START_POS);
	ScoreBoard_Initialize();
//	Cube01tex = Texture_Load(L"rom\\saikoro_image.png");
	Billboard_Initialize();
	Gulid_Initialize(10,10,1);
	Map_Initialize();
	Light_Initialize();
	
//	Ball_Initialize({3,5,2});
	Shot_Initialize(BALL_START_POS,g_pDebugCamera->GetFront());
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
	g_FrameCount = 0;
	g_ShotCount = 0;
	g_PrevDownPinCount = 0;


	/*g_pTestAnim2 = new AnimPattern(g_texid2, 10, 5, 0.1, { 0,0 }, { 140,200 }, false);
	g_pAnimPlayer2 = new AnimPatternPlayer(g_pTestAnim2);*/

}
static double g_time{ 0.0 };
void Game_Update(double elapsed_time)
{
	g_time += elapsed_time;
	g_pDebugCamera->Update(elapsed_time);
	g_BowlingBall.Update(elapsed_time);
	Shot_SetPosition(g_BowlingBall.GetPosition()); // ★追加


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
	//Direct3D_SetDepthTest(true);
	//g_pAnimPlayer->BillboardDraw({ 3.0, 2.0f, 2.0f }, { 0.7,1 }, {0.5,0.5});
	

	Score_Update();
	if (KeyLogger_IsPressed(KK_R))
	{
		g_Pinmanager.ResetPins();
	}
	if (KeyLogger_IsPressed(KK_T))
	{
		Scene_SetNextScene(SCENE_RESULT);
	}

	

	
	bool fired = Shot_ConsumeFire();

	if (!g_BallInPlay && fired)
	{
		// 投球直前の倒れているピン数を記録（基準）
		g_PrevDownPinsThisThrow = g_Pinmanager.GetDownPinCount();
		// 追加: 投球直前の「残っているピン数」も記録しておく（削除を踏まえた差分算出用）
		g_PrevRemainingPinsThisThrow = g_Pinmanager.GetRemainingPinCount();

		// デバッグログ：投球時の基準値
		hal::dout << "Shot fired. prevDownPinsThisThrow=" << g_PrevDownPinsThisThrow
			<< " prevRemaining=" << g_PrevRemainingPinsThisThrow << std::endl;

		g_BowlingBall.AddForce(Shot_GetShotVelocity());
		Shot_ResetPower();
		g_BallInPlay = true;
	}
	if (g_BallInPlay && g_BowlingBall.IsStopped())
	{
		g_BallInPlay = false;
		g_WaitingPinSettle = true;
		g_PinSettleTimer = 0.0f;
		// ※ 記録は投球時に行うためここで上書きしない
	}
	if (g_WaitingPinSettle)
	{
		g_PinSettleTimer += elapsed_time;

		if (g_PinSettleTimer >= PIN_SETTLE_TIME)
		{
			g_WaitingPinSettle = false;


			g_ShotCount++;

			// --- 変更: 倒れた本数の算出を RemoveDeadPins 後の残数差分に変更 ---
			// RemoveDeadPins に依存すると dead フラグの遅延で不安定になるため、
			// 投球時の remaining から RemoveDeadPins 後の remaining を引く方法に変更します。

			// デバッグログ（確定前）
			int nowDown = g_Pinmanager.GetDownPinCount();
			hal::dout << "Pin settle (before remove): prevDown=" << g_PrevDownPinsThisThrow
				<< " nowDown=" << nowDown
				<< " prevRemaining=" << g_PrevRemainingPinsThisThrow
				<< std::endl;

			// 実際に死んだピンを削除（ここで remaining が減る）
			g_Pinmanager.RemoveDeadPins();

			int remainingAfterRemove = g_Pinmanager.GetRemainingPinCount();

			// 投球時の残数 - 現在の残数 = 今回倒れた本数
			int fallenPins = g_PrevRemainingPinsThisThrow - remainingAfterRemove;
			if (fallenPins < 0) fallenPins = 0;
			fallenPins = static_cast<int>(GameClamp((float)fallenPins, 0.0f, 10.0f));

			// デバッグログ：確定時の情報（削除後ベース）
			hal::dout << "Pin settle (after remove): remainingAfterRemove=" << remainingAfterRemove
				<< " computedFallen=" << fallenPins
				<< std::endl;

			bool isStrike = (fallenPins == 10 && g_ShotCount == 1);
			Score_AddThrow(fallenPins);

			// デバッグログ：RemoveDeadPins 後の残数/ダウン数
			hal::dout << "After RemoveDeadPins, remainingPins=" << g_Pinmanager.GetRemainingPinCount()
				<< " downCount=" << g_Pinmanager.GetDownPinCount() << std::endl;

			g_BowlingBall.Reset(BALL_START_POS);

			if (isStrike || g_ShotCount >= MAX_SHOT)
			{
				// g_Pinmanager.ResetPins();
				g_Pinmanager.DestroyAndRecreatePins(); // <- ここを変更
				g_PrevDownPinCount = 0;
				g_ShotCount = 0;
				// ★ フレーム進行
				g_FrameCount++;

				// ★ 4フレーム終了 → リザルトへ
				if (g_FrameCount >= MAX_FRAME)
				{
					Scene_SetNextScene(SCENE_RESULT);
					return;
				}
			}
		}
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

	AuraEffect_Update(elapsed_time);
	ParticleEffect_Update(elapsed_time);
	
	
}

void Game_Draw()
{

	Direct3D_SetDepthTest(true);
	//g_pShotCamera->SetMatrix();
	g_pDebugCamera->SetMatrix();   // ★これが最重要
	Billboard_SetViewMatrix(g_pDebugCamera->GetViewMatrix());
	//Direct3D_SetDepthWriteDisable();
	//g_pAnimPlayer->BillboardDraw({ 3.0, 2.0f, 2.0f }, { 0.7,1 }, {0.5,0.5});
	

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
		


	



	//Billboard_Draw(g_texid, g_BowlingBall.GetWorldMatrix(),
	//	{ 140.0f,200.0f }, { 140.0f,145.0f }); // world を Identity にするか、任意のワールド行列を渡す
	//
	Map_Draw();
	DebugDraw_Draw();
	//2D描画はここに
	Direct3D_SetDepthTest(false);
	Shot_DrawUI();
	ScoreBoard_Draw();


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
	//g_pAnimPlayer->BillboardDraw({ 3.0, 2.0f }, { 0.14,0.2 });
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



float GameClamp(float v, float minV, float maxV)
{
	if (v < minV) return minV;
	if (v > maxV) return maxV;
	return v;
}