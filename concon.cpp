//
//  concon.cpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/08.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#include "concon.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common/text2D.hpp"

Concon::Concon(GLFWwindow *window, const char *texture)
{
    initOBJ(window, texture);
}

Concon::~Concon()
{
    deinitOBJ();
}

void Concon::initOBJ(
                         GLFWwindow *window,
                         const char *texture
                         )
{
    // DDSをロード
    initText2D(texture);
}

void Concon::playOBJ(std::vector<std::string> &message, const int &spaceX, const int &spaceY)
{
    
    glClear(GL_DEPTH_BUFFER_BIT);
    char text[256];
    for (int i=0; i<message.size(); i++) {
        sprintf(text,"%s", message[i].c_str());
        printText2D(text, spaceX, spaceY * i + 4, 11);
    }
    
}

void Concon::deinitOBJ()
{
    
    cleanupText2D();
    
}
