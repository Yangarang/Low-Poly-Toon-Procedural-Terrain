#ifndef Terrain_H
#define Terrain_H

#include "Perlin.h"

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

void terrain(std::vector <glm::vec3>& TerrainVector_Vertices, std::vector <glm::vec3>& TerrainVector_Normals, std::vector <glm::vec2>& TerrainVector_UV, 
    std::vector <glm::vec2>& TerrainVector_Biomes, int terrain_height, int terrain_width, 
    std::vector<std::vector<float>>& Height_Vertices, std::vector<std::vector<float>>& BiomeColor_Vertices) {
    float terrain_scaling = 3.0f;
    glm::vec3 U;
    glm::vec3 V;
    int count_i = 0;
    for (int i = -terrain_height / 2; i < (terrain_height / 2); i++) {
        int count_j = 0;
        for (int j = -terrain_width / 2; j < (terrain_width / 2); j++) {
            unsigned char y = 0;
            // vertex
            float terrain_x = i / terrain_scaling;
            float terrain_z = j / terrain_scaling;
            float interval_x = 1.0f / terrain_scaling;
            float interval_z = 1.0f / terrain_scaling;

            // setup which heights based on location
            float quad_topleft = Height_Vertices[count_i][count_j];
            float quad_topright = Height_Vertices[count_i][count_j + 1];
            float quad_bottomleft = Height_Vertices[count_i + 1][count_j];
            float quad_bottomright = Height_Vertices[count_i + 1][count_j + 1];

            // add vertices of top left triangle
            TerrainVector_Vertices.push_back(glm::vec3(terrain_x, quad_topleft, terrain_z)); // top left of left triangle
            TerrainVector_Vertices.push_back(glm::vec3(terrain_x + interval_x, quad_bottomleft, terrain_z)); // bottom left of left triangle
            TerrainVector_Vertices.push_back(glm::vec3(terrain_x, quad_topright, terrain_z + interval_z)); // top right of left triangle
            // add normal of top left triangle
            U = glm::vec3(terrain_x + interval_x, quad_bottomleft, terrain_z) - glm::vec3(terrain_x, quad_topleft, terrain_z);
            V = glm::vec3(terrain_x, quad_topright, terrain_z + interval_z) - glm::vec3(terrain_x, quad_topleft, terrain_z);
            TerrainVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            TerrainVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            TerrainVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            //std::cout << glm::normalize(-glm::cross(U, V)).x << ", " << glm::normalize(-glm::cross(U, V)).y << ", " << glm::normalize(-glm::cross(U, V)).z << std::endl;

            // add vertices of bottom right triangle
            TerrainVector_Vertices.push_back(glm::vec3(terrain_x + interval_x, quad_bottomleft, terrain_z)); // bottom left of right triangle
            TerrainVector_Vertices.push_back(glm::vec3(terrain_x, quad_topright, terrain_z + interval_z)); // top right of right triangle
            TerrainVector_Vertices.push_back(glm::vec3(terrain_x + interval_x, quad_bottomright, terrain_z + interval_z)); // bottom right of right triangle
            U = glm::vec3(terrain_x, quad_topright, terrain_z + interval_z) - glm::vec3(terrain_x + interval_x, quad_bottomright, terrain_z + interval_z);
            V = glm::vec3(terrain_x + interval_x, quad_bottomleft, terrain_z) - glm::vec3(terrain_x + interval_x, quad_bottomright, terrain_z + interval_z);
            TerrainVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            TerrainVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            TerrainVector_Normals.push_back(glm::normalize(-glm::cross(U, V)));
            //std::cout << glm::normalize(-glm::cross(U, V)).x << ", " << glm::normalize(-glm::cross(U, V)).y << ", " << glm::normalize(-glm::cross(U, V)).z << std::endl;

            // As for the heights based on location, do  the same for biome color vertice heights
            quad_topleft = BiomeColor_Vertices[count_i][count_j];
            quad_topright = BiomeColor_Vertices[count_i][count_j + 1];
            quad_bottomleft = BiomeColor_Vertices[count_i + 1][count_j];
            quad_bottomright = BiomeColor_Vertices[count_i + 1][count_j + 1];
            TerrainVector_Biomes.push_back(glm::vec2(quad_topleft, 0.0f));
            TerrainVector_Biomes.push_back(glm::vec2(quad_bottomleft, 0.0f));
            TerrainVector_Biomes.push_back(glm::vec2(quad_topright, 0.0f));
            TerrainVector_Biomes.push_back(glm::vec2(quad_bottomleft, 0.0f));
            TerrainVector_Biomes.push_back(glm::vec2(quad_topright, 0.0f));
            TerrainVector_Biomes.push_back(glm::vec2(quad_bottomright, 0.0f));

            // add UV coordinates for terrain texture
            TerrainVector_UV.push_back(glm::vec2(0.0f, 0.0f));
            TerrainVector_UV.push_back(glm::vec2(1.0f, 0.0f));
            TerrainVector_UV.push_back(glm::vec2(0.0f, 1.0f));
            TerrainVector_UV.push_back(glm::vec2(1.0f, 0.0f));
            TerrainVector_UV.push_back(glm::vec2(0.0f, 1.0f));
            TerrainVector_UV.push_back(glm::vec2(1.0f, 1.0f));

            count_j++;
        }
        count_i++;
    }
   
}

void terrain_perlin(std::vector<std::vector<float>>& Height_Vertices, std::vector<std::vector<float>>& BiomeColor_Vertices, 
    int terrain_height, int terrain_width) {
    // Generate heights using Perlin Noise
    for (int i = 0; i < terrain_height + 2; i++) {
        for (int j = 0; j < terrain_width + 2; j++) {
            // OctavePerlin(float x, float y, int octaves, float persistence, float frequency, float amplitude)
            Height_Vertices[i][j] = OctavePerlin((float)i + 0.5f, (float)j + 0.4f, 8, 0.5f, 0.01f, 1.0f) * 50.0f+5.0f;
            BiomeColor_Vertices[i][j] = OctavePerlin((float)i + 0.5f, (float)j + 0.4f, 6, 0.8f, 0.05f, 2.0f) * 3.0f;
            //std::cout << GradientCircle_Vertices[i][j] << " | " << Height_Vertices[i][j] << std::endl;
            //std::cout << Height_Vertices[i][j]<<", ";
        }
    }

}

#endif
