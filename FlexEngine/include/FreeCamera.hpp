#pragma once

#include "GameContext.hpp"
#include "InputManager.hpp"

namespace flex
{
	class FreeCamera final
	{
	public:
		FreeCamera(GameContext& gameContext, real FOV = glm::radians(45.0f), real zNear = 0.1f, real zFar = 10000.0f);
		~FreeCamera();

		void Update(const GameContext& gameContext);

		void SetFOV(real FOV);
		real GetFOV() const;
		void SetZNear(real zNear);
		real GetZNear() const;
		void SetZFar(real zFar);
		real GetZFar() const;
		glm::mat4 GetViewProjection() const;
		glm::mat4 GetView() const;
		glm::mat4 GetProjection() const;

		// speed: Lerp amount to new rotation
		void LookAt(glm::vec3 point, real speed = 1.0f);

		void SetMoveSpeed(real moveSpeed);
		real GetMoveSpeed() const;
		void SetRotationSpeed(real rotationSpeed);
		real GetRotationSpeed() const;

		void Translate(glm::vec3 translation);
		void SetPosition(glm::vec3 position);
		glm::vec3 GetPosition() const;

		void SetViewDirection(real yawRad, real pitchRad);

		glm::vec3 GetRight() const;
		glm::vec3 GetUp() const;
		glm::vec3 GetForward() const;

		void ResetPosition();
		void ResetOrientation();

		void LoadDefaultKeybindings();
		void LoadAzertyKeybindings();

		void SetYaw(real rawRad);
		real GetYaw() const;
		void SetPitch(real pitchRad);
		real GetPitch() const;

	private:
		void RecalculateViewProjection(const GameContext& gameContext);

		glm::mat4 m_View;
		glm::mat4 m_Proj;
		glm::mat4 m_ViewProjection;

		real m_FOV = 0;
		real m_ZNear = 0;
		real m_ZFar = 0;

		glm::vec3 m_Position;
		glm::vec3 m_DragStartPosition;

		real m_Yaw = 0;
		real m_Pitch = 0;
		glm::vec3 m_Forward;
		glm::vec3 m_Up;
		glm::vec3 m_Right;

		real m_MoveSpeed = 0; // Keyboard
		real m_PanSpeed = 0; // MMB
		real m_DragDollySpeed = 0; // RMB
		real m_ScrollDollySpeed = 0; // Scroll wheel
		real m_MoveSpeedFastMultiplier = 0;
		real m_MoveSpeedSlowMultiplier = 0;
		real m_RotationSpeed = 0;

		InputManager::KeyCode m_MoveForwardKey;
		InputManager::KeyCode m_MoveBackwardKey;
		InputManager::KeyCode m_MoveLeftKey;
		InputManager::KeyCode m_MoveRightKey;
		InputManager::KeyCode m_MoveUpKey;
		InputManager::KeyCode m_MoveDownKey;
		InputManager::KeyCode m_MoveFasterKey;
		InputManager::KeyCode m_MoveSlowerKey;

	};
} // namespace flex
