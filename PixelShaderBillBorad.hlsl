
/*==========================================================================
ビルボード用のピクセルシェーダ[PixcelShaderBillBorad.hlsl]

	


													Author : hidetoshi muramatu
													
---------------------------------------------------------------------------



==========================================================================*/




cbuffer PS_CONSTANT_BUFFER0:register(b0)
{
   float4  color;

}

cbuffer PS_CONSTANT_BUFFER1:register(b1)
{
   float2  offset;
   float2  size;

}



//ここにスペキュラーハイライトとかリムライトとか入れてみる




//PS_INPUT  構造体
//色と表示座標を持つ
struct PS_INPUT
{
    
   
    float4 posH     : SV_POSITION0; //セマンティクス　画面上の位置
  
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

    
    float2 uv = psin.texCoord*size+offset;
    return  major_Texture.Sample(major_Sampler,uv)*color;
    //これで円形のマップとか出来る
    //clip(0.5f-distance(psin.texCoord,float2(0.5,0.5)));

   // float4 rcolor = major_Texture.Sample(major_Sampler,uv)*color
    //if(rcolor.a<0.5){
      
     //  discard;
    ///}
    //return rcolor;
    
  //return  major_Texture.Sample(major_Sampler,uv)*color;
 
}