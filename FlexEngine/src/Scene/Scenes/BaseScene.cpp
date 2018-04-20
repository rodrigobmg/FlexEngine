#include "stdafx.hpp"

#include "Scene/Scenes/BaseScene.hpp"

#include <glm/vec3.hpp>

#include "Scene/GameObject.hpp"
#include "Logger.hpp"
#include "Physics/PhysicsWorld.hpp"
#include "JSONParser.hpp"

#include "Physics/PhysicsWorld.hpp"
#include "Physics/PhysicsManager.hpp"
#include "Physics/RigidBody.hpp"
#include "Scene/ReflectionProbe.hpp"
#include "Scene/MeshPrefab.hpp"
#include "Helpers.hpp"

namespace flex
{
	BaseScene::BaseScene(const std::string& name) :
		m_Name(name)
	{
	}

	BaseScene::~BaseScene()
	{
		auto iter = m_Children.begin();
		while (iter != m_Children.end())
		{
			delete *iter;
			iter = m_Children.erase(iter);
		}

		if (m_PhysicsWorld)
		{
			m_PhysicsWorld->Destroy();
			SafeDelete(m_PhysicsWorld);
		}
	}

	void BaseScene::InitializeFromJSON(const std::string& jsonFilePath, const GameContext& gameContext)
	{
		ParsedJSONFile parsedFile;

		JSONParser::Parse(jsonFilePath, parsedFile);

		Logger::LogInfo("Parsed scene file:");
		Logger::LogInfo(parsedFile.rootObject.Print(0));

		std::string sceneName = parsedFile.rootObject.GetString("name");
		m_Name = sceneName;

		// This holds all the entities in the scene which do not have a parent
		std::vector<JSONObject> rootEntities = parsedFile.rootObject.GetObjectArray("entities");
		for (const auto& rootEntity : rootEntities)
		{
			AddChild(gameContext, CreateEntityFromJSON(gameContext, rootEntity));
		}
	}

