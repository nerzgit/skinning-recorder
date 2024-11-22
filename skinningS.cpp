//
//  skinningS.cpp
//  AnimSningRecorder
//
//  Created by IDE KENTARO on 2019/04/17.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//
//
//  * * *  ATTENTION  * * *
//  This skinning program can recorde only simple RTS
//  Strictly, this is not skinning, just animate
//  RTS is Rotate Trans Scale.
//
//

#include "skinningS.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

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

AnimS::AnimS(GLFWwindow *window, const char *keyframefile)
{
    this -> window = window;
 
    //
    // KEYFRAME Initialization
    //

    FILE * file2 = fopen(keyframefile, "r");
    while( 1 ){
        
        char      line[128];
        int res = fscanf(file2, "%s", line);
        
        // EOF
        if (res == EOF)
            break;
        
        // FRAME
        if ( strcmp( line, "r" ) == 0 ){
            fscanf(file2, "%i\n", &frame);
        }
        
        // FPS
        if ( strcmp( line, "f" ) == 0 ){
            fscanf(file2, "%i\n", &fps);
        }

        // KEYFRAME
        else if ( strcmp( line, "m" ) == 0 ){
            bones_keyframe.push_back({
                glm::vec3(0.0f, 0.0f, 0.0f),      // R
                glm::vec3(0.0f, 0.0f, 0.0f),      // T
                glm::vec3(1.0f, 1.0f, 1.0f)       // S
            });
        }
        
    }
    fclose(file2);
    
    editedFrame.push_back(0);
    editedFrame.push_back(frame);
    editedFrameCount = 0;
    
}

AnimS::~AnimS()
{
}

