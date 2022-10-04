#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera
{
public:
	//generate rays for each screen-space coordinate
	virtual Ray generateRay( const Vector2f& point ) = 0 ; 
	
	virtual float getTMin() const = 0 ; 
	virtual ~Camera(){}
protected:
	Vector3f center; 
	Vector3f direction;
	Vector3f up;
	Vector3f horizontal;

};

///TODO: Implement Perspective camera
///Fill in functions and add more fields if necessary
class PerspectiveCamera: public Camera
{
public:
	PerspectiveCamera(const Vector3f& center, const Vector3f& direction,const Vector3f& up , float angle){
		this->center = center;
		this->direction = direction;
		this->horizontal = Vector3f::cross(direction, up);
		this->up = Vector3f::cross(direction, horizontal);
		fovAngle = angle;
	}

	virtual Ray generateRay( const Vector2f& point){
		float D = 1 / tan(fovAngle / 2);

		Vector3f dir = point.x() * horizontal + point.y() * up + direction * D;
		
		return Ray(center, dir);
	}

	virtual float getTMin() const { 
		return 0.0f;
	}

private:
	float fovAngle;
};

#endif //CAMERA_H
