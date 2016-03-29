#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <GL/glut.h>
#include <string.h>
#include "Leap.h"
#include "LeapMath.h"
#include <math.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include "include/ShadowMapping.h"

extern "C"{
    int ilutGLBindTexImage();
    int glGenerateMipmap(GLenum target);
}


int width =1920;      //1920;
int height = 1080;    //1080;
int lastx = 0;
int lasty = 0;
bool lighting = 0;
bool rgb = 0;
bool objectScale = 0;
bool animation = 0;
float animCount = 0;
float raio = 0.5f;
float pi = 3.1415f;
float angleX_obj = 0.0f;
float angleY_obj = 0.0f;
float angleX_cam = 3.1415f;
float angleY_cam = 0.0f;
float posX = 0.0f;
float posY = 1.0f;
float posZ = 1.0f;
float lz = 0.0f;
float ly = 0.0f;
float lx = 0.0f;
float objectscaleX = 1.0f;
float objectscaleY = 1.0f;
float objectscaleZ = 1.0f;
float lightR = 1.5f;
float lightG = 1.5f;
float lightB = 1.5f;
int rockSlice = 5;
int rockStack = 7;
float rockScaleY = 1.5f;
float rockScaleXZ = 0.5f;
float alpha = 0.0f;
float speed = 0.02f;
float scaleFactor = 1.0f/200.0f;
float dimensionSclFactor = 1.0f/700.0f;
float p0[3], p1[3], p2[3], p3[3], vec1 [3], vec2[3];
GLfloat color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat emissionColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lt0Position[4] = {4.0f, 6.0f, 2.0f, 1.0f};           // location of light 0

struct texture{
    GLuint ID;
    GLsizei width;
    GLsizei height;
    ILuint imageID;
    ILubyte *data;
    const char *Path;
};
struct texture Textures[10];

struct timeval t;
static unsigned int previousTime = 0;
unsigned int actualTime;

ShadowMapping shadowRenderer;

using namespace Leap;

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};
Controller controller;

void Reshape(int w, int h) {
  width = w;
  height = h;
  if (h == 0)
    h = 1;
  float ratio = 1.0f*width/ height;
  glViewport (0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, ratio, 0.1, 100);
  glMatrixMode(GL_MODELVIEW);
}

struct texture initTexture(struct texture Tex, GLfloat paramValue){
    ilGenImages(1, &Tex.imageID);
    ilBindImage(Tex.imageID);
    ilLoadImage(Tex.Path);                      // Loads into the current bound image
    Tex.width = ilGetInteger(IL_IMAGE_WIDTH);
    Tex.height = ilGetInteger(IL_IMAGE_HEIGHT);
    if (Tex.width > 1024 || Tex.height > 1024){
        iluScale(1024, 1024, 1);
        Tex.width = ilGetInteger(IL_IMAGE_WIDTH);
        Tex.height = ilGetInteger(IL_IMAGE_HEIGHT);
    }
    iluImageParameter(ILU_FILTER, ILU_LINEAR);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Tex.ID  = ilutGLBindTexImage();                // This generates the texture
    Tex.data = ilGetData();
    glGenTextures(1, &Tex.ID);
    glBindTexture(GL_TEXTURE_2D, Tex.ID);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, paramValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, paramValue);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -2.0f);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                                                 0,
                                     GL_RGBA,
                                    Tex.width,
                                   Tex.height,
                                                 0,
      GL_RGBA, GL_UNSIGNED_BYTE,
                                     Tex.data);

    glGenerateMipmap(GL_TEXTURE_2D);
    gluBuild2DMipmaps(GL_TEXTURE_2D,
                                             GL_RGBA,
                                            Tex.width,
                                           Tex.height,
              GL_RGBA, GL_UNSIGNED_BYTE,
                                             Tex.data);

    return Tex;

}

