#ifndef Water_H
#define Water_H

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

void water(std::vector <glm::vec3>& WaterVector_Vertices, std::vector <glm::vec3>& WaterVector_Normals, std::vector <glm::vec2>& WaterVector_UV,
    int water_height, int water_width) {
    float water_scaling = 0.5f;
    glm::vec3 U;
    glm::vec3 V;
    for (int i = -water_height / 2; i < (water_height / 2); i++) {
        for (int j = -water_width / 2; j < (water_width / 2); j++) {
            // vertex
            float terrain_x = i / water_scaling;
            float terrain_y = 0.0f;
            float terrain_z = j / water_scaling;
            float interval_x = 1.0f / water_scaling;
            float interval_z = 1.0f / water_scaling;

            // add vertices of top left triangle
            WaterVector_Vertices.push_back(glm::vec3(terrain_x, terrain_y, terrain_z)); // top left of left triangle
            WaterVector_Vertices.push_back(glm::vec3(terrain_x + interval_x, terrain_y, terrain_z)); // bottom left of left triangle
            WaterVector_Vertices.push_back(glm::vec3(terrain_x, terrain_y, terrain_z + interval_z)); // top right of left triangle
            // add normal of top left triangle
            U = glm::vec3(terrain_x + interval_x, terrain_y, terrain_z) - glm::vec3(terrain_x, terrain_y, terrain_z);
            V = glm::vec3(terrain_x, terrain_y, terrain_z + interval_z) - glm::vec3(terrain_x, terrain_y, terrain_z);
            WaterVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            WaterVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            WaterVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));

            // add vertices of bottom right triangle
            WaterVector_Vertices.push_back(glm::vec3(terrain_x + interval_x, terrain_y, terrain_z)); // bottom left of right triangle
            WaterVector_Vertices.push_back(glm::vec3(terrain_x, terrain_y, terrain_z + interval_z)); // top right of right triangle
            WaterVector_Vertices.push_back(glm::vec3(terrain_x + interval_x, terrain_y, terrain_z + interval_z)); // bottom right of right triangle
            U = glm::vec3(terrain_x, terrain_y, terrain_z + interval_z) - glm::vec3(terrain_x + interval_x, terrain_y, terrain_z + interval_z);
            V = glm::vec3(terrain_x + interval_x, terrain_y, terrain_z) - glm::vec3(terrain_x + interval_x, terrain_y, terrain_z + interval_z);
            WaterVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            WaterVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            WaterVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));

            // add UV coordinates for water texture
            WaterVector_UV.push_back(glm::vec2(0.0f, 0.0f));
            WaterVector_UV.push_back(glm::vec2(1.0f, 0.0f));
            WaterVector_UV.push_back(glm::vec2(0.0f, 1.0f));
            WaterVector_UV.push_back(glm::vec2(1.0f, 0.0f));
            WaterVector_UV.push_back(glm::vec2(0.0f, 1.0f));
            WaterVector_UV.push_back(glm::vec2(1.0f, 1.0f));

        }
    }
    
   
}

#endif
