#include "entity.h"

CharacterEntity::CharacterEntity(std::vector<Model> models, vec3 radius)
{
  this->radius = radius;
  position = vec3(0.0f);
  velocity = vec3(0.0f);

  this->models = models;
  grounded = 0;
}

void CharacterEntity::collideAndSlide(const vec3& gravity)
{
  // Do collision detection:
	collisionPackage.R3Position = position;
	collisionPackage.R3Velocity = velocity;
  collisionPackage.eRadius = radius;

	// calculate position and velocity in eSpace
	vec3 eSpacePosition = collisionPackage.R3Position/
	collisionPackage.eRadius;
	vec3 velocity = collisionPackage.R3Velocity/
	collisionPackage.eRadius;
	// no gravity
    velocity[1] = 0.0f;

	// Iterate until we have our final position.
	collisionPackage.collisionRecursionDepth = 0;

	int g = grounded;
	vec3 finalPosition;
	finalPosition = collideWithWorld(eSpacePosition, velocity);
	grounded = g;

	// Add gravity pull:
	// To remove gravity uncomment from here .....

	// Set the new R3 position (convert back from eSpace to R3)
	collisionPackage.R3Position = finalPosition * collisionPackage.eRadius;
	collisionPackage.R3Velocity = gravity;

    // convert velocity to e-space
	velocity = gravity / collisionPackage.eRadius;

	// gravity iteration
	collisionPackage.collisionRecursionDepth = 0;
	finalPosition = collideWithWorld(finalPosition, velocity);

	// ... to here

	// finally set entity position
	position = finalPosition * collisionPackage.eRadius;
}

vec3 CharacterEntity::collideWithWorld(const vec3& pos, const vec3& vel)
{
	// All hard-coded distances in this function is
	// scaled to fit the setting above..
	float unitScale = unitsPerMeter / 100.0f;
	float veryCloseDistance = 0.0000005f * unitScale;

	// do we need to worry?
	if (collisionPackage.collisionRecursionDepth > 5)
		return pos;

	// Ok, we need to worry:
	collisionPackage.velocity = vel;
    collisionPackage.normalizedVelocity = normalize(vel);
	collisionPackage.basePoint = pos;
	collisionPackage.foundCollision = false;
    collisionPackage.nearestDistance = FLT_MAX;

	// Check for collision (calls the collision routines)
	// Application specific!!
    checkCollision();

	// If no collision we just move along the velocity
	if (collisionPackage.foundCollision == false) {
	  return pos + vel;
	}

	// *** Collision occured ***

	// The original destination point
	vec3 destinationPoint = pos + vel;
	vec3 newBasePoint = pos;
	// only update if we are not already very close
	// and if so we only move very close to intersection..not
	// to the exact spot.
	if (collisionPackage.nearestDistance >= veryCloseDistance)
	{
		vec3 v = (float)MIN(length(vel),  collisionPackage.nearestDistance - veryCloseDistance) * vel;
		newBasePoint = collisionPackage.basePoint + v;

		// Adjust polygon intersection point (so sliding
		// Plane will be unaffected by the fact that we
		// move slightly less than collision tells us)
		normalize(v);
		collisionPackage.intersectionPoint -=
						  veryCloseDistance * v;
	}

	// Determine the sliding Plane
	vec3 slidePlaneOrigin =
			collisionPackage.intersectionPoint;
	vec3 slidePlaneNormal =
			newBasePoint-collisionPackage.intersectionPoint;
	normalize(slidePlaneNormal);

	Plane slidingPlane(slidePlaneOrigin, slidePlaneNormal);

	// Again, sorry about formatting.. but look carefully ;)
	vec3 newDestinationPoint = destinationPoint -
	(float)slidingPlane.signedDistanceTo(destinationPoint)*
	slidePlaneNormal;

	// Generate the slide vectpr, which will become our new
	// velocity vector for the next iteration
	vec3 newVelocityVector = newDestinationPoint -
						collisionPackage.intersectionPoint;

	if (collisionPackage.intersectionPoint[1] <= pos[1]-collisionPackage.eRadius[1]+0.1f && vel[1] <= 0.0f)
		grounded = 1;

	// Recurse:

	// dont recurse if the new velocity is very small
	if (length(newVelocityVector) < veryCloseDistance) {

		return newBasePoint;
	}

    collisionPackage.collisionRecursionDepth++;

    return collideWithWorld(newBasePoint, newVelocityVector);
}

void CharacterEntity::checkCollision()
{
  // check collision against triangles
  // **!CHEAP TESTING METHOD PLS REPLACE WITH OCTREE!** //

  for (unsigned int i = 0; i < models.size(); i++){
    Model model = models.at(i);
    for (unsigned int j = 0; j < model.meshes.size(); j++) {
      Mesh mesh = model.meshes.at(j);
      for (unsigned int i=0; i<mesh.vertices.size();) {
        vec3 a, b, c;
        a = mesh.vertices.at(i++).Position / collisionPackage.eRadius;
        b = mesh.vertices.at(i++).Position / collisionPackage.eRadius;
        c = mesh.vertices.at(i++).Position / collisionPackage.eRadius;
        checkTriangle(&collisionPackage, a, b, c);
      }
    }
  }
  // **!CHEAP TESTING METHOD PLS REPLACE WITH OCTREE!** //
}

void CharacterEntity::update()
{
  this->grounded = 0;
  vec3 gravity = {0.0f, this->velocity[1], 0.0f};
  collideAndSlide(gravity);
}
