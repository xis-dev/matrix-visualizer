#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
struct Vertex
{
    Vertex() = default;
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords): position(position), normal(normal), texCoords(texCoords){}
    Vertex(float pos_x, float pos_y, float pos_z,
           float norm_x, float norm_y, float norm_z,
           float tex_u, float tex_v):
                                        position(glm::vec3(pos_x, pos_y, pos_z)),
                                        normal(glm::vec3(norm_x, norm_y, norm_z)),
                                        texCoords(glm::vec2(tex_u, tex_v)){}
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texCoords{};
};