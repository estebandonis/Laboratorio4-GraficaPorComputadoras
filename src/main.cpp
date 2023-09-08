#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <array>
#include <fstream>
#include <sstream>
#include <string>

#include "fragment.h"
#include "uniform.h"
#include "color.h"
#include "shaders.h"
#include "triangle.h"

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 500;

std::array<std::array<float, WINDOW_WIDTH>, WINDOW_HEIGHT> zbuffer;


SDL_Renderer* renderer;

// Declare a global clearColor of type Color
Color clearColor = {0, 0, 0}; // Initially set to black

// Declare a global currentColor of type Color
Color currentColor = {255, 255, 255}; // Initially set to white

// Function to clear the framebuffer with the clearColor
void clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);


    for (auto &row : zbuffer) {
        std::fill(row.begin(), row.end(), 99999.0f);
    }
}

// Function to set a specific pixel in the framebuffer to the currentColor
void point(Fragment f) {
    if (f.z < zbuffer[f.y][f.x]) {
        SDL_SetRenderDrawColor(renderer, f.color.r, f.color.g, f.color.b, f.color.a);
        SDL_RenderDrawPoint(renderer, f.x, f.y);
        zbuffer[f.y][f.x] = f.z;
    }
}

std::vector<std::vector<Vertex>> primitiveAssembly(const std::vector<Vertex>& transformedVertices) {
    std::vector<std::vector<Vertex>> groupedVertices;

    for (int i = 0; i < transformedVertices.size(); i += 3) {
        std::vector<Vertex> vertexGroup;
        vertexGroup.push_back(transformedVertices[i]);
        vertexGroup.push_back(transformedVertices[i+1]);
        vertexGroup.push_back(transformedVertices[i+2]);
        
        groupedVertices.push_back(vertexGroup);
    }

    return groupedVertices;
}


void render(std::vector<glm::vec3> VBO, const Uniform& uniforms) {
    // 1. Vertex Shader
    // vertex -> trasnformedVertices
    
    std::vector<Vertex> transformedVertices(VBO.size() / 2);

    for (int i = 0; i < VBO.size(); i+=2) {
        glm::vec3 v = VBO[i];
        glm::vec3 u = VBO[i+1];

        Vertex vertex = {v, u};
        transformedVertices.push_back(vertexShader(vertex, uniforms));
    }


    // 2. Primitive Assembly
    // transformedVertices -> triangles
    std::vector<std::vector<Vertex>> triangles = primitiveAssembly(transformedVertices);

    // 3. Rasterize
    // triangles -> Fragments
    std::vector<Fragment> fragments;
    for (const std::vector<Vertex>& triangleVertices : triangles) {
        std::vector<Fragment> rasterizedTriangle = triangle(
            triangleVertices[0],
            triangleVertices[1],
            triangleVertices[2]
        );
        
        fragments.insert(
            fragments.end(),
            rasterizedTriangle.begin(),
            rasterizedTriangle.end()
        );
    }

    // 4. Fragment Shader
    // Fragments -> colors

    for (Fragment fragment : fragments) {
        point(fragmentShader(fragment));
    }
}

void render1(std::vector<glm::vec3> VBO, const Uniform& uniforms) {
    // 1. Vertex Shader
    // vertex -> trasnformedVertices

    std::vector<Vertex> transformedVertices(VBO.size() / 2);

    for (int i = 0; i < VBO.size(); i+=2) {
        glm::vec3 v = VBO[i];
        glm::vec3 u = VBO[i+1];

        Vertex vertex = {v, u};
        transformedVertices.push_back(vertexShader(vertex, uniforms));
    }


    // 2. Primitive Assembly
    // transformedVertices -> triangles
    std::vector<std::vector<Vertex>> triangles = primitiveAssembly(transformedVertices);

    // 3. Rasterize
    // triangles -> Fragments
    std::vector<Fragment> fragments;
    for (const std::vector<Vertex>& triangleVertices : triangles) {
        std::vector<Fragment> rasterizedTriangle = triangle(
                triangleVertices[0],
                triangleVertices[1],
                triangleVertices[2]
        );

        fragments.insert(
                fragments.end(),
                rasterizedTriangle.begin(),
                rasterizedTriangle.end()
        );
    }

    // 4. Fragment Shader
    // Fragments -> colors

    for (Fragment fragment : fragments) {
        point(fragmentShader1(fragment));
    }
}


