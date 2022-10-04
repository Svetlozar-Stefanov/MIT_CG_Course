#ifndef SPHERE_H
#define SPHERE_H

#include "Object3D.h"
#include <vecmath.h>
#include <cmath>

#include <iostream>
using namespace std;
///TODO:
///Implement functions and add more fields as necessary
class Sphere: public Object3D
{
public:
	Sphere(){ 
		//unit ball at the center
		origin = Vector3f(0, 0, 0);
		radius = 1;
	}

	Sphere( Vector3f center , float radius , Material* material ):Object3D(material){
		origin = center;
		this->radius = radius;
	}
	

	~Sphere(){}

	virtual bool intersect( const Ray& r , Hit& h , float tmin){
		float a = Vector3f::dot(r.getDirection(), r.getDirection());
		float b = 2 * Vector3f::dot(r.getDirection(), r.getOrigin());
		float c = Vector3f::dot(r.getOrigin(), r.getOrigin()) - radius * radius;

		float D = sqrt(b * b  - 4*a*c);
		
		if (D < 0)
		{
			return false;
		}

		float t1 = FLT_MAX;
		float t2 = FLT_MAX;

		if (D == 0)
		{
			t1 = (-b + D) / (2 * a);
		}
		else if (D > 0)
		{
			t1 = (-b + D) / (2 * a);
			t2 = (-b - D) / (2 * a);
		}

		float smallerPositive;
		if (t1 >= 0 && t2 >= 0)
		{
			smallerPositive = min(t1, t2);
		}
		else
		{
			smallerPositive = max(t1, t2);
		}

		if (smallerPositive >= tmin && smallerPositive < h.getT())
		{
			Vector3f norm = (origin - r.pointAtParameter(smallerPositive)).normalized();
			h.set(smallerPositive, material, norm);

			return true;
		}

		return false;
	}

protected:
	Vector3f origin;
	float radius;
};


#endif
