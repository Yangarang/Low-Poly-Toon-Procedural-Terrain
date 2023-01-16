#ifndef Sphere_H
#define Sphere_H

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

// Generate point from the u, v coordinates in (0..1, 0..1)
void sphere_point(float& u, float& v) {
    float pi = std::atan(1.0f) * 4.0f;
    float r = std::sin(pi * v);
    u = r * std::cos(2.0f * pi * u);
    v = r * std::sin(2.0f * pi * u);
}

void sphere(float sphereRadius, int sectorCount, int stackCount, std::vector<glm::vec3>& vertex,
    std::vector<glm::vec3>& normal, std::vector<glm::vec2>& uv, std::vector<glm::ivec3>& tria) {
    // init variables
    vertex.resize(0);
    normal.resize(0);
    tria.resize(0);
    uv.resize(0);
    // temp variables
    glm::vec3 sphereVertexPos;
    glm::vec2 uvPos;
    float xy;
    float sectorStep = 2.0f * M_PI / float(sectorCount);
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    // compute vertices and normals
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2.0f - i * stackStep;
        xy = sphereRadius * cosf(stackAngle);
        sphereVertexPos.z = sphereRadius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            // vertex position
            sphereVertexPos.x = xy * cosf(sectorAngle);
            sphereVertexPos.y = xy * sinf(sectorAngle);
            vertex.push_back(sphereVertexPos);

            // normalized vertex normal
            normal.push_back(sphereVertexPos / sphereRadius);

            // compute uv texture coordinates for sphere vertices
            uvPos.x = (float)j / (sectorCount + 1);   //longitude
            uvPos.y = (float)i / stackCount;    //latitude
            //std::cout << uvPos.x << ", " << uvPos.y << std::endl;
            uv.push_back(uvPos);

        }
    }

    // compute triangle indices
    int k1, k2;
    glm::vec2 uvPos1;
    glm::vec2 uvPos2;
    glm::vec2 uvPos3;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                tria.push_back(glm::ivec3(k1, k2, k1 + 1));
            }
            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                tria.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
            }
        }
    }
}

#endif
