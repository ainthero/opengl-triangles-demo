#include "geometry.h"


GLfloat norm_x(GLfloat x, int width) {
    return x * (2.0f / width) - 1.0f;
}

GLfloat norm_y(GLfloat y, int height) {
    return -y * (2.0f / height) + 1.0f;
}

bool is_same_side(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b) {
    glm::vec3 cross1 = glm::cross(b - a, p1 - a);
    glm::vec3 cross2 = glm::cross(b - a, p2 - a);
    return glm::dot(cross1, cross2) >= 0.0f;
}

bool is_point_in_triangle(glm::vec3 point, triangle tri) {
    return is_same_side(point, tri.a, tri.b, tri.c) &&
           is_same_side(point, tri.b, tri.a, tri.c) &&
           is_same_side(point, tri.c, tri.a, tri.b);
}

triangle gen_triangle_by_pos(GLfloat x, GLfloat y, GLfloat h) {
    GLfloat x1 = x - h;
    GLfloat x2 = x + h;
    GLfloat y1 = y + 2.f * h;
    GLfloat y2 = y + 2.f * h;
    return triangle{glm::vec3(x, y, 0.0f),
                    glm::vec3(x1, y1, 0.0f),
                    glm::vec3(x2, y2, 0.0f)};
}



