#include "MathUtils.h"

glm::mat4 MathUtils::initScreenSpace(float half_width, float half_height)
{
    glm::vec4 c1 { half_width, 0.0f,        0.0f, 0.0f };
    glm::vec4 c2 { 0.0f,       -half_height, 0.0f, 0.0f };
    glm::vec4 c3 { 0.0f,       0.0f,        1.0f, 0.0f };
    glm::vec4 c4 { half_width - 0.5f, half_height - 0.5f, 0.0f, 1.0f };

    return glm::mat4(c1, c2, c3, c4);
}
