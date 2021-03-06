#pragma once

#pragma warning(push, 0)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#pragma warning(pop)

#include "JSONTypes.hpp"

namespace flex
{
	class RigidBody;
	class GameObject;

	class Transform
	{
	public:
		Transform();
		Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
		Transform(const glm::vec3& position, const glm::quat& rotation);
		Transform(const glm::vec3& position);

		Transform(const Transform& other);
		Transform(const Transform&& other);
		Transform& operator=(const Transform& other);
		Transform& operator=(const Transform&& other);

		~Transform();

		static Transform ParseJSON(const JSONObject& object);
		JSONField SerializeToJSON();

		void SetAsIdentity();

		glm::vec3 GetLocalPosition() const;
		glm::vec3 GetWorldPosition() const;

		glm::quat GetLocalRotation() const;
		glm::quat GetWorldRotation() const;

		glm::vec3 GetLocalScale() const;
		glm::vec3 GetWorldScale() const;

		void SetLocalPosition(const glm::vec3& position, bool bUpdateChain = true);
		void SetWorldPosition(const glm::vec3& position, bool bUpdateChain = true);

		void SetLocalRotation(const glm::quat& quatRotation, bool bUpdateChain = true);
		void SetWorldRotation(const glm::quat& quatRotation, bool bUpdateChain = true);
		void SetLocalRotation(const glm::vec3& eulerAnglesRad, bool bUpdateChain = true);
		void SetWorldRotation(const glm::vec3& eulerAnglesRad, bool bUpdateChain = true);
		void SetLocalRotation(real eulerXRad, real eulerYRad, real eulerZRad, bool bUpdateChain = true);
		void SetWorldRotation(real eulerXRad, real eulerYRad, real eulerZRad, bool bUpdateChain = true);

		void SetLocalScale(const glm::vec3& scale, bool bUpdateChain = true);
		void SetWorldScale(const glm::vec3& scale, bool bUpdateChain = true);

		void SetWorldFromMatrix(const glm::mat4& mat, bool bUpdateChain = true);

		void Translate(const glm::vec3& deltaPosition);
		void Translate(real deltaX, real deltaY, real deltaZ);

		void Rotate(const glm::quat& deltaQuatRotation);
		void Rotate(const glm::vec3& deltaEulerRotationRad);
		void Rotate(real deltaX, real deltaY, real deltaZ);
		
		void Scale(const glm::vec3& deltaScale);
		void Scale(real deltaScale);
		void Scale(real deltaX, real deltaY, real deltaZ);

		void SetWorldTransform(const glm::mat4& desiredWorldTransform);

		bool IsIdentity() const;
		static Transform Identity();

		void SetGameObject(GameObject* gameObject);
		GameObject* GetGameObject() const;

		glm::mat4 GetWorldTransform();
		glm::mat4 GetLocalTransform();

		void UpdateParentTransform(); // Used to go all the way to the base of the parent-child tree

	private:
		void UpdateChildTransforms(); // Used to go back down to the lowest node of the parent-child tree

		glm::mat4 localTransform;
		glm::mat4 worldTransform;

		glm::vec3 localPosition;
		glm::quat localRotation;
		glm::vec3 localScale;

		glm::vec3 worldPosition;
		glm::quat worldRotation;
		glm::vec3 worldScale;

		GameObject* m_GameObject = nullptr;

		static Transform m_Identity;

	};
} // namespace flex
