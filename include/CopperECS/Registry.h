#pragma once

#include <vector>
#include <bitset>

#include "Object.h"

extern int cCounter;

template<typename T> int GetCID() {

	static int cID = cCounter++;
	return cID;

}

struct ComponentPool {

public:
	ComponentPool(size_t size) : size(size), data(new char[maxComponents * size]) {}
	~ComponentPool() { delete data; }

	void* Get(uint32_t index) { return data + index * size;  }

private:
	size_t size;
	char* data = nullptr;

};

class Registry {

public:
	Object CreateObject(Scene* scene, const char* name) {

		if (!gaps.empty()) {

			Object obj;
			obj.id = gaps.front();
			obj.scene = scene;
			obj.name = name;

			objects[obj.id] = obj;
			gaps.erase(gaps.begin());

			return obj;

		}

		Object obj;
		obj.id = (int32_t) objects.size();
		obj.scene = scene;
		obj.name = name;

		objects.push_back(obj);

		return obj;

	}
	void DestroyObject(Object& obj) {

		gaps.push_back(obj.id);

		objects[obj.id] = Object();

		obj.name = nullptr;
		obj.scene = nullptr;
		obj.id = -1;
		obj.componentMask = std::bitset<maxComponents>();

	}

	template<typename T> T* AddComponent(Object& obj) {

		if (!obj) return nullptr;
		if (!objects[obj.id]) return nullptr;

		int cID = GetCID<T>();

		if (pools.size() <= cID) pools.resize(cID + 1, nullptr);
		if (pools[cID] == nullptr) pools[cID] = new ComponentPool(sizeof(T));

		T* component = new (pools[cID]->Get(obj.id)) T();
		objects[obj.id].componentMask.set(cID);
		obj.componentMask.set(cID);

		return component;

	}
	template<typename T> T* GetComponent(int32_t id) {

		if (!objects[id]) return nullptr;

		int cID = GetCID<T>();
		if (!objects[id].componentMask.test(cID)) return nullptr;

		T* component = static_cast<T*>(pools[cID]->Get(id));
		return component;

	}
	template<typename T> bool HasComponent(int32_t id) {

		if (!objects[id]) return false;

		int cID = GetCID<T>();
		
		return objects[id].componentMask.test(cID);

	}
	template<typename T> void RemoveComponent(Object& obj) {

		if (!obj) return;
		if (!objects[obj.id]) return;

		int cID = GetCID<T>();
		if (!objects[obj.id].componentMask.test(cID)) return;

		objects[obj.id].componentMask.reset(cID);
		obj.componentMask.reset(cID);

	}

	std::vector<Object> GetObjects() const { return objects; }

private:
	std::vector<Object> objects;
	std::vector<uint32_t> gaps;
	std::vector<ComponentPool*> pools;

};