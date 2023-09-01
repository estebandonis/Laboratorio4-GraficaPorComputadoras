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
    vertex.original
  };
};

float rand(glm::vec3 co) {
    return glm::fract(sin(glm::dot(co, glm::vec3(12.9898, 78.233, 54.53))) * 43758.5453);
}
Fragment fragmentShader(Fragment& fragment) {
    Color color;

    // Base color of the sun
    glm::vec3 tmpColor = glm::vec3(244.0f/255.0f, 140.0f/255.0f, 6.0f/255.0f) * glm::vec3(1.5f, 0.0f, 0.0f);

    // Introduce some pseudo-random noise into the equation for a more realistic look
    float noise = rand(fragment.original);

    // Affect the color intensity with the noise
    tmpColor += glm::vec3(noise, noise, noise) * 0.5f;

    // Add a slight gradient from the center to the edge to give a sense of depth
    float distanceFromCenter = glm::length(fragment.original);
    tmpColor *= 1.0f - distanceFromCenter;

    // Convert tmpColor to color
    color = Color(tmpColor.x, tmpColor.y, tmpColor.z);

    // Apply glow effect
    float glow = 5.0f + 5.0f * sin(3.0f * noise);
    fragment.color = color * fragment.intensity * glow;

    return fragment;
};