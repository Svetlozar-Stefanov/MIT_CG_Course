#ifndef STARTER0_UTIL_H
#define STARTER0_UTIL_H

#include <cstdint>

float deg2rad(float deg);
float rad2deg(float rad);

struct GLFWwindow;
// creates a window using GLFW and initializes an OpenGL 3.3+ context.
GLFWwindow* createOpenGLWindow(int width, int height, const char* title);

// returns 0 on error
// program must be freed with glDeleteProgram()
uint32_t compileProgram(const char* vertexshader, const char* fragmentshader);

static const char* c_vertexshader = R"RAWSTR(
#version 330
layout(location=0) in vec3 Position;
layout(location=1) in vec3 Normal;

out vec4 var_Color;
out vec3 var_Normal;
out vec3 var_FragPos;

uniform vec4 diffColor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 N;

// shaders can have #defines, too
#define PI_INV 0.318309886183791

void main () {

    gl_Position = P * V * M * vec4(Position, 1);
    var_FragPos = vec3(M * vec4(Position, 1.0));
    var_Normal = mat3(N) * Normal;
    var_Color = diffColor;
}
)RAWSTR";

static const char* c_fragmentshader = R"RAWSTR(
#version 330

uniform vec4 lighDiff;
uniform vec4 specColor;
uniform float shininess;

uniform vec3 lightPos;
uniform vec3 camPos;

in vec4 var_Color;
in vec3 var_Normal;
in vec3 var_FragPos;

layout(location=0) out vec4 out_Color;

void main () {
    //Ambient
    float ambientStrength = 0.3;
    
    //Difuse
    vec3 normalDir = normalize(var_Normal);
    vec3 lightDir = normalize(lightPos - var_FragPos);
    float diffuse = max(dot(normalDir, lightDir), 0.0);

    //Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(camPos - var_FragPos);
    vec3 reflectionDir = reflect(-lightDir, normalDir);
    float specular = pow(max(dot(viewDir, reflectionDir), 0.0), shininess);
    vec4 specularColor = lighDiff * specular * specularStrength;

    out_Color = var_Color * (ambientStrength + diffuse) + specularColor;
}
)RAWSTR";

#endif

//// transform vertex position and normal
//    // into world coordinates
//vec3 normal_world = (N * vec4(Normal, 1)).xyz;
//normal_world = normalize(normal_world);
//
//// Implement Blinn-Phong Shading Model
//// 1. Convert everything to world space
////    and normalize directions
//vec4 pos_world = M * vec4(Position, 1);
//pos_world /= pos_world.w;
//vec3 light_dir = (lightPos - pos_world).xyz;
//vec3 cam_dir = camPos - pos_world.xyz;
//float distsq = dot(light_dir, light_dir);
//light_dir = normalize(light_dir);
//cam_dir = normalize(cam_dir);
//
//// 2. Compute Diffuse Contribution
//float ndotl = max(dot(normal_world, light_dir), 0.0);
//vec3 diffContrib = PI_INV * lightDiff.xyz * diffColor.xyz
//                   * ndotl / distsq;
//
//// 3. Compute Specular Contribution
//vec3 halfVec = light_dir + cam_dir;
//vec3 R = reflect( -light_dir, normal_world );
//float eyedotr = max(dot(cam_dir, R), 0.0);
//vec3 specContrib = pow(eyedotr, shininess) *
//                   specColor.xyz * lightDiff.xyz / distsq;
//
//// 4. Add specular and diffuse contributions
//var_Color = vec4(diffColor.xyz * (ambientStrength + diffContrib + specContrib), diffColor.w);