//
//  ModelS.hpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/17.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#ifndef ModelS_hpp
#define ModelS_hpp

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class AnimS;

class ModelS
{
private:
    GLFWwindow *window;
    
    AnimS      *anims;
    
    GLuint     modelVAO;
    GLuint     modelProgramID;
    GLuint     modelMatrixID;
    GLuint     ViewMatrixID;
    GLuint     ModelMatrixID;
    glm::mat4  animMatrix;
    GLuint     Texture;
    GLuint     TextureID;
    glm::mat4  modelMVP;
    GLuint     modelVBO[4];
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
    void initOBJ(GLFWwindow* window, const char* obj, const char* kfm, const char* vert, const char* frag);
    void playOBJ(const glm::mat4 &M, const glm::mat4 &BoneM);
    
    void playstop();
    void stop();
    void changefps(int &fps);
    void changeframe(int &frame);
    void selectBone(int &sbone);
    void updateRotate(int &axis, float &rot, const int &rotate_mm);
    void updateTrans(int &axis, float &trans, const int &trans_mm);
    int  getfps();
    int  getframe();
    std::vector<glm::vec3> copy();
    void paste(std::vector<glm::vec3> &rot);
    void recorde(std::string &name);
};

#endif /* ModelS_hpp */
