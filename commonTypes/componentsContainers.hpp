#pragma once

#include <commonTypes/componentId.hpp>

#include <globals/componentIdGenerator.hpp>

#include <deque>
#include <unordered_map>
#include <unordered_set>

class ComponentsBase
{
public:
	static void CleanupAll();

	virtual void teardown() = 0;
	virtual void teardownReset() = 0;
	virtual void clear() = 0;
};

class StaticComponentsBase: public ComponentsBase
{
	friend ComponentsBase;

public:
	static void CleanupAll();

	StaticComponentsBase()
	{
		instances.insert(this);
	}

	virtual ~StaticComponentsBase()
	{
		instances.erase(this);
	}

private:
	static inline std::unordered_set<StaticComponentsBase*> instances;
};

class DynamicComponentsBase: public ComponentsBase
{
	friend ComponentsBase;

public:
	static void CleanupAll();
	static void CleanupAllEnding();
	static void OutdateAll();

	DynamicComponentsBase()
	{
		instances.insert(this);
	}

	virtual ~DynamicComponentsBase()
	{
		instances.erase(this);
	}

	virtual void cleanupEnding() = 0;
	virtual void markOutdated() = 0;

private:
	static inline std::unordered_set<DynamicComponentsBase*> instances;
};

template <typename Component>
class StaticComponents : public StaticComponentsBase
{
public:
	using Container = std::deque<Component>;

	template <typename ContainerIterator, typename Value>
	class IteratorBase
	{
	public:
		using Self = IteratorBase<ContainerIterator, Value>;

		IteratorBase(ContainerIterator initIt) :
			containerIt(initIt)
		{
		}

		bool operator !=(Self rhs) const
		{
			return containerIt != rhs.containerIt;
		}

		Self& operator ++()
		{
			++containerIt;
			return *this;
		}

		Self operator +(ComponentId rhs)
		{
			return Self(containerIt + rhs);
		}

		Self operator -(ComponentId rhs)
		{
			return Self(containerIt - rhs);
		}

		Value& operator *()
		{
			return *containerIt;
		}

		Value* operator ->()
		{
			return &*containerIt;
		}

		operator ContainerIterator()
		{
			return containerIt;
		}

	private:
		typename ContainerIterator containerIt;
	};

	using iterator = IteratorBase<typename Container::iterator, Component>;
	using const_iterator = IteratorBase<typename Container::const_iterator, const Component>;
	using reverse_iterator = IteratorBase<typename Container::reverse_iterator, Component>;
	using const_reverse_iterator = IteratorBase<typename Container::const_reverse_iterator, const Component>;

	Component& operator [](ComponentId id)
	{
		assert(id < components.size());
		return components[id];
	}

	const Component& operator [](ComponentId id) const
	{
		assert(id < components.size());
		return components[id];
	}

	iterator begin()
	{
		return iterator(components.begin());
	}

	const_iterator begin() const
	{
		return const_iterator(components.cbegin());
	}

	iterator end()
	{
		return iterator(components.end());
	}

	const_iterator end() const
	{
		return const_iterator(components.cend());
	}

	reverse_iterator rbegin()
	{
		return reverse_iterator(components.rbegin());
	}

	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(components.crbegin());
	}

	reverse_iterator rend()
	{
		return reverse_iterator(components.rend());
	}

	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(components.rcend());
	}

	Component& add(const Component& component)
	{
		components.push_back(component);
		last_ = &components.back();
		last_->init((ComponentId)components.size() - 1, true);
		return *last_;
	}

	template <typename... Params>
	Component& emplace(Params&&... params)
	{
		last_ = &components.emplace_back(std::forward<Params>(params)...);
		last_->init((ComponentId)components.size() - 1, true);
		return *last_;
	}

	Component& last()
	{
		assert(last_);
		return *last_;
	}

	ComponentId size() const
	{
		return (ComponentId)components.size();
	}

	bool empty() const
	{
		return components.empty();
	}

	Container& underlyingContainer()
	{
		return components;
	}

	const Container& underlyingContainer() const
	{
		return components;
	}

	bool contains(ComponentId id) const
	{
		return id < components.size();
	}

	void teardown() override
	{
		for (auto& component : components)
			if (component.teardownF)
				component.teardownF();
	}

	void teardownReset() override
	{
		for (auto& component : components)
			if (component.teardownF)
			{
				component.teardownF();
				component.teardownF = nullptr;
			}
	}

	void clear() override
	{
		components.clear();
		last_ = nullptr;
	}

private:
	Container components;
	Component* last_ = nullptr;
};

template <typename Component>
class DynamicComponents : public DynamicComponentsBase
{
public:
	using Container = std::unordered_map<ComponentId, Component>;

	template <typename ContainerIterator, typename Value>
	class IteratorBase
	{
	public:
		using Self = IteratorBase<ContainerIterator, Value>;

		IteratorBase(ContainerIterator initIt):
			containerIt(initIt)
		{
		}

		bool operator !=(Self rhs) const
		{
			return containerIt != rhs.containerIt;
		}

		Self& operator ++()
		{
			++containerIt;
			return *this;
		}

		Value& operator *()
		{
			return containerIt->second;
		}

		Value* operator ->()
		{
			return &containerIt->second;
		}

		operator ContainerIterator()
		{
			return containerIt;
		}

	private:
		typename ContainerIterator containerIt;
	};

	using iterator = IteratorBase<typename Container::iterator, Component>;
	using const_iterator = IteratorBase<typename Container::const_iterator, const Component>;

