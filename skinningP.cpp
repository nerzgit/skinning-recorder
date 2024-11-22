//
//  skinningP.cpp
//  Gme550
//
//  Created by IDE KENTARO on 2019/03/27.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//
//
//  * * *  ATTENTION  * * *
//  This skinning program is different from normal
//  skinning.hpp because it records the
//  rotation angle for each key frame.
//
//

#include "skinningP.hpp"

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

Bone::~Bone()
{
    delete boneCH;
    delete boneSL;
}

void Bone::initBone(const int       &id,
                    const glm::vec3 &T,
                    const glm::vec3 &R,
                    const glm::vec3 &S,
                    const glm::vec3 &tail)
{
    
    // Parameter initialization
    this -> id = id;
    this -> T  = T;
    this -> R  = R;
    this -> S  = S;
    
    glm::mat4 TranslationMatrix = translate(glm::mat4(1.0), T);
    glm::mat4 RotationMatrix    = eulerAngleYXZ(R.y, R.x, R.z);
    glm::mat4 ScalingMatrix     = scale(glm::mat4(1.0), S);
    matINIT = TranslationMatrix * RotationMatrix * ScalingMatrix;
    
    this -> matOFFSET = glm::inverse(matINIT);
    
    
    // Rendering initialization
    glGenVertexArrays(1, &boneVAO);
    glBindVertexArray(boneVAO);
    
    boneProgramID = LoadShaders( "Bone.vertexshader", "Bone.fragmentshader" );
    boneMatrixID  = glGetUniformLocation(boneProgramID, "MVP");
    
    GLfloat bone_vbo_data[6] = {
        0,0,0,tail.x,tail.y,tail.z
    };
    
    GLfloat bone_color_data[6] = {
        (float)  0/255, (float)255/255, (float)255/255,
        (float) 50/255, (float)205/255, (float) 50/255
    };
    
    glGenBuffers(1, &boneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, boneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bone_vbo_data), bone_vbo_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &boneCBO);
    glBindBuffer(GL_ARRAY_BUFFER, boneCBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bone_color_data), bone_color_data, GL_STATIC_DRAW);
    
    glEnable(GL_LINE_SMOOTH);
    
}

void Bone::playBone(const glm::mat4 &ModelMatrix)
{

    boneMVP = getProjectionMatrix() * getViewMatrix() * ModelMatrix * matBONE;
    
    glUseProgram(boneProgramID);
    glUniformMatrix4fv(boneMatrixID, 1, GL_FALSE, &boneMVP[0][0]);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, boneVBO);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, boneCBO);
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
    glDrawElements(
                   GL_LINES,
                   2,
                   GL_UNSIGNED_SHORT,
                   (void*)0
                   );
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    
}





