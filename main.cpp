/*==========================================================================

main.cpp [main.cpp]



												Author :
												Date   : 2025/
---------------------------------------------------------------------------






=========================================================================*/












#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN	// Windows11用 必要なプログラムだけ動かす
#include <Windows.h>
#include <algorithm>
#include "direct3d.h"
#include"shader.h"
#include "polygon.h"
#include "texture.h"
#include "sprite.h"
#include "sprite_anim.h"
#include "system_timer.h"
#include "shader3d.h"
#include "cube.h"
#include "grid.h"
#include "debug_text.h"
#include <DirectXMath.h>
#include <sstream>
#include "keylogger.h"
#include <Xinput.h>
#pragma comment(lib, "xinput.lib") // XInputライブラリのリンク
#include "mouse.h"
#include "Audio.h"
#include "ShaderBillBoard.h"
#include "fade.h"

#include "scene.h"
#include "Mouselogger.h"
using namespace DirectX;
//===============================================================
//  ウィンドウ情報
//===============================================================
static constexpr char WINDOW_CLASS[] = "GAMEWINDOW"; // メインウィンドウクラス名
static constexpr char TITLE[] = "ゲームウィンドウ"; // タイトルバーのテキスト

//===============================================================
//  ウィンドウプロシージャ　プロトタイプ宣言
//===============================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//===============================================================
//  メイン
//===============================================================
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ int nCmdShow)
{
	(void)CoInitializeEx(nullptr, COINIT_MULTITHREADED); // COMライブラリの初期化

	//DPIスケーリング
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// ウィンドウクラスの登録
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;			//関数プロシージャを追加しないといけない
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);	//改造すると好きなアイコンに変えることができる
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);		//上に同じく
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr; // メニューは作らない
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);

	// クライアント領域のサイズを持った短形（左からleft, top, right, bottom）
	RECT window_rect = { 0, 0, 1600, 900 };

	// ウィンドウのスタイル
	DWORD window_styie = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);

	// 指定したクライアント領域を確保するための新たな短形座標を計算
	AdjustWindowRect(&window_rect, window_styie, FALSE);

	// ウィンドウの幅と高さを算出
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	// プライマリモニターの画面解像度取得
	int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	int desktop_height = GetSystemMetrics(SM_CYSCREEN);

	// デスクトップの真ん中にウィンドウが生成されるように座標を計算
	// ただし万が一、デスクトップよりウィンドウが大きい場合は左上に表示
	int window_x = std::max((desktop_width - window_width) / 2, 0);
	int window_y = std::max((desktop_height - window_height) / 10, 0);

	// メインウィンドウの作成
	HWND hWnd = CreateWindow(
		WINDOW_CLASS,
		TITLE,
		window_styie,
		window_x,	//ウィンドウの初期座標指定
		window_y,				//上に同じく
		window_width,	//ウィンドウの幅
		window_height,	//ウィンドウの高さ
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	float angle = 0.0f;
	float x = 0.0f;
	float y = 0.0f;


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	Mouse_Initialize(hWnd);
	MouseLogger_Initialize();   // ← 追加！

	//Fade_StartIn(0.0f, false, { 0.0f,0.0f,0.0f,1.0f }); // フェードインを開始

	KeyLogger_Initialize();
	
	SystemTimer_Initialize(); // システムタイマーの初期化
	InitAudio(); // オーディオの初期化
	//初期化
	if (!Direct3D_Initialize(hWnd)) {
		PostQuitMessage(0);
		Direct3D_Finalize();
		return 0;	// 初期化に失敗したので終了

	}
	else {

		if (!Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetContext())) {
			PostQuitMessage(0);
		}
		else {

			Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
			//シェーダーの初期化
			Polygon_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
			Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetContext()); // スプライトの初期化
			//SpriteAnim_Initialize();

		}

	}
	Fade_Initialize();
	Shader3d_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	ShaderBillboard_Initialize();
	hal::DebugText dt(Direct3D_GetDevice(), Direct3D_GetContext(),
		L"rom\\consolab_ascii_512.png", // フォントテクスチャのパス
		Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight(),
		0.0f, 0.0f,
		0, 0,
		0.0f, 0.0f

	); // デバッグテキストの初期化


	//

	Mouse_SetVisible(true); // マウスカーソルを表示する
	Scene_Initialize();
	
	Cube_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	
	//Gulid_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	//int texid = Texture_Load(L"rom\\1.png"); // テクスチャの読み込み
	//int texid2 = Texture_Load(L"rom\\20170529154631.png"); // テクスチャの読み込み
	//int tex_coco = Texture_Load(L"rom\\kokosozai.png"); // テクスチャの読み込み
	//int tex_run001 = Texture_Load(L"rom\\runningman001.png"); // テクスチャの読み込み



	double exec_last_time = 0.0f;
	double fps_last_time = 0.0f;
	double current_time = 0.0f;
	ULONG frame_count = 0; // フレームカウント
	double fps = 0.0f; // フレームレート（FPS）計算用変数
	exec_last_time = fps_last_time = SystemTimer_GetTime(); // 最初の時間を取得


	// メッセージ&ゲームループ
	MSG msg;

	do
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{// ウィンドウメッセージが来ていたら
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

			current_time = SystemTimer_GetTime(); // 現在の時間を取得
			double elapsed_time = current_time - fps_last_time; // 前回の時間との差分を計算

			if (elapsed_time >= 1.0)
			{


				fps = frame_count / elapsed_time; // フレームレートを計算

				fps_last_time = current_time; // 最後の時間を更新


				frame_count = 0; // フレームカウントをリセット
			}

			elapsed_time = current_time - exec_last_time; // 前回の実行時間との差分を計算

			//if(true)

			if ((elapsed_time) >= (1.0 / 60.0))
			{

				exec_last_time = current_time; // 最後の実行時間を更新


				KeyLogger_Update();
				Fade_Update(elapsed_time); // フェードの更新処理
				Scene_Update(elapsed_time); // シーンの更新処理
				MouseLogger_Update();
				//MouseState_tag ms{};
				//Mouse_GetState(&ms); // マウスの状態を取得
			


				// ゲームの処理
				Direct3D_Clear();

				XINPUT_STATE xs{};

				//1台は0
				//MAXは4台まで
				XInputGetState(0, &xs); // コントローラーの状態を取得


				XINPUT_VIBRATION xv{};




				//現在のXSと
				//1フレームのXSを取得しないと行けない
				if (xs.Gamepad.wButtons & XINPUT_GAMEPAD_A) {

					xv.wLeftMotorSpeed = 65535; // 左モーターの振動速度
					xv.wRightMotorSpeed = 65535; // 右モーターの振動速度
					XInputSetState(0, &xv); // コントローラーの状態を取得
				}

				if (KeyLogger_IsPressed(KK_D)) {

					x += (float)(200.0f * elapsed_time); // 右に移動

				}
				if (KeyLogger_IsPressed(KK_A)) {

					x -= (float)(200.0f * elapsed_time); // 右に移動

				}
				if (KeyLogger_IsPressed(KK_W)) {

					y -= (float)(200.0f * elapsed_time); // 右に移動

				}
				if (KeyLogger_IsPressed(KK_S)) {

					y += (float)(200.0f * elapsed_time); // 右に移動

				}
				if (KeyLogger_IsPressed(KK_P)) {
					angle += (float)(100.0f * elapsed_time); // 角度を更新（ラジアン単位）
				}











				
				//Fade_Draw();
				Scene_Draw(); // シーンの描画
				Fade_Draw();
#if defined(DEBUG) || defined(_DEBUG)
				std::stringstream ss;
				ss << "FPS: " << fps << std::endl;
				dt.SetText(ss.str().c_str()); // デバッグテキストの設定
				dt.Draw(); // デバッグテキストの描画
				dt.Clear(); // デバッグテキストのクリア
#endif // DEBUG




				Direct3D_Present();
				frame_count++; // フレームカウントを増やす
				Scene_Change(); // シーン変更
			}



		}
	} while (msg.message != WM_QUIT);


	// 最後に確保した順から解放
	Gulid_Finalize();
	Cube_Finalize();
	Shader3d_Finalize();
	Scene_Finalize();
	Polygon_Finalize();
	Sprite_Finalize();
	UninitAudio();
	Texture_Finalize();
	Shader_Finalize();
	Fade_Finalize();
	
	Direct3D_Finalize();
	Mouse_Finalize();

	////最後に確保した順から解放
	////SpriteAnim_Finalize();
	//Game_Finalize();
	//Polygon_Finalize();
	//Sprite_Finalize(); // スプライトの終了処理
	//UninitAudio(); // オーディオの終了処理
	//Texture_Finalize();	// テクスチャの終了処理
	//Shader_Finalize();
	//Fade_Finalize();
	//Title_Finalize(); // タイトルの終了
	//Direct3D_Finalize();	// Direct3Dの終了処理
	//Mouse_Finalize(); // マウスの終了処理
	return (int)msg.wParam;
}

//===============================================================
//  ウィンドウプロシージャ
//===============================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATEAPP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		Mouse_ProcessMessage(message, wParam, lParam);

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse_ProcessMessage(message, wParam, lParam);
		break;


	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE || wParam == VK_SPACE)
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0); // WM_CLOSEメッセージの送信
		}
	case WM_SYSKEYDOWN:


	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break;



	case WM_CLOSE: // ウィンドウを閉じるメッセージ
		if (MessageBox(hWnd, "本当に終了してよろしいですか？", "確認", MB_OKCANCEL | MB_DEFBUTTON1) == IDOK)
		{
			DestroyWindow(hWnd); // 指定のウィンドウにWM_DESTROYメッセージを送る
		}
		break; // DefWindowProc関数にメッセージを流さず終了することによって何もなかったことにする

	case WM_DESTROY: // ウィンドウの破棄メッセージ
		PostQuitMessage(0); // WM_QUITメッセージの送信
		break;

	default:

		// 通常のメッセージ処理はこの関数に任せる
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



