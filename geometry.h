#ifndef JBTREUG_GEOMETRY_H
#define JBTREUG_GEOMETRY_H

#include <GL/gl.h>
#include <GL/glx.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

struct triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;
};


GLfloat norm_x(GLfloat x, int width);

GLfloat norm_y(GLfloat y, int height);

bool is_same_side(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b);

bool is_point_in_triangle(glm::vec3 point, triangle tri);

triangle gen_triangle_by_pos(GLfloat x, GLfloat y, GLfloat h);

#endif //JBTREUG_GEOMETRY_H