Skin::Skin(GLFWwindow *window, const char *skinfile, const char *keyframefile)
{
    this -> window = window;
    
    //
    // Bone initialization
    //
    
    std::vector<int>                    op_i;    // arr_id
    std::vector<int>                    op_c;    // arr_child
    std::vector<int>                    op_s;    // arr_sibling
    std::vector<std::vector<float>>     op_w;    // arr_weight
    std::vector<glm::vec3>              op_h;    // arr_head
    std::vector<glm::vec3>              op_t;    // arr_tail
    
    FILE * file = fopen(skinfile, "r");
    while( 1 ){
        
        char      line[128];
        int       i = 0;           // id
        int       c;               // child
        int       s;               // sibling
        float     w;               // weight
        glm::vec3 h;               // head
        glm::vec3 t;               // tail
        
        int res = fscanf(file, "%s", line);
        
        // EOF
        if (res == EOF)
            break;
        
        // ID
        if ( strcmp( line, "i" ) == 0 ){
            fscanf(file, "%i\n", &i);
            op_i.push_back(i);
            op_w.push_back({});
            
            // CHILD
        }else if ( strcmp( line, "c" ) == 0 ){
            fscanf(file, "%i\n", &c);
            op_c.push_back(c);
            
            // SIBLING
        }else if ( strcmp( line, "s" ) == 0 ){
            fscanf(file, "%i\n", &s);
            op_s.push_back(s);
            
            // WEIGHT
        }else if ( strcmp( line, "w" ) == 0 ){
            fscanf(file, "%f\n", &w);
            op_w[op_i.back()].push_back(w);
            
            // HEAD
        }else if ( strcmp( line, "h" ) == 0 ){
            fscanf(file, "%f %f %f\n", &h.x, &h.z, &h.y);
            h.y *= -1;
            op_h.push_back(h);
            
            // TAIL
        }else if ( strcmp( line, "t" ) == 0 ){
            fscanf(file, "%f %f %f\n", &t.x, &t.z, &t.y);
            t.y *= -1;
            op_t.push_back(t);
            
        }
        
    }
    fclose(file);
    
    // Bone -> Add ID,WEIGHT,HEAD,TAIL
    for (int i=0; i<op_i.size(); i++) {
        Bone *bone = new Bone();
        for (int l=0; l<op_w[i].size(); l++) {
            bone -> arrWeight.push_back(op_w[i][l]);
        }
        bone -> initBone(op_i[i], op_h[i], glm::vec3(0), glm::vec3(1), op_t[i] - op_h[i]);
        bones.push_back(bone);
    }
    
    // Bone -> Add CHILD
    for (int i=0; i<op_c.size(); i++) {
        if (op_c[i] != -1) {
            bones[i]->boneCH = bones[op_c[i]];
        }
    }
    
    // Bone -> Add SIBLING
    for (int i=0; i<op_s.size(); i++) {
        if (op_s[i] != -1) {
            bones[i]->boneSL = bones[op_s[i]];
        }
    }
    
    // Convert initial pose to relative pose on parent space (Run rootbone)
    glm::mat4 iden = glm::mat4(1.0f);
    convertToRelativeMatrix(bones[0], iden);
    
    
    
    
    //
    // KEYFRAME Initialization
    //
    
    std::vector<int>                    kf_i;                  // Bones id

    for (int i=0; i<op_i.size(); i++) { bones_keyframe.push_back({}); }  // Initialization of "kf_m" ONE DIMENSTIONAL ARRAY

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

        // ID
        else if ( strcmp( line, "i" ) == 0 ){
            int i;
            fscanf(file2, "%i\n", &i);
            kf_i.push_back(i);
        }

        // KEYFRAME
        else if ( strcmp( line, "m" ) == 0 ){
            glm::vec3 m;
            fscanf(file2, "%f %f %f\n", &m.x, &m.z, &m.y);
            m.z *= -1;
            bones_keyframe[kf_i.back()].push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        }

    }
    fclose(file2);
    
    for (int i=0; i<op_i.size(); i++) {
        editedFrame.push_back({0,frame});
        editedFrameCount.push_back(0);
    }

}

Skin::~Skin()
{
    for (int i=0; i<bones.size(); i++) { delete bones[i]; }
}

