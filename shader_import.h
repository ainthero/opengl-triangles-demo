#ifndef JBTREUG_SHADER_IMPORT_H
#define JBTREUG_SHADER_IMPORT_H

#define GL_GLEXT_PROTOTYPES

#include <iostream>
#include <sstream>
#include <fstream>
#include <GL/gl.h>
#include <GL/glx.h>


std::string file_to_string(const std::string &file_path);

GLuint compile_shader(const std::string &shader_source, GLenum shader_type);

bool check_shader_compilation(GLuint shader);

bool check_shader_linking(GLuint shader_program);

GLuint get_shader_program(const std::string &vertex_source, const std::string &fragment_source);



#endif //JBTREUG_SHADER_IMPORT_H
