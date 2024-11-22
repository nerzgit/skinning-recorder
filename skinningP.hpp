//
//  skinningP.hpp
//  Gme550
//
//  Created by IDE KENTARO on 2019/03/27.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#ifndef skinningP_hpp
#define skinningP_hpp

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Bone
{
private:
    GLuint     boneVAO;
    GLuint     boneProgramID;
    GLuint     boneMatrixID;
    glm::mat4  boneMVP;
    GLuint     boneVBO;
    GLuint     boneCBO;
    
public:
    int                id;
    Bone               *boneCH = 0;
    Bone               *boneSL = 0;
    glm::vec3          T,R,S;
    glm::mat4          matOFFSET;
    glm::mat4          matINIT;
    glm::mat4          matBONE;
    glm::mat4          matLOCAL;
    std::vector<float> arrWeight;
    
    ~Bone();
    void initBone(const int &id, const glm::vec3 &T, const glm::vec3 &R, const glm::vec3 &S, const glm::vec3 &tail);
    void playBone(const glm::mat4 &ModelMatrix);
};

class Skin
{
private:
    GLFWwindow *window;
    int        fps;
    int        frame;
    
    bool       caniplay;
    int        selectedBone;
    std::vector<int> editedFrameCount;
    std::vector<std::vector<int>> editedFrame;  // [Bone][Frame]
    
public:
    std::vector<Bone*> bones;
    std::vector<std::vector<glm::vec3>> bones_keyframe; // Keyframes rotate vec3 per frame
    
    Skin(GLFWwindow *window, const char *skinfile, const char *keyframefile);
    ~Skin();
    void convertToRelativeMatrix(Bone *me, glm::mat4 &matParentOFFSET);
    void updateBone(Bone *me, glm::mat4 &matParentWORLD);
    void playSkin();
    void initWeight(const char                      * object,
                    std::vector<Bone*>              & bones,
                    std::vector<glm::vec3>          & in_vertices,
                    std::vector<std::vector<float>> & out_weights);
    void getBoneMatrix(const int &bone, glm::mat4 &boneMatrix);
    
    void playstop();
    void stop();
    void changefps(int &fps);
    void changeframe(int &frame);
    void selectBone(int &sbone);
    bool findFrame(int &tframe);
    void updateRotate(int &axis, float &rot, const int &rotate_mm);
    int  getfps();
    int  getframe();
    glm::vec3 copy();
    void paste(glm::vec3 &rot);
    void recorde(std::string &name);
    
};


#endif /* skinningP_hpp */