void Skin::initWeight(const char                      * object,
                      std::vector<Bone*>              & bones,
                      std::vector<glm::vec3>          & in_vertices,
                      std::vector<std::vector<float>> & out_weights
                      ){
    
    std::vector<glm::vec3>          temp_vertices;
    std::vector<std::vector<float>> temp_weights;
    
    // Get vertices from obj file
    FILE * filevtx = fopen(object, "r");
    while( 1 ){
        char  line[128];
        int res = fscanf(filevtx, "%s", line);
        if (res == EOF)
            break;
        if ( strcmp( line, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(filevtx, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
    }
    fclose(filevtx);
    
    // Sort weights in the order of indexed_vertices.
    // vec3(x1,y1,z1)'sweight   is   W1
    // vec3(x2,y2,z2)'sweight   is   W2
    // vec3(x3,y3,z3)'sweight   is   W3
    // vec3(x4,y4,z4)'sweight   is   W4
    // ...
    for (int z=0; z<bones.size(); z++) {
        temp_weights.push_back({});
        for (int i=0; i<in_vertices.size(); i++) {
            for (int l=0; l<temp_vertices.size(); l++) {
                if (in_vertices[i].x == temp_vertices[l].x &&
                    in_vertices[i].y == temp_vertices[l].y &&
                    in_vertices[i].z == temp_vertices[l].z) {
                    temp_weights[z].push_back(bones[z]->arrWeight[l]);
                }
            }
        }
    }
    
    out_weights = temp_weights;
    
}

double   currentTime;
double   lastTime;
double   elapsedTime;
int      per = 0;
void Skin::playSkin()
{
    if (caniplay) {
        // Timer
        currentTime = glfwGetTime();
        elapsedTime = currentTime - lastTime;
        if ( elapsedTime >= 1.0/double(fps) ) {
            per ++;
            if (per > frame) {
                per = 0;
            }
            lastTime = glfwGetTime();
        }
    }
    
    // 5: Calculate matrix after change of each bone
    for (int i=0; i<bones.size(); i++) {

        // calculate each bone animation
        int   ekcp1 = this->editedFrame[i][editedFrameCount[i]+1];   // エンドキーフレーム
        int   ekc   = this->editedFrame[i][editedFrameCount[i]];     // スタートキーフレーム
        float per_f = float(per);
        
        glm::vec3 res_BoneVec;
        if (per == 0 || per == frame) {
            // perが0または終点フレームならばそのフレームの値をそのまま出力
            res_BoneVec = bones_keyframe[i][per];
        }else{
            // アニメーションを滑らかにするためのベジェ式
            glm::vec2 xy1, xy2, xy3, xy4;
            xy1 = glm::vec2( 0.0, 0.0 );
            xy2 = glm::vec2( 0.5, 0.0 );
            xy3 = glm::vec2( 0.5, 1.0 );
            xy4 = glm::vec2( 1.0, 1.0 );
            float f_t = float(per_f - float(ekc))/(float(ekcp1) - float(ekc));
            float f_y = (1-f_t)*(1-f_t)*(1-f_t)*xy1.y + 3*((1-f_t)*(1-f_t))*f_t*xy2.y + 3*((1-f_t))*(f_t*f_t)*xy3.y + (f_t*f_t*f_t)*xy4.y;
            // キーフレーム間ならば前後フレームの回転角とでアニメーションをする
            res_BoneVec = (float((per_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[i][ekcp1] - bones_keyframe[i][ekc]) + bones_keyframe[i][ekc]);
            
        }
        
        // play bone
        bones[i]->matBONE = bones[i]->matINIT * eulerAngleYXZ(res_BoneVec.y, res_BoneVec.x, res_BoneVec.z);
        
        if (per+1>ekcp1) {
            this->editedFrameCount[i] ++;    // キーフレーム間のアニメーションが終了したら次のキーフレーム間へと移行
        }
        
        if (per == 0 || per == frame) {
            this->editedFrameCount[i] = 0;   // キーフレームが0または終点フレームならば0に戻す
        }
        
    }
    
    // 6: Convert from parent space base to local space base
    glm::mat4 iden = glm::mat4(1.0f);
    updateBone(bones[0], iden);
    
}

void Skin::convertToRelativeMatrix(Bone *me, glm::mat4 &matParentOFFSET)
{
    me -> matINIT = matParentOFFSET * me -> matINIT;
    if (me -> boneCH) {
        convertToRelativeMatrix(me->boneCH, me->matOFFSET);
    }
    if (me -> boneSL) {
        convertToRelativeMatrix(me->boneSL, matParentOFFSET);
    }
}

void Skin::updateBone(Bone *me, glm::mat4 &matParentWORLD)
{
    me -> matBONE  = matParentWORLD * me -> matBONE;
    me -> matLOCAL = me -> matBONE  * me -> matOFFSET;
    if (me -> boneCH) {
        updateBone(me -> boneCH, me -> matBONE);
    }
    if (me -> boneSL) {
        updateBone(me -> boneSL, matParentWORLD);
    }
}

void Skin::getBoneMatrix(const int &bone, glm::mat4 &boneMatrix)
{
    boneMatrix = this->bones[bone]->matBONE;
}





void Skin::playstop()
{
    if (caniplay) {
        caniplay = false;
    }else{
        caniplay = true;
    }
    
}

void Skin::stop()
{
    caniplay = false;
}

void Skin::changefps(int &fps)
{
    this -> fps = fps;
    this -> stop();
}

void Skin::changeframe(int &frame)
{
    if (-1 < frame && frame < this -> frame + 1 ) {
        per = frame;
    }
    this -> stop();
}

void Skin::selectBone(int &sbone)
{
    this -> selectedBone = sbone;
}

bool Skin::findFrame(int &tframe)
{
    for (int i=0; i<editedFrame[this->selectedBone].size(); i++) {
        if (editedFrame[this->selectedBone][i] == tframe) {
            return true;
        }
    }
    return false;
}

void Skin::updateRotate(int &axis, float &rot, const int &rotate_mm)
{
    if (!findFrame(per)) {
        // すでに編集キーフレームが登録(配列内に存在)されていなければ登録
        // ** これがないと同じキーフレーム多重登録となりplay時に重複部分が点滅する **
        this->editedFrame[this->selectedBone].push_back(per);      // 現在のフレーム値を追加
        std::sort(this->editedFrame[this->selectedBone].begin(), this->editedFrame[this->selectedBone].end());   // フレーム値を昇順ソート
    }
    if (rotate_mm == 0) {
        this->bones_keyframe[this->selectedBone][per][axis] = rot * (3.1415926535f/180.0f);   // 回転角を更新
    }else{
        this->bones_keyframe[this->selectedBone][per][axis] += rot * (3.1415926535f/180.0f);   // 回転角を更新(加算)
    }
    
}

int Skin::getfps()
{
    return this->fps;
}

int Skin::getframe()
{
    return per;
}

glm::vec3 Skin::copy()
{
    return this->bones_keyframe[this->selectedBone][per];
}

void Skin::paste(glm::vec3 &rot)
{
    this->bones_keyframe[this->selectedBone][per] = rot;
}

void Skin::recorde(std::string &name)
{
    std::vector<std::vector<glm::vec3>> recorde_keyframes;
    for (int i=0; i<bones.size(); i++) {
        recorde_keyframes.push_back({});
    }
    
    for (int l=0; l<=frame; l++) {
        for (int i=0; i<bones.size(); i++) {
            
            // calculate each bone animation
            int   ekcp1 = this->editedFrame[i][editedFrameCount[i]+1];   // エンドキーフレーム
            int   ekc   = this->editedFrame[i][editedFrameCount[i]];     // スタートキーフレーム
            float per_f = float(l);
            
            glm::vec3 res_BoneVec;
            if (l == 0 || l == frame) {
                // perが0または終点フレームならばそのフレームの値をそのまま出力
                res_BoneVec = bones_keyframe[i][l];
            }else{
                // アニメーションを滑らかにするためのベジェ式
                glm::vec2 xy1, xy2, xy3, xy4;
                xy1 = glm::vec2( 0.0, 0.0 );
                xy2 = glm::vec2( 0.5, 0.0 );
                xy3 = glm::vec2( 0.5, 1.0 );
                xy4 = glm::vec2( 1.0, 1.0 );
                float f_t = float(per_f - float(ekc))/(float(ekcp1) - float(ekc));
                float f_y = (1-f_t)*(1-f_t)*(1-f_t)*xy1.y + 3*((1-f_t)*(1-f_t))*f_t*xy2.y + 3*((1-f_t))*(f_t*f_t)*xy3.y + (f_t*f_t*f_t)*xy4.y;
                // キーフレーム間ならば前後フレームの回転角とでアニメーションをする
                res_BoneVec = (float((per_f - float(ekc))*f_y/(float(ekcp1) - float(ekc))) * (bones_keyframe[i][ekcp1] - bones_keyframe[i][ekc]) + bones_keyframe[i][ekc]);
                
            }
            
            recorde_keyframes[i].push_back(res_BoneVec);
            
            if (l+1>ekcp1) {
                this->editedFrameCount[i] ++;    // キーフレーム間のアニメーションが終了したら次のキーフレーム間へと移行
            }
            
            if (l == 0 || l == frame) {
                this->editedFrameCount[i] = 0;   // キーフレームが0または終点フレームならば0に戻す
            }
            
        }
    }
    
    FILE * file = fopen(name.c_str(), "w");
    
    fprintf(file, "# EDI programmable game engine ANIMATION KEYFRAME file\n");
    fprintf(file, "# Formed by EDI SkinningRecorder\n");
    std::string filew_r = "r " + std::to_string(frame) + "\n";
    fprintf(file, "%s", filew_r.c_str());
    std::string filew_f = "f " + std::to_string(fps) + "\n";
    fprintf(file, "%s", filew_f.c_str());
    for (int i=0; i<bones.size(); i++) {
        std::string filew_i = "i " + std::to_string(bones[i]->id) + "\n";
        fprintf(file, "%s", filew_i.c_str());
        for (int l=0; l<=frame; l++) {
            std::string filew_m = "m " +
            std::to_string(recorde_keyframes[i][l][0]) +
            " " +
            std::to_string(recorde_keyframes[i][l][1]) +
            " " +
            std::to_string(recorde_keyframes[i][l][2]) +
            "\n";
            fprintf(file, "%s", filew_m.c_str());
        }
    }
    fclose(file);
    
}
