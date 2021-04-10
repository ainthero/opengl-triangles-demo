#include "shader_import.h"


std::string file_to_string(const std::string &file_path) {
    std::string shader_code;
    std::ifstream shader_stream(file_path, std::ios::in);
    if (shader_stream.is_open()) {
        std::stringstream sstr;
        sstr << shader_stream.rdbuf();
        shader_code = sstr.str();
        shader_stream.close();
    }
    return shader_code;
}

GLuint compile_shader(const std::string &shader_source, GLenum shader_type) {
    GLuint shader;
    shader = glCreateShader(shader_type);
    char const *shader_pointer = shader_source.c_str();
    glShaderSource(shader, 1, &shader_pointer, NULL);
    glCompileShader(shader);
    return shader;
}

bool check_shader_compilation(GLuint shader) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        std::cerr << "SHADER COMPILATION FAILED\n" << info_log << std::endl;
        return false;
    }
    return true;
}

bool check_shader_linking(GLuint shader_program) {
    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
        std::cerr << "SHADER LINKING FAILED\n" << info_log << std::endl;
        return false;
    }
    return true;
}

GLuint get_shader_program(const std::string &vertex_source, const std::string &fragment_source) {
    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    check_shader_compilation(vertex_shader);
    check_shader_compilation(fragment_shader);
    //link shaders
    GLuint shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    check_shader_linking(shader_program);
    //
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}

