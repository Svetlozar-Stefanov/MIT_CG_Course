#include "gl.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vecmath.h>
#include "starter0_util.h"
#include "recorder.h"
#include "teapot.h"

using namespace std;

// Globals
const int COLORS_COUNT = 4;
const float COLOR_TRANSITION_TIME = 0.001f;
const float SCALE_WEIGHT = 0.5f;

uint32_t program;

// Input
struct XYCoord
{
    float x;
    float y;
};

bool firstMouseDown;
bool isDraging;
XYCoord pressMouse;

//Color
uint32_t currentColor = 0;
GLfloat diffuseColor[4] = { 0.5f, 0.5f, 0.9f, 1.0f };
float ambientStrength = 0.5;

//Transforms
float lightX = 2.0;
float lightY = 3.0;

XYCoord lastRot;
XYCoord rotation;
float rotationDegree;
float scale = 1;

// This is the list of points (3D vectors)
vector<Vector3f> vecv;

// This is the list of normals (also 3D vectors)
vector<Vector3f> vecn;

// This is the list of faces (indices into vecv and vecn)
vector<vector<unsigned>> vecf;

vector<string> parse(const string &str) 
{
    vector<string> output;
    string temp = "";
    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == '/')
        {
            output.push_back(temp);
            temp.clear();
            continue;
        }
        temp += str[i];
    }
    output.push_back(temp);

    return output;
}

void keyCallback(GLFWwindow* window, int key,
    int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) { // only handle PRESS and REPEAT
        return;
    }

    // Special keys (arrows, CTRL, ...) are documented
    // here: http://www.glfw.org/docs/latest/group__keys.html
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == 'A') {
        printf("Key A pressed\n");
    } 
    else if (key == 'C') {
        printf("Key C pressed\n");
        currentColor++;
        if (currentColor >= COLORS_COUNT)
        {
            currentColor = 0;
        }
    }
    else if (key == 'R') {
        printf("Key R pressed\n");
        rotationDegree+=0.05;
        if (rotationDegree >= 360)
        {
            rotationDegree = 0.0f;
        }
    }
    else if (key == 263) {
        printf("Key Left Arrow pressed\n");
        lightX -= 0.5;
    }
    else if (key == 262) {
        printf("Key Right Arrow pressed\n");
        lightX += 0.5;
    }
    else if (key == 265) {
        printf("Key Up Arrow pressed\n");
        lightY += 0.5;
    }
    else if (key == 264) {
        printf("Key Down Arrow pressed\n");
        lightY -= 0.5;
    }
    else {
        printf("Unhandled key press %d\n", key);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    scale += yoffset * SCALE_WEIGHT;
    if (scale < 1)
    {
        scale = 1;
    }
    printf("%f \n", scale);
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouseDown && isDraging)
    {
        firstMouseDown = false;
        pressMouse.x = xpos;
        pressMouse.y = ypos;
    }
    else if(isDraging)
    {
        rotation.x = xpos - pressMouse.x;
        rotation.y = ypos - pressMouse.y;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !isDraging)
    {
        firstMouseDown = true;
        isDraging = true;
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isDraging)
    {
        isDraging = false;
        firstMouseDown = false;
        lastRot.x += rotation.x;
        lastRot.y += rotation.y;
        rotation.x = 0;
        rotation.y = 0;
        pressMouse.x = 0;
        pressMouse.y = 0;
    }
}