float a = 3.14f / 3.0f;
float b = 3.14f / 3.0f;
float orbitRadius = 1.5f;
float orbitSpeed = 0.1f;

glm::mat4 createModelMatrix() {
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.8f, 0.8f, 0.8f));
    a += 3;
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(a), glm::vec3(0.0f, 1.0f, 0.0f));

    return translation * scale * rotation;
}

glm::mat4 createModelMatrix1(float deltaTime) {
    float orbitalAngle = glm::radians(a) * deltaTime;
    float x = orbitRadius * glm::cos(orbitalAngle);
    float z = orbitRadius * glm::sin(orbitalAngle);

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
    b += 20;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(b), glm::vec3(0.0f, 1.0f, 0.0f));

    return translation * scale * rotation;
}

glm::mat4 createViewMatrix() {
    return glm::lookAt(
        // donde esta
        glm::vec3(0, 0, -5),
        // hacia adonde mira
        glm::vec3(0, 0, 0),
        // arriba
        glm::vec3(0, -20, 0)
    );
}

glm::mat4 createProjectionMatrix() {
  float fovInDegrees = 45.0f;
  float aspectRatio = 1000.0f / 800.0f;
  float nearClip = 0.1f;
  float farClip = 100.0f;

  return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);

    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}


struct Face {
    std::array<int, 3> vertexIndices;
    std::array<int, 3> normalIndices;
};

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec3>& out_textures, std::vector<glm::vec3>& out_normals, std::vector<Face>& out_faces)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cout << "Failed to open the file: " << path << std::endl;
        return false;
    }

    std::string line;
    std::istringstream iss;
    std::string lineHeader;
    glm::vec3 vertex;
    Face face;

    while (std::getline(file, line))
    {
        iss.clear();
        iss.str(line);
        iss >> lineHeader;

        if (lineHeader == "v")
        {
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_vertices.push_back(vertex);
        }
        else if (lineHeader == "vn")
        {
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_normals.push_back(vertex);
        }
        else if (lineHeader == "vt")
        {
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_textures.push_back(vertex);
        }
        else if (lineHeader == "f")
        {
            Face face;
            for (int i = 0; i < 3; ++i)
            {
                std::string faceData;
                iss >> faceData;
                
                std::replace(faceData.begin(), faceData.end(), '/', ' ');

                std::istringstream faceDataIss(faceData);
                int temp;
                faceDataIss >> face.vertexIndices[i] >> temp >> face.normalIndices[i];

                face.vertexIndices[i]--;
                face.normalIndices[i]--;
            }

            out_faces.push_back(face);
        }
    }

    return true;
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(".obj Renderer", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> textures;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;
    std::vector<glm::vec3> vertexBufferObject;

    if (loadOBJ("assets/sphere.obj", vertices, textures, normals, faces)) {
        // For each face
        for (const auto& face : faces)
        {
            for (int i = 0; i < 3; ++i)
            {
                glm::vec3 vertexPosition = vertices[face.vertexIndices[i]];
                glm::vec3 vertexNormal = normals[face.normalIndices[i]];

                vertexBufferObject.push_back(vertexPosition);
                vertexBufferObject.push_back(vertexNormal);
            }
        }
    }

    Uniform uniformsPlaneta;
    Uniform uniformsLuna;

    while (running) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        clear();

        uniformsPlaneta.model = createModelMatrix();
        uniformsPlaneta.view = createViewMatrix();
        uniformsPlaneta.projection = createProjectionMatrix();
        uniformsPlaneta.viewport = createViewportMatrix();

        render(vertexBufferObject, uniformsPlaneta);

        uniformsLuna.model = createModelMatrix1(1.0f);
        uniformsLuna.view = createViewMatrix();
        uniformsLuna.projection = createProjectionMatrix();
        uniformsLuna.viewport = createViewportMatrix();

//        uniformsLuna.model = glm::translate(uniformsLuna.model, glm::vec3(0.0f, 0.5f, 4.0f));

        // Renderizar la luna utilizando el fragment shader moonFragmentShader
        render1(vertexBufferObject, uniformsLuna);

        // Present the frame buffer to the screen
        SDL_RenderPresent(renderer);

        // Delay to limit the frame rate
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}