#pragma once
#include "color.h"
#include "uniform.h"
#include "fragment.h"
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
    vertex.position
  };
};

Fragment fragmentShader(Fragment& fragment) {
    Color color;

    glm::vec3 baseColor = glm::vec3(232, 174, 104);

    float stripePattern = glm::abs(glm::cos(fragment.original.y * 15.0f)) * 70.0f;

    glm::vec3 tmpColor = baseColor + stripePattern;

    color = Color(static_cast<int>(tmpColor.x), static_cast<int>(tmpColor.y), static_cast<int>(tmpColor.z));

    fragment. color = color * fragment.intensity;

    return fragment;
};
