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

float mouse_x;
float mouse_y;
float click_x;
float click_y;
bool is_rbutton_pressed = false;


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


GLchar *vertexShaderSource =
        #include "vertexS.glsl"
;

GLchar *fragmentShaderSource =
        #include "fragmentS.glsl"
;


GLuint get_shader_program() {
    //compile shaders
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    //check shaders compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    //link shaders
    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    //
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
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

triangle get_triangle_by_pos(float x, float y){
    float h = 0.10;
    x = x * (2.0f / WIDTH) - 1.0f;
    y = -y * (2.0f / HEIGHT) + 1.0f;
    float x1 = x - h;
    float x2 = x + h;
    float y1 = y + 1.5f * h;
    float y2 = y + 1.5f * h;
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

    void load_data(const std::vector<triangle> &data) {
        _data = data;
        shaderProgram = get_shader_program();
        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(triangle), _data.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);
        glEnableVertexAttribArray(0);
    }

    void add_triangle_by_one_vertex(float x, float y) {
        size_t prev_capacity = _data.capacity();
        size_t prev_size = _data.size();
        auto to_push_back = get_triangle_by_pos(x, y);
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

    int get_index_of_clicked_triangle(float x, float y) {
        x = x * (2.0f / WIDTH) - 1.0f;
        y = -y * (2.0f / HEIGHT) + 1.0f;
        auto point = glm::vec3(x, y, 0.0f);
        for (int i = _data.size() - 1; i >= 0; --i)
            if (is_point_in_triangle(point, _data[i])){
                return i;
            }
        return -1;
    }

    void update_triangle_pos(int index, float x, float y){
        auto new_tri = get_triangle_by_pos(x, y);
        _data[index] = new_tri;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(triangle), sizeof(triangle), &new_tri);
    }


    void Draw() {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, _data.size() * 9);
        glBindVertexArray(0);
    }
};


std::array<GLfloat, 3> get_float_from_rgb(unsigned long rgb) {
    std::array<GLfloat, 3> float_colors{};
    std::get<0>(float_colors) = ((rgb & (0xff))) / static_cast<GLfloat>(0xff);
    std::get<1>(float_colors) = ((rgb & (0xff << 8)) >> 8) / static_cast<GLfloat>(0xff);
    std::get<2>(float_colors) = ((rgb & (0xff << 16)) >> 16) / static_cast<GLfloat>(0xff);
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
    glClearColor(std::get<0>(bg_fcolor), std::get<1>(bg_fcolor), std::get<2>(bg_fcolor), 1.0f);
}


int main() {
    initX();
    initGL();
    triangles_manager triangles;
    triangles.load_data({});
    Window window_returned;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask_return;
    int index_of_clicked_triangle;
    while (True) {
        XNextEvent(display, &xev);
        XQueryPointer(display, main_win, &window_returned,
                      &window_returned, &root_x, &root_y, &win_x, &win_y,
                      &mask_return);
        mouse_x = win_x;
        mouse_y = win_y;
        if (xev.type == Expose) {
            XGetWindowAttributes(display, main_win, &gwa);//get win atts, including current width and height
            glViewport(0, 0, gwa.width, gwa.height);
            WIDTH = gwa.width;
            HEIGHT = gwa.height;
        } else if (xev.type == KeyPress) {
            if (XLookupKeysym(&xev.xkey, 0) == XK_Escape){
                glXMakeCurrent(display, None, NULL);
                glXDestroyContext(display, glc);
                XDestroyWindow(display, main_win);
                XCloseDisplay(display);
                return 0;
            }
        } else if (xev.type == ButtonPress) {
            switch (xev.xbutton.button) {
                case Button1:
                    click_x = xev.xbutton.x;
                    click_y = xev.xbutton.y;
                    triangles.add_triangle_by_one_vertex(click_x, click_y);
                    std::cout << "Mouse click left button: " << click_x << ' ' << click_y << std::endl;
                    break;
                case Button3:
                    click_x = xev.xbutton.x;
                    click_y = xev.xbutton.y;
                    is_rbutton_pressed = true;
                    index_of_clicked_triangle = triangles.get_index_of_clicked_triangle(click_x, click_y);
                    std::cout << "Mouse click right button: " << index_of_clicked_triangle << std::endl;
                    break;
                default:
                    break;
            }
        } else if (xev.type == ButtonRelease) {
            if (xev.xbutton.button == Button3)
                is_rbutton_pressed = false;
        }
        if (is_rbutton_pressed && index_of_clicked_triangle != -1){
            triangles.update_triangle_pos(index_of_clicked_triangle, mouse_x, mouse_y);
        }
        triangles.Draw();
        glXSwapBuffers(display, main_win);//exchange front and back buffers
    }
    return 0;
}
