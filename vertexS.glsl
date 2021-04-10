#version 330 core
layout (location = 0) in vec3 aPos;
out vec4 coord;
void main()
{
    gl_Position = vec4(aPos, 1.0);
    coord = vec4(aPos.xy, aPos.x * aPos.y, 1.0);
}