void init( ){
    // Ilumination
    GLfloat ambientIntensity[4] = {0.2f, 0.2f, 0.2f, 1.0f};             // Ambient light color
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientIntensity);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);                                          // set up light 0 properties

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

    glutWarpPointer(width/2, height/2);
    glutSetCursor(GLUT_CURSOR_NONE);

    ilInit ();
    iluInit ();
    ilutRenderer(ILUT_OPENGL);                            // Switch the renderer
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    Textures[0].Path =  "Textures/ConcreteFloors.jpg";
    Textures[0] = initTexture(Textures[0], GL_REPEAT);

    Textures[1].Path =  "Textures/wall.jpg";
    Textures[1] = initTexture(Textures[1], GL_REPEAT);

    Textures[2].Path =  "Textures/rock.jpg";
    Textures[2] = initTexture(Textures[2], GL_REPEAT);

    Textures[3].Path =  "Textures/concrete.jpg";
    Textures[3] = initTexture(Textures[3], GL_REPEAT);

    Textures[4].Path =  "Textures/RockSmooth1.jpg";
    Textures[4] = initTexture(Textures[4], GL_REPEAT);

    Textures[5].Path =  "Textures/lightOff.png";
    Textures[5] = initTexture(Textures[5], GL_REPEAT);

    Textures[6].Path =  "Textures/lightOn.png";
    Textures[6] = initTexture(Textures[6], GL_REPEAT);

    Textures[7].Path =  "Textures/rgb.png";
    Textures[7] = initTexture(Textures[7], GL_REPEAT);

    Textures[8].Path =  "Textures/scale.png";
    Textures[8] = initTexture(Textures[8], GL_REPEAT);

    Textures[9].Path =  "Textures/animation.png";
    Textures[9] = initTexture(Textures[9], GL_REPEAT);

}

    void drawCube(float dimX, float dimY, float dimZ, GLenum oneFaceTex,  GLuint TextureID, float s2, float t2, float s1, float t1){
    glBegin(GL_QUADS);
        p0[0] = 0.0f;     p0[1] = dimY;   p0[2] = dimZ;
        p1[0] = 0.0f;     p1[1] = 0.0f;     p1[2] = dimZ;
        p2[0] = dimX;   p2[1] = 0.0f;     p2[2] = dimZ;
        p3[0] = dimX;   p3[1] = dimY;   p3[2] = dimZ;

        glNormal3fv(p0); glTexCoord2f(0.0f, t1);  glVertex3fv(p0);
        glNormal3fv(p1); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p1);
        glNormal3fv(p2); glTexCoord2f(s1, 0.0f);  glVertex3fv(p2);
        glNormal3fv(p3); glTexCoord2f(s1, t1);  glVertex3fv(p3);
    glEnd();


    glBegin(GL_QUADS);
        p0[0] = dimX;     p0[1] = dimY;     p0[2] = 0.0f;
        p1[0] = dimX;     p1[1] = 0.0f;       p1[2] = 0.0f;
        p2[0] = 0.0f;       p2[1] = 0.0f;       p2[2] = 0.0f;
        p3[0] = 0.0f;       p3[1] = dimY;     p3[2] = 0.0f;

        glNormal3fv(p0); glTexCoord2f(s1, t1);  glVertex3fv(p0);
        glNormal3fv(p1); glTexCoord2f(s1, 0.0f);  glVertex3fv(p1);
        glNormal3fv(p2); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p2);
        glNormal3fv(p3); glTexCoord2f(0.0f, t1);  glVertex3fv(p3);
    glEnd();

    glBegin(GL_QUADS);
        p0[0] = 0.0f;        p0[1] = dimY;     p0[2] = 0.0f;
        p1[0] = 0.0f;        p1[1] = dimY;     p1[2] = dimZ;
        p2[0] = dimX;      p2[1] = dimY;     p2[2] = dimZ;
        p3[0] = dimX;      p3[1] = dimY;     p3[2] = 0.0f;

        glNormal3fv(p0); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p0);
        glNormal3fv(p1); glTexCoord2f(0.0f, t1);  glVertex3fv(p1);
        glNormal3fv(p2); glTexCoord2f(s1, t1);  glVertex3fv(p2);
        glNormal3fv(p3); glTexCoord2f(s1, 0.0f);  glVertex3fv(p3);
    glEnd();

    glBegin(GL_QUADS);
        p0[0] = 0.0f;      p0[1] = dimY;     p0[2] = dimZ;
        p1[0] = 0.0f;      p1[1] = dimY;     p1[2] = 0.0f;
        p2[0] = 0.0f;      p2[1] = 0.0f;       p2[2] = 0.0f;
        p3[0] = 0.0f;      p3[1] = 0.0f;       p3[2] = dimZ;

        glNormal3fv(p0); glTexCoord2f(s1, t1);  glVertex3fv(p0);
        glNormal3fv(p1); glTexCoord2f(s1, 0.0f);  glVertex3fv(p1);
        glNormal3fv(p2); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p2);
        glNormal3fv(p3); glTexCoord2f(0.0f, t1);  glVertex3fv(p3);
    glEnd();

    glBegin(GL_QUADS);
        p0[0] = dimX;      p0[1] = 0.0f;       p0[2] = dimZ;
        p1[0] = dimX;      p1[1] = 0.0f;       p1[2] = 0.0f;
        p2[0] = dimX;      p2[1] = dimY;     p2[2] = 0.0f;
        p3[0] = dimX;      p3[1] = dimY;     p3[2] = dimZ;

        glNormal3fv(p0); glTexCoord2f(0.0f, t1);  glVertex3fv(p0);
        glNormal3fv(p1); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p1);
        glNormal3fv(p2); glTexCoord2f(s1, 0.0f);  glVertex3fv(p2);
        glNormal3fv(p3); glTexCoord2f(s1, t1);  glVertex3fv(p3);
    glEnd();

    if(oneFaceTex){
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        glBegin(GL_QUADS);
            p0[0] = dimX;     p0[1] = 0.0f;     p0[2] = 0.0f;
            p1[0] = dimX;     p1[1] = 0.0f;     p1[2] = dimZ;
            p2[0] = 0.0f;       p2[1] = 0.0f;     p2[2] = dimZ;
            p3[0] = 0.0f;       p3[1] = 0.0f;     p3[2] = 0.0f;

            glNormal3fv(p0); glTexCoord2f(s2, 0.0f);  glVertex3fv(p0);
            glNormal3fv(p1); glTexCoord2f(s2, t2);  glVertex3fv(p1);
            glNormal3fv(p2); glTexCoord2f(0.0f, t2);  glVertex3fv(p2);
            glNormal3fv(p3); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else{
        glBegin(GL_QUADS);
            p0[0] = dimX;     p0[1] = 0.0f;     p0[2] = 0.0f;
            p1[0] = dimX;     p1[1] = 0.0f;     p1[2] = dimZ;
            p2[0] = 0.0f;       p2[1] = 0.0f;     p2[2] = dimZ;
            p3[0] = 0.0f;       p3[1] = 0.0f;     p3[2] = 0.0f;

            glNormal3fv(p0); glTexCoord2f(s1, 0.0f);  glVertex3fv(p0);
            glNormal3fv(p1); glTexCoord2f(s1, t1);  glVertex3fv(p1);
            glNormal3fv(p2); glTexCoord2f(0.0f, t1);  glVertex3fv(p2);
            glNormal3fv(p3); glTexCoord2f(0.0f, 0.0f);  glVertex3fv(p3);
        glEnd();
    }
}

