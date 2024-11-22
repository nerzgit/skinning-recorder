//
//  ModelS.cpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/17.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#include "ModelS.hpp"
#include "skinningS.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/objloader.hpp"
#include "common/controls.hpp"
#include "common/vboindexer.hpp"
#include "common/quaternion_utils.hpp"


using namespace glm;


void ModelS::initOBJ(GLFWwindow* window,
                     const char* obj,
                     const char* kfm,
                     const char* vert,
                     const char* frag)
{
    
    this -> window = window;
    
    
    // Skin initialization
    anims = new AnimS(window, kfm);
    
    
    glGenVertexArrays(1, &modelVAO);
    glBindVertexArray(modelVAO);
    modelProgramID = LoadShaders( vert, frag );
    modelMatrixID = glGetUniformLocation(modelProgramID, "MVP");
    ViewMatrixID  = glGetUniformLocation(modelProgramID, "V");
    ModelMatrixID = glGetUniformLocation(modelProgramID, "M");
    TextureID     = glGetUniformLocation(modelProgramID, "myTextureSampler");
    
    Texture = loadDDS("uvmap.dds");
    TextureID = glGetUniformLocation(modelProgramID, "myTextureSampler");
    
    loadOBJ(obj, vertices, uvs, normals);
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    
    
    glGenBuffers(1, &modelVBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &modelVBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
    glGenBuffers(1, &modelVBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
    glGenBuffers(1, &modelVBO[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelVBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    
    glUseProgram(modelProgramID);
    LightID = glGetUniformLocation(modelProgramID, "LightPosition_worldspace");
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void ModelS::playOBJ(const glm::mat4 &M,
                     const glm::mat4 &BoneM)
{
    
    anims -> playAnimS();                    // Play anim
    animMatrix = anims->matANIMS;            // Get skinning bone's matrices
    
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    
    modelMVP = ProjectionMatrix * ViewMatrix * M * BoneM * animMatrix;
    
    glUseProgram(modelProgramID);
    // MVP行列
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
    
    glm::vec3 lightPos = glm::vec3(0,6,4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    
    glUniform1i(TextureID, 0);
    
    // 頂点
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[0]);
    glVertexAttribPointer(
                          0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)0
                          );
    
    // UV
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[1]);
    glVertexAttribPointer(
                          1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)0
                          );
    
    // 法線
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO[2]);
    glVertexAttribPointer(
                          2,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)0
                          );
    
    // VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelVBO[3]);
    
    
    glDrawElements(
                   GL_TRIANGLES,
                   (int)indices.size(),
                   GL_UNSIGNED_SHORT,
                   (void*)0
                   );
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    
}

void ModelS::playstop()
{
    this -> anims -> playstop();
}

void ModelS::stop()
{
    this -> anims -> stop();
}

void ModelS::changefps(int &fps)
{
    this -> anims -> changefps(fps);
}

void ModelS::changeframe(int &frame)
{
    this -> anims -> changeframe(frame);
}

void ModelS::selectBone(int &sbone)
{
    this -> anims -> selectBone(sbone);
}

void ModelS::updateRotate(int &axis, float &rot, const int &rotate_mm)
{
    this -> anims -> updateRotate(axis, rot, rotate_mm);
}

void ModelS::updateTrans(int &axis, float &trans, const int &trans_mm)
{
    this -> anims -> updateTrans(axis, trans, trans_mm);
}

int ModelS::getfps()
{
    return this -> anims -> getfps();
}

int ModelS::getframe()
{
    return this -> anims -> getframe();
}

std::vector<glm::vec3> ModelS::copy()
{
    return this -> anims -> copy();
}

void ModelS::paste(std::vector<glm::vec3> &rot)
{
    this -> anims -> paste(rot);
}

void ModelS::recorde(std::string &name)
{
    this -> anims -> recorde(name);
}