//void drawTriangle()
//{
//    // set a reasonable upper limit for the buffer size
//    GeometryRecorder rec(1024);
//    rec.record(Vector3f(0.0, 0.0, 0.0), // Position
//        Vector3f(0.0, 0.0, 1.0));// Normal
//
//    rec.record(Vector3f(3.0, 0.0, 0.0),
//        Vector3f(0.0, 0.0, 1.0));
//
//    rec.record(Vector3f(3.0, 3.0, 0.0),
//        Vector3f(0.0, 0.0, 1.0));
//    rec.draw();
//}
//
//void drawTeapot()
//{
//    // set the required buffer size exactly.
//    GeometryRecorder rec(teapot_num_faces * 3);
//    for (int idx : teapot_indices) {
//        Vector3f position(teapot_positions[idx * 3 + 0],
//            teapot_positions[idx * 3 + 1],
//            teapot_positions[idx * 3 + 2]);
//
//        Vector3f normal(teapot_normals[idx * 3 + 0],
//            teapot_normals[idx * 3 + 1],
//            teapot_normals[idx * 3 + 2]);
//
//        rec.record(position, normal);
//    }
//    rec.draw();
//}

void drawObjMesh() {
    // draw obj mesh here
    // read vertices and face indices from vecv, vecn, vecf

    GeometryRecorder rec((vecf.size() / 2) * 3);
    for (size_t i = 0; i < vecf.size(); i += 2)
    {
        for (int j = 0; j < 3; j++)
        {
            unsigned i1 = vecf[i][j] - 1;
            unsigned i2 = vecf[i + 1][j] - 1;

            if (i1 < vecv.size() && i2 < vecv.size())
            {
                rec.record(vecv[i1], vecn[i2]);
            }
        }
    }

    rec.draw();
}

// This function is responsible for displaying the object.
void drawScene()
{
    drawObjMesh();
}

void setViewport(GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // make sure the viewport is square-shaped.
    if (width > height) {
        int offsetx = (width - height) / 2;
        glViewport(offsetx, 0, height, height);
    } else {
        int offsety = (height - width) / 2;
        glViewport(0, offsety, width, width);
    }
}

void updateCameraUniforms()
{
    // Set up a perspective view, with square aspect ratio
    float fovy_radians = deg2rad(50.0f);
    float nearz = 1.0f;
    float farz = 100.0f;
    float aspect = 1.0f;
    Matrix4f P = Matrix4f::perspectiveProjection(
        fovy_radians, aspect, nearz, farz);

    // See https://www.opengl.org/sdk/docs/man/html/glUniform.xhtml
    // for the many version of glUniformXYZ()
    // Returns -1 if uniform not found.
    int loc = glGetUniformLocation(program, "P");
    glUniformMatrix4fv(loc, 1, false, P);

    Vector3f eye(0.0, 0.0, 7.0f);
    Vector3f center(0.0, 0.0, 0.0);
    Vector3f up(0.0, 1.0f, -0.2f);
    Matrix4f V = Matrix4f::lookAt(eye, center, up);
    loc = glGetUniformLocation(program, "V");
    glUniformMatrix4fv(loc, 1, false, V);
    loc = glGetUniformLocation(program, "camPos");
    glUniform3fv(loc, 1, eye);

    // Make sure the model is centered in the viewport
    // We translate the model using the "Model" matrix
    Matrix4f M = Matrix4f::translation(0, -2.0, -4);
    M = M * M.rotateY((lastRot.x + rotation.x) * deg2rad(1.0f));
    M = M * M.rotateX((lastRot.y + rotation.y) * deg2rad(1.0f));
    M = M * Matrix4f::uniformScaling(scale);
    
    loc = glGetUniformLocation(program, "M");
    glUniformMatrix4fv(loc, 1, false, M);

    // Transformation matrices act differently
    // on vectors than on points.
    // The inverse-transpose is what we want.
    Matrix4f N = M.inverse().transposed();
    loc = glGetUniformLocation(program, "N");
    glUniformMatrix4fv(loc, 1, false, N);
}

