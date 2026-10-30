#include "Renderer/ModelLoader.h"
#include <assimp/Importer.hpp>				//読み込み本体
#include <assimp/scene.h>					//aiScene(読んだ結果)
#include <assimp/postprocess.h>				//aiProcess_のフラグ

bool ModelLoader::Load(const char* path, std::vector<Vertex>& vertices, std::vector<UINT>& indices)
{
	Assimp::Importer importer;				//破棄時にsceneも消える
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |				//多角形→三角形
		aiProcess_ConvertToLeftHanded |		//左手系・時計回り・UVの上下反転
		aiProcess_JoinIdenticalVertices |	//同じ頂点をまとめる
		aiProcess_PreTransformVertices);	//ノードの位置を頂点に焼き込む

	if (!scene || !scene->HasMeshes())
	{
		Log::Error("Model load failed: %s\n  %s", path, importer.GetErrorString());
		return false;
	}

	vertices.clear();
	indices.clear();
	for (unsigned int m = 0; m < scene->mNumMeshes; m++)	//全メッシュを1つにまとめる
	{
		const aiMesh* mesh = scene->mMeshes[m];
		const UINT base = (UINT)vertices.size();			//このメッシュの先頭の頂点番号

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex v = {};
			v.position[0] = mesh->mVertices[i].x;
			v.position[1] = mesh->mVertices[i].y;
			v.position[2] = mesh->mVertices[i].z;
			v.color[0] = 1.0f;								//白
			v.color[1] = 1.0f;
			v.color[2] = 1.0f;
			v.color[3] = 1.0f;
			if (mesh->HasTextureCoords(0))					//UVが無いモデルは0,0のまま
			{
				v.uv[0] = mesh->mTextureCoords[0][i].x;
				v.uv[1] = mesh->mTextureCoords[0][i].y;
			}
			vertices.push_back(v);
		}

		for (unsigned int f = 0; f < mesh->mNumFaces; f++)
		{
			const aiFace& face = mesh->mFaces[f];
			if (face.mNumIndices != 3) continue;			//点・線は描かない
			for (unsigned int j = 0; j < 3; j++)
			{
				indices.push_back(base + face.mIndices[j]);	//まとめた後の番号にずらす
			}
		}
	}

	Log::Info("Model load: %s (vertices %zu, triangles %zu)", path, vertices.size(), indices.size() / 3);
	return true;
}
