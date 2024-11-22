//
//  ModelA.cpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/08.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#include "ModelA.hpp"
#include "skinningP.hpp"

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


void ModelA::initOBJ(GLFWwindow* window,
                     const char* obj,
                     const char* wgt,
                     const char* kfm,
                     const char* vert,
                     const char* frag)
{
    
    this -> window = window;
    
    
    // Skin initialization
    skin = new Skin(window, wgt, kfm);
    
    
    glGenVertexArrays(1, &modelVAO);
    glBindVertexArray(modelVAO);
    modelProgramID = LoadShaders( vert, frag );
    modelMatrixID = glGetUniformLocation(modelProgramID, "MVP");
    ViewMatrixID  = glGetUniformLocation(modelProgramID, "V");
    ModelMatrixID = glGetUniformLocation(modelProgramID, "M");
    TextureID     = glGetUniformLocation(modelProgramID, "myTextureSampler");
    boneMatrixID = glGetUniformLocation(modelProgramID, "BoneMatrix");

    Texture = loadDDS("uvmap.dds");
    TextureID = glGetUniformLocation(modelProgramID, "myTextureSampler");
    
    loadOBJ(obj, vertices, uvs, normals);
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
    
    
    skin -> initWeight(obj, skin->bones, indexed_vertices, indexed_weights);
    
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
    
    for (int i=0; i<skin->bones.size(); i++) {
        glGenBuffers(1, &weightVBO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, weightVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, indexed_weights[i].size() * sizeof(float), &indexed_weights[i][0], GL_STATIC_DRAW);
    }
    
    glUseProgram(modelProgramID);
    LightID = glGetUniformLocation(modelProgramID, "LightPosition_worldspace");
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void ModelA::playOBJ(const glm::mat4 &M,
                     const glm::mat4 &BoneM)
{
    
    skin -> playSkin();                                  // Play skin
    glm::mat4 BoneMM = M * BoneM;
    for (int i=0; i<skin->bones.size(); i++) {
        skin->bones[i]->playBone(BoneMM);                 // Play skinning with bone
        boneMatrix[i] = skin->bones[i]->matLOCAL;        // Get skinning bone's matrices
    }
    
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    
    modelMVP = ProjectionMatrix * ViewMatrix * M * BoneM;
    
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
    
    // ボーン行列
    glUniformMatrix4fv(boneMatrixID, 12, GL_FALSE, &boneMatrix[0][0][0]);
    
    
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
    
    
    // ウェイト
    for (int i=0; i<skin->bones.size(); i++) {
        glEnableVertexAttribArray(i+4);
        glBindBuffer(GL_ARRAY_BUFFER, weightVBO[i]);
        glVertexAttribPointer( i+4, 1, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }
    
    
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
    for (int i=0; i<skin->bones.size(); i++) { glDisableVertexAttribArray(i+4); }
    
}

void ModelA::getBoneMatrix(const int &bone, glm::mat4 &boneMatrix)
{
    this -> skin -> getBoneMatrix(bone, boneMatrix);
}


void ModelA::playstop()
{
    this -> skin -> playstop();
}

void ModelA::stop()
{
    this -> skin -> stop();
}

void ModelA::changefps(int &fps)
{
    this -> skin -> changefps(fps);
}

void ModelA::changeframe(int &frame)
{
    this -> skin -> changeframe(frame);
}

void ModelA::selectBone(int &sbone)
{
    this -> skin -> selectBone(sbone);
}

void ModelA::updateRotate(int &axis, float &rot, const int &rotate_mm)
{
    this -> skin -> updateRotate(axis, rot, rotate_mm);
}

int ModelA::getfps()
{
    return this -> skin -> getfps();
}

int ModelA::getframe()
{
    return this -> skin -> getframe();
}

glm::vec3 ModelA::copy()
{
    return this -> skin -> copy();
}

void ModelA::paste(glm::vec3 &rot)
{
    this -> skin -> paste(rot);
}

void ModelA::recorde(std::string &name)
{
    this -> skin -> recorde(name);
}
