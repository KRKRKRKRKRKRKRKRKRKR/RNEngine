#pragma once
#include <vector>							//std::vector
#include "Renderer/Mesh.h"

//3Dモデルファイルを頂点とインデックスに変換する(Assimpを使うのはここだけ)
class ModelLoader
{
public:
	static bool Load(const char* path, std::vector<Vertex>& vertices, std::vector<UINT>& indices);	//失敗でfalse(理由はログ)
};