void drawPainel(){

    color[0] = 0.917f;     color[1] = 0.917f;     color[2] = 0.682f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Textures[3].ID);
    glPushMatrix();
    drawCube(1.0f, 0.3f, 1.0f, GL_FALSE, 0, 1.0f, 2.0f, 1.0f, 2.0f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    float dimX = 0.2f;
    float dimY = 0.08f;
    float dimZ = 0.2f;

    glPushMatrix();
        glTranslatef(0.2f, -0.1f, 0.7f);
        if(lighting){
            glTranslatef(0.0f, 0.09f, 0.0f);
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[6].ID, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[5].ID, 1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.6f, -0.1f, 0.7f);
         if(rgb){
            glTranslatef(0.0f, 0.09f, 0.0f);
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[7].ID, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[7].ID, 1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.2f, -0.1f, 0.4f);
         if(objectScale){
            glTranslatef(0.0f, 0.09f, 0.0f);
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[8].ID, 1.0f, 1.0f, 1.0f, 1.0f);
         }
         else
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[8].ID, 1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.6f, -0.1f, 0.4f);
         if(animation){
            glTranslatef(0.0f, 0.09f, 0.0f);
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[9].ID, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
            drawCube(dimX, dimY, dimZ, GL_TRUE, Textures[9].ID, 1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void drawWoodenBox(){
    color[0] = 1.0f;     color[1] = 1.0f;     color[2] = 1.0f;      color[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Textures[1].ID);
    drawCube(1.0f, 1.0f, 1.0f, GL_FALSE, 0, 1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_TEXTURE_2D);
}

void solidSphere(int radius, int stacks, int columns, GLenum activateTex)
{
    GLUquadric *quadObj = gluNewQuadric();
    gluQuadricNormals(quadObj, GLU_SMOOTH);
    gluQuadricTexture(quadObj, activateTex);
    gluSphere(quadObj, radius, stacks, columns);
    gluDeleteQuadric(quadObj);
}

void drawRock(float r, int stacks, int slices ) {
    color[0] = 0.65f;     color[1] = 0.65f;     color[2] = 0.65f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Textures[2].ID);
    solidSphere(r, stacks, slices, GL_TRUE);
    glDisable(GL_TEXTURE_2D);

}

void drawRoom(){
    color[0] = 1.0f;     color[1] = 1.0f;     color[2] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, color);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Textures[3].ID);
    drawCube(1.0f, 1.0f, 1.0f, GL_TRUE, Textures[0].ID, 6.0f, 6.0f, 2.0f, 2.0f);
    glDisable(GL_TEXTURE_2D);
}

