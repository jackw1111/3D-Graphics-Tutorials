#include "collision.h"

Plane::Plane(const vec3& origin, const vec3& normal)
{
  this->origin = origin;
  this->normal = normal;
  equation[0] = normal[0];
  equation[1] = normal[1];
  equation[2] = normal[2];
  equation[3] = -dot(origin, normal);
}

// Construct from triangle:
Plane::Plane(const vec3& p1, const vec3& p2, const vec3& p3)
{
  normal = cross(p2 - p1, p3 - p1);
  normalize(normal);

  origin = p1;

  equation[0] = normal[0];
  equation[1] = normal[1];
  equation[2] = normal[2];
  equation[3] = -dot(normal, origin);
}

double Plane::signedDistanceTo(const vec3& point) const {
	return (dot(point, normal)) + equation[3];
}

bool Plane::isFrontFacingTo(const vec3& direction) const
{
	double d = dot(normal, direction);
	return (d <= 0);
}

bool checkPointInTriangle(const vec3& point,
                          const vec3& p1, const vec3& p2, const vec3& p3)
{
  vec3 u, v, w, vw, vu, uw, uv;

   u = p2 - p1;
   v = p3 - p1;
   w = point - p1;

  vw = cross(v, w);
  vu = cross(v, u);

  if (dot(vw, vu) < 0.0f) {
    return 0;
  }

  uw = cross(u, w);
  uv = cross(u, v);

  if (dot(uw, uv) < 0.0f) {
    return 0;
  }

  float d = length(uv);
  float r = length(vw) / d;
  float t = length(uw) / d;

  return ((r + t) <= 1);

}

bool getLowestRoot(float a, float b, float c, float maxR,
				   float* root) {
	// Check if a solution exists
	float determinant = b*b - 4.0f*a*c;

	// If determinant is negative it means no solutions.
	if (determinant < 0.0f) return false;

	// calculate the two roots: (if determinant == 0 then
	// x1==x2 but let’s disregard that slight optimization)
	float sqrtD = sqrt(determinant);
	float r1 = (-b - sqrtD) / (2*a);
	float r2 = (-b + sqrtD) / (2*a);

	// Sort so x1 <= x2
	if (r1 > r2) {
		float temp = r2;
		r2 = r1;
		r1 = temp;
	}
	// Get lowest root:
	if (r1 > 0 && r1 < maxR) {
		*root = r1;
		return true;
	}
	// It is possible that we want x2 - this can happen
	// if x1 < 0
	if (r2 > 0 && r2 < maxR) {
		*root = r2;
		return true;
	}
	// No (valid) solutions
	return false;
}

