#define GL_GLEXT_PROTOTYPES

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <cassert>
#include <array>
#include <iostream>
#include <vector>
#include <fstream>
#include <streambuf>
#include <string.h>
#include <sstream>

Display *display;
Window root_win;
GLint att_list[] = {GLX_RGBA, None};//describes the color encoding properties
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
Window main_win;
GLXContext glc;
XWindowAttributes gwa;
XEvent xev;
unsigned long background_color = 0x25854b;


GLchar *vertexShaderSource =
        #include "vertexS.glsl"
;

GLchar *fragmentShaderSource =
        #include "fragmentS.glsl"
;

unsigned int get_shader_program() {
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

struct VBO_manager {
    GLuint VBO;
    GLuint VAO;
    GLuint shaderProgram;
    std::vector<float> _data;

    explicit VBO_manager(GLuint size) {
        _data.reserve(size);
        shaderProgram = get_shader_program();
        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    explicit VBO_manager(const std::vector<float> &data) : _data(data) {
        shaderProgram = get_shader_program();
        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), _data.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);
    }

    void add_triangle(float x, float y){
        float x1 = x - 20;
        float x2 = x + 20;
        float y1 = y + 25;
        float y2 = y + 25;
        XGetWindowAttributes(display, main_win, &gwa);
        size_t prev_capacity = _data.capacity();
        size_t prev_size = _data.size();
        std::vector<float> to_push_back = {x * (2.0f / gwa.width) - 1.0f, -y * (2.0f / gwa.height) + 1.0f, 0.0f,
                                           x1 * (2.0f / gwa.width) - 1.0f, -y1 * (2.0f / gwa.height) + 1.0f, 0.0f,
                                           x2 * (2.0f / gwa.width) - 1.0f, -y2 * (2.0f / gwa.height) + 1.0f, 0.0f};
        std::copy (to_push_back.begin(), to_push_back.end(), back_inserter( _data));
        if (_data.capacity() != prev_capacity){
            glBufferData(GL_ARRAY_BUFFER, _data.capacity() * sizeof(float), NULL, GL_DYNAMIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, _data.size() * sizeof(float), _data.data());
            std::cout<<"VBO realloc" << std::endl;
        }
        else{
            glBufferSubData(GL_ARRAY_BUFFER, prev_size * sizeof(float), to_push_back.size() * sizeof(float), to_push_back.data());
        }
        return;
    }

    void Draw(){
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, _data.size());
        glBindVertexArray(0);
    }
};


std::array<float, 3> get_float_from_rgb(unsigned long rgb) {
    std::array<float, 3> float_colors;
    std::get<0>(float_colors) = ((rgb & (0xff))) / static_cast<float>(0xff);
    std::get<1>(float_colors) = ((rgb & (0xff << 8)) >> 8) / static_cast<float>(0xff);
    std::get<2>(float_colors) = ((rgb & (0xff << 16)) >> 16) / static_cast<float>(0xff);
    return float_colors;
}


int main() {
    //connect to X server with NULL display
    //graphical output will be sent to the computer on which it is executed
    display = XOpenDisplay(NULL);
    //root window is the "desktop background"
    root_win = DefaultRootWindow(display);
    vi = glXChooseVisual(display, 0, att_list);
    cmap = XCreateColormap(display, root_win, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    main_win = XCreateWindow(display, root_win, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual,
                             CWColormap | CWEventMask, &swa);
    //CWColormap | CWEventMask - which fields from swa should be taken into account
    XMapWindow(display, main_win);//show window
    XStoreName(display, main_win, "triforcestriforcestriforces");//change title
    glc = glXCreateContext(display, vi, NULL, GL_TRUE);//create context for opengl, GL_FALSE for network
    glXMakeCurrent(display, main_win, glc);//bind context to window

    VBO_manager triangles(0);

    auto bg_fcolor = get_float_from_rgb(background_color);
    glClearColor(std::get<0>(bg_fcolor), std::get<1>(bg_fcolor), std::get<2>(bg_fcolor), 1.0f);
    while (True) {
        XNextEvent(display, &xev);
        int x = -1;
        int y = -1;
        if (xev.type == Expose) {
            XGetWindowAttributes(display, main_win, &gwa);//get win atts, including current width and height
            glViewport(0, 0, gwa.width, gwa.height);
        } else if (xev.type == KeyPress) {
            glXMakeCurrent(display, None, NULL);
            glXDestroyContext(display, glc);
            XDestroyWindow(display, main_win);
            XCloseDisplay(display);
            return 0;
        } else if (xev.type == ButtonPress) {
            x = xev.xbutton.x;
            y = xev.xbutton.y;
            triangles.add_triangle(x, y);
            std::cout << "Mouse click: " << x << ' ' << y << std::endl;
        }
        triangles.Draw();
        glXSwapBuffers(display, main_win);//exchange front and back buffers
    }
    return 0;
}