	GameObject* BaseScene::CreateEntityFromJSON(const GameContext& gameContext, const JSONObject& obj)
	{
		GameObject* result = nullptr;

		std::string objectName = obj.GetString("name");

		JSONObject meshObj = obj.GetObject("mesh");
		std::string meshFilePath = meshObj.GetString("file");
		std::string meshPrefabName = meshObj.GetString("prefab");
		bool flipNormalYZ = meshObj.GetBool("flipNormalYZ");
		bool flipZ = meshObj.GetBool("flipZ");
		bool flipU = meshObj.GetBool("flipU");
		bool flipV = meshObj.GetBool("flipV");

		bool visibleInSceneGraph = obj.HasField("visibleInSceneGraph") ? obj.GetBool("visibleInSceneGraph") : true;
		bool visible = obj.HasField("visibleInSceneGraph") ? obj.GetBool("visible") : true;

		JSONObject transformObj = obj.GetObject("transform");
		Transform transform = JSONParser::ParseTransform(transformObj);

		JSONObject material = obj.GetObject("material");
		std::string materialName = material.GetString("name");
		std::string shaderName = material.GetString("shader");

		if (shaderName.empty())
		{
			Logger::LogError("Shader name not set in material " + materialName);
			return nullptr;
		}


		MaterialCreateInfo matCreateInfo = {};
		{
			matCreateInfo.name = materialName;
			matCreateInfo.shaderName = shaderName;

			struct FilePathMaterialParam
			{
				std::string* member;
				std::string name;
			};

			std::vector<FilePathMaterialParam> filePathParams =
			{
				{ &matCreateInfo.diffuseTexturePath, "diffuseTexturePath" },
				{ &matCreateInfo.normalTexturePath, "normalTexturePath" },
				{ &matCreateInfo.albedoTexturePath, "albedoTexturePath" },
				{ &matCreateInfo.metallicTexturePath, "metallicTexturePath" },
				{ &matCreateInfo.roughnessTexturePath, "roughnessTexturePath" },
				{ &matCreateInfo.aoTexturePath, "aoTexturePath" },
				{ &matCreateInfo.hdrEquirectangularTexturePath, "hdrEquirectangularTexturePath" },
				{ &matCreateInfo.environmentMapPath, "environmentMapPath" },
			};

			for (u32 i = 0; i < filePathParams.size(); ++i)
			{
				if (material.HasField(filePathParams[i].name))
				{
					*filePathParams[i].member = RESOURCE_LOCATION + material.GetString(filePathParams[i].name);
				}
			}


			struct BoolMaterialParam
			{
				bool* member;
				std::string name;
			};

			std::vector<BoolMaterialParam> boolParams =
			{
				{ &matCreateInfo.generateDiffuseSampler, "generateDiffuseSampler" },
				{ &matCreateInfo.enableDiffuseSampler, "enableDiffuseSampler" },
				{ &matCreateInfo.generateNormalSampler, "generateNormalSampler" },
				{ &matCreateInfo.enableNormalSampler, "enableNormalSampler" },
				{ &matCreateInfo.generateAlbedoSampler, "generateAlbedoSampler" },
				{ &matCreateInfo.enableAlbedoSampler, "enableAlbedoSampler" },
				{ &matCreateInfo.generateMetallicSampler, "generateMetallicSampler" },
				{ &matCreateInfo.enableMetallicSampler, "enableMetallicSampler" },
				{ &matCreateInfo.generateRoughnessSampler, "generateRoughnessSampler" },
				{ &matCreateInfo.enableRoughnessSampler, "enableRoughnessSampler" },
				{ &matCreateInfo.generateAOSampler, "generateAOSampler" },
				{ &matCreateInfo.enableAOSampler, "enableAOSampler" },
				{ &matCreateInfo.generateHDREquirectangularSampler, "generateHDREquirectangularSampler" },
				{ &matCreateInfo.enableHDREquirectangularSampler, "enableHDREquirectangularSampler" },
				{ &matCreateInfo.generateHDRCubemapSampler, "generateHDRCubemapSampler" },
				{ &matCreateInfo.enableIrradianceSampler, "enableIrradianceSampler" },
				{ &matCreateInfo.generateIrradianceSampler, "generateIrradianceSampler" },
				{ &matCreateInfo.enableBRDFLUT, "enableBRDFLUT" },
				{ &matCreateInfo.renderToCubemap, "renderToCubemap" },
				{ &matCreateInfo.enableCubemapSampler, "enableCubemapSampler" },
				{ &matCreateInfo.enableCubemapTrilinearFiltering, "enableCubemapTrilinearFiltering" },
				{ &matCreateInfo.generateCubemapSampler, "generateCubemapSampler" },
				{ &matCreateInfo.generateCubemapDepthBuffers, "generateCubemapDepthBuffers" },
				{ &matCreateInfo.generatePrefilteredMap, "generatePrefilteredMap" },
				{ &matCreateInfo.enablePrefilteredMap, "enablePrefilteredMap" },
				{ &matCreateInfo.generateReflectionProbeMaps, "generateReflectionProbeMaps" },
			};

			for (u32 i = 0; i < boolParams.size(); ++i)
			{
				if (material.HasField(boolParams[i].name))
				{
					*boolParams[i].member = material.GetBool(boolParams[i].name);
				}
			}

			if (material.HasField("colorMultiplier"))
			{
				std::string colorStr = material.GetString("colorMultiplier");
				matCreateInfo.colorMultiplier = JSONParser::ParseColor4(colorStr);
			}

			if (material.HasField("generatedIrradianceCubemapSize"))
			{
				std::string irradianceCubemapSizeStr = material.GetString("generatedIrradianceCubemapSize");
				matCreateInfo.generatedIrradianceCubemapSize = JSONParser::ParseVec2(irradianceCubemapSizeStr);
			}

			//std::vector<std::pair<std::string, void*>> frameBuffers; // Pairs of frame buffer names (as seen in shader) and IDs
			//matCreateInfo.irradianceSamplerMatID = InvalidMaterialID; // The id of the material who has an irradiance sampler object (generateIrradianceSampler must be false)
			//matCreateInfo.prefilterMapSamplerMatID = InvalidMaterialID;

			//matCreateInfo.cubeMapFilePaths; // RT, LF, UP, DN, BK, FT

			if (material.HasField("generatedCubemapSize"))
			{
				std::string generatedCubemapSizeStr = material.GetString("generatedCubemapSize");
				matCreateInfo.generatedCubemapSize = JSONParser::ParseVec2(generatedCubemapSizeStr);
			}

			if (material.HasField("generatedPrefilteredCubemapSize"))
			{
				std::string generatedPrefilteredCubemapSizeStr = material.GetString("generatedPrefilteredCubemapSize");
				matCreateInfo.generatedPrefilteredCubemapSize = JSONParser::ParseVec2(generatedPrefilteredCubemapSizeStr);
			}

			if (material.HasField("constAlbedo"))
			{
				std::string albedoStr = material.GetString("constAlbedo");
				matCreateInfo.constAlbedo = JSONParser::ParseColor3(albedoStr);
			}

			if (material.HasField("constMetallic"))
			{
				matCreateInfo.constMetallic = material.GetFloat("constMetallic");
			}

			if (material.HasField("constRoughness"))
			{
				matCreateInfo.constRoughness = material.GetFloat("constRoughness");
			}

			if (material.HasField("constAO"))
			{
				matCreateInfo.constAO = material.GetFloat("constAO");
			}
		}
		MaterialID matID = gameContext.renderer->InitializeMaterial(gameContext, &matCreateInfo);

		if (!meshFilePath.empty())
		{
			MeshPrefab* mesh = new MeshPrefab(matID, objectName);

			RenderObjectCreateInfo createInfo = {};
			createInfo.visibleInSceneExplorer = visibleInSceneGraph;

			if (meshObj.HasField("cullFace"))
			{
				std::string cullFaceStr = meshObj.GetString("cullFace");
				CullFace cullFace = StringToCullFace(cullFaceStr);
				createInfo.cullFace = cullFace;
			}
			mesh->LoadFromFile(gameContext, RESOURCE_LOCATION + meshFilePath,
				flipNormalYZ,
				flipZ,
				flipU,
				flipV,
				&createInfo);

			mesh->GetTransform() = transform;

			result = mesh;
		}
		else if (!meshPrefabName.empty())
		{
			MeshPrefab* mesh = new MeshPrefab(matID, objectName);

			RenderObjectCreateInfo createInfo = {};
			createInfo.visibleInSceneExplorer = visibleInSceneGraph;

			if (meshObj.HasField("cullFace"))
			{
				std::string cullFaceStr = meshObj.GetString("cullFace");
				CullFace cullFace = StringToCullFace(cullFaceStr);
				createInfo.cullFace = cullFace;
			}
			MeshPrefab::PrefabShape prefabShape = MeshPrefab::PrefabShapeFromString(meshPrefabName);
			mesh->LoadPrefabShape(gameContext, prefabShape, &createInfo);

			mesh->GetTransform() = transform;

			if (!visible)
			{
				gameContext.renderer->SetRenderObjectVisible(mesh->GetRenderID(), visible);
			}

			result = mesh;
		}
		else
		{
			GameObject* gameObject = new GameObject();
			// TODO: Throw error here?
			result = gameObject;
		}

		if (result && obj.HasField("children"))
		{
			std::vector<JSONObject> children = obj.GetObjectArray("children");
			for (const auto& child : children)
			{
				result->AddChild(CreateEntityFromJSON(gameContext, child));
			}
		}

		return result;
	}

