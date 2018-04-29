
#include "stdafx.hpp"

#include "Scene/ReflectionProbe.hpp"
#include "Scene/MeshPrefab.hpp"
#include "Scene/GameObject.hpp"

namespace flex
{
	ReflectionProbe::ReflectionProbe(const std::string& name, bool startVisible, const glm::vec3& startPosition) :
		GameObject(name, SerializableType::REFLECTION_PROBE),
		m_StartVisible(startVisible),
		m_StartPosition(startPosition)
	{
	}

	ReflectionProbe::~ReflectionProbe()
	{
	}

	void ReflectionProbe::Initialize(const GameContext& gameContext)
	{
		// Reflective chrome ball material
		MaterialCreateInfo reflectionProbeMaterialCreateInfo = {};
		reflectionProbeMaterialCreateInfo.name = "Reflection probe sphere";
		reflectionProbeMaterialCreateInfo.shaderName = "pbr";
		reflectionProbeMaterialCreateInfo.constAlbedo = glm::vec3(0.75f, 0.75f, 0.75f);
		reflectionProbeMaterialCreateInfo.constMetallic = 1.0f;
		reflectionProbeMaterialCreateInfo.constRoughness = 0.0f;
		reflectionProbeMaterialCreateInfo.constAO = 1.0f;
		MaterialID reflectionProbeMaterialID = gameContext.renderer->InitializeMaterial(gameContext, &reflectionProbeMaterialCreateInfo);

		// Probe capture material
		MaterialCreateInfo probeCaptureMatCreateInfo = {};
		probeCaptureMatCreateInfo.name = "Reflection probe capture";
		probeCaptureMatCreateInfo.shaderName = "deferred_combine_cubemap";
		probeCaptureMatCreateInfo.generateReflectionProbeMaps = true;
		probeCaptureMatCreateInfo.generateHDRCubemapSampler = true;
		probeCaptureMatCreateInfo.generatedCubemapSize = glm::vec2(512.0f, 512.0f); // TODO: Add support for non-512.0f size
		probeCaptureMatCreateInfo.generateCubemapDepthBuffers = true;
		probeCaptureMatCreateInfo.enableIrradianceSampler = true;
		probeCaptureMatCreateInfo.generateIrradianceSampler = true;
		probeCaptureMatCreateInfo.generatedIrradianceCubemapSize = { 32, 32 };
		probeCaptureMatCreateInfo.enablePrefilteredMap = true;
		probeCaptureMatCreateInfo.generatePrefilteredMap = true;
		probeCaptureMatCreateInfo.generatedPrefilteredCubemapSize = { 128, 128 };
		probeCaptureMatCreateInfo.enableBRDFLUT = true;
		probeCaptureMatCreateInfo.frameBuffers = {
			{ "positionMetallicFrameBufferSampler", nullptr },
			{ "normalRoughnessFrameBufferSampler", nullptr },
			{ "albedoAOFrameBufferSampler", nullptr },
		};
		m_CaptureMatID = gameContext.renderer->InitializeMaterial(gameContext, &probeCaptureMatCreateInfo);


		m_SphereMesh = new MeshPrefab(reflectionProbeMaterialID, "Reflection probe mesh");
		MeshPrefab::ImportSettings importSettings = {};
		importSettings.swapNormalYZ = true;
		importSettings.flipNormalZ = true;
		m_SphereMesh->LoadFromFile(gameContext, RESOURCE_LOCATION + "models/ico-sphere.gltf", &importSettings);
		AddChild(m_SphereMesh);
		m_SphereMesh->GetTransform()->Scale(1.5f);

		if (!m_StartVisible)
		{
			SetSphereVisible(m_StartVisible, gameContext);
		}

		std::string captureName = m_Name + " capture";
		m_Capture = new GameObject(captureName, SerializableType::NONE);
		m_Capture->SetVisible(false);

		RenderObjectCreateInfo captureObjectCreateInfo = {};
		captureObjectCreateInfo.vertexBufferData = nullptr;
		captureObjectCreateInfo.materialID = m_CaptureMatID;
		captureObjectCreateInfo.gameObject = m_Capture;
		captureObjectCreateInfo.visibleInSceneExplorer = false;
		
		RenderID captureRenderID = gameContext.renderer->InitializeRenderObject(gameContext, &captureObjectCreateInfo);
		m_Capture->SetRenderID(captureRenderID);

		m_SphereMesh->AddChild(m_Capture);

		m_Transform.SetGlobalPosition(m_StartPosition);
	}

	void ReflectionProbe::PostInitialize(const GameContext& gameContext)
	{
		gameContext.renderer->SetReflectionProbeMaterial(GetCaptureMaterialID());
		GameObject::PostInitialize(gameContext);
	}

	MaterialID ReflectionProbe::GetCaptureMaterialID() const
	{
		return m_CaptureMatID;
	}

	bool ReflectionProbe::IsSphereVisible(const GameContext& gameContext) const
	{
		return m_SphereMesh->IsVisible();
	}

	void ReflectionProbe::SetSphereVisible(bool visible, const GameContext& gameContext)
	{
		m_SphereMesh->SetVisible(visible);
	}
}