double   currentTimeA;
double   lastTimeA;
double   elapsedTimeA;
int      perA = 0;
void AnimS::playAnimS()
{
    if (caniplay) {
        // Timer
        currentTimeA = glfwGetTime();
        elapsedTimeA = currentTimeA - lastTimeA;
        if ( elapsedTimeA >= 1.0/double(fps) ) {
            perA ++;
            if (perA > frame) {
                perA = 0;
            }
            lastTimeA = glfwGetTime();
        }
    }
    
    // calculate each bone animation
    int   ekcp1 = this->editedFrame[editedFrameCount+1];   // エンドキーフレーム
    int   ekc   = this->editedFrame[editedFrameCount];     // スタートキーフレーム
    float perA_f = float(perA);
    
    glm::vec3 res_r, res_t, res_s;
    if (perA == 0 || perA == frame) {
        // perAが0または終点フレームならばそのフレームの値をそのまま出力
        res_r = bones_keyframe[perA][0];
        res_t = bones_keyframe[perA][1];
        res_s = bones_keyframe[perA][2];
    }else{
        // アニメーションを滑らかにするためのベジェ式
        glm::vec2 xy1, xy2, xy3, xy4;
        xy1 = glm::vec2( 0.0, 0.0 );
        xy2 = glm::vec2( 0.5, 0.0 );
        xy3 = glm::vec2( 0.5, 1.0 );
        xy4 = glm::vec2( 1.0, 1.0 );
        float f_t = float(perA_f - float(ekc))/(float(ekcp1) - float(ekc));
        float f_y = (1-f_t)*(1-f_t)*(1-f_t)*xy1.y + 3*((1-f_t)*(1-f_t))*f_t*xy2.y + 3*((1-f_t))*(f_t*f_t)*xy3.y + (f_t*f_t*f_t)*xy4.y;
        // キーフレーム間ならば前後フレームの回転角とでアニメーションをする
        res_r = (float((perA_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[0][ekcp1] - bones_keyframe[0][ekc]) + bones_keyframe[0][ekc]);
        res_t = (float((perA_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[1][ekcp1] - bones_keyframe[1][ekc]) + bones_keyframe[1][ekc]);
        res_s = (float((perA_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[2][ekcp1] - bones_keyframe[2][ekc]) + bones_keyframe[2][ekc]);
        
    }
    
    // play bone
    glm::mat4 RotateMatrix = eulerAngleYXZ(res_r.y, res_r.x, res_r.z);
    glm::mat4 TransitionMatrix = translate(glm::mat4(1.0f), res_t);
    glm::mat4 ScaleMatrix = scale(glm::mat4(1.0f), res_s);
    matANIMS = TransitionMatrix * RotateMatrix * ScaleMatrix;
    
    if (perA+1>ekcp1) {
        this->editedFrameCount ++;    // キーフレーム間のアニメーションが終了したら次のキーフレーム間へと移行
    }
    
    if (perA == 0 || perA == frame) {
        this->editedFrameCount = 0;   // キーフレームが0または終点フレームならば0に戻す
    }
    
}




void AnimS::playstop()
{
    if (caniplay) {
        caniplay = false;
    }else{
        caniplay = true;
    }
    
}

void AnimS::stop()
{
    caniplay = false;
}

void AnimS::changefps(int &fps)
{
    this -> fps = fps;
    this -> stop();
}

void AnimS::changeframe(int &frame)
{
    if (-1 < frame && frame < this -> frame + 1 ) {
        perA = frame;
    }
    this -> stop();
}

void AnimS::selectBone(int &sbone)
{
    this -> selectedBone = sbone;
}

bool AnimS::findFrame(int &tframe)
{
    for (int i=0; i<editedFrame.size(); i++) {
        if (editedFrame[i] == tframe) {
            return true;
        }
    }
    return false;
}

void AnimS::updateRotate(int &axis, float &rot, const int &rotate_mm)
{
    if (!findFrame(perA)) {
        // すでに編集キーフレームが登録(配列内に存在)されていなければ登録
        // ** これがないと同じキーフレーム多重登録となりplay時に重複部分が点滅する **
        this->editedFrame.push_back(perA);      // 現在のフレーム値を追加
        std::sort(this->editedFrame.begin(), this->editedFrame.end());   // フレーム値を昇順ソート
    }
    if (rotate_mm == 0) {
        this->bones_keyframe[perA][0][axis] = rot * (3.1415926535f/180.0f);   // 回転角を更新
    }else{
        this->bones_keyframe[perA][0][axis] += rot * (3.1415926535f/180.0f);   // 回転角を更新(加算)
    }
    
}

void AnimS::updateTrans(int &axis, float &trans, const int &trans_mm)
{
    if (!findFrame(perA)) {
        // すでに編集キーフレームが登録(配列内に存在)されていなければ登録
        // ** これがないと同じキーフレーム多重登録となりplay時に重複部分が点滅する **
        this->editedFrame.push_back(perA);      // 現在のフレーム値を追加
        std::sort(this->editedFrame.begin(), this->editedFrame.end());   // フレーム値を昇順ソート
    }
    if (trans_mm == 0) {
        this->bones_keyframe[perA][1][axis] = trans * (3.1415926535f/180.0f);   // 更新
    }else{
        this->bones_keyframe[perA][1][axis] += trans * (3.1415926535f/180.0f);   // 更新(加算)
    }
    
}

int AnimS::getfps()
{
    return this->fps;
}

int AnimS::getframe()
{
    return perA;
}

std::vector<glm::vec3> AnimS::copy()
{
    return this->bones_keyframe[perA];
}

void AnimS::paste(std::vector<glm::vec3> &rot)
{
    this->bones_keyframe[perA] = rot;
}

void AnimS::recorde(std::string &name)
{
    std::vector<std::vector<glm::vec3>> recorde_keyframes;
    
    for (int l=0; l<=frame; l++) {
        // calculate each bone animation
        int   ekcp1 = this->editedFrame[editedFrameCount+1];   // エンドキーフレーム
        int   ekc   = this->editedFrame[editedFrameCount];     // スタートキーフレーム
        float perA_f = float(l);
        
        glm::vec3 res_r, res_t, res_s;
        if (l == 0 || l == frame) {
            // perAが0または終点フレームならばそのフレームの値をそのまま出力
            res_r = bones_keyframe[l][0];
            res_t = bones_keyframe[l][0];
            res_s = bones_keyframe[l][0];
        }else{
            // アニメーションを滑らかにするためのベジェ式
            glm::vec2 xy1, xy2, xy3, xy4;
            xy1 = glm::vec2( 0.0, 0.0 );
            xy2 = glm::vec2( 0.5, 0.0 );
            xy3 = glm::vec2( 0.5, 1.0 );
            xy4 = glm::vec2( 1.0, 1.0 );
            float f_t = float(perA_f - float(ekc))/(float(ekcp1) - float(ekc));
            float f_y = (1-f_t)*(1-f_t)*(1-f_t)*xy1.y + 3*((1-f_t)*(1-f_t))*f_t*xy2.y + 3*((1-f_t))*(f_t*f_t)*xy3.y + (f_t*f_t*f_t)*xy4.y;
            // キーフレーム間ならば前後フレームの回転角とでアニメーションをする
            res_r = (float((perA_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[0][ekcp1] - bones_keyframe[0][ekc]) + bones_keyframe[0][ekc]);
            res_t = (float((perA_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[1][ekcp1] - bones_keyframe[1][ekc]) + bones_keyframe[1][ekc]);
            res_s = (float((perA_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[2][ekcp1] - bones_keyframe[2][ekc]) + bones_keyframe[2][ekc]);
            
        }
        
        // play bone
        recorde_keyframes.push_back({});
        recorde_keyframes[l].push_back(res_r);
        recorde_keyframes[l].push_back(res_t);
        recorde_keyframes[l].push_back(res_s);
        
        if (l+1>ekcp1) {
            this->editedFrameCount ++;    // キーフレーム間のアニメーションが終了したら次のキーフレーム間へと移行
        }
        
        if (l == 0 || l == frame) {
            this->editedFrameCount = 0;   // キーフレームが0または終点フレームならば0に戻す
        }
        
    }
    
    
    
    FILE * file = fopen(name.c_str(), "w");
    
    fprintf(file, "# EDI programmable game engine ANIMATION KEYFRAME file\n");
    fprintf(file, "# Formed by EDI SkiningRecorder\n");
    std::string filew_r = "r " + std::to_string(frame) + "\n";
    fprintf(file, "%s", filew_r.c_str());
    std::string filew_f = "f " + std::to_string(fps) + "\n";
    fprintf(file, "%s", filew_f.c_str());
    for (int l=0; l<=frame; l++) {
        std::string filew_m = "m " +
        std::to_string(recorde_keyframes[l][0][0]) +
        " " +
        std::to_string(recorde_keyframes[l][0][1]) +
        " " +
        std::to_string(recorde_keyframes[l][0][2]) +
        " " +
        std::to_string(recorde_keyframes[l][1][0]) +
        " " +
        std::to_string(recorde_keyframes[l][1][1]) +
        " " +
        std::to_string(recorde_keyframes[l][1][2]) +
        " " +
        std::to_string(recorde_keyframes[l][2][0]) +
        " " +
        std::to_string(recorde_keyframes[l][2][1]) +
        " " +
        std::to_string(recorde_keyframes[l][2][2]) +
        "\n";
        fprintf(file, "%s", filew_m.c_str());
    }
    fclose(file);
    
}
