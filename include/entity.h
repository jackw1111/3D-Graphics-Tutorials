#ifndef ENTITY_H
#define ENTITY_H

#include <vector>

#include <glm/glm.hpp>

#include "collision.h"
#include "model.h"

class CharacterEntity {
public:
  CharacterEntity(std::vector<Model> models, vec3 radius);
  void update();
  void checkCollision();
  void collideAndSlide(const vec3& gravity);
  vec3 collideWithWorld(const vec3& pos, const vec3& velocity);

  vec3 position, velocity, radius;
  CollisionPacket collisionPackage;
  std::vector<Model> models;
  int grounded;
};

#endif // ENTITY_H
