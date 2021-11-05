#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <ctime>
#include <chrono>

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <reactphysics3d/reactphysics3d.h>

#include "GLMemoryHelpers.h"
#include "GLColor.h"
#include "GLGameObject.h"
#include "GLCamera.h"
#include "GLPhysics.h"

class GLScene
{
public:
	GLScene(const std::string& name)
		: Name(name),
		  Background(1.0f, 1.0f, 1.0f)
	{
		this->Root = GCreate(GLGameObject);
		this->Root->Scene = GLSharedPtr<GLScene>(this);

		this->Physics = GLCreate<GLPhysics>();
	}

	virtual ~GLScene() { }

	void AddCamera(const GLSharedPtr<GCamera>& camera)
	{
		assert(camera != nullptr);

		this->Cameras.push_back(camera);
	}

	void RemoveCamera(const GLSharedPtr<GCamera>& camera)
	{
		assert(camera != nullptr);
		
		auto position = std::find(this->Cameras.begin(), this->Cameras.end(), camera);

		if (position != this->Cameras.end())
		{
			this->Cameras.erase(position);
		}
	}

	void Update(float deltaTime)
	{
		this->TimeStepAccumulator += deltaTime;

		while (this->TimeStepAccumulator >= this->FixedUpdateTimeStep)
		{
			this->Physics->Update(this->FixedUpdateTimeStep);
			this->Root->Update(this->FixedUpdateTimeStep);

			this->TimeStepAccumulator -= this->FixedUpdateTimeStep;
		}
	}

	void Render(const glm::vec2& windowSize)
	{
		std::sort(this->Cameras.begin(), this->Cameras.end(),
			[](const GLSharedPtr<GCamera>& a, const GLSharedPtr<GCamera>& b)
			{
				return a->GetOrder() < b->GetOrder();
			}
		);

		for (const auto& camera : this->Cameras)
		{
			if (camera->IsActive())
			{
				auto cameraViewportPosition = camera->GetViewportPosition();
				auto cameraViewportSize = camera->GetViewportSize();

				int x = cameraViewportPosition.x * windowSize.x;
				int y = cameraViewportPosition.y * windowSize.y;

				int width = cameraViewportSize.x * windowSize.x;
				int height = cameraViewportSize.y * windowSize.y;

				glViewport(x, y, width, height);
				glClear(GL_DEPTH_BUFFER_BIT);

				this->Root->Render(camera->GetLayer(), camera->GetCameraMatrix());
			}
		}
	void SetBackgroundColor(const GLColor& color)
	{
		this->Background = color;
	}

	GLColor GetBackgroudColor()
	{
		return this->Background;
	}

	GLSharedPtr<GLGameObject> Root;
	std::vector<GLSharedPtr<GCamera>> Cameras;

protected:
	std::string Name;
	GLColor Background;

	float FixedUpdateTimeStep = 0.02f;
	float TimeStepAccumulator = 0.0f;
};