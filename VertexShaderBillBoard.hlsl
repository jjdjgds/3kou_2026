

/*==========================================================================
ビルボード用の頂点シェーダ[VertexShaderBillBoard.hlsl]
	


													Author : hidetoshi muramatu
													
---------------------------------------------------------------------------



==========================================================================*/






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

    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
    
};



//VS_OUTPUT構造体宣言
struct VS_OUTPUT
{
    
     
    float4 posH     : SV_POSITION0;//セマンティクス　画面上の位置
 
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
    
    

  vsout.texCoord = vsin.texCoord; //テクスチャ座標をそのまま受け渡す
    
    return vsout;
}
