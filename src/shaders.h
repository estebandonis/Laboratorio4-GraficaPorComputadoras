#pragma once
#include "color.h"
#include "uniform.h"
#include "fragment.h"
#include "./FastNoiseLite.h"
#include <cmath>
#include <random>

Vertex vertexShader(const Vertex& vertex, const Uniform& uniforms) {

  glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

  glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

  glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);

  glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
  transformedNormal = glm::normalize(transformedNormal);

  return Vertex{
    glm::vec3(screenVertex),
    transformedNormal,
    vertex.original
  };
};

float rand(glm:: vec3 co) {
    return glm::fract(sin(glm::dot(co, glm::vec3(12.9898, 78.233, 54.53))) * 43758.5453);
}
Fragment fragmentShader(Fragment& fragment) {
    Color color;

    glm:: vec3 groundColor = glm:: vec3(0.44f, 0.51f, 0.33f);
    glm:: vec3 oceanColor = glm::vec3(0.12f, 0.38f, 0.57f);

    glm:: vec2 uv = glm::clamp(glm::vec2(fragment.original.x, fragment.original.y), 0.0f, 1.0f);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    float ox = 200;
    float oy = 650;
    float zoom = 1000.0f;

    float noiseValue = noiseGenerator.GetNoise((uv.x + ox) * zoom, (uv.y + oy) * zoom);

    glm::vec3 tmpColor = (noiseValue < 0.5f) ? oceanColor : groundColor;

    color = Color (tmpColor.x, tmpColor.y, tmpColor.z);

    fragment .color = color * fragment. intensity;

    return fragment;
};