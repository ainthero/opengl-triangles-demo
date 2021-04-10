#ifndef JBTREUG_TRIANGLES_MANAGER_H
#define JBTREUG_TRIANGLES_MANAGER_H

#define GL_GLEXT_PROTOTYPES

#include <iostream>
#include <vector>
#include <GL/gl.h>
#include "geometry.h"


struct triangles_manager {
    GLuint VBO;
    GLuint VAO;
    GLuint shaderProgram;
    std::vector<triangle> _data;


    triangles_manager() = default;

    ~triangles_manager() = default;

    void load_data(const std::vector<triangle> &data, GLuint shader_program);

    void unload_data();

    void add_triangle_by_one_vertex(GLfloat x, GLfloat y);

    int get_index_of_clicked_triangle(GLfloat x, GLfloat y) const;

    void update_triangle_pos(int index, GLfloat x, GLfloat y);

    glm::vec3 get_vertex(int index_tri, int ind_vertex) const;

    void draw() const;
};

#endif //JBTREUG_TRIANGLES_MANAGER_H
