#include "OtherPlayer.h"
#include <Gizmos.h>
#include <glm\gtx\transform.hpp>

OtherPlayer::OtherPlayer()
{
}


OtherPlayer::~OtherPlayer()
{
}

void OtherPlayer::update(float dt)
{
	position += velocity.xyz * dt * velocity.w;
}

void OtherPlayer::draw(void)
{
	aie::Gizmos::addCapsule(position, size * 2, size / 2, 16, 16, colour);
}

void OtherPlayer::initialize(glm::vec4 col, glm::vec3 startPos, float si)
{
	velocity = glm::vec4(0);
	colour = col;
	position = startPos;
	size = si;
}

void OtherPlayer::setVel(glm::vec4 vel)
{
	velocity = vel;
}

void OtherPlayer::setVel(glm::vec3 dir, float speed)
{
	velocity = glm::vec4(dir, speed);
}

void OtherPlayer::setPos(glm::vec3 pos)
{
	position = pos;
}
