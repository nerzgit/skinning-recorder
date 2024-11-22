//
//  main.cpp
//  SkinningRecorder
//
//  Created by IDE KENTARO on 2019/04/08.
//  Copyright © 2019 IdeKentaro. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

//#include <SDL/SDL_mixer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "common/objloader.hpp"
#include "common/vboindexer.hpp"

#include "concon.hpp"
#include "ModelA.hpp"
#include "ModelS.hpp"


void initOpenGL();
//void recControl();
//void select_AS();
void insert_char_key_module(GLFWwindow* window, unsigned int charinfo);
void insert_key_module(GLFWwindow* window, int key, int scancode, int action, int mods);
void skinning_control();


std::vector<std::string> conconTxt = {
    " ",
    "f fps",
    "g frame",
    "r recorde",
    "p play",
    "x xrotate",
    "y yrotate",
    "z zrotate",
    "v armature",
    "b bone",
    "n copy",
    "m paste",
    "c camera"
};

std::vector<std::string> conconCos = {
    " "
};

std::vector<std::string> conconInt = {
    " ",
    "",
    "",
    "",
    ""
};

std::vector<ModelA*> modelAArr;
std::vector<ModelS*> modelSArr;


bool key_space,
     key_f,
     key_g,
     key_r,
     key_p,
     key_v,
     key_b,
     key_x,
     key_y,
     key_z,
     key_n,
     key_m,
     key_c = false;


int main( void )
{
    
    initOpenGL();
    
    Concon *conconcon = new Concon(window, "green_display.DDS");
    Concon *conconcos = new Concon(window, "green_display.DDS");
    Concon *conconint = new Concon(window, "green_display.DDS");

    ModelA *modelB = new ModelA();
    modelB -> initOBJ(window,
                      "richtofen_arm_r.obj",
                      "richtofen_arm_r.wgt",
                      "richtofen_arm_r.kfm",
                      "Model.vertexshader",
                      "Model.fragmentshader");
    
    ModelA *modelA = new ModelA();
    modelA -> initOBJ(window,
                      "richtofen_arm_l.obj",
                      "richtofen_arm_l.wgt",
                      "richtofen_arm_l.kfm",
                      "Model.vertexshader",
                      "Model.fragmentshader");
    
    ModelS *modelS = new ModelS();
    modelS -> initOBJ(window,
                      "waltherp38.obj",
                      "waltherp38.kfm",
                      "ModelS.vertexshader",
                      "ModelS.fragmentshader");
    
    modelAArr.push_back(modelB);
    modelAArr.push_back(modelA);
    modelSArr.push_back(modelS);
    
    glm::mat4 fdfsf;
    
    do{
        computeMatricesFromInputs();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, 1024*2, 768*2);
        
        glfwSetCharCallback(window, insert_char_key_module);
        glfwSetKeyCallback(window, insert_key_module);
        skinning_control();

        
        glm::mat4 modelA_TranslationMatrix   = translate(glm::mat4(1.0), glm::vec3(0.0f,1.3f,0.0f));
        glm::mat4 modelA_RotationMatrix      = eulerAngleYXZ(0.0f,0.0f,0.0f);
        glm::mat4 modelA_ScalingMatrix       = scale(glm::mat4(1.0), glm::vec3(1.0f,1.0f,1.0f));
        glm::mat4 modelA_ModelMatrix = modelA_TranslationMatrix * modelA_RotationMatrix * modelA_ScalingMatrix;
        glm::mat4 modelA_BoneMatrix = glm::mat4(1.0f);
        modelA -> playOBJ(modelA_ModelMatrix, modelA_BoneMatrix);
        
        glm::mat4 modelB_TranslationMatrix   = translate(glm::mat4(1.0), glm::vec3(0.0f,1.3f,0.0f));
        glm::mat4 modelB_RotationMatrix      = eulerAngleYXZ(0.0f,0.0f,0.0f);
        glm::mat4 modelB_ScalingMatrix       = scale(glm::mat4(1.0), glm::vec3(1.0f,1.0f,1.0f));
        glm::mat4 modelB_ModelMatrix = modelB_TranslationMatrix * modelB_RotationMatrix * modelB_ScalingMatrix;
        glm::mat4 modelB_BoneMatrix = glm::mat4(1.0f);
        modelB -> playOBJ(modelB_ModelMatrix, modelB_BoneMatrix);
        
        conconcon -> playOBJ(conconTxt, 15, 12);
        conconcos -> playOBJ(conconCos, 150, 12);
        conconint -> playOBJ(conconInt, 500, 12);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
    
    glfwTerminate();
    
    return 0;
}

