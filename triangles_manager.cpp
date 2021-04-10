#include "triangles_manager.h"

void triangles_manager::load_data(const std::vector<triangle> &data, GLuint shader_program) {
    _data = data;
    shaderProgram = shader_program;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(triangle), _data.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(0);
}


void triangles_manager::unload_data() {
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    _data.clear();
}

void triangles_manager::add_triangle_by_one_vertex(GLfloat x, GLfloat y) {
    size_t prev_capacity = _data.capacity();
    size_t prev_size = _data.size();
    auto to_push_back = gen_triangle_by_pos(x, y);
    _data.push_back(to_push_back);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (_data.capacity() != prev_capacity) {
        glBufferData(GL_ARRAY_BUFFER, _data.capacity() * sizeof(triangle), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, _data.size() * sizeof(triangle), _data.data());
        std::cout << "VBO realloc" << std::endl;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, prev_size * sizeof(triangle), sizeof(triangle), &to_push_back);
    }
}

int triangles_manager::get_index_of_clicked_triangle(GLfloat x, GLfloat y) const {
    auto point = glm::vec3(x, y, 0.0f);
    for (int i = (int) _data.size() - 1; i >= 0; --i)
        if (is_point_in_triangle(point, _data[i])) {
            return i;
        }
    return -1;
}

void triangles_manager::update_triangle_pos(int index, GLfloat x, GLfloat y) {
    auto new_tri = gen_triangle_by_pos(x, y);
    _data[index] = new_tri;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(triangle), sizeof(triangle), &new_tri);
}

glm::vec3 triangles_manager::get_vertex(int index_tri, int ind_vertex) const {
    return *(&_data[index_tri].a + sizeof(glm::vec3) * ind_vertex);
}


void triangles_manager::draw() const {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, _data.size() * 9);
    glBindVertexArray(0);
}
