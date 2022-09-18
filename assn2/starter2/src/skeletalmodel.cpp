#include "skeletalmodel.h"
#include <cassert>

#include "starter2_util.h"
#include "vertexrecorder.h"

using namespace std;

namespace {
    vector<string> parse(string line, char sep) {
        vector<string> data;
        string curr;
        for (int i = 0; i < line.size(); i++)
        {
            if (line[i] == sep)
            {
                data.push_back(curr);
                curr.clear();
                continue;
            }

            curr.push_back(line[i]);
        }

        data.push_back(curr);

        return data;
    }
}

SkeletalModel::SkeletalModel() {
    program = compileProgram(c_vertexshader, c_fragmentshader_light);
    if (!program) {
        printf("Cannot compile program\n");
        assert(false);
    }
}

SkeletalModel::~SkeletalModel() {
    // destructor will release memory when SkeletalModel is deleted
    while (m_joints.size()) {
        delete m_joints.back();
        m_joints.pop_back();
    }

    glDeleteProgram(program);
}

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
    loadSkeleton(skeletonFile);

    m_mesh.load(meshFile);
    m_mesh.loadAttachments(attachmentsFile, (int)m_joints.size());

    computeBindWorldToJointTransforms();
    updateCurrentJointToWorldTransforms();
}

void SkeletalModel::draw(const Camera& camera, bool skeletonVisible)
{
    // draw() gets called whenever a redraw is required
    // (after an update() occurs, when the camera moves, the window is resized, etc)

    m_matrixStack.clear();

    glUseProgram(program);
    updateShadingUniforms();
    if (skeletonVisible)
    {
        drawJoints(camera);
        drawSkeleton(camera);
    }
    else
    {
        // Tell the mesh to draw itself.
        // Since we transform mesh vertices on the CPU,
        // There is no need to set a Model matrix as uniform
        camera.SetUniforms(program, Matrix4f::identity());
        m_mesh.draw();
    }
    glUseProgram(0);
}

void SkeletalModel::updateShadingUniforms() {
    // UPDATE MATERIAL UNIFORMS
    GLfloat diffColor[] = { 0.4f, 0.4f, 0.4f, 1 };
    GLfloat specColor[] = { 0.9f, 0.9f, 0.9f, 1 };
    GLfloat shininess[] = { 50.0f };
    int loc = glGetUniformLocation(program, "diffColor");
    glUniform4fv(loc, 1, diffColor);
    loc = glGetUniformLocation(program, "specColor");
    glUniform4fv(loc, 1, specColor);
    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, shininess[0]);

    // UPDATE LIGHT UNIFORMS
    GLfloat lightPos[] = { 3.0f, 3.0f, 5.0f, 1.0f };
    loc = glGetUniformLocation(program, "lightPos");
    glUniform4fv(loc, 1, lightPos);

    GLfloat lightDiff[] = { 120.0f, 120.0f, 120.0f, 1.0f };
    loc = glGetUniformLocation(program, "lightDiff");
    glUniform4fv(loc, 1, lightDiff);
}

void SkeletalModel::loadSkeleton(const char* filename)
{
    // Load the skeleton from file here.
    ifstream file(filename);
    if (!file.is_open())
    {   
        std::cout << "File not found: \n" << filename;
        return;
    }

    while (!file.eof())
    {
        char buffer[256];
        file.getline(buffer, 256);
        string line = buffer;

        vector<string> data = parse(line, ' ');

        if (data.size() != 4)
        {
            continue;
        }

        Vector3f transform;
        for (int i = 0; i < 3; i++)
        {
            transform[i] = stof(data[i]);
        }
        int index = stoi(data[3]);

        
        Joint * newJoint = new Joint();

        if (index == -1)
        {
            newJoint->transform = Matrix4f::translation(transform);
            newJoint->originalTransform = Matrix4f::translation(transform);
            m_rootJoint = newJoint;
            m_joints.push_back(newJoint);
            continue;
        }

        Joint* parent = m_joints[index];
        
        newJoint->transform = Matrix4f::translation(transform);
        newJoint->originalTransform = Matrix4f::translation(transform);
        parent->children.push_back(newJoint);
        m_joints.push_back(newJoint);
    }
}

void SkeletalModel::DrawJoint(const Joint* joint, const Camera& camera)
{
    m_matrixStack.push(joint->transform);
    camera.SetUniforms(program, m_matrixStack.top());
    drawSphere(0.025f, 12, 12);
    
    for (int i = 0; i < joint->children.size(); i++)
    {
        DrawJoint(joint->children[i], camera);
    }

    m_matrixStack.pop();
}

void SkeletalModel::drawJoints(const Camera& camera)
{
    DrawJoint(m_rootJoint, camera);
}

void SkeletalModel::DrawBones(const Joint* parent, const Camera& camera)
{
    m_matrixStack.push(parent->transform);
    for (int i = 0; i < parent->children.size(); i++)
    {
        Joint* child = parent->children[i];

        Vector3f loc = child->transform.getCol(3).xyz();
        Vector3f dir = loc.normalized();
        Matrix4f M = m_matrixStack.top();

        Vector3f cyl(0,1,0);

        float degree = acos(Vector3f::dot(cyl, dir));
        Vector3f rotDir = Vector3f::cross(cyl, dir).normalized();

        Matrix4f rotM = Matrix4f::rotation(rotDir, degree);

        M = M * rotM;

        camera.SetUniforms(program, M);
        float length = loc.abs();
        drawCylinder(6, 0.02f, length);

        DrawBones(child, camera);
    }

    m_matrixStack.pop();
}

void SkeletalModel::drawSkeleton(const Camera& camera)
{
    DrawBones(m_rootJoint, camera);
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
    // Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.

    Joint* joint = m_joints[jointIndex];

    Matrix4f rotX = Matrix4f::rotateX(rX);
    Matrix4f rotY = Matrix4f::rotateY(rY);
    Matrix4f rotZ = Matrix4f::rotateZ(rZ);

    joint->transform = joint->originalTransform * rotX * rotY * rotZ;
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
    // 2.3.1. Implement this method to compute a per-joint transform from
    // world-space to joint space in the BIND POSE.
    //
    // Note that this needs to be computed only once since there is only
    // a single bind pose.
    //
    // This method should update each joint's bindWorldToJointTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.

}

void SkeletalModel::updateCurrentJointToWorldTransforms()
{
    // 2.3.2. Implement this method to compute a per-joint transform from
    // joint space to world space in the CURRENT POSE.
    //
    // The current pose is defined by the rotations you've applied to the
    // joints and hence needs to be *updated* every time the joint angles change.
    //
    // This method should update each joint's currentJointToWorldTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.

}

void SkeletalModel::updateMesh()
{
    // 2.3.2. This is the core of SSD.
    // Implement this method to update the vertices of the mesh
    // given the current state of the skeleton.
    // You will need both the bind pose world --> joint transforms.
    // and the current joint --> world transforms.
}