void initOpenGL()
{
    
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Opengl 4.X
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // Opengl X.1 !!!4.1 supported from MacBook Early 2015!!!
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    int windowWidth = 1024;
    int windowHeight = 768;
    
    window = glfwCreateWindow( windowWidth, windowHeight, "PLAY", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    
    int frameBufferWidth = windowWidth;
    int frameBufferHeight = windowHeight;
    
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
    }
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwPollEvents();
    glfwSetCursorPos(window, windowWidth/2, windowHeight/2);
    
    glClearColor((float)0/255, (float)0/255, (float)0/255, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    
}





//
// ALL modules
//


int selected_armature = 0;
int selected_sobject = 0;
std::string select_model = "A";
std::string key_numbers = "";
void insert_char_key_module(GLFWwindow* window, unsigned int charinfo) {
    
    // Numbers
    if (charinfo > 47 && charinfo < 58) {
        std::string key_number = std::to_string(charinfo-48);
        key_numbers += key_number;
    }
    
}
void insert_key_module(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    // Backspace
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
        key_numbers.pop_back();
    }
    
}

void skinning_fps() {
    
    int skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stoi(key_numbers);
    }
    
    conconInt[3] = std::to_string(skval).c_str();
    
    for (int i=0; i<modelAArr.size(); i++) { modelAArr[i] -> changefps(skval); }
    for (int i=0; i<modelSArr.size(); i++) { modelSArr[i] -> changefps(skval); }
    
}

void skinning_frame() {
    
    int skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stoi(key_numbers);
    }
    
    conconInt[3] = std::to_string(skval).c_str();
    
    for (int i=0; i<modelAArr.size(); i++) { modelAArr[i] -> changeframe(skval); }
    for (int i=0; i<modelSArr.size(); i++) { modelSArr[i] -> changeframe(skval); }
    
}

void skinning_play() {
    
    for (int i=0; i<modelAArr.size(); i++) { modelAArr[i] -> playstop(); }
    for (int i=0; i<modelSArr.size(); i++) { modelSArr[i] -> playstop(); }
    
}

void skinning_select_armature() {
    
    int skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stoi(key_numbers);
    }
    
    conconInt[3] = std::to_string(skval).c_str();
    
    selected_armature = skval;
    
}

void skinning_select_bone() {
    
    int skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stoi(key_numbers);
    }
    
    conconInt[3] = std::to_string(skval).c_str();
    
    modelAArr[selected_armature] -> selectBone(skval);
    
}

void skinning_select_sobject() {
    
    int skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stoi(key_numbers);
    }
    
    conconInt[3] = std::to_string(skval).c_str();
    
    selected_sobject = skval;
    
}

void skinning_rotate() {
    
    int   mode = 0;
    int   axis = 9;
    float skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stof(key_numbers);
    }
    
    if (glfwGetKey( window, GLFW_KEY_PERIOD ) == GLFW_PRESS) {
        mode = 1;
        skval += 0.1;
        key_numbers = std::to_string(skval);
    }
    if (glfwGetKey( window, GLFW_KEY_COMMA ) == GLFW_PRESS) {
        mode = 1;
        skval -= 0.1;
        key_numbers = std::to_string(skval);
    }
    
    if (glfwGetKey( window, GLFW_KEY_X ) == GLFW_PRESS) {
        mode = 0;
        axis = 0;
    }
    if (glfwGetKey( window, GLFW_KEY_Y ) == GLFW_PRESS) {
        mode = 0;
        axis = 1;
    }
    if (glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS) {
        mode = 0;
        axis = 2;
    }
    
    if       (select_model == "A" && axis != 9 && mode == 0) {
        modelAArr[selected_armature] -> updateRotate(axis, skval, 0);
    }else if (select_model == "S" && axis != 9 && mode == 0){
        modelSArr[selected_armature] -> updateRotate(axis, skval, 0);
    }else{}
    
    if       (select_model == "A" && axis != 9 && mode == 1) {
        modelAArr[selected_armature] -> updateRotate(axis, skval, 0);
    }else if (select_model == "S" && axis != 9 && mode == 1){
        modelSArr[selected_armature] -> updateRotate(axis, skval, 0);
    }else{}
    
    conconInt[3] = std::to_string(int(skval)).c_str();
    
}

