in vec3 vertex_position;
in vec3 vertex_color;
  
varying out vec3 color;

void main() {
  color = vertex_color;
  gl_Position = vec4(vertex_position, 1.0);  
}