	std::string BaseScene::GetName() const
	{
		return m_Name;
	}

	PhysicsWorld* BaseScene::GetPhysicsWorld()
	{
		return m_PhysicsWorld;
	}

	void BaseScene::Initialize(const GameContext& gameContext)
	{
		m_PhysicsWorld = new PhysicsWorld();
		m_PhysicsWorld->Initialize(gameContext);

		m_PhysicsWorld->GetWorld()->setGravity({ 0.0f, -9.81f, 0.0f });


		MaterialCreateInfo skyboxHDRMatInfo = {};
		skyboxHDRMatInfo.name = "HDR Skybox";
		skyboxHDRMatInfo.shaderName = "background";
		skyboxHDRMatInfo.generateHDRCubemapSampler = true;
		skyboxHDRMatInfo.enableCubemapSampler = true;
		skyboxHDRMatInfo.enableCubemapTrilinearFiltering = true;
		skyboxHDRMatInfo.generatedCubemapSize = { 512, 512 };
		skyboxHDRMatInfo.generateIrradianceSampler = true;
		skyboxHDRMatInfo.generatedIrradianceCubemapSize = { 32, 32 };
		skyboxHDRMatInfo.generatePrefilteredMap = true;
		skyboxHDRMatInfo.generatedPrefilteredCubemapSize = { 128, 128 };
		skyboxHDRMatInfo.environmentMapPath = RESOURCE_LOCATION + "textures/hdri/Milkyway/Milkyway_Light.hdr";
		MaterialID skyboxMatID = gameContext.renderer->InitializeMaterial(gameContext, &skyboxHDRMatInfo);
		gameContext.renderer->SetSkyboxMaterial(skyboxMatID);

		// Generated last so it can use generated skybox maps
		m_ReflectionProbe = new ReflectionProbe(true);
		AddChild(gameContext, m_ReflectionProbe);
	}

