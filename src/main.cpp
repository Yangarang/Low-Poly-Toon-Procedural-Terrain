// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include "Sphere.h"
#include "Perlin.h"
#include "Skybox.h"
#include "Terrain.h"
#include "Water.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

// VertexBufferObject wrapper
BufferObject VBO;
// VertexBufferObject wrapper
BufferObject NBO;
// VertexBufferObject wrapper
BufferObject UVB;
// VertexBufferObject wrapper
BufferObject IndexBuffer;

// Contains the vertex positions
std::vector<glm::vec3> V(3);
// Contains the vertex positions
std::vector<glm::vec3> VN(3);
// Contains the vertex positions
std::vector<glm::ivec3> T(3);
// Contains the vertex UV positions
std::vector<glm::vec2> UV(3);

// Last position of the mouse on click
double xpos, ypos;


// PPM Reader code from http://josiahmanson.com/prose/optimize_ppm/

struct RGB {
    unsigned char r, g, b;
};

struct ImageRGB {
    int w, h;
    std::vector<RGB> data;
};

void eat_comment(std::ifstream& f) {
    char linebuf[1024];
    char ppp;
    while (ppp = f.peek(), ppp == '\n' || ppp == '\r')
        f.get();
    if (ppp == '#')
        f.getline(linebuf, 1023);
}

bool loadPPM(ImageRGB& img, const std::string& name) {
    std::ifstream f(name.c_str(), std::ios::binary);
    if (f.fail()) {
        std::cout << "Could not open file: " << name << std::endl;
        return false;
    }

    // get type of file
    eat_comment(f);
    int mode = 0;
    std::string s;
    f >> s;
    if (s == "P3")
        mode = 3;
    else if (s == "P6")
        mode = 6;

    // get w
    eat_comment(f);
    f >> img.w;

    // get h
    eat_comment(f);
    f >> img.h;

    // get bits
    eat_comment(f);
    int bits = 0;
    f >> bits;

    // error checking
    if (mode != 3 && mode != 6) {
        std::cout << "Unsupported magic number" << std::endl;
        f.close();
        return false;
    }
    if (img.w < 1) {
        std::cout << "Unsupported width: " << img.w << std::endl;
        f.close();
        return false;
    }
    if (img.h < 1) {
        std::cout << "Unsupported height: " << img.h << std::endl;
        f.close();
        return false;
    }
    if (bits < 1 || bits > 255) {
        std::cout << "Unsupported number of bits: " << bits << std::endl;
        f.close();
        return false;
    }

    // load image data
    img.data.resize(img.w * img.h);

    if (mode == 6) {
        f.get();
        f.read((char*)&img.data[0], img.data.size() * 3);
    }
    else if (mode == 3) {
        for (int i = 0; i < img.data.size(); i++) {
            int v;
            f >> v;
            img.data[i].r = v;
            f >> v;
            img.data[i].g = v;
            f >> v;
            img.data[i].b = v;
        }
    }

    // close file
    f.close();
    return true;
}