void drawSquare(){
    glBegin(GL_QUADS);
        glVertex3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();
}

void drawBone(Vector InitPosition, Vector FinalPosition){
    color[0] = 0.6f;     color[1] = 0.6f;     color[2] = 0.6f;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, color);

    glLineWidth(8.0f);
    glBegin(GL_LINES);
        glVertex3f(InitPosition[0]*dimensionSclFactor,
                         InitPosition[1]*dimensionSclFactor - 0.3f,
                         InitPosition[2]*dimensionSclFactor);

        glVertex3f(FinalPosition[0]*dimensionSclFactor,
                         FinalPosition[1]*dimensionSclFactor - 0.3f,
                         FinalPosition[2]*dimensionSclFactor);
    glEnd();
}


void drawJoints(Vector jointPosition){
    color[0] = 0.647f;     color[1] = 0.165f;     color[2] = 0.165f;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, color);

    glPushMatrix();
    glTranslatef(jointPosition[0]*dimensionSclFactor,
                       jointPosition[1]*dimensionSclFactor - 0.3f,
                       jointPosition[2]*dimensionSclFactor);
    glScalef(scaleFactor, scaleFactor, scaleFactor);
    solidSphere(2.0f, 20, 20, GL_FALSE);
    glPopMatrix();
}

Vector leftHand_Position;
Vector rightHand_Position;

