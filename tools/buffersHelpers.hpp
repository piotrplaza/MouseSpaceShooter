#pragma once

#include <components/renderingBuffers.hpp>
#include <components/renderTexture.hpp>

#include <globals/components.hpp>

#include <commonTypes/componentsContainers.hpp>

#include <ranges>

namespace Tools
{
	template <typename Component>
	inline void ProcessStaticRenderableComponents(StaticComponents<Component>& components, size_t offset = 0)
	{
		auto& staticTFBuffers = Globals::Components().renderingBuffers().staticTFBuffers;

		for (auto& component : components.underlyingContainer() | std::views::drop(offset))
		{
			if (component.state == ComponentState::Ongoing || component.state == ComponentState::Outdated)
				continue;

			assert(component.state != ComponentState::LastShot);
			assert(!component.targetTextures.empty());
			assert(component.loaded.vps.empty());

			const auto layer = (size_t)component.renderLayer;

			component.loaded.vps.reserve(component.targetTextures.size());

			for (size_t i = 0; i < component.targetTextures.size(); ++i)
			{
				assert(component.targetTextures[i].component);
				const auto& targetTexture = *component.targetTextures[i].component;
				const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
				auto& staticBuffers = standardRenderMode
					? Globals::Components().renderingBuffers().staticBuffers
					: Globals::Components().renderingBuffers().staticOfflineBuffers;
				if (i < component.vps.size())
					component.loaded.vps.push_back(component.vps[i]);
				else
					component.loaded.vps.emplace_back(standardRenderMode
						? Globals::Components().vpDefault2D()
						: Globals::Components().vpIdentity());

				auto& selectedBuffers = [&]() -> auto& {
					if (component.customShadersProgram)
						return staticBuffers.customShaders[layer].emplace_back();
					if (component.params3D)
					{
						if (i >= component.vps.size())
							component.loaded.vps.back() = Globals::Components().vpDefault3D();
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
			}

			assert(component.loaded.vps.size() == component.targetTextures.size());

			if (component.state == ComponentState::Changed)
				component.state = ComponentState::Ongoing;
		}
	}

	template <typename Component>
	inline void ProcessDynamicRenderableComponents(DynamicComponents<Component>& components)
	{
		auto& dynamicTFBuffers = Globals::Components().renderingBuffers().dynamicTFBuffers;

		for (auto& component : components)
		{
			if (component.state == ComponentState::Ongoing || component.state == ComponentState::Outdated)
				continue;

			assert(!component.targetTextures.empty());

			const auto layer = (size_t)component.renderLayer;

			component.loaded.vps.clear();
			component.loaded.vps.reserve(component.targetTextures.size());

			for (size_t i = 0; i < component.targetTextures.size(); ++i)
			{
				assert(component.targetTextures[i].component);
				const auto& targetTexture = *component.targetTextures[i].component;
				const auto& standardRenderMode = targetTexture.loaded.standardRenderMode;
				auto& dynamicBuffers = standardRenderMode
					? Globals::Components().renderingBuffers().dynamicBuffers
					: Globals::Components().renderingBuffers().dynamicOfflineBuffers;
				if (i < component.vps.size())
					component.loaded.vps.push_back(component.vps[i]);
				else
					component.loaded.vps.emplace_back(standardRenderMode
						? Globals::Components().vpDefault2D()
						: Globals::Components().vpIdentity());

				// TODO: What if a layer or a shader type was changed? Prob leak.
				auto& mapOfSelectedBuffers = [&]() -> auto& {
					if (component.customShadersProgram)
						return dynamicBuffers.customShaders[layer];
					if (component.params3D)
					{
						if (i >= component.vps.size())
							component.loaded.vps.back() = Globals::Components().vpDefault3D();
						if (std::holds_alternative<std::monostate>(component.texture))
							return dynamicBuffers.basicPhong[layer];
						return dynamicBuffers.texturedPhong[layer];
					}
					if (std::holds_alternative<std::monostate>(component.texture))
						return dynamicBuffers.basic[layer];
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
			}

			assert(component.loaded.vps.size() == component.targetTextures.size());

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
