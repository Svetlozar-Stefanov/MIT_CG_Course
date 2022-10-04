#ifndef GROUP_H
#define GROUP_H


#include "Object3D.h"
#include "Ray.h"
#include "Hit.h"
#include <iostream>
#include <cassert>

using  namespace std;

///TODO: 
///Implement Group
///Add data structure to store a list of Object* 
class Group :public Object3D
{
public:


	Group() {
		capacity = 4;
		size = 0;
		objects = new Object3D * [capacity];
	}

	Group(int num_objects) {
		capacity = num_objects;
		size = 0;
		objects = new Object3D * [capacity];
	}

	~Group() {
		capacity = 0;
		size = 0;
		delete[] objects;
	}

	virtual bool intersect(const Ray& r, Hit& h, float tmin) {
		for (int i = 0; i < size; i++)
		{
			objects[i]->intersect(r, h, tmin);
		}

		return true;
	}

	void addObject(Object3D* obj)
	{
		if (size >= capacity)
		{
			resize();
		}
		addObject(size, obj);
	}

	void addObject(int index, Object3D* obj) {
		if (index < 0 || index >= capacity)
		{
			return;
		}
		
		delete objects[index];
		objects[index] = obj;
	}

	int getGroupSize() {
		return size;
	}

private:
	Object3D** objects;
	int size;
	int capacity;

	void resize() {
		capacity *= 2;
		Object3D** newObjs = new Object3D * [capacity];
		for (int i = 0; i < size; i++)
		{
			newObjs[i] = objects[i];
		}
		delete[] objects;
		objects = newObjs;
		newObjs = nullptr;
	}
};

#endif