void drawHands(){
    const Frame frame = controller.frame();
    HandList hands = frame.hands();
    for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
        // Get the first hand
        const Hand hand = *hl;
        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";

        // Change wooden box scale
        if (hand.isLeft() && (hand.fingers().extended().count() == 0))
            leftHand_Position = hand.palmPosition();

        else if (!hand.isLeft() && (hand.fingers().extended().count() == 0))
            rightHand_Position = hand.palmPosition();

        if (objectScale){
            objectscaleX = (rightHand_Position[0] - leftHand_Position[0])/100.0f;
            if (objectscaleX < 0)
                objectscaleX *= -1.0f;
            else if(objectscaleX == 0)
                objectscaleX = 1.0f;
            objectscaleY = (rightHand_Position[1] - leftHand_Position[1])/100.0f;
            if (objectscaleY < 0)
                objectscaleY *= -1.0f;
            else if(objectscaleY == 0)
                objectscaleY = 1.0f;
            objectscaleZ = (rightHand_Position[2] - leftHand_Position[2])/100.0f;
            if (objectscaleZ < 0)
                objectscaleZ *= -1.0f;
            else if(objectscaleZ == 0)
                objectscaleZ = 1.0f;
        }

        if(hand.isLeft() && rgb && (hand.fingers().extended().count() == 3)){
            leftHand_Position = hand.palmPosition();

            lightR = (leftHand_Position[0])/150.0f;
            emissionColor[0] = ((leftHand_Position[0])/150.0f)*0.2f;
            if (lightR < 0){
                lightR *= -1;
                emissionColor[0] *= -1.0f;
            }

            lightG = (leftHand_Position[1] - 60)/150.0f;
            emissionColor[1] = ((leftHand_Position[1] - 60)/150.0f)*0.2f;
            if (lightG < 0){
                lightG *= -1;
                emissionColor[1] *= -1.0f;
            }

            lightB = (leftHand_Position[2])/150.0f;
            emissionColor[2] = ((leftHand_Position[2])/150.0f)*0.2f;
            if (lightB < 0){
                lightB *= -1;
                emissionColor[2] *= -1.0f;
            }
        }

        // Get the hand's normal and direction vectors
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();

        // Control camera "look at" with left hand
        if (hand.isLeft() && (hand.fingers().extended().count() == 2) && (normal.roll() * RAD_TO_DEG > 40)){
            angleX_cam += 0.01f;
            lx = sin(angleX_cam);
            lz = cos(angleX_cam)*cos(angleY_cam);
        }
        else if (hand.isLeft() && (hand.fingers().extended().count() == 2) && (normal.roll() * RAD_TO_DEG < -40)){
            angleX_cam -= 0.01f;
            lx = sin(angleX_cam);
            lz = cos(angleX_cam)*cos(angleY_cam);
        }
         if (hand.isLeft() && (hand.fingers().extended().count() == 2) && (direction.pitch() * RAD_TO_DEG > 40) && (angleY_cam < (pi*80/180))){
            angleY_cam += 0.01f;
            ly = sin(angleY_cam);
            lz = cos(angleX_cam)*cos(angleY_cam);
        }
        else if (hand.isLeft() && (hand.fingers().extended().count() == 2) && (direction.pitch() * RAD_TO_DEG < -40) && (angleY_cam > (-pi*80/180))){
            angleY_cam -= 0.01f;
            ly = sin(angleY_cam);
            lz = cos(angleX_cam)*cos(angleY_cam);
        }

        // Control camera position with right hand
        if (!hand.isLeft() && (hand.fingers().extended().count() == 2) && (normal.roll() * RAD_TO_DEG > 40)){
            posZ += cos(angleX_cam+(pi/2))*cos(angleY_cam)*speed;
            posX += sin(angleX_cam+(pi/2))*speed;
        }
        else if (!hand.isLeft() && (hand.fingers().extended().count() == 2) && (normal.roll() * RAD_TO_DEG < -40)){
            posZ -= cos(angleX_cam+(pi/2))*cos(angleY_cam)*speed;
            posX -= sin(angleX_cam+(pi/2))*speed;
        }
         if (!hand.isLeft() && (hand.fingers().extended().count() == 2) && (direction.pitch() * RAD_TO_DEG > 40)){
            posZ += lz*speed;
            posX += lx*speed;
        }
        else if (!hand.isLeft() && (hand.fingers().extended().count() == 2) && (direction.pitch() * RAD_TO_DEG < -40)){
            posZ -= lz*speed;
            posX -= lx*speed;
        }

        // Draw center of the hands palm
        glColor4f(0.8f, 0.0f, 0.0f, 0.3f);
        drawJoints(hand.palmPosition());

        // Get the Arm bone
        Arm arm = hand.arm();

                glColor4f(0.8f, 0.0f, 0.0f, 1.0f);
                drawJoints(arm.wristPosition());
                drawJoints(arm.elbowPosition());

                glColor4f(0.65f, 0.65f, 0.65f, 1.0f);
                drawBone(arm.elbowPosition(), arm.wristPosition());

        // Get fingers
        const FingerList fingers = hand.fingers();
        for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
            const Finger finger = *fl;

            // Get finger bones
            for (int b = 0; b < 4; ++b) {
                Bone::Type boneType = static_cast<Bone::Type>(b);
                Bone bone = finger.bone(boneType);

                glColor3f(0.5f, 0.0f, 0.0f);
                drawJoints(bone.prevJoint());
                drawJoints(bone.nextJoint());

                glColor3f(0.65f, 0.65f, 0.65f);
                drawBone(bone.prevJoint(), bone.nextJoint());
            }
        }
    }
}