bool loadOFFFile(std::string filename, std::vector<glm::vec3>& vertex, std::vector<glm::ivec3>& tria, glm::vec3& min, glm::vec3& max)
{
    min.x = FLT_MAX;
    max.x = FLT_MIN;
    min.y = FLT_MAX;
    max.y = FLT_MIN;
    min.z = FLT_MAX;
    max.z = FLT_MIN;
    try {
        std::ifstream ofs(filename, std::ios::in | std::ios_base::binary);
        if (ofs.fail()) return false;
        std::string line, tmpStr;
        // First line(optional) : the letters OFF to mark the file type.
        // Second line : the number of vertices, number of faces, and number of edges, in order (the latter can be ignored by writing 0 instead).
        int numVert = 0;
        int numFace = 0;
        int numEdge = 0;
        // first line must be OFF
        getline(ofs, line);
        if (line.rfind("OFF", 0) == 0)
            getline(ofs, line);
        std::stringstream tmpStream(line);
        getline(tmpStream, tmpStr, ' ');
        numVert = std::stoi(tmpStr);
        getline(tmpStream, tmpStr, ' ');
        numFace = std::stoi(tmpStr);
        getline(tmpStream, tmpStr, ' ');
        numEdge = std::stoi(tmpStr);

        // read all vertices and get min/max values
        vertex.resize(numVert);
        for (int i = 0; i < numVert; i++) {
            getline(ofs, line);
            tmpStream.clear();
            tmpStream.str(line);
            getline(tmpStream, tmpStr, ' ');
            vertex[i].x = std::stof(tmpStr);
            min.x = std::fminf(vertex[i].x, min.x);
            max.x = std::fmaxf(vertex[i].x, max.x);
            getline(tmpStream, tmpStr, ' ');
            vertex[i].y = std::stof(tmpStr);
            min.y = std::fminf(vertex[i].y, min.y);
            max.y = std::fmaxf(vertex[i].y, max.y);
            getline(tmpStream, tmpStr, ' ');
            vertex[i].z = std::stof(tmpStr);
            min.z = std::fminf(vertex[i].z, min.z);
            max.z = std::fmaxf(vertex[i].z, max.z);
        }

        // read all faces (triangles)
        tria.resize(numFace);
        for (int i = 0; i < numFace; i++) {
            getline(ofs, line);
            tmpStream.clear();
            tmpStream.str(line);
            getline(tmpStream, tmpStr, ' ');
            if (std::stoi(tmpStr) != 3) return false;
            getline(tmpStream, tmpStr, ' ');
            tria[i].x = std::stoi(tmpStr);
            getline(tmpStream, tmpStr, ' ');
            tria[i].y = std::stoi(tmpStr);
            getline(tmpStream, tmpStr, ' ');
            tria[i].z = std::stoi(tmpStr);
        }

        ofs.close();
    }
    catch (const std::exception& e) {
        // return false if an exception occurred
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// camera setup and matrix calculations
glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 cameraDirection;
glm::vec3 cameraUp;
glm::vec3 cameraRight;
glm::mat4 viewMatrix;
glm::mat4 projMatrix;
// Setup different camera settings
int cameraSetting = 1;
int cameraReset = 1;
float camRadius = 35.0f;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Get the position of the mouse in the window
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << "------------------------" <<  std::endl;
        std::cout << "Click Screen Position: " << xpos << " " << ypos << std::endl;
        std::cout << "Camera Position: " << cameraPos.x << "f, " << cameraPos.y << "f, " << cameraPos.z << std::endl;
        std::cout << "Camera Target: " << cameraTarget.x << "f, " << cameraTarget.y << "f, " << cameraTarget.z << std::endl;
        std::cout << "Camera Direction: " << cameraDirection.x << "f, " << cameraDirection.y << "f, " << cameraDirection.z << std::endl;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // temp variables
    glm::mat3 rot;
    glm::vec3 strafe;
    glm::vec3 prev_vec;
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key)
    {
    case GLFW_KEY_A:
        if (cameraSetting == 1) {
            strafe = cross(cameraDirection, cameraUp);
            cameraPos += strafe * 0.25f;
            cameraTarget += strafe * 0.25f;
        }
        if (cameraSetting == 2) {
            cameraPos += glm::vec3(1.0f,0.0f,0.0f);
            cameraTarget += glm::vec3(1.0f, 0.0f, 0.0f);
        }
        break;
    case GLFW_KEY_D:
        if (cameraSetting == 1) {
            strafe = cross(cameraDirection, cameraUp);
            cameraPos -= strafe * 0.25f;
            cameraTarget -= strafe * 0.25f;
        }
        if (cameraSetting == 2) {
            cameraPos -= glm::vec3(1.0f, 0.0f, 0.0f);
            cameraTarget -= glm::vec3(1.0f, 0.0f, 0.0f);
        }
        break;
    case GLFW_KEY_W:
        if (cameraSetting == 1) {
            cameraPos -= cameraDirection * 0.25f;
            cameraTarget -= cameraDirection * 0.25f;
        }
        if (cameraSetting == 2) {
            cameraPos += glm::vec3(0.0f, 0.0f, 1.0f);
            cameraTarget += glm::vec3(0.0f, 0.0f, 1.0f);
        }
        break;
    case GLFW_KEY_S:
        if (cameraSetting == 1) {
            cameraPos += cameraDirection * 0.25f;
            cameraTarget += cameraDirection * 0.25f;
        }
        if (cameraSetting == 2) {
            cameraPos -= glm::vec3(0.0f, 0.0f, 1.0f);
            cameraTarget -= glm::vec3(0.0f, 0.0f, 1.0f);
        }
        break;
    case GLFW_KEY_UP:
        if (cameraSetting == 1) {
            rot = glm::rotate(glm::mat4(1.0f), glm::radians(1.5f), cameraRight);
            cameraTarget = rot * (cameraTarget-cameraPos)+cameraPos;
            cameraDirection = glm::normalize(cameraPos - cameraTarget);
            cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        }
        if (cameraSetting == 2 || cameraSetting == 3) {
            cameraPos -= cameraDirection * 1.0f;
            cameraTarget -= cameraDirection * 1.0f;
        }
        break;
    case GLFW_KEY_DOWN:
        if (cameraSetting == 1) {
            rot = glm::rotate(glm::mat4(1.0f), glm::radians(-1.5f), cameraRight);
            cameraTarget = rot * (cameraTarget - cameraPos) + cameraPos;
            cameraDirection = glm::normalize(cameraPos - cameraTarget);
            cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        }
        if (cameraSetting == 2 || cameraSetting == 3) {
            cameraPos += cameraDirection * 1.0f;
            cameraTarget += cameraDirection * 1.0f;
        }
        break;
    case GLFW_KEY_LEFT:
        if (cameraSetting == 1) {
            rot = glm::rotate(glm::mat4(1.0f), glm::radians(1.5f), cameraUp);
            cameraTarget = (rot * (cameraTarget - cameraPos)) + cameraPos;
            cameraDirection = glm::normalize(cameraPos - cameraTarget);
            cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        }
        break;
    case GLFW_KEY_RIGHT:
        if (cameraSetting == 1) {
            rot = glm::rotate(glm::mat4(1.0f), glm::radians(-1.5f), cameraUp);
            cameraTarget = (rot * (cameraTarget - cameraPos)) + cameraPos;
            cameraDirection = glm::normalize(cameraPos - cameraTarget);
            cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        }
        break;
    case GLFW_KEY_SPACE:
        if (cameraSetting == 1) {
            cameraPos += glm::vec3(0.0f, 0.1f, 0.0f);
            cameraTarget += glm::vec3(0.0f, 0.1f, 0.0f);
        }
        // Jump to where you clicked
        if (cameraSetting == 2) {
            cameraSetting = 1;
            cameraPos = glm::vec3(cameraPos.x, 10.0f, cameraPos.z);
            cameraTarget = glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z + 1.0f);
            cameraDirection = glm::normalize(cameraPos - cameraTarget);
            cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
            cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        }
        break;
    case GLFW_KEY_LEFT_CONTROL:
        if (cameraSetting == 1) {
            cameraPos -= glm::vec3(0.0f, 0.1f, 0.0f);
            cameraTarget -= glm::vec3(0.0f, 0.1f, 0.0f);
        }
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    // reset to different default handpicked screenshots :)
    case GLFW_KEY_1:
        cameraSetting = 1;
        cameraPos = glm::vec3(0.149178f, 3.0f, -11.6130f);
        cameraTarget = glm::vec3(1.12733f, 3.0f, -11.4051f);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    case GLFW_KEY_2:
        cameraSetting = 1;
        cameraPos = glm::vec3(-10.75f, 4.0f, 15.75f);
        cameraTarget = glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z + 1.0f);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    case GLFW_KEY_3:
        cameraSetting = 1;
        cameraPos = glm::vec3(9.78418f, 6.31579f, 45.3191);
        cameraTarget = glm::vec3(8.81708f, 6.21119f, 45.0869);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    // show tree and grass objects
    case GLFW_KEY_0:
        cameraSetting = 1;
        cameraPos = glm::vec3(0.0f, -5.0f, -4.0f);
        cameraTarget = glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z + 1.0f);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    // top down view
    case GLFW_KEY_TAB:
        cameraSetting = 2;
        cameraPos = glm::vec3(cameraPos.x, 99.0f, cameraPos.z);
        cameraTarget = glm::vec3(cameraPos.x, cameraPos.y-1.0f, cameraPos.z + 0.001f);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    default:
        break;
    }
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1920, 1080, "jy1045 - Jonathan Yang Final OpenGL", NULL, NULL);
    //window = glfwCreateWindow(800, 600, "jy1045 - Jonathan Yang Final OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
    glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
    std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
#endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    std::cout << "OpenGL version recieved: " << major << "." << minor << "." << rev << std::endl;
    std::cout << "Supported OpenGL is " << (const char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "Supported GLSL is " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    ImageRGB image;

    // initialize element array buffer
    IndexBuffer.init(GL_ELEMENT_ARRAY_BUFFER);
    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();
    // initialize model matrix
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    // load fragment shader file 
    std::ifstream fragShaderFile("../shader/fragment.glsl");
    std::stringstream fragCode;
    fragCode << fragShaderFile.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile("../shader/vertex.glsl");
    std::stringstream vertCode;
    vertCode << vertShaderFile.rdbuf();
    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertCode.str(), fragCode.str(), "outColor");
    program.bind();

    /* -------------------
    //  SKYBOX SHADER : Load Skybox
    ------------------- */
    VertexArrayObject SkyboxVAO;
    SkyboxVAO.init();
    SkyboxVAO.bind();
    // Initialize the VBO with the vertices data
    BufferObject SkyboxVBO;
    SkyboxVBO.init();
    // Initialize the VBO with the UV data
    BufferObject SkyboxUVB;
    SkyboxUVB.init();
    
    // Setup skybox vertices and faces
    std::vector<std::string> SkyBoxVector_faces;
    std::vector<glm::vec3> SkyBoxVector_Vertices;
    std::vector<glm::vec3> SkyBoxVector_UV;
    // Call Skybox calculation functionand update Buffer Object
    skybox(SkyBoxVector_Vertices, SkyBoxVector_faces);
    SkyboxVBO.update(SkyBoxVector_Vertices);

    // Setup textures for each side of skybox
    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    // Disabling read PPM for skybox since my Skybox PPMs are not playing nice with GIT
    /*
    // get image data from read ppm images
    for (unsigned int i = 0; i < SkyBoxVector_faces.size(); i++) {
        bool imageAvailable = loadPPM(image, SkyBoxVector_faces[i]);
        // convert image rgb to data for glTexImage2D
        float* image_data = new float[image.h * image.w * 3];
        int count_a = 0;
        int count_b = 0;
        for (int i = 0; i < image.h; i++) {
            for (int j = 0; j < image.w; j++) {
                image_data[count_a + 0] = image.data.at(count_b).r / 255.0;
                image_data[count_a + 1] = image.data.at(count_b).g / 255.0;
                image_data[count_a + 2] = image.data.at(count_b).b / 255.0;
                count_a += 3; count_b++;
            }
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image.w, image.h, 0, GL_RGB, GL_FLOAT, image_data);
    }*/
    // Using stb to read skybox PNG images
    int width, height, nrChannels;
    for (unsigned int i = 0; i < SkyBoxVector_faces.size(); i++) {
        unsigned char* data = stbi_load(SkyBoxVector_faces.at(i).c_str(), &width, &height, &nrChannels, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    Program SkyBoxProgram;
    // load fragment shader file 
    std::ifstream fragShaderFile2("../shader/skybox-fragment.glsl");
    std::stringstream fragCode2;
    fragCode2 << fragShaderFile2.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile2("../shader/skybox-vertex.glsl");
    std::stringstream vertCode2;
    vertCode2 << vertShaderFile2.rdbuf();
    SkyBoxProgram.init(vertCode2.str(), fragCode2.str(), "outColor");
    SkyBoxProgram.bind();

    /* -------------------
    //  TERRAIN SHADER : Perlin Terrain Generator
    ------------------- */
    VertexArrayObject TerrainVAO;
    TerrainVAO.init();
    TerrainVAO.bind();
    // Initialize the VBO with the vertices data
    BufferObject TerrainVBO;
    TerrainVBO.init();
    // Initialize the VBO with the normals data
    BufferObject TerrainNBO;
    TerrainNBO.init();
    // Initialize the VBO with the UV data
    BufferObject TerrainUVB;
    TerrainUVB.init();
    // Initialize the VBO with the Color Biome Vertices data
    BufferObject TerrainCBO;
    TerrainCBO.init();

    // Generate Vertices and Normals
    int terrain_height = 500;
    int terrain_width = 500;
    // Generate heights and terrain biome heights using Perlin Noise
    std::vector<std::vector<float>> Height_Vertices(terrain_height + 2, std::vector<float>(terrain_width + 2, 0));
    std::vector<std::vector<float>> BiomeColor_Vertices(terrain_height + 2, std::vector<float>(terrain_width + 2, 0));
    // Call terrain perlin function to generate heights based on perlin noise
    terrain_perlin(Height_Vertices, BiomeColor_Vertices, terrain_height, terrain_width);

    // Setup Terrain vertices, normals, UV, and Biome colors
    std::vector <glm::vec3> TerrainVector_Vertices;
    std::vector <glm::vec3> TerrainVector_Normals;
    std::vector <glm::vec2> TerrainVector_UV;
    std::vector <glm::vec2> TerrainVector_Biomes;
    // Call terrain calculation function and update Buffer Objects
    terrain(TerrainVector_Vertices, TerrainVector_Normals, TerrainVector_UV, TerrainVector_Biomes, 
        terrain_height, terrain_width, Height_Vertices, BiomeColor_Vertices);
    TerrainVBO.update(TerrainVector_Vertices);
    TerrainNBO.update(TerrainVector_Normals);
    TerrainUVB.update(TerrainVector_UV);
    TerrainCBO.update(TerrainVector_Biomes);

    Program TerrainProgram;
    // load fragment shader file 
    std::ifstream fragShaderFile3("../shader/terrain-fragment.glsl");
    std::stringstream fragCode3;
    fragCode3 << fragShaderFile3.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile3("../shader/terrain-vertex.glsl");
    std::stringstream vertCode3;
    vertCode3 << vertShaderFile3.rdbuf();
    TerrainProgram.init(vertCode3.str(), fragCode3.str(), "outColor");
    TerrainProgram.bind();

    /* -------------------
    //  WATER SHADER
    ------------------- */
    VertexArrayObject WaterVAO;
    WaterVAO.init();
    WaterVAO.bind();
    // Initialize the VBO with the vertices data
    BufferObject WaterVBO;
    WaterVBO.init();
    // Initialize the VBO with the normals data
    BufferObject WaterNBO;
    WaterNBO.init();
    // Initialize the VBO with the UV data
    BufferObject WaterUVB;
    WaterUVB.init();

    // Generate Vertices and Normals
    int water_height = 400;
    int water_width = 400;

    std::vector <glm::vec3> WaterVector_Vertices;
    std::vector <glm::vec3> WaterVector_Normals;
    std::vector <glm::vec2> WaterVector_UV;
    // Call water calculation function and update Buffer Objects
    water(WaterVector_Vertices, WaterVector_Normals, WaterVector_UV,
        water_height, water_width);

    WaterVBO.update(WaterVector_Vertices);
    WaterNBO.update(WaterVector_Normals);
    WaterUVB.update(WaterVector_UV);

    bool imageAvailable = loadPPM(image, "../data/water.ppm");
    // create a texture and upload the image data
    GLuint water_tex;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &water_tex);
    glBindTexture(GL_TEXTURE_2D, water_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // convert image rgb to data for glTexImage2D
    float* image_data = new float[image.h * image.w * 3];
    int count_a = 0;
    int count_b = 0;
    for (int i = 0; i < image.h; i++) {
        for (int j = 0; j < image.w; j++) {
            image_data[count_a + 0] = image.data.at(count_b).r / 255.0;
            image_data[count_a + 1] = image.data.at(count_b).g / 255.0;
            image_data[count_a + 2] = image.data.at(count_b).b / 255.0;
            count_a += 3; count_b++;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.w, image.h, 0, GL_RGB, GL_FLOAT, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    Program WaterProgram;
    // load fragment shader file 
    std::ifstream fragShaderFile4("../shader/water-fragment.glsl");
    std::stringstream fragCode4;
    fragCode4 << fragShaderFile4.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile4("../shader/water-vertex.glsl");
    std::stringstream vertCode4;
    vertCode4 << vertShaderFile4.rdbuf();
    WaterProgram.init(vertCode4.str(), fragCode4.str(), "outColor");
    WaterProgram.bind();

    /* -------------------
    //  OFF FILE SHADER: Setup Shader Program for Palm Tree
    ------------------- */
    // generate sphere (radius, #sectors, #stacks, vertices, normals, uv coordinates, triangle indices)
    // Initialize the VBO with the vertices data
    VertexArrayObject TreeVAO;
    TreeVAO.init();
    TreeVAO.bind();
    VBO.init();
    // initialize normal array buffer
    NBO.init();
    // initialize UV array buffer
    UVB.init();
    // load  OFF file
    glm::vec3 min, max, tmpVec;
    std::cout << "Loading OFF file...";
    loadOFFFile("../data/tree.off", V, T, min, max);
    //loadOFFFile("../data/bunny.off", V, T, min, max);
    std::cout << " done! " << V.size() << " vertices, " << T.size() << " triangles" << std::endl;
    VBO.update(V);
    IndexBuffer.update(T);
    // compute model matrix so that the mesh is inside a -1..1 cube
    tmpVec = max - min;
    float maxVal = glm::max(tmpVec.x, glm::max(tmpVec.y, tmpVec.z));
    tmpVec /= 5.0f;
    // compute face normals
    std::cout << "Computing face normals...";
    std::vector<glm::vec3> faceN(3);
    faceN.resize(T.size());
    for (unsigned int i = 0; i < faceN.size(); i++) {
        faceN[i] = glm::normalize(glm::cross(V[T[i].y] - V[T[i].x], V[T[i].z] - V[T[i].x]));
    }
    std::cout << " done!" << std::endl;
    // compute vertex normals
    std::cout << "Computing vertex normals...";
    VN.resize(V.size());
    for (unsigned int i = 0; i < VN.size(); i++) {
        VN[i] = glm::vec3(0.0f);
    }
    for (unsigned int j = 0; j < T.size(); j++) {
        VN[T[j].x] += faceN[j];
        VN[T[j].y] += faceN[j];
        VN[T[j].z] += faceN[j];
    }
    for (unsigned int i = 0; i < VN.size(); i++) {
        VN[i] = glm::normalize(VN[i]);
    }
    std::cout << " done!" << std::endl;
    // initialize normal array buffer
    NBO.init();
    NBO.update(VN);

    // Randomly populate tree based on terrain vertices
    std::vector<glm::vec3> TreeVector_Positions;
    TreeVector_Positions.push_back(glm::vec3(1.0f, -5.0f, 0.0f));
    srand(3);
    for (int i = 0; i < TerrainVector_Vertices.size(); i++) {
        float rand_populate = (float)rand() / (float)RAND_MAX;
        if (TerrainVector_Vertices.at(i).y > 0.35f && TerrainVector_Vertices.at(i).y < 4.3f) {
            if (rand_populate < 0.01) {
                TreeVector_Positions.push_back(glm::vec3(TerrainVector_Vertices.at(i).x, TerrainVector_Vertices.at(i).y + 0.015, TerrainVector_Vertices.at(i).z));
            }
        }
    }
    std::cout << "Number of Trees: " << TreeVector_Positions.size() << std::endl;

    Program TreeProgram;
    // load fragment shader file 
    std::ifstream fragShaderFile5("../shader/fragment.glsl");
    std::stringstream fragCode5;
    fragCode5 << fragShaderFile5.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile5("../shader/vertex.glsl");
    std::stringstream vertCode5;
    vertCode5 << vertShaderFile5.rdbuf();
    TreeProgram.init(vertCode5.str(), fragCode5.str(), "outColor");
    TreeProgram.bind();

    /* -------------------
    // BILLBOARD SHADER
    ------------------- */
    VertexArrayObject BillboardVAO;
    BillboardVAO.init();
    BillboardVAO.bind();
    // Initialize the VBO with the vertices data
    BufferObject BillboardVBO;
    BillboardVBO.init();
    // Initialize the VBO with the UV data
    BufferObject BillboardUVB;
    BillboardUVB.init();

    // Setup vertex fragment shaders for drawing the full screen quad/triangles
    std::vector <glm::vec3> BillboardVertices;
    std::vector <glm::vec3> BillboardUV;
    float grassVertices[] = {
        // positions    
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };
    float grassUV[] = {
        // positions    
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };
    int count = 0;
    for (int i = 0; i < 36; i++) {
        BillboardVertices.push_back(glm::vec3(grassVertices[count], grassVertices[count + 1], grassVertices[count + 2]));
        BillboardUV.push_back(glm::vec3(grassUV[count], grassUV[count + 1], grassUV[count + 2]));
        count += 3;
    }
    BillboardVBO.update(BillboardVertices);
    BillboardUVB.update(BillboardUV);
    
    // create a texture and upload the image data
    imageAvailable = loadPPM(image, "../data/billboard.ppm");
    GLuint grass_tex;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &grass_tex);
    glBindTexture(GL_TEXTURE_2D, grass_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // convert image rgb to data for glTexImage2D
    image_data = new float[image.h * image.w * 3];
    count_a = 0;
    count_b = 0;
    for (int i = 0; i < image.h; i++) {
        for (int j = 0; j < image.w; j++) {
            image_data[count_a + 0] = image.data.at(count_b).r / 255.0;
            image_data[count_a + 1] = image.data.at(count_b).g / 255.0;
            image_data[count_a + 2] = image.data.at(count_b).b / 255.0;
            count_a += 3; count_b++;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.w, image.h, 0, GL_RGB, GL_FLOAT, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Randomly populate tree based on terrain vertices
    std::vector<glm::vec3> BillboardVector_Positions;
    BillboardVector_Positions.push_back(glm::vec3(-1.0f, -5.0f, 0.0f));
    for (int i = 0; i < TerrainVector_Vertices.size(); i++) {
        float rand_populate = (float)rand() / (float)RAND_MAX;
        if (TerrainVector_Vertices.at(i).y > 0.4f && TerrainVector_Vertices.at(i).y < 4.0f) {
            if (rand_populate < 0.01) {
                BillboardVector_Positions.push_back(glm::vec3(TerrainVector_Vertices.at(i).x, TerrainVector_Vertices.at(i).y + 0.015, TerrainVector_Vertices.at(i).z));
            }
        }
    }
    std::cout << "Number of Billboards: " << BillboardVector_Positions.size() << std::endl;
    // randomly rotate the grass
    srand(1);
    std::vector<float> BillboardVector_Rotations;
    for (int i = 0; i < BillboardVector_Positions.size(); i++) {
        float rand_spin = (float)rand() / (float)RAND_MAX;
        BillboardVector_Rotations.push_back(rand_spin);
    }

    Program BillboardProgram;
    // load fragment shader file 
    std::ifstream fragShaderFile6("../shader/billboard-fragment.glsl");
    std::stringstream fragCode6;
    fragCode6 << fragShaderFile6.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile6("../shader/billboard-vertex.glsl");
    std::stringstream vertCode6;
    vertCode6 << vertShaderFile6.rdbuf();
    BillboardProgram.init(vertCode6.str(), fragCode6.str(), "outColor");
    BillboardProgram.bind();

    /* -------------------
    //  CONFIGURATION: Setup for loop
    ------------------- */
    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);
    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Save the current time
    auto t_start = std::chrono::high_resolution_clock::now();
    // camera setup
    cameraSetting = 1;
    cameraPos = glm::vec3(0.149178f, 3.0f, -11.6130f);
    cameraTarget = glm::vec3(1.12733f, 3.0f, -11.4051f);
    cameraDirection = glm::normalize(cameraPos - cameraTarget);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));


    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Get the size of the window
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* -------------------
        //  Skybox
        ------------------- */
        // matrix calculations
        viewMatrix = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos - cameraDirection, cameraUp)));
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);
        // Bind your VAO (not necessary if you have only one)
        SkyboxVAO.bind();
        // Bind your program GLuint tex;
        glDepthMask(GL_FALSE);
        SkyBoxProgram.bind();
        SkyBoxProgram.bindVertexAttribArray("position", SkyboxVBO);
        glUniformMatrix4fv(SkyBoxProgram.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(SkyBoxProgram.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));        
        // Color the different faces for testing
        glUniform3f(SkyBoxProgram.uniform("triangleColor"), 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glUniform3f(SkyBoxProgram.uniform("triangleColor"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 6, 6);
        glUniform3f(SkyBoxProgram.uniform("triangleColor"), 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 12, 6);
        glUniform3f(SkyBoxProgram.uniform("triangleColor"), 1.0f, 1.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 18, 6);
        glUniform3f(SkyBoxProgram.uniform("triangleColor"), 0.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 24, 6);
        glUniform3f(SkyBoxProgram.uniform("triangleColor"), 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 30, 6); 
        glDepthMask(GL_TRUE);
        

        /* -------------------
        //  Terrain
        ------------------- */
        // matrix calculations
        viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);
        // Bind your VAO (not necessary if you have only one)
        TerrainVAO.bind();
        // Bind your program GLuint tex;
        TerrainProgram.bind();
        // bind your element array
        TerrainProgram.bindVertexAttribArray("position", TerrainVBO);
        TerrainProgram.bindVertexAttribArray("normal", TerrainNBO);
        TerrainProgram.bindVertexAttribArray("texturecoord", TerrainUVB);
        TerrainProgram.bindVertexAttribArray("biomecoord", TerrainCBO);
        glUniform3f(TerrainProgram.uniform("camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(TerrainProgram.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(TerrainProgram.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(TerrainProgram.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        // Color for testing
        glUniform3f(TerrainProgram.uniform("triangleColor"), (0.0 / 255.0), (158.0 / 255.0), (96.0 / 255.0));
        // Different colors based on differnt biomes
        glUniform3f(TerrainProgram.uniform("WaterColor"), (60.0 / 255.0), (95.0 / 255.0), (190.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("Sand1Color"), (210.0 / 255.0), (215.0 / 255.0), (130.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("Sand2Color"), (194.0 / 255.0), (178.0 / 255.0), (128.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("Grass1Color"), (126.0 / 255.0), (200.0 / 255.0), (80.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("Grass2Color"), (55.0 / 255.0), (174.0 / 255.0), (15.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("Rock1Color"), (130.0 / 255.0), (137.0 / 255.0), (138.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("Rock2Color"), (122.0 / 255.0), (115.0 / 255.0), (114.0 / 255.0));
        glUniform3f(TerrainProgram.uniform("SnowColor"), (180.0 / 255.0), (180.0 / 255.0), (180.0 / 255.0));
        // direction towards the light
        glUniform3fv(TerrainProgram.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(cameraPos.x + 10.0f, cameraPos.y - 3.0f, cameraPos.z - 10.0f)));
        // x: ambient;
        glUniform3f(TerrainProgram.uniform("lightParams"), 0.1f, 50.0f, 0.0f);
        for (int i = 0; i < height * 3; i++) {
            glDrawArrays(GL_TRIANGLES, height * 3 * i, width * 3);
        }

        /* -------------------
         //  OFF File - Populate Trees
         ------------------- */
         // matrix calculations
        viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 OFF_ModelMatrix = glm::mat4(1.0f);
        // Bind your VAO (not necessary if you have only one)
        TreeVAO.bind();
        // Bind your program
        TreeProgram.bind();
        // bind your element array
        IndexBuffer.bind();
        TreeProgram.bindVertexAttribArray("position", VBO);
        TreeProgram.bindVertexAttribArray("normal", NBO);
        TreeProgram.bindVertexAttribArray("texturecoord", UVB);
        // Set the uniform values
        glUniform3f(TreeProgram.uniform("Leaf1Color"), (170.0/255.0), (219.0/255.0), (30.0 / 255.0));
        glUniform3f(TreeProgram.uniform("Leaf2Color"), (113.0 / 255.0), (169.0 / 255.0), (44.0 / 255.0));
        glUniform3f(TreeProgram.uniform("Trunk1Color"), (155.0 / 255.0), (148.0 / 255.0), (95.0 / 255.0));
        glUniform3f(TreeProgram.uniform("Trunk2Color"), (92.0 / 255.0), (88.0 / 255.0), (56.0 / 255.0));
        glUniform3f(TreeProgram.uniform("camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(TreeProgram.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(OFF_ModelMatrix));
        glUniformMatrix4fv(TreeProgram.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(TreeProgram.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        // direction towards the light
        glUniform3fv(TreeProgram.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(cameraPos.x + 10.0f, cameraPos.y - 3.0f, cameraPos.z - 10.0f)));
        // x: ambient;
        glUniform3f(TreeProgram.uniform("lightParams"), 0.1f, 50.0f, 0.0f);
        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        for (int i = 0; i < TreeVector_Positions.size(); i++) {
            glm::mat4 OFF_ModelMatrix = glm::mat4(1.0f);
            OFF_ModelMatrix = glm::translate(OFF_ModelMatrix, TreeVector_Positions.at(i));
            OFF_ModelMatrix *= glm::scale(glm::mat4(1.0f), glm::vec3(0.7f / maxVal));
            OFF_ModelMatrix *= glm::translate(glm::mat4(1.0f), -(min + tmpVec));
            glUniformMatrix4fv(TreeProgram.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(OFF_ModelMatrix));
            glUniform1f(TreeProgram.uniform("modelHeight"), TreeVector_Positions.at(i).y);
            glDrawElements(GL_TRIANGLES, T.size() * 3, GL_UNSIGNED_INT, 0);
        }
        
        /* -------------------
        //  Grass Billboards
        ------------------- */
        // matrix calculations
        viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);
        // Bind your VAO (not necessary if you have only one)
        BillboardVAO.bind();
        // Bind your program GLuint tex;
        BillboardProgram.bind();
        glBindTexture(GL_TEXTURE_2D, grass_tex);
        program.bindVertexAttribArray("position", BillboardVBO);
        BillboardProgram.bindVertexAttribArray("texturecoord", BillboardUVB);
        glUniformMatrix4fv(BillboardProgram.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(BillboardProgram.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        glUniform3f(BillboardProgram.uniform("grassColor"), (30.0 / 255.0), (215.0 / 255.0), (96.0 / 255.0));
        //glUniform3f(BillboardProgram.uniform("grassColor"), (217.0 / 255.0), (230.0 / 255.0), (80.0 / 255.0));
        // loop through all billboard positions
        for (int i = 0; i < BillboardVector_Positions.size(); i++) {
            // check if billboard x,z coordinate is within camera's range
            glm::vec3 BillboardRange = glm::vec3(25.0f, 25.0f, 25.0f);
            if (cameraPos.x - BillboardRange.x < BillboardVector_Positions.at(i).x && BillboardVector_Positions.at(i).x < cameraPos.x + BillboardRange.x) {
                if (cameraPos.y - BillboardRange.y < BillboardVector_Positions.at(i).y && BillboardVector_Positions.at(i).y < cameraPos.y + BillboardRange.y) {
                    if (cameraPos.z - BillboardRange.z < BillboardVector_Positions.at(i).z && BillboardVector_Positions.at(i).z < cameraPos.z + BillboardRange.z) {
                        // draw billboards after determinig model matrix
                        glm::mat4 Billboard_ModelMatrix = glm::mat4(1.0f);
                        Billboard_ModelMatrix = glm::translate(Billboard_ModelMatrix, BillboardVector_Positions.at(i));
                        Billboard_ModelMatrix *= glm::scale(glm::mat4(1.0f), glm::vec3(1.3f / maxVal));
                        Billboard_ModelMatrix *= glm::translate(glm::mat4(1.0f), -(min + tmpVec));
                        Billboard_ModelMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(-0.6f, -0.6f, -0.6f));
                        Billboard_ModelMatrix *= glm::rotate(glm::mat4(1.0f), (float)(M_PI / BillboardVector_Rotations.at(i)), glm::vec3(0.0f, 1.0f, 0.0f));
                        // create three billboards for each grass
                        for (int j = 0; j < 3; j++) {
                            Billboard_ModelMatrix *= glm::rotate(glm::mat4(1.0f), (float)(M_PI / 3), glm::vec3(0.0f, 1.0f, 0.0f));
                            glUniformMatrix4fv(BillboardProgram.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(Billboard_ModelMatrix));
                            glDrawArrays(GL_TRIANGLES, 0, 6);
                        }
                    }
                }
            }
        }

        /* -------------------
        //  Water
        ------------------- */
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // matrix calculations
        viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);
        // Bind your VAO (not necessary if you have only one)
        WaterVAO.bind();
        // Bind your program GLuint tex;
        WaterProgram.bind();
        // bind your element array
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, water_tex);
        WaterProgram.bindVertexAttribArray("position", WaterVBO);
        WaterProgram.bindVertexAttribArray("normal", WaterNBO);
        WaterProgram.bindVertexAttribArray("texturecoord", WaterUVB);
        glUniform3f(WaterProgram.uniform("camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(WaterProgram.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(WaterProgram.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(WaterProgram.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        // Color the different faces for testing
        glUniform3f(WaterProgram.uniform("triangleColor"), (128.0 / 255.0), (197.0 / 255.0), (222.0 / 255.0));
        // direction towards the light
        glUniform3fv(WaterProgram.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(cameraPos.x-(cameraDirection.x*40.0f), -3.0f, cameraPos.z)));
        // x: ambient;
        glUniform3f(WaterProgram.uniform("lightParams"), 0.1f, 50.0f, 0.0f);
        // pass in time
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        glUniform1f(WaterProgram.uniform("time"), time);
        for (int i = 0; i < height * 3; i++) {
            glDrawArrays(GL_TRIANGLES, height * 3 * i, width * 3);
        }
        // Swap front and back buffers
        glfwSwapBuffers(window);
        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    SkyBoxProgram.free();
    TerrainProgram.free();
    WaterProgram.free();
    TreeProgram.free();
    BillboardProgram.free();
    VAO.free();
    VBO.free();
    TerrainVAO.free();
    TerrainVBO.free();
    SkyboxVAO.free();
    SkyboxVBO.free();
    BillboardVAO.free();
    BillboardVBO.free();
    WaterVAO.free();
    WaterVBO.free();
    TreeVAO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}