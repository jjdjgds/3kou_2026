
/*==========================================================================
2D用の頂点シェーダ[VertexShader2D.hlsl]
	


													Author : hidetoshi muramatu
													
---------------------------------------------------------------------------



==========================================================================*/

/*
頂点シェーダーに変換行列を描画パイプラインに設定
このシェーダーは、2Dグラフィックスの頂点シェーダーであり、変換行列を使用して頂点の位置を変換します。
四角い空間を描画するために、頂点の位置と色を入力として受け取り、変換後の位置と色を出力します。
*/

//b0 スロット番号
cbuffer VS_CONSTANT_BUFFER0 : register(b0)
{
    float4x4 proj; //変換行列
    //float4 position; //位置
    
};
cbuffer VS_CONSTANT_BUFFER1 : register(b1)
{
    
    float4x4 world; //位置
};

//VS_INPUTの構造体宣言
struct VS_INPUT
{
    float4 posL : POSITION0;//セマンティクス　画面上の位置
    float4 color : COLOR0;//セマンティクス　色
    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
    
};



//VS_OUTPUT構造体宣言
struct VS_OUTPUT
{
    
    
    float4 posH : SV_POSITION0;//セマンティクス　画面上の位置
    float4 color : COLOR0;//セマンティクス　色
    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
};

// VS_INPUT 型のデータを受け取って、その名前を vsin として使う

VS_OUTPUT main(VS_INPUT vsin)
{
   
    VS_OUTPUT vsout; //VS_INPUT 型のデータをvsoutとして定義
    
    //変換行列を掛け合わせて頂点の位置を変換
    float4x4 mtx = mul(world, proj); //変換行列を掛け合わせる
    vsout.posH = mul(vsin.posL, mtx);//
    
    
    vsout.color = vsin.color;
    vsout.texCoord = vsin.texCoord;
    
    return vsout;
}
//セマンティクス　.color引数のみ用途を記載