#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class boundingBox {

public:
	boundingBox(glm::vec3 pos, glm::vec3 max);
	void update(glm::vec3 min, glm::vec3 max);

	glm::vec3 min;
	glm::vec3 max;

};

