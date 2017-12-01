#pragma once
#define GLM_SWIZZLE
#include <glm\glm.hpp>
#include <string>

class OtherPlayer
{
public:
	OtherPlayer();
	~OtherPlayer();

	void update(float dt);
	void draw(void);
	void initialize(glm::vec4 col, glm::vec3 startPos, float size);
	void setVel(glm::vec4 vel);
	void setVel(glm::vec3 dir, float speed);
	void setPos(glm::vec3 pos);

	int id = -1;
	bool isSelf = false;
	glm::vec4 colour;

private:
	float size;
	
	glm::vec3 position;
	glm::vec4 velocity;

	glm::mat4 transform;
};

