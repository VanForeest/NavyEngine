#pragma once

#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

GLuint TextureFromFile(const std::string& Path, const std::string& directory, const aiScene* scene, bool gamma = false);

class Model{
public:

	std::vector<Texture> textures_loaded;
	std::vector<Mesh> Meshes;
	std::string directory;
	bool gammaCorrection;
	bool hasBaseColorMap = false;
	bool hasNormalMap = false;
    bool hasORM = false;

	Model(const std::string& ModelPath, bool gamma = false){
	
		LoadModel(ModelPath);
		gammaCorrection = gamma;
	}

	void Draw(Shader& shader){
	
		for (unsigned int i = 0; i < Meshes.size(); i++)
			Meshes[i].Draw(shader);
	}

private:

	void LoadModel(const std::string& Path){
		
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		directory = Path.substr(0, Path.find_last_of('/'));
		
		processNode(scene->mRootNode, scene, glm::mat4(1.0f));
	}

	void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform){
	
        aiMatrix4x4 aiMat = node->mTransformation;
        glm::mat4 localTransform(
            aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
            aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
            aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
            aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
        );
        glm::mat4 globalTransform = parentTransform * localTransform;

		unsigned int i;
		aiMesh* mesh;

		for (i = 0; i < node->mNumMeshes; i++){
			
			mesh = scene->mMeshes[node->mMeshes[i]];
			Meshes.push_back(processMesh(mesh, scene, globalTransform));
		}

		for (i = 0; i < node->mNumChildren; i++)
			processNode(node->mChildren[i], scene, globalTransform);
	
	}


	Mesh processMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 transform){
		
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

		Vertex vertex;
		glm::vec3 vector; //Vector usado para asimilar las propiedades del vertice

		unsigned int i, j;
		aiFace face;

		for (i = 0; i < mesh->mNumVertices; i++){
		
			//Posiciones 
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = glm::vec3(transform * glm::vec4(vector, 1.0f));

            // Posicion Objetivo para Animaciones (Shape Keys)
            if (mesh->mNumAnimMeshes > 0 && mesh->mAnimMeshes[0]->HasPositions()) {
                glm::vec3 vectorTarget;
                vectorTarget.x = mesh->mAnimMeshes[0]->mVertices[i].x;
                vectorTarget.y = mesh->mAnimMeshes[0]->mVertices[i].y;
                vectorTarget.z = mesh->mAnimMeshes[0]->mVertices[i].z;
                vertex.PositionTarget = glm::vec3(transform * glm::vec4(vectorTarget, 1.0f));
            } else {
                vertex.PositionTarget = vertex.Position; // Si no hay shape key, se queda estatico
            }

			//Normales
			if(mesh->HasNormals()){
			
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = glm::normalize(normalMatrix * vector);
			}

			//TextureUV
			if(mesh->mTextureCoords[0]){
				
				//UV coordinates
				vector.x = mesh->mTextureCoords[0][i].x;
				vector.y = mesh->mTextureCoords[0][i].y;
				vertex.TextureUV.x = vector.x;
				vertex.TextureUV.y = vector.y;

				//Tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = glm::normalize(normalMatrix * vector);

				//Bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = glm::normalize(normalMatrix * vector);

			}else{
				vertex.TextureUV = glm::vec2(0.0f);
			}

			vertices.push_back(vertex);
		}

		for (i = 0; i < mesh->mNumFaces; i++){
			
			face = mesh->mFaces[i];
			for (j = 0; j < face.mNumIndices; j++){	
				indices.push_back(face.mIndices[j]);
			}
		}

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		//Materiales PBR: BaseColor-Albedo // NormalMap // RoughnessMap // MetallicMap // AOmap

		//1. BaseColor-Albedo
		std::vector<Texture> BaseColorMaps = LoadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_BaseColor", scene);
		if(BaseColorMaps.empty()){
			BaseColorMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_BaseColor", scene);
		}
		textures.insert(textures.end(), BaseColorMaps.begin(), BaseColorMaps.end());

		//2. NormalMap
		std::vector<Texture> NormalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, "texture_Normal", scene);
		if(NormalMaps.empty()){
			NormalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_Normal", scene);
		}
		textures.insert(textures.end(), NormalMaps.begin(), NormalMaps.end());

		//Para los mapas restantes, utilizamos el estandar GLTF y a rezar de que blender meta el AO en el canal
		//R, esta gente de verdad no se decide (U n U)

		//3.Estandar ORM (O(AmbientOclusion)-> R;  R(Roghness)-> G; M(Metallic)-> B)
		std::vector<Texture> ORM_Maps = LoadMaterialTextures(material, aiTextureType_GLTF_METALLIC_ROUGHNESS, "texture_ORM", scene);
		textures.insert(textures.end(), ORM_Maps.begin(), ORM_Maps.end());


		/*// 1. diffuse maps
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		*/


		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> LoadMaterialTextures(aiMaterial* Mat, aiTextureType Type, const std::string& typeName, const aiScene* scene){

		std::vector<Texture> textures;

		for (unsigned int i = 0; i < Mat->GetTextureCount(Type); i++){
		
			aiString str;
			Mat->GetTexture(Type, i, &str);

			bool skip = false;

			for (unsigned int j = 0; j < textures_loaded.size(); j++){
				
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0){
				
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}

			if(!skip){
			
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory, scene);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}

			if (Type == aiTextureType_NORMALS)
				hasNormalMap = true;
			if (Type == aiTextureType_GLTF_METALLIC_ROUGHNESS)
				hasORM = true;
			if (Type == aiTextureType_BASE_COLOR || Type == aiTextureType_DIFFUSE)
				hasBaseColorMap = true;
		}

		return textures;
	}
};

GLuint TextureFromFile(const std::string& Path, const std::string& directory, const aiScene* scene, bool gamma){
	
	GLuint TextureID;
	glGenTextures(1, &TextureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = nullptr;
	bool isUncompressed = false;

	const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(Path.c_str());

	if (embeddedTexture) {
		if (embeddedTexture->mHeight == 0) {
			data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTexture->pcData), embeddedTexture->mWidth, &width, &height, &nrComponents, 0);
		} else {
			isUncompressed = true;
			width = embeddedTexture->mWidth;
			height = embeddedTexture->mHeight;
			nrComponents = 4; 
			int dataSize = width * height * 4;
			data = (unsigned char*)malloc(dataSize);
			unsigned char* src = reinterpret_cast<unsigned char*>(embeddedTexture->pcData);
			for(int i=0; i < dataSize; i+=4) {
				data[i] = src[i+2];   // R
				data[i+1] = src[i+1]; // G
				data[i+2] = src[i];   // B
				data[i+3] = src[i+3]; // A
			}
		}
	} else {
		std::string filename = directory + '/' + Path;
		data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if(!data){		
			std::cout << "ERROR::FAILED_TO_LOAD_TEXTURE::" << filename << std::endl;
		}
	}

	if(data){
	
	    int format = GL_RED;

		if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		// Critical fix for Access Violation (0xc0000005)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, TextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

	}

	if (data) {
		if (isUncompressed) free(data);
		else stbi_image_free(data);
	}

	return TextureID;

}
