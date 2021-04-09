#define GL_GLEXT_PROTOTYPES

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <array>
#include <iostream>
#include <vector>

int WIDTH = 800;
int HEIGHT = 600;

GLfloat mouse_x;
GLfloat mouse_y;
GLfloat norm_mouse_x;
GLfloat norm_mouse_y;

bool is_rbutton_pressed = false;
bool is_lbutton_pressed = false;

Window window_returned;
int root_x, root_y;
int win_x, win_y;
unsigned int mask_return;


Display *display;
Window root_win;
Window main_win;
GLint att_list[] = {GLX_RGBA, None};//describes the color encoding properties
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
GLXContext glc;
XWindowAttributes gwa;
XEvent xev;


GLchar *vertex_shader_source =
        #include "vertexS.glsl"
                ;
GLchar *fragment_shader_source =
        #include "fragmentS.glsl"
                ;

GLuint compile_shader(GLchar *shader_source, GLenum shader_type){
    GLuint shader;
    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    return shader;
}

bool check_shader_compilation(GLuint shader){
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

bool check_shader_linking(GLuint shader_program){
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

GLuint get_shader_program(GLchar* vertex_source, GLchar* fragment_source) {
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

GLfloat norm_x(GLfloat x, int width) {
    return x * (2.0f / width) - 1.0f;
}

GLfloat norm_y(GLfloat y, int height) {
    return -y * (2.0f / height) + 1.0f;
}


struct triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;
};

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

triangle gen_triangle_by_pos(GLfloat x, GLfloat y) {
    GLfloat h = 0.10;
    GLfloat x1 = x - h;
    GLfloat x2 = x + h;
    GLfloat y1 = y + 1.5f * h;
    GLfloat y2 = y + 1.5f * h;
    return triangle{glm::vec3(x, y, 0.0f),
                    glm::vec3(x1, y1, 0.0f),
                    glm::vec3(x2, y2, 0.0f)};
}


struct triangles_manager {
    GLuint VBO;
    GLuint VAO;
    GLuint shaderProgram;
    std::vector<triangle> _data;


    triangles_manager() = default;

    ~triangles_manager() = default;

    void load_data(const std::vector<triangle> &data, GLuint shader_program) {
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

    void unload_data(){
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        _data.clear();
    }

    void add_triangle_by_one_vertex(GLfloat x, GLfloat y) {
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

    int get_index_of_clicked_triangle(GLfloat x, GLfloat y) {
        auto point = glm::vec3(x, y, 0.0f);
        for (int i = (int)_data.size() - 1; i >= 0; --i)
            if (is_point_in_triangle(point, _data[i])) {
                return i;
            }
        return -1;
    }

    void update_triangle_pos(int index, GLfloat x, GLfloat y) {
        auto new_tri = gen_triangle_by_pos(x, y);
        _data[index] = new_tri;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(triangle), sizeof(triangle), &new_tri);
    }

    glm::vec3 get_vertex(int index_tri, int ind_vertex) {
        return *(&_data[index_tri].a + sizeof(glm::vec3) * ind_vertex);
    }


    void draw() {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, _data.size() * 9);
        glBindVertexArray(0);
    }
};


std::array<GLfloat, 3> get_float_from_rgb(unsigned long rgb) {
    std::array<GLfloat, 3> float_colors{};
    std::get<0>(float_colors) = ((rgb & (0xff))) / (GLfloat)0xff;
    std::get<1>(float_colors) = ((rgb & (0xff << 8)) >> 8) / (GLfloat)(0xff);
    std::get<2>(float_colors) = ((rgb & (0xff << 16)) >> 16) / (GLfloat)(0xff);
    return float_colors;
}

void initX() {
    //connect to X server with NULL display
    //graphical output will be sent to the computer on which it is executed
    display = XOpenDisplay(NULL);
    //root window is the "desktop background"
    root_win = DefaultRootWindow(display);
    vi = glXChooseVisual(display, 0, att_list);
    cmap = XCreateColormap(display, root_win, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    main_win = XCreateWindow(display, root_win, 0, 0, WIDTH, HEIGHT, 0, vi->depth, InputOutput, vi->visual,
                             CWColormap | CWEventMask, &swa);
    //CWColormap | CWEventMask - which fields from swa should be taken into account
    XMapWindow(display, main_win);//show window
    XStoreName(display, main_win, "tri");//change title
}

void initGL() {
    glc = glXCreateContext(display, vi, NULL, GL_TRUE);//create context for opengl, GL_FALSE for network
    glXMakeCurrent(display, main_win, glc);//bind context to window
    auto bg_fcolor = get_float_from_rgb(0x25854b);
    glClearColor(std::get<0>(bg_fcolor),
               std::get<1>(bg_fcolor),
                std::get<2>(bg_fcolor), 1.0f);
}

void update_mouse(){
    XQueryPointer(display, main_win, &window_returned,
                  &window_returned, &root_x, &root_y, &win_x, &win_y,
                  &mask_return);
    mouse_x = win_x;
    mouse_y = win_y;
    norm_mouse_x = norm_x(mouse_x, WIDTH);
    norm_mouse_y = norm_y(mouse_y, HEIGHT);
}

void update_viewport(){
    XGetWindowAttributes(display, main_win, &gwa);
    glViewport(0, 0, gwa.width, gwa.height);
    WIDTH = gwa.width;
    HEIGHT = gwa.height;
}



int main() {
    initX();
    initGL();
    triangles_manager triangles;
    triangles.load_data({}, get_shader_program(vertex_shader_source, fragment_shader_source));
    int index_of_clicked_triangle;
    glm::vec3 d_vec_pos;
    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, main_win, &wmDeleteMessage, 1);
    while (True) {
        XNextEvent(display, &xev);
        update_mouse();
        if (xev.type == Expose) {
            update_viewport();
        } else if (xev.type == KeyPress) {
            if (XLookupKeysym(&xev.xkey, 0) == XK_Escape) {
                break;
            }
        }
        else if(xev.type == ClientMessage && xev.xclient.data.l[0] == wmDeleteMessage)
        {
            break;
        }
        else if (xev.type == ButtonPress) {
            if (xev.xbutton.button == Button1) {
                is_lbutton_pressed = true;
                triangles.add_triangle_by_one_vertex(norm_mouse_x, norm_mouse_y);
                std::cout << "Mouse click left button: " << mouse_x << ' ' << mouse_y << std::endl;
            } else if (xev.xbutton.button == Button3) {
                is_rbutton_pressed = true;
                index_of_clicked_triangle = triangles.get_index_of_clicked_triangle(norm_mouse_x, norm_mouse_y);
                d_vec_pos = triangles.get_vertex(index_of_clicked_triangle, 0) -
                            glm::vec3(norm_mouse_x, norm_mouse_y, 0.0f);
                std::cout << "Mouse click right button: " << index_of_clicked_triangle << std::endl;
            }
        } else if (xev.type == ButtonRelease) {
            if (xev.xbutton.button == Button3)
                is_rbutton_pressed = false;
            if (xev.xbutton.button == Button1)
                is_lbutton_pressed = false;
        }
        if (is_rbutton_pressed && index_of_clicked_triangle != -1) {
            triangles.update_triangle_pos(index_of_clicked_triangle, norm_mouse_x + d_vec_pos.x, norm_mouse_y + d_vec_pos.y);
        }
        if (is_lbutton_pressed) {
            triangles.update_triangle_pos((int)triangles._data.size() - 1, norm_mouse_x, norm_mouse_y);
        }
        triangles.draw();
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
            break;
        }
        glXSwapBuffers(display, main_win);//exchange front and back buffers
    }
    triangles.unload_data();
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, glc);
    XDestroyWindow(display, main_win);
    XCloseDisplay(display);
    return 0;
}