void verifyTouch(){
    Pointable pointable = controller.frame().pointables().frontmost();
    Vector position = pointable.tipPosition();

    if( actualTime - previousTime > 1
        && posX+ lx + position[0]*dimensionSclFactor > -1.8f
        && posX+ lx + position[0]*dimensionSclFactor < -1.6f
        && posY+ ly + position[1]*dimensionSclFactor > 1.0f
        && posY+ ly + position[1]*dimensionSclFactor < 1.3f
        && posZ+ lz + position[2]*dimensionSclFactor > -2.0f
        && posZ+ lz + position[2]*dimensionSclFactor < -1.9f){
        lighting = lighting? 0 : 1;
        gettimeofday(&t, NULL);
        previousTime = t.tv_sec;
    }

    if( actualTime - previousTime > 1
        && posX+ lx + position[0]*dimensionSclFactor > -1.4f
        && posX+ lx + position[0]*dimensionSclFactor < -1.1f
        && posY+ ly + position[1]*dimensionSclFactor > 1.0f
        && posY+ ly + position[1]*dimensionSclFactor < 1.3f
        && posZ+ lz + position[2]*dimensionSclFactor > -2.0f
        && posZ+ lz + position[2]*dimensionSclFactor < -1.9f){
        rgb = rgb? 0 : 1;
        gettimeofday(&t, NULL);
        previousTime = t.tv_sec;
    }

    if( actualTime - previousTime > 1
        && posX+ lx + position[0]*dimensionSclFactor > -1.8f
        && posX+ lx + position[0]*dimensionSclFactor < -1.6f
        && posY+ ly + position[1]*dimensionSclFactor > 0.8f
        && posY+ ly + position[1]*dimensionSclFactor < 1.0f
        && posZ+ lz + position[2]*dimensionSclFactor > -2.0f
        && posZ+ lz + position[2]*dimensionSclFactor < -1.9f){
        objectScale = objectScale? 0 : 1;
        gettimeofday(&t, NULL);
        previousTime = t.tv_sec;
    }

    if( actualTime - previousTime > 1
        && posX+ lx + position[0]*dimensionSclFactor > -1.4f
        && posX+ lx + position[0]*dimensionSclFactor < -1.1f
        && posY+ ly + position[1]*dimensionSclFactor > 0.8f
        && posY+ ly + position[1]*dimensionSclFactor < 1.0f
        && posZ+ lz + position[2]*dimensionSclFactor > -2.0f
        && posZ+ lz + position[2]*dimensionSclFactor < -1.9f){
        animation = animation? 0 : 1;
        gettimeofday(&t, NULL);
        previousTime = t.tv_sec;
    }
}

void SpecialKeys(int key, int x, int y){

    switch (key){
        case GLUT_KEY_RIGHT:
            lt0Position[0] += 0.2f;
            //angleY_obj -= 5.0f;
            break;

        case GLUT_KEY_LEFT:
            lt0Position[0] -= 0.2f;
            //angleY_obj += 5.0f;
            break;

        case GLUT_KEY_UP:
            lt0Position[1] += 0.2f;
            //angleX_obj += 5.0f;
            break;

        case GLUT_KEY_DOWN:
            lt0Position[1] -= 0.2f;
            //angleX_obj -= 5.0f;
            break;

        case GLUT_KEY_F1:
            lighting = lighting? 0 : 1;
            break;

            case GLUT_KEY_F2:
            animation = animation? 0 : 1;
            break;

    }
}

