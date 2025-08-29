#pragma once

#include <components/renderingBuffers.hpp>
#include <components/renderTexture.hpp>

#include <globals/components.hpp>

#include <commonTypes/componentsContainers.hpp>

#include <ranges>

namespace Tools
{
	template <typename Component>
	inline void ProcessStaticComponents(StaticComponents<Component>& components, size_t offset = 0)
	{
		auto& staticTFBuffers = Globals::Components().renderingBuffers().staticTFBuffers;

		for (auto& component : components.underlyingContainer() | std::views::drop(offset))
		{
			if (component.state == ComponentState::Ongoing || component.state == ComponentState::Outdated)
				continue;

			assert(component.state != ComponentState::LastShot);
			assert(component.targetTexture.component);

			auto& selectedBuffers = [&]() -> auto& {
				const auto layer = (size_t)component.renderLayer;
				const auto& standardRenderMode = component.targetTexture.component->loaded.standardRenderMode;
				auto& staticBuffers = [&]() ->auto& {
					if (standardRenderMode)
					{
						component.vpMatrix = Globals::Components().vpDefault2D();
						return Globals::Components().renderingBuffers().staticBuffers;
					}
					else
					{
						component.vpMatrix = Globals::Components().vpIdentity();
						return Globals::Components().renderingBuffers().staticOfflineBuffers;
					}
				}();

				if (component.customShadersProgram)
					return staticBuffers.customShaders[layer].emplace_back();
				if (component.params3D)
				{
					if (standardRenderMode)
						component.vpMatrix = Globals::Components().vpDefault3D();
					if (std::holds_alternative<std::monostate>(component.texture))
						return staticBuffers.basicPhong[layer].emplace_back();
					return staticBuffers.texturedPhong[layer].emplace_back();
				}
				if (std::holds_alternative<std::monostate>(component.texture))
					return staticBuffers.basic[layer].emplace_back();
				return staticBuffers.textured[layer].emplace_back();
			}();

			selectedBuffers.applyComponent(component, true);

			if (component.tfShaderProgram)
				staticTFBuffers.emplace_back(true).applyTFComponent(component, true);

			if (component.state == ComponentState::Changed)
				component.state = ComponentState::Ongoing;
		}
	}

	template <typename Component>
	inline void ProcessDynamicComponents(DynamicComponents<Component>& components)
	{
		auto& dynamicTFBuffers = Globals::Components().renderingBuffers().dynamicTFBuffers;

		for (auto& component : components)
		{
			if (component.state == ComponentState::Ongoing || component.state == ComponentState::Outdated)
				continue;

			assert(component.targetTexture.component);

			// TODO: What if a layer or a shader type was changed? Prob leak.
			auto& mapOfSelectedBuffers = [&]() -> auto& {
				const auto layer = (size_t)component.renderLayer;
				const auto& standardRenderMode = component.targetTexture.component->loaded.standardRenderMode;
				auto& dynamicBuffers = [&]() ->auto& {
					if (standardRenderMode)
					{
						component.vpMatrix = Globals::Components().vpDefault2D();
						return Globals::Components().renderingBuffers().dynamicBuffers;
					}
					else
					{
						component.vpMatrix = Globals::Components().vpIdentity();
						return Globals::Components().renderingBuffers().dynamicOfflineBuffers;
					}
				}();

				if (component.customShadersProgram)
					return dynamicBuffers.customShaders[layer];
				else if (component.params3D)
				{
					if (standardRenderMode)
						component.vpMatrix = Globals::Components().vpDefault3D();
					if (std::holds_alternative<std::monostate>(component.texture))
						return dynamicBuffers.basicPhong[layer];
					else
						return dynamicBuffers.texturedPhong[layer];
				}
				else if (std::holds_alternative<std::monostate>(component.texture))
					return dynamicBuffers.basic[layer];
				else
					return dynamicBuffers.textured[layer];
			}();

			if (!component.loaded.buffers)
			{
				component.teardownF = [&, prevTeardownF = std::move(component.teardownF)]() {
					if (prevTeardownF)
						prevTeardownF();

					if (component.tfShaderProgram)
						dynamicTFBuffers.erase(component.getComponentId());

					mapOfSelectedBuffers.erase(component.getComponentId());
				};
			}

			mapOfSelectedBuffers[component.getComponentId()].applyComponent(component, false);

			if (component.tfShaderProgram)
				dynamicTFBuffers.emplace(component.getComponentId(), true).first->second.applyTFComponent(component, false);

			if (component.state == ComponentState::Changed)
				component.state = ComponentState::Ongoing;
		}
	}

	struct SubImageData
	{
		const float* data;
		glm::ivec2 size;
		glm::ivec2 offset;
	};

	SubImageData GetSubImage(
		const float* sourceImageData,
		const glm::ivec2& sourceImageSize,
		const glm::ivec2& offset,
		const glm::ivec2& subImageSize,
		int numOfChannels,
		std::vector<float>& operationalBuffer);

	SubImageData ClipSubImage(
		const float* subImageData,
		const glm::ivec2& subImagebSize,
		const glm::ivec2& offset,
		const glm::ivec2& targetImageSize,
		const int numOfChannels,
		std::vector<float>& operationalBuffer);
}
