#version 150 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
                                                                         
uniform mat4 projectionMatrix;
in vec3 color[];
out vec2 Vertex_UV;
out vec3 geom_color;
                                                                         
const float radius = 0.5;
                                                                         
void main() {
  gl_Position = gl_in[0].gl_Position + vec4  (-1 * radius, -1 * radius, 0.0, 0.0);
  gl_Position = projectionMatrix * gl_Position;
  Vertex_UV = vec2(0.0, 0.0);
  geom_color = color[0];
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(-1 * radius,  1 * radius, 0.0, 0.0);
  gl_Position = projectionMatrix * gl_Position;
  Vertex_UV = vec2(0.0, 1.0);
  geom_color = color[0];
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius, -1 * radius, 0.0, 0.0);
  gl_Position = projectionMatrix * gl_Position;
  Vertex_UV = vec2(1.0, 0.0);
  geom_color = color[0];
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius,  1 * radius, 0.0, 0.0);
  gl_Position = projectionMatrix * gl_Position;"
  Vertex_UV = vec2(1.0, 1.0);"
  geom_color = color[0];
  EmitVertex();

  EndPrimitive();
}