void NormalKeys(unsigned char key, int xx, int yy){
    switch (key){
        case 119:               // w
            posZ += lz*0.05f;
            posX += lx*0.05f;
            break;

        case 115:               // s
            posZ -= lz*0.05f;
            posX -= lx*0.05f;
            break;

        case 97:                // a
            posZ += cos(angleX_cam+(pi/2))*cos(angleY_cam)*0.05f;
            posX += sin(angleX_cam+(pi/2))*0.05f;
            break;

        case 100:              // d
            posZ -= cos(angleX_cam+(pi/2))*cos(angleY_cam)*0.05f;
            posX -= sin(angleX_cam+(pi/2))*0.05f;
            break;

        case 105:              // i
            rgb = rgb? 0 : 1;
            break;

        case 106:              // j
            animation = animation? 0 : 1;
            break;

        case 107:              // k
            objectScale = objectScale? 0 : 1;
            break;

        case 108:              // l
            lighting = lighting? 0 : 1;
            emissionColor[0] = 0.2f;     emissionColor[1] = 0.2f;     emissionColor[2] = 0.2f;
            break;

        case 27:                // escape
            exit(0);
            break;

        case 114:              // r
            lighting = 0;
            rgb = 0;
            objectScale = 0;
            animation = 0;
            animCount = 0.0f;
            angleX_obj = 0.0f;
            angleY_obj = 0.0f;
            angleX_cam = 3.1415f;
            angleY_cam = 0.0f;
            posX = 0.0f;
            posY = 1.0f;
            posZ = 1.0f;
            lz = 0.0f;
            ly = 0.0f;
            lx = 0.0f;
            objectscaleX = 1.0f;
            objectscaleY = 1.0f;
            objectscaleZ = 1.0f;
            lightR = 1.5f;
            lightG = 1.5f;
            lightB = 1.5f;
            rockSlice = 5;
            rockStack = 7;
            rockScaleY = 1.5f;
            rockScaleXZ = 0.5f;
            emissionColor[0] = 0.2f;
            emissionColor[1] = 0.2f;
            emissionColor[2] = 0.2f;
            alpha = 0.0f;
            glutWarpPointer(width/2, height/2);
            break;
    }
}


void Camera(int x, int y){
    if((y < lasty) && (angleY_cam < (pi*80/180))) {
        angleY_cam += 0.02f;
        ly = sin(angleY_cam);
        lz = cos(angleX_cam)*cos(angleY_cam);
    }

    else if((y > lasty) && (angleY_cam > (-pi*80/180))){
        angleY_cam -= 0.02f;
        ly = sin(angleY_cam);
        lz = cos(angleX_cam)*cos(angleY_cam);
    }

    if(x < lastx){
        angleX_cam += 0.02f;
        lx = sin(angleX_cam);
        lz = cos(angleX_cam)*cos(angleY_cam);
    }

    else if(x > lastx){
        angleX_cam -= 0.02f;
        lx = sin(angleX_cam);
        lz = cos(angleX_cam)*cos(angleY_cam);
    }

    if ((x > width-60) && (x < width))
        glutWarpPointer(width-100, y);
    else if((x > 0) && (x < 60))
        glutWarpPointer(x+100, y);

    if ((y > height-60) && (y < height))
        glutWarpPointer(x, height-100);
    else if((y > 0) && (y < 60))
        glutWarpPointer(x, y+100);

    lastx = x;
    lasty = y;
}

void cycle(){
    gettimeofday(&t, NULL);
    actualTime =  t.tv_sec;

    if(animation){
        if(alpha < 1.0f){
            rockSlice = (int)((1 - alpha) * 5 + alpha*32);
            rockStack = (int)((1 - alpha) * 7 + alpha*32);
            rockScaleY = (1 - alpha) * 1.5f + alpha*1.0f;
            rockScaleXZ = (1 - alpha) * 0.5f + alpha*1.0f;
            alpha += 0.01f;
        }
        if(alpha){
            animCount += 0.01f;

            if (animCount*180/pi >= 360.0f)
                animCount = 0.0f;
        }
    }
    else {
        if((alpha > 0.0f) && (animCount*180/pi >= 300.0f)){
            rockSlice = (int)((1 - alpha) * 5 + alpha*32);
            rockStack = (int)((1 - alpha) * 7 + alpha*32);
            rockScaleY = (1 - alpha) * 1.5f + alpha*1.0f;
            rockScaleXZ = (1 - alpha) * 0.5f + alpha*1.0f;
            alpha -= 0.01f;
        }
        if(animCount){
            animCount += 0.01f;

            if (animCount*180/pi >= 360.0f)
                animCount = 0.0f;
        }
    }
}

