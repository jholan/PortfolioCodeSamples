#include "Engine/Core/StaticScene.hpp"

//-----------------------------------------------------------------------------------------------
// Link in the appropriate Assimp static library (32-bit or 64-bit)
//
#if defined( _WIN64 )
#pragma comment( lib, "ThirdParty/assimp/assimp-vc140-mt64.lib" )
#else
#pragma comment( lib, "ThirdParty/assimp/assimp-vc140-mt.lib" )
#endif

#define ASSIMP_DLL
#include "ThirdParty/assimp/Importer.hpp"
#include "ThirdParty/assimp/scene.h"
#include "ThirdParty/assimp/postprocess.h"



// ----------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.h"
#include "Engine/Rendering/DebugRender.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/RGBA.hpp"
#include "Engine/Rendering/Light.hpp"
#include "Engine/Rendering/Renderable.hpp"
#include "Engine/Rendering/Renderer.hpp"
#include "Engine/Rendering/MeshBuilder.hpp"



StaticScene::StaticScene()
{

}



StaticScene::~StaticScene()
{

}



Matrix4 ConvertMatrix(const aiMatrix4x4& mat)
{
	Matrix4 matrix;

	// For sanity set every value that is essentially 0.0f to 0.0f
	aiMatrix4x4 tempAiMat = mat;
	float* matrixValue = &tempAiMat.a1;
	for (int i = 0; i < 16; ++i)
	{
		if (FloatEquals(*matrixValue, 0.0f, 0.001f))
		{
			*matrixValue = 0.0f;
		}
	
		++matrixValue;
	}

	Matrix4 C = Matrix4(-Vector4::X_AXIS, Vector4::Y_AXIS, Vector4::Z_AXIS, Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	Vector4 i = Vector4(tempAiMat.a1, tempAiMat.b1, tempAiMat.c1, tempAiMat.d1);
	Vector4 j = Vector4(tempAiMat.a2, tempAiMat.b2, tempAiMat.c2, tempAiMat.d2);
	Vector4 k = Vector4(tempAiMat.a3, tempAiMat.b3, tempAiMat.c3, tempAiMat.d3);
	Vector4 t = Vector4(tempAiMat.a4, tempAiMat.b4, tempAiMat.c4, tempAiMat.d4);
	matrix = Matrix4(i, j, k, t);
	matrix.Tw = 1.0f;

	Matrix4 out = C * matrix * C;

	//out.Tx *= -1.0f;


	return out;
}


Vector3 ConvertVector3(const aiVector3D& vec)
{
	Vector3 vector = Vector3(-vec.x, vec.y, vec.z);
	return vector;
}



float ExtractEnergyFromColor(const aiColor3D& vec)
{
	float energy = 1.0f;

	// At least 1 color channel must have been specified as 255 for this to work
	float largestValue = Max(vec.r, vec.g);
	largestValue = Max(largestValue, vec.b);
	energy = largestValue;

	return energy;
}



RGBA ConvertColor(const aiColor3D& vec)
{
	float energy = ExtractEnergyFromColor(vec);
	Vector3 extractedColor = Vector3(vec.r, vec.g, vec.b) * (1.0f / energy);

	RGBA color;
	color.SetAsFloats(extractedColor.x, extractedColor.y, extractedColor.z, 1.0f);

	return color;
}



RGBA ConvertColor4(const aiColor4D& vec)
{
	RGBA color;
	color.SetAsFloats(vec.r, vec.g, vec.b, vec.a);

	return color;
}



Vector3 ConvertEulerAngles(const Vector3& eulers)
{
	Vector3 correctedEulers;

	correctedEulers.x = -eulers.x;
	correctedEulers.y = -eulers.y;
	correctedEulers.z = -eulers.z;


	return correctedEulers;
}



#include "Engine/Core/Transform.hpp"
void StaticScene::ImportFile(const std::string& filepath)
{
	// Load the scene
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace); 
	
	// If the scene doesn't exist or corrupt
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
	{
		DebugDraw_Log(30.0f, RGBA(255, 0, 0), "StaticScene %s could not be loaded", filepath.c_str());
		DebugDraw_Log(30.0f, RGBA(255, 0, 0), "    Error: %s", importer.GetErrorString());
		return;
	}
	aiNode* rootNode = scene->mRootNode;

	// Process Lights
	for (int i = 0; i < (int)scene->mNumLights; ++i)
	{
		aiLight* sLight = scene->mLights[i];
		aiString sName = sLight->mName;
		aiNode* sLightNode = rootNode->FindNode(sName);
		

		Matrix4 sLightNodeTransformMatrix = ConvertMatrix(sLightNode->mTransformation);
		Vector3 trans = sLightNodeTransformMatrix.GetTranslation();
		Transform sLightNodeTransform = Transform(sLightNodeTransformMatrix);

		Vector4 iBasis = Vector4(sLightNode->mTransformation.a1, sLightNode->mTransformation.b1, sLightNode->mTransformation.c1, sLightNode->mTransformation.d1);
		Vector4 jBasis = Vector4(sLightNode->mTransformation.a2, sLightNode->mTransformation.b2, sLightNode->mTransformation.c2, sLightNode->mTransformation.d2);
		Vector4 kBasis = Vector4(sLightNode->mTransformation.a3, sLightNode->mTransformation.b3, sLightNode->mTransformation.c3, sLightNode->mTransformation.d3);
		Vector4 tBasis = Vector4(sLightNode->mTransformation.a4, sLightNode->mTransformation.b4, sLightNode->mTransformation.c4, sLightNode->mTransformation.d4);
		Matrix4 tM = Matrix4(iBasis, jBasis, kBasis, tBasis);
		Vector3 tPos = (tM * Vector4(0.0f, 0.0f, 0.0f, 1.0f)).XYZ();
		tPos.x *= -1.0f;
		Vector3 tDir = (tM * Vector4(ConvertVector3(sLight->mDirection), 0.0f)).XYZ();
		tDir.x *= -1.0f;

		Vector3 attenuation = Vector3(sLight->mAttenuationConstant, sLight->mAttenuationLinear, sLight->mAttenuationQuadratic); 
		Vector3 direction = (sLightNodeTransformMatrix * Vector4(ConvertVector3(sLight->mDirection), 0.0f)).XYZ().GetNormalized();
		direction.x *= -1.0f;
		RGBA	color = ConvertColor(sLight->mColorDiffuse);
		float	energy = ExtractEnergyFromColor(sLight->mColorDiffuse);
		float	innerAngleDegrees = ConvertRadiansToDegrees(sLight->mAngleInnerCone) / 2.0f;
		float   outerAngleDegrees = ConvertRadiansToDegrees(sLight->mAngleOuterCone) / 2.0f;

		Vector3 wPos = (sLightNodeTransformMatrix * Vector4(0.0f, 0.0f, 0.0f, 1.0f)).XYZ();/*sLightNodeTransform.GetWorldPosition()*/;
		wPos.x *= -1.0f;
		Vector3 wDir = direction;
		DebugDraw_WireSphere(1000.0f, /*wPos*/tPos, 0.25f, RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH );
		DebugDraw_LineSegment(1000.0f, /*wPos*/tPos, RGBA(), /*wPos + (wDir * 10.0f)*/tPos + (tDir * 10.0f), RGBA(), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);

		Light* light = new Light();
		switch(sLight->mType)
		{
		case aiLightSource_DIRECTIONAL:
		{
			light->InitializeDirectionalLight(tDir, color, energy);
			break;
		}

		case aiLightSource_POINT:
		{
			light->InitializePointLight(tPos, color, energy, attenuation);
			break;
		}

		case aiLightSource_SPOT:
		{
			light->InitializeConeLight(tPos, tDir, innerAngleDegrees, outerAngleDegrees, color, energy, attenuation);
			break;
		}
		default:
			break;
		}
		m_lights.push_back(light);
	}


	ProcessNodeForMeshData(Matrix4(), (void*)rootNode, (void*)scene);
}



