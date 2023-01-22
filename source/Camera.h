#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include "SettingsStruct.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle, float _aspectRatio):
			origin{_origin},
			fovAngle{_fovAngle},
			aspectRatio{_aspectRatio}
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float aspectRatio{};

		float nearClipPlane{ 0.1f };
		float farClipPlane{ 100.f };

		float maxFovAngle{ 179.5f };
		float minFovAngle{ 0.5f };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float _aspectRatio = 1)
		{
			aspectRatio = _aspectRatio;

			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			CalculateProjectionMatrix();
		}

		void CalculateViewMatrix()
		{
			//W1
			//ONB => invViewMatrix
			Vector3 tempRight{Vector3::Cross(Vector3::UnitY, forward)};
			tempRight.Normalize();
			Vector3 tempUp{ Vector3::Cross(forward, tempRight) };
			tempUp.Normalize();

			right = tempRight;
			up = tempUp;

			invViewMatrix = Matrix{ tempRight, tempUp, forward, origin };
			//Inverse(ONB) => ViewMatrix
			viewMatrix = invViewMatrix.Inverse();

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			// W2
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
			const float invFOV{ 1.f / fov };
			const float invFarMinusNear{ 1.f / (farClipPlane - nearClipPlane) };
			projectionMatrix = Matrix{  {invFOV / aspectRatio, 0,	   0,												  0}, 
										{0,					   invFOV, 0,												  0}, 
										{0,					   0,	   farClipPlane * invFarMinusNear,					  1}, 
										{0,					   0,	   -(farClipPlane * nearClipPlane) * invFarMinusNear, 0}};

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer* pTimer, DualRasterizerSettings settings)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic

			float fovChangeSpeed{ 10.f };
			float mouseMovementSpeed{ 40.f };
			float movementSpeed{ 30.f };
			float rotationSpeed{ 4.f };

			if (settings.rasterizerMode == RasterizerMode::SoftWare)
			{
				fovChangeSpeed = 10.f;
				mouseMovementSpeed = 0.5f;
				movementSpeed = 10.f;
				rotationSpeed = 0.01f;
			}

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_RSHIFT] || pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				fovChangeSpeed *= 4;
				mouseMovementSpeed *= 4;
				movementSpeed *= 4;
				rotationSpeed *= 4;
			}

			if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				if (fovAngle > minFovAngle)
				{
					fovAngle -= fovChangeSpeed * deltaTime;
					if (fovAngle < minFovAngle)
					{
						fovAngle = minFovAngle;
					}
					CalculateProjectionMatrix();
				}
			}
			else if (pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				if (fovAngle < maxFovAngle)
				{
					fovAngle += fovChangeSpeed * deltaTime;
					if (fovAngle > maxFovAngle)
					{
						fovAngle = maxFovAngle;
					}
					CalculateProjectionMatrix();
				}
			}

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += movementSpeed * deltaTime * forward;
			}
			else if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= movementSpeed * deltaTime * forward;
			}
			else if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= movementSpeed * deltaTime * right;
			}
			else if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += movementSpeed * deltaTime * right;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState == SDL_BUTTON_LMASK)
			{
				origin += (-mouseY) * mouseMovementSpeed * deltaTime * forward;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if (mouseState == SDL_BUTTON_RMASK)
			{
				totalPitch += (-mouseY) * rotationSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			{
				origin += (-mouseY) * mouseMovementSpeed * deltaTime * up;
			}

			// new rotation
			const Matrix finalRotation{ Matrix::CreateRotation(Vector3{totalPitch, totalYaw, 0}) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			//Update Matrices
			CalculateViewMatrix();
		}
	};
}