	void BaseScene::PostInitialize(const GameContext& gameContext)
	{
		gameContext.renderer->SetReflectionProbeMaterial(m_ReflectionProbe->GetCaptureMaterialID());
	}

	void BaseScene::Destroy(const GameContext& gameContext)
	{
	}

	void BaseScene::Update(const GameContext& gameContext)
	{
	}

	void BaseScene::AddChild(const GameContext& gameContext, GameObject* gameObject)
	{
		UNREFERENCED_PARAMETER(gameContext);

		for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
		{
			if (*iter == gameObject)
			{
				Logger::LogWarning("Attempting to add child to scene again");
				return;
			}
		}

		m_Children.push_back(gameObject);
	}

	void BaseScene::RemoveChild(GameObject* gameObject, bool deleteChild)
	{
		auto iter = m_Children.begin();
		while (iter != m_Children.end())
		{
			if (*iter == gameObject)
			{
				if (deleteChild)
				{
					SafeDelete(*iter);
				}

				iter = m_Children.erase(iter);
				return;
			}
			else
			{
				++iter;
			}
		}

		Logger::LogWarning("Attempting to remove non-existent child from scene");
	}

	void BaseScene::RemoveAllChildren(bool deleteChildren)
	{
		auto iter = m_Children.begin();
		while (iter != m_Children.end())
		{
			if (deleteChildren)
			{
				delete *iter;
			}

			iter = m_Children.erase(iter);
		}
	}

	void BaseScene::RootInitialize(const GameContext& gameContext)
	{
		Initialize(gameContext);

		for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
		{
			(*iter)->Initialize(gameContext);
		}
	}

	void BaseScene::RootPostInitialize(const GameContext& gameContext)
	{
		PostInitialize(gameContext);

		for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
		{
			(*iter)->PostInitialize(gameContext);
		}
	}

	void BaseScene::RootUpdate(const GameContext& gameContext)
	{
		if (m_PhysicsWorld)
		{
			m_PhysicsWorld->Update(gameContext.deltaTime);
		}

		Update(gameContext);

		for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
		{
			(*iter)->RootUpdate(gameContext);
		}
	}

	void BaseScene::RootDestroy(const GameContext& gameContext)
	{
		Destroy(gameContext);

		for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
		{
			(*iter)->RootDestroy(gameContext);
		}
	}
} // namespace flex
