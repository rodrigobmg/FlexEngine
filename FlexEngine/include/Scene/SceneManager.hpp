﻿#pragma once

#include <vector>

#include "Scene/BaseScene.hpp"

namespace flex
{
	class SceneManager
	{
	public:
		SceneManager();
		virtual ~SceneManager();

		void UpdateCurrentScene();

		void AddScene(BaseScene* newScene);

		/* To be called after AddScene */
		void InitializeCurrentScene();

		/* To be called after InitializeCurrentScene */
		void PostInitializeCurrentScene();

		void RemoveScene(BaseScene* scene);

		/* Destroys previous scene if exists, then sets current index (NOTE: Does *not* initialize new scene! */
		bool SetCurrentScene(u32 sceneIndex, bool bPrintErrorOnFailure = true);
		bool SetCurrentScene(BaseScene* scene, bool bPrintErrorOnFailure = true);
		bool SetCurrentScene(const std::string& sceneFileName, bool bPrintErrorOnFailure = true);
		void SetNextSceneActiveAndInit();
		void SetPreviousSceneActiveAndInit();
		void ReloadCurrentScene();

		// Adds all scenes found in scenes directory
		void AddFoundScenes();
		void RemoveDeletedScenes();

		void DeleteScene(BaseScene* scene);
		void CreateNewScene(const std::string& name, bool bSwitchImmediately);

		u32 CurrentSceneIndex() const;
		BaseScene* CurrentScene() const;
		u32 GetSceneCount() const;

		i32 GetCurrentSceneIndex() const;
		BaseScene* GetSceneAtIndex(i32 index);

		void DestroyAllScenes();

	private:
		bool SceneExists(const std::string& fileName) const;

		u32 m_CurrentSceneIndex = u32_max;
		std::vector<BaseScene*> m_Scenes;

		std::string m_SavedDirStr;
		std::string m_DefaultDirStr;

		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

	};
} // namespace flex
