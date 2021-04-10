#ifndef JBTREUG_TRIANGLES_MANAGER_H
#define JBTREUG_TRIANGLES_MANAGER_H

#define GL_GLEXT_PROTOTYPES

#include <iostream>
#include <vector>
#include <GL/gl.h>
#include "geometry.h"


class triangles_manager {
    GLuint VBO;
    GLuint VAO;
    GLuint shaderProgram;
    std::vector<triangle> _data;
public:


    triangles_manager() = default;

    ~triangles_manager() = default;

    void load_data(const std::vector<triangle> &data, GLuint shader_program);

    void unload_data();

    void add_triangle(triangle tri);

    int get_index_of_clicked_triangle(GLfloat x, GLfloat y) const;

    void update_triangle(int index, triangle tri);

    glm::vec3 get_vertex(int index_tri, int ind_vertex) const;

    void draw() const;

    int size() const;
};

#endif //JBTREUG_TRIANGLES_MANAGER_H