void updateMaterialUniforms()
{
    int loc;

    // Here are some colors you might use - feel free to add more
    GLfloat diffColors[COLORS_COUNT][4] = { 
    { 0.5f, 0.5f, 0.9f, 1.0f },
    { 0.9f, 0.5f, 0.5f, 1.0f },
    { 0.5f, 0.9f, 0.3f, 1.0f },
    { 0.3f, 0.8f, 0.9f, 1.0f } };

    for (size_t i = 0; i < 4; i++)
    {
        if (diffuseColor[i] < diffColors[currentColor][i])
        {
            diffuseColor[i] += COLOR_TRANSITION_TIME;
        }
        else if (diffuseColor[i] > diffColors[currentColor][i])
        {
            diffuseColor[i] -= COLOR_TRANSITION_TIME;
        }
    }

    loc = glGetUniformLocation(program, "diffColor");
    glUniform4fv(loc, 1, diffuseColor);

    // Define specular color and shininess
    GLfloat specColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat shininess[] = { 256.0f };

    // Note that the specular color and shininess can stay constant
    loc = glGetUniformLocation(program, "specColor");
    glUniform4fv(loc, 1, specColor);
    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, shininess[0]);
}

void updateLightUniforms()
{
    // Light Position
    GLfloat lightPos[] = { lightX, lightY, 5.0f};
    int loc = glGetUniformLocation(program, "lightPos");
    glUniform3fv(loc, 1, lightPos);

    // Light Color
    GLfloat lightDiff[] = { 120.0, 120.0, 120.0, 1.0 };
    loc = glGetUniformLocation(program, "lightDiff");
    glUniform4fv(loc, 1, lightDiff);
}

void loadInput(const string filePath)
{
    // load the OBJ file here
    fstream file;
    file.open(filePath, fstream::in);
    if (!file.is_open())
    {
        printf("Failed to open file!");
        return;
    }

    while (!file.eof())
    {
        string identifier;
        file >> identifier;
        if (identifier == "v")
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;

            file >> x >> y >> z;

            vecv.push_back(Vector3f(x, y, z));
        }
        else if (identifier == "vn")
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;

            file >> x >> y >> z;

            vecn.push_back(Vector3f(x, y, z));
        }
        else if (identifier == "f")
        {
            string set1;
            string set2;
            string set3;

            file >> set1 >> set2 >> set3;

            vector<string> parsedSet1 = parse(set1);
            vector<string> parsedSet2 = parse(set2);
            vector<string> parsedSet3 = parse(set3);

            vector<unsigned> faces;
            faces.push_back(stoi(parsedSet1[0]));
            faces.push_back(stoi(parsedSet2[0]));
            faces.push_back(stoi(parsedSet3[0]));
            vecf.push_back(faces);
            
            faces.clear();
            faces.push_back(stoi(parsedSet1[2]));
            faces.push_back(stoi(parsedSet2[2]));
            faces.push_back(stoi(parsedSet3[2]));
            vecf.push_back(faces);
        }
    }

    file.close();
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char** argv)
{
    loadInput("../data/garg.obj");

    GLFWwindow* window = createOpenGLWindow(640, 480, "a0");
    
    // setup the keyboard event handler
    glfwSetKeyCallback(window, keyCallback);

    // setup mouse event handler
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // glEnable() and glDisable() control parts of OpenGL's
    // fixed-function pipeline, such as rasterization, or
    // depth-buffering. What happens if you remove the next line?
    glEnable(GL_DEPTH_TEST);

    // The program object controls the programmable parts
    // of OpenGL. All OpenGL programs define a vertex shader
    // and a fragment shader.
    program = compileProgram(c_vertexshader, c_fragmentshader);
    if (!program) {
        printf("Cannot compile program\n");
        return -1;
    }

    glUseProgram(program);

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the rendering window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setViewport(window);

        updateCameraUniforms();
        updateLightUniforms();
        updateMaterialUniforms();

        // Draw to back buffer
        drawScene();

        // Make back buffer visible
        glfwSwapBuffers(window);

        // Check if any input happened during the last frame
        glfwPollEvents();
    }

    // All OpenGL resource that are created with
    // glGen* or glCreate* must be freed.
    glDeleteProgram(program);

    glfwTerminate(); // destroy the window
    return 0;
}