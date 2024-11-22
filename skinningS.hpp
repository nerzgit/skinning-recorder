//
//  skinningS.hpp
//  AnimSningRecorder
//
//  Created by IDE KENTARO on 2019/04/17.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#ifndef skinningS_hpp
#define skinningS_hpp

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class AnimS
{
private:
    GLFWwindow *window;
    
    int        fps;
    int        frame;
    
    bool       caniplay;
    int        selectedBone;
    int        editedFrameCount;
    std::vector<int> editedFrame;  // [Frame]
    
public:
    glm::mat4  matANIMS;
    
    std::vector<std::vector<glm::vec3>> bones_keyframe; // Keyframes [r,t,s] vec3 per frame
    
    AnimS(GLFWwindow *window, const char *keyframefile);
    ~AnimS();
    void playAnimS();
    
    void playstop();
    void stop();
    void changefps(int &fps);
    void changeframe(int &frame);
    void selectBone(int &sbone);
    bool findFrame(int &tframe);
    void updateRotate(int &axis, float &rot, const int &rotate_mm);
    void updateTrans(int &axis, float &trans, const int &trans_mm);
    int  getfps();
    int  getframe();
    std::vector<glm::vec3> copy();
    void paste(std::vector<glm::vec3> &rot);
    void recorde(std::string &name);
    
};

#endif /* skinningS_hpp */
