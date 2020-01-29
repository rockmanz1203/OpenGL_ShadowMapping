#ifndef MESH_H
#define MESH_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
//learnGL�o�䦳��shader.h �O�o�O�i�H���� �������ݬ�

#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<vector>
using namespace std;

//�۩w�q�@��mesh���c�ӱ�assimp�^�ǥX�Ӫ�mesh���

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;

	//---------------
	glm::vec3 Tangent;//���u
	glm::vec3 Bitangent;//�Ƥ��u �H�W��Ӧn���O�b���k�u�K�Ϸ|�Ϊ��F��	
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

//�D�n�n���ӱ�assimp mesh���ۭqmesh class �|�Ψ�W����Ӧۭqstruct
class Mesh {
	public:
		vector<Vertex> vertices;//��C�ӳ��I���������
		vector<unsigned int> indices;//�Ω�EBO��index rendering ���D�C�ӳ��I��index�i�H�֦s�@�I�O����Ŷ�
		vector<Texture> textures;//��K��
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
			unsigned int heightNr = 1; //�U���K�Ϫ��p�ƾ�

			for(unsigned int i=0 ; i<textures.size() ; i++){//�������texture run�L�@�M
				glActiveTexture(GL_TEXTURE0 + i);//�έp�ƾ�i��@�첾�q ��C�ӷ|�Ψ쪺texture���ҥΤ@��texture unit�Ӱt�X�ϥ�

				string number;//�Ȧs�� �H��K����t�m��shader
				string name = textures[i].type;//�����O���� material texture

				if (name == "texture_diffuse") {
					number = std::to_string(diffuseNr++);//�]���O i++ �ҥH�|���]��to_string���� diffuseNr�~�|+1
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
				cout << "�e�Jshader�emtl���check" << endl;
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
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);//�`�@���X�ճ��I��Ʋ�(���I �k�V�q �K�Ϯy��)*�s�@�ճ��I��Ʋժ��Ŷ�->�p��X�`�j�p

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);//�ĥ�index render
			
			//���t��� ���O���}VBO shader�����C�@��layout

			//���I���
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			//�k�V�q
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			//�K�Ϯy��
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

