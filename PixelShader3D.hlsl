
/*==========================================================================
3D用のピクセルシェーダ[PixcelShader3D.hlsl]

	


													Author : hidetoshi muramatu
													Date   : 2026/
---------------------------------------------------------------------------



==========================================================================*/



float4 color;


cbuffer PS_CONSTANT_BUFFER0:register(b0)
{
   float4  ambient_color;

}

cbuffer PS_CONSTANT_BUFFER1:register(b1)
{
   float4 diffuse_color;
   float4 diffuse_world_vector;
  


}

cbuffer PS_CONSTANT_BUFFER2:register(b2)
{
   float4 material_diffuse_color;
  // float4 diffuse_world_vector;
  


}

//ここにスペキュラーハイライトとかリムライトとか入れてみる




//PS_INPUT  構造体
//色と表示座標を持つ
struct PS_INPUT
{
    
   
    float4 posH     : SV_POSITION0; //セマンティクス　画面上の位置
    float3 normalW  : NORMAL0;   
    float4 color    : COLOR0; //セマンティクス　色
    float2 texCoord : TEXCOORD0; //セマンティクス　テクスチャ座標
    
};





//色を返す 
// ピクセルシェーダー：頂点シェーダーから渡された色をそのまま描画する
// PS_INPUT 型のデータを受け取って、その名前を psin として使う
//SV_TARGET　 return psin.colorのカラーで描いてねってやつ
//テクスチャから情報を取得する場合は、テクスチャサンプラーを使用して、
//テクスチャ座標を指定して色を取得します。

Texture2D major_Texture; // テクスチャ
SamplerState major_Sampler; //  テクスチャサンプラー

float4 main(PS_INPUT psin): SV_TARGET
{
    

  float4 material = major_Texture.Sample(major_Sampler,psin.texCoord)*psin.color*material_diffuse_color;
  float3 ambient  = material.rgb* ambient_color.rgb;
  
  float brightness = max(dot(-diffuse_world_vector.xyz,normalize(psin.normalW)),0.0f);
  float3 diffuse = material.rgb*diffuse_color.rgb*brightness;
  float alpha = material.a*ambient_color.a;  
  return float4(ambient+diffuse,alpha);
  
    //material_color.rrr
    //material_color.gggも出来るよ
//return major_Texture.Sample(major_Sampler,psin.texCoord);
    //構造体psinのcolorメンバを返す
    //return psin.color;
	
 
}