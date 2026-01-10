
#include "direct3d.h"
#include "DirectXTex.h"
#include "model.h"
#include "WICTextureLoader11.h"
#include "shader3d.h"
#include "texture.h"
#include <algorithm>
using namespace DirectX;
// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT3 normal;   // 法線
	XMFLOAT4 color;    // 頂点色
	XMFLOAT2 texcoord; // テクスチャ座標（必要なら追加）

};
static int g_TextureWhite = -1;

MODEL* ModelLoad(const char* FileName, float scale, bool isblender)
{
    MODEL* model = new MODEL{};

    model->AiScene =
        aiImportFile(FileName,
            aiProcessPreset_TargetRealtime_MaxQuality |
            aiProcess_ConvertToLeftHanded);

    if (!model->AiScene)
    {
        delete model;
        return nullptr;
    }

    model->VertexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes] {};
    model->IndexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes] {};

    XMFLOAT3 min{}, max{};
    bool firstVertex = true;

    for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
    {
        aiMesh* mesh = model->AiScene->mMeshes[m];
        Vertex3d* vertex = new Vertex3d[mesh->mNumVertices];

        for (unsigned int v = 0; v < mesh->mNumVertices; v++)
        {
            vertex[v].position = {
                mesh->mVertices[v].x * scale,
                mesh->mVertices[v].y * scale,
                mesh->mVertices[v].z * scale
            };

            // ★ texcoord NULL ガード
            if (mesh->mTextureCoords[0])
            {
                vertex[v].texcoord = {
                    mesh->mTextureCoords[0][v].x,
                    mesh->mTextureCoords[0][v].y
                };
            }
            else
            {
                vertex[v].texcoord = { 0.0f, 0.0f };
            }

            vertex[v].color = { 1,1,1,1 };
            vertex[v].normal = {
                mesh->mNormals[v].x,
                mesh->mNormals[v].y,
                mesh->mNormals[v].z
            };

            if (firstVertex)
            {
                min = max = vertex[v].position;
                firstVertex = false;
            }
            else
            {
                min.x = std::min(min.x, vertex[v].position.x);
                min.y = std::min(min.y, vertex[v].position.y);
                min.z = std::min(min.z, vertex[v].position.z);

                // ★ max は max
                max.x = std::max(max.x, vertex[v].position.x);
                max.y = std::max(max.y, vertex[v].position.y);
                max.z = std::max(max.z, vertex[v].position.z);
            }
        }

        // VertexBuffer
        D3D11_BUFFER_DESC vbd{};
        vbd.Usage = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth = sizeof(Vertex3d) * mesh->mNumVertices;
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vsd{};
        vsd.pSysMem = vertex;

        HRESULT hr = Direct3D_GetDevice()->CreateBuffer(
            &vbd, &vsd, &model->VertexBuffer[m]);

        delete[] vertex;
        if (FAILED(hr)) continue;

        // IndexBuffer
        unsigned int* index = new unsigned int[mesh->mNumFaces * 3];
        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            const aiFace& face = mesh->mFaces[f];
            index[f * 3 + 0] = face.mIndices[0];
            index[f * 3 + 1] = face.mIndices[1];
            index[f * 3 + 2] = face.mIndices[2];
        }

        D3D11_BUFFER_DESC ibd{};
        ibd.Usage = D3D11_USAGE_DEFAULT;
        ibd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA isd{};
        isd.pSysMem = index;

        hr = Direct3D_GetDevice()->CreateBuffer(
            &ibd, &isd, &model->IndexBuffer[m]);

        delete[] index;
    }

    model->Aabb = { min, max };

    // ★ 内蔵テクスチャ（WIC）安全読み込み
    for (unsigned int i = 0; i < model->AiScene->mNumTextures; i++)
    {
        aiTexture* aitex = model->AiScene->mTextures[i];
        ScratchImage image;
        TexMetadata meta;

        if (FAILED(LoadFromWICMemory(
            aitex->pcData, aitex->mWidth,
            WIC_FLAGS_NONE, &meta, image)))
            continue;

        ID3D11ShaderResourceView* srv = nullptr;
        if (FAILED(CreateShaderResourceView(
            Direct3D_GetDevice(),
            image.GetImages(),
            image.GetImageCount(),
            meta,
            &srv)))
            continue;

        model->Texture[aitex->mFilename.C_Str()] = srv;
    }

    g_TextureWhite = Texture_Load(L"rom\\white_8x8.png");
    return model;
}



void ModelRelease(MODEL* model)
{
    if (!model) return;

    if (model->VertexBuffer && model->IndexBuffer)
    {
        for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
        {
            if (model->VertexBuffer[m])
            {
                model->VertexBuffer[m]->Release();
                model->VertexBuffer[m] = nullptr;
            }

            if (model->IndexBuffer[m])
            {
                model->IndexBuffer[m]->Release();
                model->IndexBuffer[m] = nullptr;
            }
        }

        delete[] model->VertexBuffer;
        delete[] model->IndexBuffer;
    }

    for (auto& pair : model->Texture)
    {
        if (pair.second)
        {
            pair.second->Release();
            pair.second = nullptr;
        }
    }
    model->Texture.clear();

    if (model->AiScene)
    {
        aiReleaseImport(model->AiScene);
        model->AiScene = nullptr;
    }

    delete model;
}


void ModelDraw(const MODEL* model, const XMMATRIX& mtxWorld)
{
    Shader3d_Begin();

    for (unsigned int i = 0; i < model->AiScene->mNumMeshes; i++)
    {
        UINT stride = sizeof(Vertex3d);
        UINT offset = 0;

        if (!model->VertexBuffer[i] || !model->IndexBuffer[i])
            continue;

        Direct3D_GetContext()->IASetVertexBuffers(
            0, 1, &model->VertexBuffer[i], &stride, &offset);

        Direct3D_GetContext()->IASetIndexBuffer(
            model->IndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

        Shader3d_SetWorldMatrix(mtxWorld);
        Direct3D_GetContext()->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        aiMaterial* mat =
            model->AiScene->mMaterials[
                model->AiScene->mMeshes[i]->mMaterialIndex];

        aiString texName;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &texName);

        // ★ at() 廃止
        auto it = model->Texture.find(texName.C_Str());
        if (it != model->Texture.end())
        {
            ID3D11ShaderResourceView* srv = it->second;
            Direct3D_GetContext()->PSSetShaderResources(0, 1, &srv);
            Shader3d_SetMaterialDiffuse({ 1,1,1,1 });
        }
        else
        {
            Tecture_SetTexture(g_TextureWhite);
            aiColor3D diffuse{};
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            Shader3d_SetMaterialDiffuse(
                { diffuse.r, diffuse.g, diffuse.b, 1.0f });
        }

        Direct3D_GetContext()->DrawIndexed(
            model->AiScene->mMeshes[i]->mNumFaces * 3, 0, 0);
    }
}





