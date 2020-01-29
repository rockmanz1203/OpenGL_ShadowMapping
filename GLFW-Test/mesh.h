#ifndef MESH_H
#define MESH_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
//learnGL這邊有個shader.h 記得是可以不用 先不做看看

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<vector>
using namespace std;

//自定義一個mesh結構來接assimp回傳出來的mesh資料

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;

	//---------------
	glm::vec3 Tangent;//切線
	glm::vec3 Bitangent;//副切線 以上兩個好像是在做法線貼圖會用的東西	
};

struct Texture {
	unsigned int id;
	string type;
	string path;
	
};

struct Material {
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float Ns;
};

//主要要拿來接assimp mesh的自訂mesh class 會用到上面兩個自訂struct
class Mesh {
	public:
		vector<Vertex> vertices;//放每個頂點的相關資料
		vector<unsigned int> indices;//用於EBO做index rendering 知道每個頂點的index可以少存一點記憶體空間
		vector<Texture> textures;//放貼圖
		Material material;
		unsigned int VAO;

		Mesh(vector<Vertex>vertices, vector<unsigned int>indices, vector<Texture> textures, Material material) {//constructer
			this->vertices = vertices;
			this->indices = indices;
			this->textures = textures;
			this->material = material;
			setupMesh();
		}

		void Draw(Shader shader) {
			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			unsigned int heightNr = 1; //各類貼圖的計數器

			for(unsigned int i=0 ; i<textures.size() ; i++){//把全部的texture run過一遍
				glActiveTexture(GL_TEXTURE0 + i);//用計數器i當作位移量 把每個會用到的texture都啟用一個texture unit來配合使用

				string number;//暫存用 以方便後續配置給shader
				string name = textures[i].type;//紀錄是哪種 material texture

				if (name == "texture_diffuse") {
					number = std::to_string(diffuseNr++);//因為是 i++ 所以會先跑完to_string之後 diffuseNr才會+1
				}
				else if (name == "texture_specular") {
					number = std::to_string(specularNr++);
				}
				else if (name == "texture_normal") {
					number = std::to_string(normalNr++);
				}
				else if (name == "texture_height") {
					number = std::to_string(heightNr++);
				}
			
				glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()),i);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);

				glUniform1i(glGetUniformLocation(shader.ID, "shiness"), material.Ns);
				glUniform3fv(glGetUniformLocation(shader.ID, "Ka"),1,glm::value_ptr(material.ka));
				glUniform3fv(glGetUniformLocation(shader.ID, "Kd"), 1, glm::value_ptr(material.kd));
				glUniform3fv(glGetUniformLocation(shader.ID, "Ks"), 1, glm::value_ptr(material.ks));

				/*
				cout << "送入shader前mtl資料check" << endl;
				cout << "shiness:" << material.Ns << endl;
				cout << "Ka:(" << material.ka.x << ", " << material.ka.y << ", " << material.ka.z << ") " << endl;
				cout << "Kd:(" << material.kd.x << ", " << material.kd.y << ", " << material.kd.z << ") " << endl;
				cout << "Ks:(" << material.ks.x << ", " << material.ks.y << ", " << material.ks.z << ") " << endl;
				*/
			
			}
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

			glActiveTexture(GL_TEXTURE0);
		}

	private:
		unsigned int VBO, EBO;
		void setupMesh() {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);//總共有幾組頂點資料組(頂點 法向量 貼圖座標)*存一組頂點資料組的空間->計算出總大小

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);//採用index render
			
			//分配資料 分別打開VBO shader中的每一個layout

			//頂點資料
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			//法向量
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			//貼圖座標
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

			glBindVertexArray(0);

		}
};





#endif // !MESH_H