// Assumes: p1,p2 and p3 are given in ellisoid space:
void checkTriangle(CollisionPacket* colPackage,
	 const vec3& p1, const vec3& p2, const vec3& p3)
{
	// Make the Plane containing this triangle.
	Plane trianglePlane(p1,p2,p3);
	// Is triangle front-facing to the velocity vector?
	// We only check front-facing triangles
	// (your choice of course)
	if (trianglePlane.isFrontFacingTo(
						colPackage->normalizedVelocity)) {
		// Get interval of Plane intersection:
		double t0, t1;
		bool embeddedInPlane = false;

		// Calculate the signed distance from sphere
		// position to triangle Plane
		double signedDistToTrianglePlane =
		trianglePlane.signedDistanceTo(colPackage->basePoint);

		// cache this as we’re going to use it a few times below:
		float normalDotVelocity =
			dot(trianglePlane.normal, colPackage->velocity);
		// if sphere is travelling parrallel to the Plane:
		if (normalDotVelocity == 0.0f) {
			if (fabs(signedDistToTrianglePlane) >= 1.0f) {
				// Sphere is not embedded in Plane.
				// No collision possible:
				return;
			}
			else {
				// sphere is embedded in Plane.
				// It intersects in the whole range [0..1]
				embeddedInPlane = true;
				t0 = 0.0;
				t1 = 1.0;
			}
		}
		else {
			// N dot D is not 0. Calculate intersection interval:
			t0=(-1.0-signedDistToTrianglePlane)/normalDotVelocity;
			t1=( 1.0-signedDistToTrianglePlane)/normalDotVelocity;

			// Swap so t0 < t1
			if (t0 > t1) {
				double temp = t1;
				t1 = t0;
				t0 = temp;
			}

			// Check that at least one result is within range:
			if (t0 > 1.0f || t1 < 0.0f) {
				// Both t values are outside values [0,1]
				// No collision possible:
				return;
			}
			// Clamp to [0,1]
			if (t0 < 0.0) t0 = 0.0;
			if (t1 < 0.0) t1 = 0.0;
			if (t0 > 1.0) t0 = 1.0;
			if (t1 > 1.0) t1 = 1.0;
		}

		// OK, at this point we have two time values t0 and t1
		// between which the swept sphere intersects with the
		// triangle Plane. If any collision is to occur it must
		// happen within this interval.
		vec3 collisionPoint;
		bool foundCollison = false;
		float t = 1.0;

		// First we check for the easy case - collision inside
		// the triangle. If this happens it must be at time t0
		// as this is when the sphere rests on the front side
		// of the triangle Plane. Note, this can only happen if
		// the sphere is not embedded in the triangle Plane.
		if (!embeddedInPlane) {
			vec3 PlaneIntersectionPoint =
					(colPackage->basePoint-trianglePlane.normal)
					+ (float)t0*colPackage->velocity;

			if (checkPointInTriangle(PlaneIntersectionPoint,
									 p1,p2,p3))
			{
				foundCollison = true;
				t = t0;
				collisionPoint = PlaneIntersectionPoint;
			}
		}
		// if we haven’t found a collision already we’ll have to
		// sweep sphere against points and edges of the triangle.
		// Note: A collision inside the triangle (the check above)
		// will always happen before a vertex or edge collision!
		// This is why we can skip the swept test if the above
		// gives a collision!

		if (foundCollison == false) {
			// some commonly used terms:
			vec3 velocity = colPackage->velocity;
			vec3 base = colPackage->basePoint;
			float velocitySquaredLength = dot(velocity, velocity);
			float a,b,c; // Params for equation
			float newT;

			// For each vertex or edge a quadratic equation have to
			// be solved. We parameterize this equation as
			// a*t^2 + b*t + c = 0 and below we calculate the
			// parameters a,b and c for each test.

			// Check against points:
			a = velocitySquaredLength;

			// P1
			b = 2.0*(dot(velocity, base-p1));
			c = dot(p1-base, p1-base) - 1.0;
			if (getLowestRoot(a,b,c, t, &newT)) {
				t = newT;
				foundCollison = true;
				collisionPoint = p1;
			}

			// P2
			b = 2.0*(dot(velocity, base-p2));
			c = dot(p2-base, p2-base) - 1.0;
			if (getLowestRoot(a,b,c, t, &newT)) {
				t = newT;
				foundCollison = true;
				collisionPoint = p2;
			}

			// P3
			b = 2.0*(dot(velocity, base-p3));
			c = dot(p3-base, p3-base) - 1.0;
			if (getLowestRoot(a,b,c, t, &newT)) {
				t = newT;
				foundCollison = true;
				collisionPoint = p3;
			}

			// Check agains edges:

			// p1 -> p2:
			vec3 edge = p2-p1;
			vec3 baseToVertex = p1 - base;
			float edgeSquaredLength = dot(edge, edge);
			float edgeDotVelocity = dot(edge, velocity);
			float edgeDotBaseToVertex = dot(edge, baseToVertex);

			// Calculate parameters for equation
			a = edgeSquaredLength*-velocitySquaredLength +
			edgeDotVelocity*edgeDotVelocity;
			b = edgeSquaredLength*(2*dot(velocity, baseToVertex))-
			2.0*edgeDotVelocity*edgeDotBaseToVertex;
			c = edgeSquaredLength*(1-dot(baseToVertex, baseToVertex))+
			edgeDotBaseToVertex*edgeDotBaseToVertex;

			// Does the swept sphere collide against infinite edge?
			if (getLowestRoot(a,b,c, t, &newT)) {
				// Check if intersection is within line segment:
				float f=(edgeDotVelocity*newT-edgeDotBaseToVertex)/
				edgeSquaredLength;
				if (f >= 0.0 && f <= 1.0) {
					// intersection took place within segment.
					t = newT;
					foundCollison = true;
					collisionPoint = p1 + f*edge;
				}
			}

			// p2 -> p3:
			edge = p3-p2;
			baseToVertex = p2 - base;
			edgeSquaredLength = dot(edge, edge);
			edgeDotVelocity = dot(edge, velocity);
			edgeDotBaseToVertex =dot(edge, baseToVertex);

			a = edgeSquaredLength*-velocitySquaredLength +
			edgeDotVelocity*edgeDotVelocity;
			b = edgeSquaredLength*(2*dot(velocity, baseToVertex))-
			2.0*edgeDotVelocity*edgeDotBaseToVertex;
			c = edgeSquaredLength*(1-dot(baseToVertex, baseToVertex))+
			edgeDotBaseToVertex*edgeDotBaseToVertex;

			if (getLowestRoot(a,b,c, t, &newT)) {
				float f=(edgeDotVelocity*newT-edgeDotBaseToVertex)/
				edgeSquaredLength;
				if (f >= 0.0 && f <= 1.0) {
					t = newT;
					foundCollison = true;
					collisionPoint = p2 + f*edge;
				}
			}

			// p3 -> p1:
			edge = p1-p3;
			baseToVertex = p3 - base;
			edgeSquaredLength = dot(edge, edge);
			edgeDotVelocity = dot(edge, velocity);

			edgeDotBaseToVertex = dot(edge, baseToVertex);

			a = edgeSquaredLength*-velocitySquaredLength +
			edgeDotVelocity*edgeDotVelocity;
			b = edgeSquaredLength*(2*dot(velocity, baseToVertex))-
			2.0*edgeDotVelocity*edgeDotBaseToVertex;
			c = edgeSquaredLength*(1-dot(baseToVertex, baseToVertex))+
			edgeDotBaseToVertex*edgeDotBaseToVertex;

			if (getLowestRoot(a,b,c, t, &newT)) {
				float f=(edgeDotVelocity*newT-edgeDotBaseToVertex)/
				edgeSquaredLength;
				if (f >= 0.0 && f <= 1.0) {
					t = newT;
					foundCollison = true;
					collisionPoint = p3 + f*edge;
				}
			}
		}
		// Set result:
		if (foundCollison == true) {
			// distance to collision: ’t’ is time of collision
			float distToCollision = t*length(colPackage->velocity);
			// Does this triangle qualify for the closest hit?
			// it does if it’s the first hit or the closest
			if (colPackage->foundCollision == false ||
			distToCollision < colPackage->nearestDistance) {
				// Collision information nessesary for sliding
				colPackage->nearestDistance = distToCollision;
				colPackage->intersectionPoint=collisionPoint;
				colPackage->foundCollision = true;
			}
		}
	} // if not backface
}
