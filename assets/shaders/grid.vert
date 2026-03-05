#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 u_Model;
layout (std140) uniform Matrices {
    mat4 u_Proj;
    mat4 u_View;
};
void main() {
	gl_Position = u_Proj * u_View * u_Model * vec4(aPos, 1.0);
}