void drawScene(){

    //Draw hand(s)
    if(controller.isConnected() && controller.frame().hands().frontmost().isValid() ){
        glPushMatrix();
        glTranslatef(posX+ lx, posY+ ly, posZ+ lz);
        glRotatef((angleX_cam - pi)*180/pi, 0, 1, 0);
        glRotatef(angleY_cam*180/pi, 1, 0, 0);
        drawHands();
        glPopMatrix();

        verifyTouch();
    }
    glPushMatrix();
    glTranslatef(-4.0f, 0.0f, -17.0f);
    glScalef(15.0f, 8.0f, 20.0f);
    drawRoom();
    glPopMatrix();

    //Draw a painel with interactive buttons
    glColor4f(0.917f, 0.917f,  0.682f, 1.0f);
    color[0] = 0.917f;     color[1] = 0.917f;     color[2] = 0.682f;      color[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.8f);
    glPushMatrix();
    glTranslatef(-2.0f, 0.01f, -2.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    drawPainel();
    glPopMatrix();

    // Draw wooden box
    glPushMatrix();
    glTranslatef(2 + angleX_obj/10.0f,         //Translate X
           (cosf(animCount)/2.0f) + 1.0f,         //Translate Y
                     -9 + angleY_obj/10.0f);         //Translate Z
    glRotatef(angleX_obj, 1, 0, 0);
    glRotatef(angleY_obj, 0, 1, 0);
    glScalef(objectscaleX, objectscaleY, objectscaleZ);
    drawWoodenBox();
    glPopMatrix();

    // Draw rock
    glPushMatrix();
    glTranslatef(0.0f + cosf(animCount), 1.0f, -12.0f + sinf(animCount));
    glRotatef(40*sinf(animCount), 1, 0, 0);
    glRotatef(-40*cosf(animCount)+40, 0, 0, 1);
    glScalef(rockScaleXZ, rockScaleY, rockScaleXZ);
    drawRock(1.0f, rockStack, rockSlice);
    glPopMatrix();

}

void Display(void) {

    if(lighting)
        glEnable(GL_LIGHT0);
    else
        glDisable(GL_LIGHT0);

    GLfloat lt0Intensity[4] = {lightR, lightG, lightB, 1.0};         // white
    glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, lt0Intensity);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lt0Intensity);
    glLightfv(GL_LIGHT0, GL_POSITION, lt0Position);
    glLightf (GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);         // attenuation params (a,b,c)
    glLightf (GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.01f);
    // glLightf (GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


    glClearColor(0.529f, 0.808f, 0.8f, 0.980f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if(lighting){

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(110.0f, 1.0f, 0.01f, 30.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(  lt0Position[0], lt0Position[1], lt0Position[2],
                4.0f, 4.0f, -4.0f,
                0.0f, 1.0f, 0.0f);

        shadowRenderer.enableDepthCapture(GL_LEQUAL);
        drawScene();
        shadowRenderer.disableDepthCapture();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, (float)width/height, 0.1f, 100.0f);

        // Reset transformations
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Set the camera
        gluLookAt( posX, posY, posZ,                //Position
                        posX+lx, posY+ly, posZ+lz, //Look at
                        0.0f, 1.0f, 0.0f);                    //Orientation


        shadowRenderer.enableShadowTest();
        drawScene();
        shadowRenderer.disableShadowTest();
    }

    else{

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, (float)width/height, 0.1f, 100.0f);

        // Reset transformations
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        gluLookAt( posX, posY, posZ,                //Position
                        posX+lx, posY+ly, posZ+lz, //Look at
                        0.0f, 1.0f, 0.0f);                    //Orientation
        drawScene();

    }

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    //Draw sun
    color[0] = 1.0f;     color[1] = 1.0f;     color[2] = 1.0f;      color[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
    if(lighting){
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissionColor);
    }
    else{
        emissionColor[0] = 0.0f;     emissionColor[1] = 0.0f;     emissionColor[2] = 0.0f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissionColor);
    }
    glColor4f(1.0f, 1.0f, 0.765f, 1.0f);
    glPushMatrix();
    glTranslatef(lt0Position[0], lt0Position[1]+2.0f, lt0Position[2]);
    glutSolidSphere((lt0Position[2] / 20), 30, 30);
    glPopMatrix();

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Computer Graphics Project");
    init();

    glutDisplayFunc(Display);
    glutIdleFunc(cycle);
    glutPassiveMotionFunc(Camera);
    glutKeyboardFunc(NormalKeys);
    glutReshapeFunc(Reshape);
    glutSpecialFunc(SpecialKeys);
    glutMainLoop();

    return 0;
}