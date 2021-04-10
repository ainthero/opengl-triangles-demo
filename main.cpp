#define GL_GLEXT_PROTOTYPES

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <array>
#include <iostream>
#include <vector>

#include "shader_import.h"
#include "geometry.h"
#include "triangles_manager.h"

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


std::array<GLfloat, 3> get_float_from_rgb(unsigned long rgb) {
    std::array<GLfloat, 3> float_colors{};
    std::get<0>(float_colors) = ((rgb & (0xff))) / (GLfloat) 0xff;
    std::get<1>(float_colors) = ((rgb & (0xff << 8)) >> 8) / (GLfloat) (0xff);
    std::get<2>(float_colors) = ((rgb & (0xff << 16)) >> 16) / (GLfloat) (0xff);
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

void update_mouse() {
    XQueryPointer(display, main_win, &window_returned,
                  &window_returned, &root_x, &root_y, &win_x, &win_y,
                  &mask_return);
    mouse_x = win_x;
    mouse_y = win_y;
    norm_mouse_x = norm_x(mouse_x, WIDTH);
    norm_mouse_y = norm_y(mouse_y, HEIGHT);
}

void update_viewport() {
    XGetWindowAttributes(display, main_win, &gwa);
    glViewport(0, 0, gwa.width, gwa.height);
    WIDTH = gwa.width;
    HEIGHT = gwa.height;
}


int main() {
    initX();
    initGL();
    triangles_manager triangles;
    triangles.load_data({}, get_shader_program(file_to_string("../vertexS.glsl"),
                                               file_to_string("../fragmentS.glsl")));
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
        } else if (xev.type == ClientMessage && xev.xclient.data.l[0] == wmDeleteMessage) {
            break;
        } else if (xev.type == ButtonPress) {
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
            triangles.update_triangle_pos(index_of_clicked_triangle, norm_mouse_x + d_vec_pos.x,
                                          norm_mouse_y + d_vec_pos.y);
        }
        if (is_lbutton_pressed) {
            triangles.update_triangle_pos((int) triangles._data.size() - 1, norm_mouse_x, norm_mouse_y);
        }
        triangles.draw();
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
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
