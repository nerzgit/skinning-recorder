//
//  ModelA.hpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/08.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#ifndef ModelA_hpp
#define ModelA_hpp

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Skin;

class ModelA
{
private:
    GLFWwindow *window;
    
    Skin       *skin;
    
    GLuint     modelVAO;
    GLuint     modelProgramID;
    GLuint     modelMatrixID;
    GLuint     ViewMatrixID;
    GLuint     ModelMatrixID;
    glm::mat4  boneMatrix[12];
    GLuint     boneMatrixID;
    GLuint     Texture;
    GLuint     TextureID;
    glm::mat4  modelMVP;
    GLuint     modelVBO[4];
    GLuint     weightVBO[12];
    GLuint     modelCBO;
    GLuint     LightID;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    
    std::vector<std::vector<float>> indexed_weights;
    
public:
    void initOBJ(GLFWwindow* window, const char* obj, const char* wgt, const char* kfm, const char* vert, const char* frag);
    void playOBJ(const glm::mat4 &M, const glm::mat4 &BoneM);
    void getBoneMatrix(const int &bone, glm::mat4 &boneMatrix);
    
    void playstop();
    void stop();
    void changefps(int &fps);
    void changeframe(int &frame);
    void selectBone(int &sbone);
    void updateRotate(int &axis, float &rot, const int &rotate_mm);
    int  getfps();
    int  getframe();
    glm::vec3 copy();
    void paste(glm::vec3 &rot);
    void recorde(std::string &name);
};

#endif /* ModelA_hpp */
