#version 330 core
out vec4 FragColor;
in vec4 coord;
void main()
{
    FragColor = coord;
}
