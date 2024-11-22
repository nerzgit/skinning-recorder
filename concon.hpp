//
//  concon.hpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/08.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#ifndef concon_hpp
#define concon_hpp

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Concon
{
private:
    
public:
    
    Concon(GLFWwindow *window, const char *texture);
    ~Concon();
    
    void initOBJ(GLFWwindow *window, const char *texture);
    void playOBJ(std::vector<std::string> &message, const int &spaceX, const int &spaceY);
    void deinitOBJ();
    
};

#endif /* concon_hpp */