	Component& operator [](ComponentId id)
	{
		auto it = components.find(id);
		assert(it != components.end());
		return it->second;
	}

	const Component& operator [](ComponentId id) const
	{
		auto it = components.find(id);
		assert(it != components.end());
		return it->second;
	}

	iterator begin()
	{
		return iterator(components.begin());
	}

	const_iterator begin() const
	{
		return const_iterator(components.cbegin());
	}

	iterator end()
	{
		return iterator(components.end());
	}

	const_iterator end() const
	{
		return const_iterator(components.cend());
	}

	Component& add(const Component& component)
	{
		ComponentId id = Globals::ComponentIdGenerator().acquire();
		auto it = components.insert({ id, component });
		assert(it.second);
		last_ = &it.first->second;
		last_->init(id, false);
		return *last_;
	}

	template <typename... Params>
	Component& emplace(Params&&... params)
	{
		ComponentId id = Globals::ComponentIdGenerator().acquire();
		auto it = components.try_emplace(id, std::forward<Params>(params)...);
		assert(it.second);
		last_ = &it.first->second;
		last_->init(id, false);
		return *last_;
	}

	Component& last()
	{
		assert(last_);
		return *last_;
	}

	ComponentId size() const
	{
		return (ComponentId)components.size();
	}

	bool empty() const
	{
		return components.empty();
	}

	Container& underlyingContainer()
	{
		return components;
	}

	const Container& underlyingContainer() const
	{
		return components;
	}

	bool contains(ComponentId id) const
	{
		return components.contains(id);
	}

	iterator find(ComponentId id)
	{
		return iterator(components.find(id));
	}

	const_iterator find(ComponentId id) const
	{
		return const_iterator(components.find(id));
	}

	void teardown() override
	{
		for (auto& [id, component] : components)
			if (component.teardownF)
				component.teardownF();
	}

	void teardownReset() override
	{
		for (auto& [id, component] : components)
			if (component.teardownF)
			{
				component.teardownF();
				component.teardownF = nullptr;
			}
	}

	void clear() override
	{
		components.clear();
		last_ = nullptr;
	}

	void cleanupEnding() override
	{
		auto it = components.begin();
		while (it != components.end())
		{
			if (it->second.state == ::ComponentState::Outdated || it->second.state == ::ComponentState::LastShot)
			{
				if (it->second.teardownF)
					it->second.teardownF();

				Globals::ComponentIdGenerator().release(it->first);
				it = components.erase(it);
			}
			else
				++it;
		}

		if (empty())
			last_ = nullptr;
	}

	void markOutdated() override
	{
		for (auto& [id, component] : components)
			component.state = ::ComponentState::Outdated;
	}

private:
	Container components;
	Component* last_ = nullptr;
};

template <typename Component>
class DynamicOrderedComponents : public DynamicComponentsBase
{
public:
	using Container = std::list<Component>;

	template <typename ContainerIterator, typename Value>
	class IteratorBase
	{
	public:
		using Self = IteratorBase<ContainerIterator, Value>;

		IteratorBase(ContainerIterator initIt) :
			containerIt(initIt)
		{
		}

		bool operator !=(Self rhs) const
		{
			return containerIt != rhs.containerIt;
		}

		Self& operator ++()
		{
			++containerIt;
			return *this;
		}

		Value& operator *()
		{
			return *containerIt;
		}

		Value* operator ->()
		{
			return &*containerIt;
		}

		operator ContainerIterator()
		{
			return containerIt;
		}

	private:
		typename ContainerIterator containerIt;
	};

	using iterator = IteratorBase<typename Container::iterator, Component>;
	using const_iterator = IteratorBase<typename Container::const_iterator, const Component>;

	iterator begin()
	{
		return iterator(components.begin());
	}

	const_iterator begin() const
	{
		return const_iterator(components.cbegin());
	}

	iterator end()
	{
		return iterator(components.end());
	}

	const_iterator end() const
	{
		return const_iterator(components.cend());
	}

	Component& add(const Component& component)
	{
		components.push_back(component);
		last_ = &components.back();
		return *last_;
	}

	template <typename... Params>
	Component& emplace(Params&&... params)
	{
		components.emplace_back(std::forward<Params>(params)...);
		last_ = &components.back();
		return *last_;
	}

	Component& last()
	{
		assert(last_);
		return *last_;
	}

	ComponentId size() const
	{
		return components.size();
	}

	bool empty() const
	{
		return components.empty();
	}

	Container& underlyingContainer()
	{
		return components;
	}

	const Container& underlyingContainer() const
	{
		return components;
	}

	iterator remove(iterator it)
	{
		return iterator(components.erase(it));
	}

	void teardown() override
	{
		for (auto& component : components)
			if (component.teardownF)
				component.teardownF();
	}

	void teardownReset() override
	{
		for (auto& component : components)
			if (component.teardownF)
			{
				component.teardownF();
				component.teardownF = nullptr;
			}
	}

	void clear() override
	{
		components.clear();
		last_ = nullptr;
	}

	void cleanupEnding() override
	{
		auto it = components.begin();
		while (it != components.end())
		{
			if (it->state == ::ComponentState::Outdated || it->state == ::ComponentState::LastShot)
			{
				if (it->teardownF)
					it->teardownF();

				Globals::ComponentIdGenerator().release(it->getComponentId());
				it = components.erase(it);
			}
			else
				++it;
		}

		if (empty())
			last_ = nullptr;
	}

	void markOutdated() override
	{
		for (auto& component : components)
			component.state = ::ComponentState::Outdated;
	}

private:
	Container components;
	Component* last_ = nullptr;
};
