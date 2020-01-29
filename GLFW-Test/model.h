#ifndef MODEL_H
#define MODEL_H
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//先實作mesh.h
#include "mesh.h"
#include "shader.h"
//learnGL這邊有個shader.h 記得是可以不用 先不做看看

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model {
	public:
		vector<Texture> textures_loaded;//存所有的texture
		vector<Mesh> meshes;
		string directory;//目錄
		bool gammaCorrection;

		Model(string const &path, bool gamma = false) :gammaCorrection(gamma) {
			loadModel(path);
		}

		void Draw(Shader shader) {
			for (unsigned int i = 0; i < meshes.size(); i++) {
				meshes[i].Draw(shader);//為run到的那個mesh配置好所有的材質貼圖後 送到shader裡做index render
			}
		}

	private:
		void loadModel(string const path) {
			//load整個場景模型進來
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
				return;
			}
			directory = path.substr(0, path.find_last_of('/'));
			processNode(scene->mRootNode, scene);
		}

		void processNode(aiNode *node,const aiScene *scene){
			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				//把每個讀到assimp中的mesh取出來處理好 然後放到自己的mesh結構中
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.push_back(processMesh(mesh, scene));
			}
			//取完這個mesh後去取它下面的子mesh
			for (unsigned int i = 0; i < node->mNumChildren; i++) {
				processNode(node->mChildren[i], scene);
			}
		}
		Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
			vector<Vertex> vertices;
			vector<unsigned int> indices;
			vector<Texture> textures;

			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				glm::vec3 vector;

				//取出mesh中的資料 頂點 法向量 貼圖座標等
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;

				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;

				if (mesh->mTextureCoords[0]) {//如果mesh有給貼圖座標(取第一個來用)
					glm::vec2 vec;

					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}
				else {
					vertex.TexCoords = glm::vec2(0.0f, 0.0f);
				}
				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
				vertices.push_back(vertex);
			}
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {//run給個index
				aiFace face = mesh->mFaces[i];
				
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);//取出構成每一面face(三角片)的頂點index 並用於給EBO做index render
				}
			}
			if (mesh->mMaterialIndex >= 0) {//check是否有material
					aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
					// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
					// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
					// Same applies to other texture as the following list summarizes:
					// diffuse: texture_diffuseN
					// specular: texture_specularN
					// normal: texture_normalN

					// 1. diffuse maps
					vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
					textures.insert(textures.end(),diffuseMaps.begin(),diffuseMaps.end());

					// 2. specular maps
					vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
					textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
					// 3. normal maps
					std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
					textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
					// 4. height maps
					std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
					textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());


					// return a mesh object created from the extracted mesh data 

					Material material_custom;
					aiColor3D temp_ka;
					aiColor3D temp_kd;
					aiColor3D temp_ks;
					float temp_shiness;

					material->Get(AI_MATKEY_COLOR_AMBIENT, temp_ka);
					material->Get(AI_MATKEY_COLOR_DIFFUSE, temp_kd);
					material->Get(AI_MATKEY_COLOR_SPECULAR, temp_ks);
					material->Get(AI_MATKEY_SHININESS, temp_shiness);

					material_custom.ka = glm::vec3(temp_ka.r,temp_ka.g,temp_ka.b);
					material_custom.kd = glm::vec3(temp_kd.r, temp_kd.g, temp_kd.b);
					material_custom.ks = glm::vec3(temp_ks.r, temp_ks.g, temp_ks.b);
					material_custom.Ns = temp_shiness;

					cout << "Kd:(" << material_custom.kd.x << "," << material_custom.kd.y << "," << material_custom.kd.z << ")" << endl;
					cout << "shiness exponent:" << (int)material_custom.Ns << endl <<endl;
					//回傳一個新建立的mesh 並用constructer做好default
					return Mesh(vertices, indices, textures,material_custom);
			}
			
		}
		vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
		{
			vector<Texture> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				
				mat->GetTexture(type, i, &str);   //???????????????

				// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
				bool skip = false;
				for (unsigned int j = 0; j < textures_loaded.size(); j++)
				{
					if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
					{
						textures.push_back(textures_loaded[j]);
						skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
						break;
					}
				}
				if (!skip)
				{   // if texture hasn't been loaded already, load it
					Texture texture;
					texture.id = TextureFromFile(str.C_Str(), this->directory);
					texture.type = typeName;
					texture.path = str.C_Str();
					/*aiColor3D ai_kd;
					//mat->Get(AI_MATKEY_COLOR_AMBIENT, ai_ka);
					mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_kd);
					mat->Get(AI_MATKEY_COLOR_SPECULAR, texture.ks);
					mat->Get(AI_MATKEY_SHININESS, texture.Ns);
					texture.kd = glm::vec3(ai_kd.r, ai_kd.g, ai_kd.b);*/
					//cout <<"Kd:("<<texture.kd.x<<","<<texture.kd.y<<","<<texture.kd.z<<")"<< endl;
					//cout << "shiness: "<<texture.Ns <<endl<< endl;
					//根本沒有提出來 shiness是錯的 ka kd ks都是0



					//cout << "hello" << endl;
					textures.push_back(texture);
					textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
				}
			}
			return textures;
		}
};


unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}




#endif // !MODEL_H

