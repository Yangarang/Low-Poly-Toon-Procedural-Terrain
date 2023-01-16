#ifndef Perlin_H
#define Perlin_H

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

#define _USE_MATH_DEFINES
#include <math.h>

/* Heavily based on the below links:
* These references were used to help me understand and implement Perlin Noise:
* https://en.wikipedia.org/wiki/Perlin_noise 
* http://adrianb.io/2014/08/09/perlinnoise.html
*/

// get pseudorandom direction vector
glm::vec2 random_gradient(float ix, float iy) {
    // setup random values for psueorandom direction vector
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= a << s | a >> w - s;
    b *= 1911520717; a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1));
    // get gradient values given the random integers for x and y
    glm::vec2 v;
    v.x = cos(random);
    v.y = sin(random);
    return v;
}

// Linear interpolatation between a0 and a1
float lerp(float a0, float a1, float w) {
    return w*(a1 - a0) + a0;
}

// return gradient by getting the dot product of distance and pseudorandom direction 
float gradient(float x0, float y0, float x1, float y1) {
    // get distance using dot product
    glm::vec2 gradient = random_gradient(x0, y0);
    return (-(x0 - x1)*gradient.x) + (-(y0 - y1)*gradient.y);
}

// return perlin noise given coordinates x and y (add 0.5 to both)
float perlin_noise(float x, float y) {
    // calculate based on coordinates (x, y) and interpolation
    float x_floor = floor(x);
    float y_floor = floor(y);
    float x_ceil = x_floor + 1;
    float y_ceil = y_floor + 1;

    // Dot product between pseudorandom direction and input coordinate
    float n0 = gradient(x_floor, y_floor, x, y);
    float n1 = gradient(x_ceil, y_floor, x, y);
    float lerp_grid_x0 = lerp(n0, n1, x - x_floor);
    n0 = gradient(x_floor, y_ceil, x, y);
    n1 = gradient(x_ceil, y_ceil, x, y);
    float lerp_grid_x1 = lerp(n0, n1, x - x_floor);

    return lerp(lerp_grid_x0, lerp_grid_x1, y - y_floor);
}

// octave calcualtion to include amplitude, frequency, octaves, and persistence by totalling 
float OctavePerlin(float x, float y, int octaves, float persistence, float freq, float amp) {
    float total = 0;
    float denominator = 0;
    for (int i = 0; i < octaves; i++) {
        total = total + perlin_noise(x * freq, y * freq) * amp;
        denominator += amp;
        freq *= 2;
        amp *= persistence;
    }
    return total / denominator;
}

// generate gradient square to add to perlin noise to create one island
void GradientCircle(float terrain_width, float terrain_height, std::vector<std::vector<float>>& GradSqrHeight_Vertices) {
    int middle_i = terrain_width/2;
    int middle_j = terrain_height/2;
    for (int i = 0; i < terrain_width; i++) {
        for (int j = 0; j < terrain_height; j++) {

            float dist =  sqrt(pow(middle_i - i, 2) + pow(middle_j - j, 2) * 1.0);
            dist = dist / terrain_width;

            GradSqrHeight_Vertices[i][j] = dist;
        }
    }

}
#endif