void skinning_trans() {
    
    int   mode = 0;
    int   axis = 9;
    float skval;
    if (key_numbers == "") {
        skval = 0;
    }else{
        skval = std::stof(key_numbers);
    }
    
    if (glfwGetKey( window, GLFW_KEY_PERIOD ) == GLFW_PRESS) {
        mode = 1;
        skval += 0.1;
        key_numbers = std::to_string(skval);
    }
    if (glfwGetKey( window, GLFW_KEY_COMMA ) == GLFW_PRESS) {
        mode = 1;
        skval -= 0.1;
        key_numbers = std::to_string(skval);
    }
    
    if (glfwGetKey( window, GLFW_KEY_X ) == GLFW_PRESS) {
        mode = 0;
        axis = 0;
    }
    if (glfwGetKey( window, GLFW_KEY_Y ) == GLFW_PRESS) {
        mode = 0;
        axis = 1;
    }
    if (glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS) {
        mode = 0;
        axis = 2;
    }
    
    if       (select_model == "A" && axis != 9 && mode == 0) {
    }else if (select_model == "S" && axis != 9 && mode == 0){
        modelSArr[selected_armature] -> updateTrans(axis, skval, 0);
    }else{}
    
    if       (select_model == "A" && axis != 9 && mode == 1) {
    }else if (select_model == "S" && axis != 9 && mode == 1){
        modelSArr[selected_armature] -> updateTrans(axis, skval, 0);
    }else{}
    
    conconInt[3] = std::to_string(int(skval)).c_str();
    
}

glm::vec3 copy_skn_A;
std::vector<glm::vec3> copy_skn_S;
void skinning_copy() {
    
    if       (select_model == "A") {
        copy_skn_A = modelAArr[selected_armature] -> copy();
    }else if (select_model == "S"){
        copy_skn_S = modelSArr[selected_sobject] -> copy();
    }else{}
    
}

void skinning_paste() {
    
    if       (select_model == "A") {
        modelAArr[selected_armature] -> paste(copy_skn_A);
    }else if (select_model == "S"){
        modelSArr[selected_sobject] -> paste(copy_skn_S);
    }else{}
    
}

void skinning_camera() {
    
    cameraMode(0);
    
}

void skinning_recorde() {
    
    auto nowtime = std::chrono::system_clock::now();
    std::time_t nowtime_time_t = std::chrono::system_clock::to_time_t(nowtime);
    std::stringstream ss;
    ss << nowtime_time_t;
    std::string timeme = ss.str();
    for (int i=0; i<modelAArr.size(); i++) {
        std::string filename = "RECKEYFRAME/" + timeme + "_a_" + std::to_string(i) + ".kfm";
        modelAArr[i] -> recorde(filename);
        std::string recorde_cos = "Recorded" + filename;
        conconCos.insert(conconCos.begin()+1, recorde_cos.c_str());
    }
    for (int i=0; i<modelSArr.size(); i++) {
        std::string filename = "RECKEYFRAME/" + timeme + "_s_" + std::to_string(i) + ".kfm";
        modelSArr[i] -> recorde(filename);
        std::string recorde_cos = "Recorded" + filename;
        conconCos.insert(conconCos.begin()+1, recorde_cos.c_str());
    }
    
}

int return_FPS()
{
    return modelAArr[0] -> getfps();
}

int return_FRAME()
{
    return modelAArr[0] -> getframe();
}

