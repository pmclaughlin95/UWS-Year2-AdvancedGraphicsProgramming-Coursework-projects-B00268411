#include "boundingBox.h"

boundingBox::boundingBox(glm::vec3 pos, glm::vec3 max) {

	this->min.x = pos.x;
	this->min.y = pos.y;
	this->min.z = pos.z;

	this->max.x = max.x;
	this->max.y = max.y;
	this->max.z = max.z;


}

void boundingBox::update(glm::vec3 pos, glm::vec3 max)
{
	this->min.x = pos.x;
	this->min.y = pos.y;
	this->min.z = pos.z;

	this->max.x = max.x;
	this->max.y = max.y;
	this->max.z = max.z;
}
