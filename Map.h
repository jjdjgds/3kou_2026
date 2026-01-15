/*==========================================================================
マップデータの管理[Map.h]


												Author : hidetoshi muramatu
												Date   : 2025/11/04
---------------------------------------------------------------------------






=========================================================================*/

#pragma once
#include <DirectXMath.h>
#include "Collision.h"
class Block
{
public:
	enum Type
	{
		EMPTY,
		Normal,
		Wood,
		Gutter,
		Tree,
		Audience,
		Rock,
		WALL,
		WALL2,
		START,
		GOAL,
		Sand,
		UFO,
		Star,
		StarCore,
		Moon,
		TYPE_MAX
	};

private:
	Type m_Type{ Normal };
	DirectX::XMFLOAT3 m_Position{ 0.0f,0.0f,0.0f };
	DirectX::XMFLOAT3 m_Scale{ 1.0f,1.0f,1.0f };
	AABB m_Aabb{};

public:
	Block(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale, Type type)
		:m_Position(position),m_Scale(scale), m_Type(type)
		
	{
		

	}

	

	const DirectX::XMFLOAT3& GetScale() const { return m_Scale; }

	const DirectX::XMFLOAT3& GetPosition()const { return m_Position; }
	void SetAABB(const AABB& aabb) { m_Aabb = aabb; }
	const AABB& GetAABB() { return m_Aabb; }
	Type GetType() const  { return m_Type; }
	void Draw() const;
};


const Block& MapGetBlock(int index);
int MapGetBlockCount();
const AABB& GetCollision(int index);
void Map_Initialize();
void Map_Finalize();
void Map_Draw();

