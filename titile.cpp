/*==========================================================================

タイトルの表示[title.cpp]

												Author : hidetoshi muramatu
												Date   : 2025/09/12
---------------------------------------------------------------------------






=========================================================================*/

#include "titile.h"
#include "keylogger.h"
#include "texture.h" 
#include "sprite.h"
#include "fade.h"
#include "Audio.h"
#include "direct3d.h"
#include "math.h"
#include <DirectXMath.h>
#include "scene.h"

static int g_WaitTime = 1.0;
using namespace DirectX;
static int g_TitleTex = -1; // タイトルテクスチャ
static int g_TexPushAnyKeyId = -1; // キーを押してくださいテクスチャ
static double g_AccumulatedTime = 0.0; // 累積時間
static double g_keyInputWaitTime = 0.0; // キー入力待機時間
static int PushCount = 0;
static int setumeiTexId[4] = { -1,-1,-1,-1 };
enum TitleState {

	TITLE_STATE_FADE_IN, // フェードイン
	TITLE_STATE_KEYINPUT_WAIT,    // 待機
	TITLE_STATE_KEYINPUT_ACTION,  // キー入力アクション
	TITLE_STATE_FADE_OUT, // フェードアウト
	TITLE_STATE_END,  // 終了


};
static TitleState g_TitleState = TITLE_STATE_FADE_IN; // タイトルの状態

void Title_Initialize()
{
	//Fade_StartIn(100,false); // フェードイン開始

	g_TitleTex = Texture_Load(L"rom\\Texture\\title.png"); // タイトルテクスチャの読み込み
	g_TexPushAnyKeyId = Texture_Load(L"rom\\Push_Enter_Key.png"); // キーを押してくださいテクスチャの読み込み
	
	//setumeiTexId[3] = Texture_Load(L"rom\\Push_Enter_Key.png");

	PushCount = 0;
	
	g_TitleState = TITLE_STATE_KEYINPUT_WAIT;
	g_AccumulatedTime = 0.0;
}

void Title_Update(double elapsed_time)
{
	g_AccumulatedTime += elapsed_time;
	switch (g_TitleState)
	{


	case TITLE_STATE_FADE_IN: 

		if (Fade_GetState() == FADE_STATE_FADE_IN_FINISHED) {
			g_TitleState = TITLE_STATE_KEYINPUT_WAIT;
		}

		break;


	case TITLE_STATE_KEYINPUT_WAIT:

		if (KeyLogger_IsTrigger(KK_ENTER) && PushCount == 0) {
			g_TitleState = TITLE_STATE_KEYINPUT_ACTION;
			g_keyInputWaitTime = g_AccumulatedTime;
			//PushCount++;
			//サウンド
		}
		
		
		
		
		break;

	case TITLE_STATE_KEYINPUT_ACTION:

		if (g_AccumulatedTime - g_keyInputWaitTime > g_WaitTime) {
			g_TitleState = TITLE_STATE_FADE_OUT;
			Fade_StartIn(1.0, false);
		}

		break;

	case TITLE_STATE_FADE_OUT: 

		if (Fade_GetState() == FADE_STATE_FADE_OUT_FINISHED) {
			//ゲームシーンへ移行
			Scene_SetNextScene(SCENE_GAME);
		}

				break;

	case TITLE_STATE_END:  


		break;



	}

}

void Title_Draw()
{
	

	if (PushCount == 0)
	{
		Sprite_Draw(g_TitleTex, 0, 0, 1600, 900);

		if (g_TitleState != TITLE_STATE_FADE_IN) {

			float speed = g_TitleState == TITLE_STATE_KEYINPUT_WAIT ? 4.0 : 32.0f;
			float alpha = static_cast<float>((sin(g_AccumulatedTime * speed) + 1.0f) * 0.5f);
			float dx = (Direct3D_GetBackBufferWidth() - 400) * 0.5f;
			Sprite_Draw(g_TexPushAnyKeyId, dx, Direct3D_GetBackBufferHeight() * 0.5f, 400, 100, { 1.0f,1.0f,1.0f,alpha });
		}
	}
	else if (PushCount == 1)
	{
		Sprite_Draw(setumeiTexId[0], 0, 0, 1600, 900);
	}
	else if (PushCount == 2)
	{
		Sprite_Draw(setumeiTexId[1], 0, 0, 1600, 900);
	}
	else if (PushCount == 3)
	{
		Sprite_Draw(setumeiTexId[2], 0, 0, 1600, 900);
	}
	/*else if (PushCount == 4)
	{
		Sprite_Draw(setumeiTexId[3], 0, 0, 1600, 900);
	}*/
	

	

}

void Title_Finalize()
{

	
}
