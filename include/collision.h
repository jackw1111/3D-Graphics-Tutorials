#ifndef COLLISION_H
#define COLLISION_H

#include <math.h>
#include <iostream>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define unitsPerMeter 100.0f

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::perspective;
using glm::length;
using glm::cross;
using glm::dot;
using glm::max;
using glm::min;
using glm::radians;

class CollisionPacket {
public:
	vec3 eRadius; // ellipsoid radius
	// Information about the move being requested: (in R3)
	vec3 R3Velocity;
	vec3 R3Position;
	// Information about the move being requested: (in eSpace)
	vec3 velocity;
	vec3 normalizedVelocity;
	vec3 basePoint;
	// Hit information
	bool foundCollision;
	double nearestDistance;
	vec3 intersectionPoint;

  // iteration depth
  int collisionRecursionDepth;
};

class Plane {
public:
	vec4 equation;
	vec3 origin;
	vec3 normal;

	Plane(const vec3& origin, const vec3& normal);
	Plane(const vec3& p1, const vec3& p2, const vec3& p3);

	bool isFrontFacingTo(const vec3& direction) const;
	double signedDistanceTo(const vec3& point) const;
};

bool checkPointInTriangle(const vec3& point,
                          const vec3& pa,const vec3& pb, const vec3& pc);

bool getLowestRoot(float a, float b, float c, float maxR,
				   float* root);

void checkTriangle(CollisionPacket* colPackage,
                    const vec3& p1, const vec3& p2, const vec3& p3);

#endif // COLLISION_H
