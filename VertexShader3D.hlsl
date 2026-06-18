/*==========================================================================
3D用の頂点シェーダ[VertexShader3D.hlsl]
	


													Author : hidetoshi muramatu
													
---------------------------------------------------------------------------



==========================================================================*/


/*
頂点シェーダーに変換行列を描画パイプラインに設定
このシェーダーは、2Dグラフィックスの頂点シェーダーであり、変換行列を使用して頂点の位置を変換します。
四角い空間を描画するために、頂点の位置と色を入力として受け取り、変換後の位置と色を出力します。
*/


cbuffer VS_CONSTANT_BUFFER0:register(b0)
{
    float4x4 world;

};
cbuffer VS_CONSTANT_BUFFER1:register(b1)
{
    float4x4 view;

};
cbuffer VS_CONSTANT_BUFFER2:register(b2)
{
    float4x4 proj;

};




//VS_INPUTの構造体宣言
struct VS_INPUT
{
    float3 posL     : POSITION0;//セマンティクス　画面上の位置
    float3 normalL  : NORMAL0;
    float4 color    : COLOR0;//セマンティクス　色
    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
    
};



//VS_OUTPUT構造体宣言
struct VS_OUTPUT
{
    
     
    float4 posH     : SV_POSITION0;//セマンティクス　画面上の位置
    float3 normalW  : NORMAL0;
    float4 color    : COLOR0;//セマンティクス　色
    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
};

// VS_INPUT 型のデータを受け取って、その名前を vsin として使う

VS_OUTPUT main(VS_INPUT vsin)
{
   
    VS_OUTPUT vsout; //VS_INPUT 型のデータをvsoutとして定義
    
    //変換行列を掛け合わせて頂点の位置を変換
    float4x4 mtxWV = mul(world, view); //変換行列を掛け合わせる
   float4x4 mtxWVP = mul(mtxWV, proj); //変換行列を掛け合わせる
    vsout.posH = mul(float4(vsin.posL,1.0f), mtxWVP);//
    
    

    //float4 posW = mul(vsin.posL,world);
    // float4 posWV = mul(posW,view);
    //vsout.posH=mul(posWV,proj);


    //ワールド法線を作れる
    //同じ比率以外での拡大縮小の禁止

    //ローカルライトとローカル法線でのライティング
    //ワールド変換行列の転置逆行列でワールド法線を作る
    vsout.normalW = mul(float4(vsin.normalL,0.0f),world).xyz;
    vsout.color = vsin.color;
    vsout.texCoord = vsin.texCoord;
    
    return vsout;
}
//セマンティクス　.color引数のみ用途を記載