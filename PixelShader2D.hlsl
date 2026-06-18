
/*==========================================================================
2D用のピクセルシェーダ[PixcelShader2D.hlsl]

	


													Author : hidetoshi muramatu
													
---------------------------------------------------------------------------



==========================================================================*/








//PS_INPUT  構造体
//色と表示座標を持つ
struct PS_INPUT
{
    
   
    float4 posH : SV_POSITION0; //セマンティクス　画面上の位置
    float4 color : COLOR0; //セマンティクス　色
    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
    
};


float4 color;


//色を返す 
// ピクセルシェーダー：頂点シェーダーから渡された色をそのまま描画する
// PS_INPUT 型のデータを受け取って、その名前を psin として使う
//SV_TARGET　 return psin.colorのカラーで描いてねってやつ
//テクスチャから情報を取得する場合は、テクスチャサンプラーを使用して、
//テクスチャ座標を指定して色を取得します。

Texture2D major_Texture; // テクスチャ
SamplerState major_Sampler; //  テクスチャサンプラー

float4 main(PS_INPUT psin) : SV_TARGET
{
    
    float4 texcolor=major_Texture.Sample(major_Sampler, psin.texCoord);
    
    return psin.color*texcolor * color;
   // return color;
    //構造体psinのcolorメンバを返す
    //return psin.color;
	
 
}