Mesh* ParseMesh(const aiMesh* sMesh)
{
	MeshBuilder builder;
	builder.Initialize(Renderer::PRIMITIVE_TRIANGLES, true);

	// Add all of the vertex data
	std::vector<VertexBuilder>& vertices = builder.GetVertices();
	vertices.reserve(sMesh->mNumVertices);
	for (int i = 0; i < (int)sMesh->mNumVertices; ++i)
	{
		VertexBuilder vertex;

		if (sMesh->HasPositions())
		{
			vertex.m_position = ConvertVector3(sMesh->mVertices[i]);
			//vertex.m_position.x *= -1.0f;
		}

		if (sMesh->HasVertexColors(0))
		{
			vertex.m_color = ConvertColor4(sMesh->mColors[0][i]);
		}

		if (sMesh->HasNormals())
		{
			vertex.m_normal = ConvertVector3(sMesh->mNormals[i]);
			//vertex.m_normal.x *= -1.0f;
			//DebugDraw_LineSegment(1000.0f, vertex.m_position, RGBA(0,0,255), vertex.m_position + (vertex.m_normal), RGBA(0,0,255), RGBA(), RGBA(), DEBUG_RENDER_USE_DEPTH);
		}

		if (sMesh->HasTextureCoords(0))
		{
			Vector3 UVWs = ConvertVector3(sMesh->mTextureCoords[0][i]);
			vertex.m_UVs = Vector2(sMesh->mTextureCoords[0][i].x, sMesh->mTextureCoords[0][i].y);
		}

		if (sMesh->HasTangentsAndBitangents())
		{
			Vector3 tangent = ConvertVector3(sMesh->mTangents[i]);
			//tangent.x *= -1.0f;
			Vector3 bitangent = ConvertVector3(sMesh->mBitangents[i]);
			//bitangent.x *= -1.0f;
			Vector3 testBitangent = CrossProduct(vertex.m_normal, tangent);
			float bitanDirection = 1.0f;
			if (DotProduct(bitangent, testBitangent) < 0.0f)
			{
				bitanDirection = -1.0f;
			}
			vertex.m_tangent = Vector4(tangent, bitanDirection);
		}
		vertices.push_back(vertex);
	}


	// Add all of the index data
	std::vector<unsigned int>& indices = builder.GetIndices();
	indices.reserve(sMesh->mNumFaces * 3);						// all faces are triangles because we triangulated the scene on load
	for(int i = 0; i < (int)sMesh->mNumFaces; i++)
	{
		aiFace face = sMesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for(int j = 0; j < (int)face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}


	builder.Finalize();
	Mesh* finalMesh = builder.CreateMesh();
	return finalMesh;
}


 void StaticScene::ProcessNodeForMeshData(const Matrix4& parentMatrix, void* tnode, void* tscene)
 {
	aiNode* node = (aiNode*)tnode;
	const aiScene* scene = (const aiScene*)tscene;
	

 	Matrix4 localTransform = ConvertMatrix(node->mTransformation);
	Matrix4 transformMatrix = parentMatrix * localTransform;

	std::string name = node->mName.C_Str();
	//DebugDraw_Log(100.0f, "%s (%i meshes)", name.c_str(), node->mNumMeshes);
 	
	for (int i = 0; i < (int)node->mNumMeshes; ++i)
	{
		unsigned int meshIndex = node->mMeshes[i];
		aiMesh* sMesh = scene->mMeshes[meshIndex];
		
		aiMaterial* sMat = scene->mMaterials[sMesh->mMaterialIndex];
		aiString sMatName;
		sMat->Get(AI_MATKEY_NAME, sMatName);
		std::string matName = sMatName.C_Str();

		Renderable* renderable = new Renderable();
		renderable->SetModelMatrix(transformMatrix);
		Mesh* mesh = ParseMesh(sMesh);
		renderable->SetMesh(mesh);
		renderable->SetMaterial(matName);
		m_renderables.push_back(renderable);
	}

 	for (int i = 0; i < (int)node->mNumChildren; ++i)
 	{
 		aiNode* childNode = node->mChildren[i];
 		ProcessNodeForMeshData(transformMatrix, (void*)childNode, (void*)scene);
 	}
 }



const std::vector<Camera*>& StaticScene::GetCameras() const
{
	return m_cameras;
}



const std::vector<Camera*>& StaticScene::GetCameras()
{
	return m_cameras;
}

const std::vector<Light*>& StaticScene::GetLights() const
{
	return m_lights;
}



const std::vector<Light*>& StaticScene::GetLights()
{
	return m_lights;
}



const std::vector<Renderable*>& StaticScene::GetRenderables() const
{
	return m_renderables;
}



const std::vector<Renderable*>& StaticScene::GetRenderables()
{
	return m_renderables;
}