std::string edit_type = "";
void skinning_control() {
    
    // F -> fps
    if (edit_type == "F") {
        skinning_fps();
    }
    
    if (glfwGetKey( window, GLFW_KEY_F ) == GLFW_PRESS && edit_type == ""){
        edit_type = "F";
        conconCos.insert(conconCos.begin()+1, "Update FPS");
    }
    
    // G -> frame
    if (edit_type == "G") {
        skinning_frame();
    }
    
    if (glfwGetKey( window, GLFW_KEY_G ) == GLFW_PRESS && edit_type == ""){
        edit_type = "G";
        conconCos.insert(conconCos.begin()+1, "Move FRAME");
    }
    
    // P -> play
    if (edit_type == "P") {
        skinning_play();
    }
    
    if (glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS && edit_type == ""){
        edit_type = "P";
        conconCos.insert(conconCos.begin()+1, "Play");
    }
    
    // V -> armature select
    if (edit_type == "V") {
        skinning_select_armature();
    }
    
    if (glfwGetKey( window, GLFW_KEY_V ) == GLFW_PRESS && edit_type == ""){
        edit_type = "V";
        select_model = "A";
        conconCos.insert(conconCos.begin()+1, "Select Armature");
    }
    
    // B -> bone select
    if (edit_type == "B") {
        skinning_select_bone();
    }
    
    if (glfwGetKey( window, GLFW_KEY_B ) == GLFW_PRESS && edit_type == ""){
        edit_type = "B";
        select_model = "A";
        conconCos.insert(conconCos.begin()+1, "Select Bone");
    }
    
    // SPACE -> sobject select
    if (edit_type == "SPACE") {
        skinning_select_sobject();
    }
    
    if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS && edit_type == ""){
        edit_type = "SPACE";
        select_model = "S";
        conconCos.insert(conconCos.begin()+1, "Select SObject");
    }
    
    // R -> rotate xyz
    if (edit_type == "R") {
        skinning_rotate();
    }
    
    if (glfwGetKey( window, GLFW_KEY_R ) == GLFW_PRESS && edit_type == ""){
        edit_type = "R";
        conconCos.insert(conconCos.begin()+1, "Rotate");
    }
    
    // T -> trans xyz (only sobject)
    if (edit_type == "T") {
        skinning_trans();
    }
    
    if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS && edit_type == ""){
        edit_type = "T";
        conconCos.insert(conconCos.begin()+1, "Translate");
    }
    
    // N -> copy
    if (edit_type == "N") {
        skinning_copy();
    }
    
    if (glfwGetKey( window, GLFW_KEY_N ) == GLFW_PRESS && edit_type == ""){
        edit_type = "N";
        conconCos.insert(conconCos.begin()+1, "Copy");
    }
    
    // M -> paste
    if (edit_type == "M") {
        skinning_paste();
    }
    
    if (glfwGetKey( window, GLFW_KEY_M ) == GLFW_PRESS && edit_type == ""){
        edit_type = "M";
        conconCos.insert(conconCos.begin()+1, "Paste");
    }
    
    // C -> camera
    if (edit_type == "C") {
        skinning_camera();
    }
    
    if (glfwGetKey( window, GLFW_KEY_C ) == GLFW_PRESS && edit_type == ""){
        edit_type = "C";
        conconCos.insert(conconCos.begin()+1, "Camera moved to origin");
    }
    
    // I -> Recorde
    if (glfwGetKey( window, GLFW_KEY_I ) == GLFW_RELEASE && edit_type == "I") {
        skinning_recorde();
        edit_type = "";
    }
    
    if (glfwGetKey( window, GLFW_KEY_I ) == GLFW_PRESS && edit_type == ""){
        edit_type = "I";
        conconCos.insert(conconCos.begin()+1, "Recorded and Ok");
    }
    
    // TAB -> Done
    if (glfwGetKey( window, GLFW_KEY_TAB) == GLFW_PRESS && edit_type != "") {
        edit_type = "";
        conconCos.insert(conconCos.begin()+1, "Ok");
        key_numbers = "";
    }
    
    
    
    
    if (conconCos.size() > 100) { conconCos.erase(conconCos.begin()+99); }
    conconInt[1] = "fps->"+std::to_string(return_FPS());
    conconInt[2] = "frame->"+std::to_string(return_FRAME());
    
    
    
}

