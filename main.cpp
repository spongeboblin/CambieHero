//CGgame
//Cambie Hero
#include "glm/glm.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>

using namespace std;

#define PI 3.1415926
#define MAX 100000
#define BKGRD_FILE "bkgrd.bmp"
#define EVIL_OBJ "evil.obj"
#define ZOMBIE_OBJ "zombie.obj"
#define PLAYER_OBJ "player.obj"
#define PLAYER_2_OBJ "player2.obj"
#define PLAYER_3_OBJ "player3.obj"

vector<glm::vec3> vertices[6];
vector<glm::vec2> uvs[6];
vector<glm::vec3> normals[6];

int width = 800, height = 800, num_blood = -1, pause = 0, player = 0,tot_num=1,tot_time=0,super_time=0;
float x = 0, y = 2.6, z = 5, xd = 0, yd = 0, zd = 0, delta = PI / 2, deltay = 0,cx,cy,kind,cube=0;
int rd = 0, rw = 0, ra = 0, rs = 0, num_zoom = 4, num_evil = 1, num_boom,
    shoot = 0, pshoot = 0, blood = 100, score_int = 0;
float zoom_x[30], zoom_y[30], zoom_z[30], zoom_h[30] = {0},
      evil_h[10] = {0}, evil_x[10], evil_y[10], evil_z[10],
      boom_x[5], boom_z[5], bdx[5], bdz[5], blood_x[1000] = {0}, blood_z[1000], blood_r[1000];
float material[4];
int map[20][12] = {0};
int kk = 0, flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0;
string score;

static int AddKeyModifier(int key);
static int ConvertSpecialKey(int special);
static void ChangeSize(int w, int h);
static void RenderScene(void);
static void SpecialKeys(int key, int x, int y);
static void SpecialKeysUp(int key, int x, int y);
static void KeyboardFunc(unsigned char cAscii, int x, int y);
static void KeyboardFuncUp(unsigned char cAscii, int x, int y);
static void MouseClickMessage(int button, int state, int x, int y);
static void MouseMoveMessage(int x, int y);
static void MouseWheelMessage(int wheel, int dir, int x, int y);
static void MousePassiveMoveMessage(int x, int y);
static void Timer(int x);

void output(int x, int y, char *string) {
    int len, i; glRasterPos2f(x, y);
    len = (int)strlen(string);
    for(i = 0; i < len; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
}

bool loadOBJ(const char * path,
             std::vector<glm::vec3> & out_vertices,
             std::vector<glm::vec2> & out_uvs,
             std::vector<glm::vec3> & out_normals
             ) {
    printf("Loading OBJ file %s...\n", path);
    
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    FILE * file = fopen(path, "r");
    if(file == NULL) {
        cout << "Impossible to open the file! Are you in the right path?" << endl;
        return false;
    }
    
    while(1) {
        char lineHeader[128];
        //read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if(res == EOF) break; //EOF = End Of File. Quit the loop.
        //else parse lineHeader
        
        if(strcmp(lineHeader, "v") == 0) {
            //cout << "Get v" << endl;
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        } else if(strcmp(lineHeader, "vt") == 0) {
            //cout << "Get vt" << endl;
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = -uv.y; //Invert V coordinate since we will only use DDS texture,
                          //which are inverted. Remove if you want to use TGA or BMP loaders.
            temp_uvs.push_back(uv);
        } else if(strcmp(lineHeader, "vn") == 0) {
            //cout << "Get vn" << endl;
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if(strcmp(lineHeader, "f") == 0) {
            //cout << "Get f" << endl;
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d//%d %d//%d %d//%d\n",
            	                 &vertexIndex[0], &normalIndex[0], &vertexIndex[1],
            	                 &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
            if(matches != 6) {
                cout << "File can't be read by our simple parser :-( Try exporting with other options." << endl;
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        } else {
            //Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }
    
    //For each vertex of each triangle
    for(unsigned int i = 0; i < vertexIndices.size(); i++) {
        //Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int normalIndex = normalIndices[i];
        
        //Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];
        
        //Put the attributes in buffers
        out_vertices.push_back(vertex);
        out_normals.push_back(normal);
    }
    return true;
}

GLuint maketex(const char* tfile, GLint xSize, GLint ySize) {
    GLuint rmesh;
    FILE* file;
    unsigned char* texdata = (unsigned char*)malloc(xSize * ySize * 3); //3 is {R,G,B}
    
    file = fopen(tfile, "rb" );
    fseek(file,54,SEEK_CUR); //54 is bmp header size
    fread(texdata, xSize * ySize * 3, 1, file);
    fclose(file);
    glEnable(GL_TEXTURE_2D);
    
    char* colorbits = new char[xSize * ySize * 3];
    
    for(GLint a = 0; a < xSize * ySize * 3; ++a) colorbits[a] = 0xFF;
    
    glGenTextures(1, &rmesh);
    glBindTexture(GL_TEXTURE_2D, rmesh);
    
    glTexImage2D(GL_TEXTURE_2D, 0 ,3 , xSize,
                 ySize, 0 , GL_RGB, GL_UNSIGNED_BYTE, colorbits);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //Save viewport and set up new one
    GLint viewport[4]; //4 is {X,Y,Width,Height}
    glGetIntegerv(GL_VIEWPORT, (GLint*)viewport);
    glViewport(0, 0, xSize, ySize);
    
    //Clear target and depth buffers
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    glPushMatrix(); //Duplicates MODELVIEW stack top
    glLoadIdentity(); //Replace new top with {1}
    
    glDrawPixels(xSize, ySize, GL_BGR, GL_UNSIGNED_BYTE, texdata);
    glPopMatrix();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     0, 0, xSize, ySize, 0);
    
    //Restore viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); //{X,Y,Width,Height}
    
    delete[] colorbits;
    free(texdata);
    return rmesh;
}

void sky(GLuint haze) {
    glBindTexture(GL_TEXTURE_2D, haze);
    material[0] = 1.0; material[1] = 1.0; material[2] = 1.0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(70 / 4.0f, 70 / 4.0f, -25.0f);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(-70 / 4.0f, 70 / 4.0f, -25.0f);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(-70 / 4.0f, -70 / 4.0f, -25.0f);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(70 / 4.0f, -70 / 4.0f, -25.0f);
    glEnd();
}

int judge(float x,float y,float z, int k) {
    for(int i = 0; i < num_zoom; i++) {
        if(k == i) continue;
        if(fabs(x - zoom_x[i]) < 0.3 && fabs(z - zoom_z[i]) < 0.3) return 0;
    }
    k -= num_zoom;
    for(int i = 0; i < num_evil; i++) {
        if(k == i) continue;
        if(fabs(x - evil_x[i]) < 0.3 && fabs(z - evil_z[i]) < 0.3) return 0;
    }
    if(k > -num_zoom - num_evil && fabs(x - xd) < 0.2 && fabs(z - zd) < 0.2) return 0;
    return 1;
}

void random_boss() {
    double xx, yy, zz;
    srand((unsigned int)time(0));
    for(int i = 0; i < num_zoom; i++) {
        while(1) {
            xx = rand() % 9 - 4;
            zz = rand() % 9 - 4;
            yy = 0;
            if(judge(xx, yy, zz, -1)) {
                zoom_x[i] = xx;
                zoom_y[i] = yy;
                zoom_z[i] = zz;
                break;
            }
        }
    }
    for(int i=0;i<num_evil;i++) {
        while(1) {
            xx = rand() % 7 - 3;
            zz = rand() % 7 - 3;
            yy = 0;
            if(judge(xx, yy, zz, -1)) {
                evil_x[i] = xx;
                evil_y[i] = yy;
                evil_z[i] = zz;
                break;
            }
        }
    }
    for(int i = 0; i < num_evil; i++) {
        boom_x[i] =- 100;
        boom_z[i] =- 100;
    }
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Cambie Hero");
    glEnable(GL_NORMALIZE);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);
    
    bool res = loadOBJ(EVIL_OBJ, vertices[0], uvs[0], normals[0]);
    res = loadOBJ(ZOMBIE_OBJ, vertices[1], uvs[1], normals[1]);
    res = loadOBJ(PLAYER_OBJ, vertices[2], uvs[2], normals[2]);
    //res = loadOBJ("cylinder.obj", vertices[3], uvs[3], normals[3]);
    res = loadOBJ(PLAYER_2_OBJ, vertices[4], uvs[4], normals[4]);
    res = loadOBJ(PLAYER_3_OBJ, vertices[5], uvs[5], normals[5]);

    srand((unsigned int)time(0));

    random_boss();
    // Register callback functions
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutSpecialUpFunc(SpecialKeysUp);
    glutKeyboardFunc(KeyboardFunc);
    glutKeyboardUpFunc(KeyboardFuncUp);
    glutDisplayFunc(RenderScene);
    glutMouseFunc(MouseClickMessage);
    glutPassiveMotionFunc(MousePassiveMoveMessage);
    glutMotionFunc(MouseMoveMessage);
    glutTimerFunc(33, Timer, 1);
    //glutMouseWheelFunc(MouseWheelMessage);
    
    glutMainLoop();
}

static void MouseClickMessage(int button, int state, int x, int y) {}
static void MouseMoveMessage(int x, int y) {}
static void MousePassiveMoveMessage(int xn, int yn) {}
static void MouseWheelMessage(int wheel, int dir, int x, int y) {}
static void SpecialKeys(int key, int xn, int yn) {
    if(key == GLUT_KEY_F1) {
        y = 3.5;
        z = 8;
    }
}

static void SpecialKeysUp(int key, int xn, int yn) {
    if(key == GLUT_KEY_F1) {
        y = 2.6;
        z = 5;
    }
}

static void KeyboardFunc(unsigned char cAscii, int xn, int yn) {
    if(cAscii == 'w') rw = 1;
    if(cAscii == 's') rs = 1;
    if(cAscii == 'a') ra = 1;
    if(cAscii == 'd') rd = 1;
    if(cAscii == 'p') shoot = 1;
    if(cAscii == 'l') player = (player + 1) % tot_num;
    if(cAscii == 'm') {
        pause = pause ^ 1;
        if(blood > 0 && pause == 0) glutTimerFunc(33, Timer, 1);
    }
    if(cAscii == 'r') {
        xd = 0; zd = 0;
        tot_num = 1; player = 0;
        pause = 0; blood = 100; score_int = 0;
        num_blood =- 1;
        num_zoom = 4, num_evil = 1;
        memset(zoom_x, 0, sizeof(zoom_x)); memset(zoom_z, 0, sizeof(zoom_z));
        memset(evil_x, 0, sizeof(evil_x)); memset(evil_z, 0, sizeof(evil_z));
        random_boss();
        kk = 0, flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0;
        glutTimerFunc(33, Timer, 1);
    }
}

static void KeyboardFuncUp(unsigned char cAscii, int x, int y) {
    if(cAscii == 'w' || cAscii == 'W') rw = 0;
    if(cAscii == 's' || cAscii == 'S') rs = 0;
    if(cAscii == 'a' || cAscii == 'A') ra = 0;
    if(cAscii == 'd' || cAscii == 'D') rd = 0;
    if(cAscii == 'p' || cAscii == 'P') shoot = 0;
    if(cAscii == 'q' || cAscii == 'Q') exit(0);
}

static void ChangeSize(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 30.0);
    glMatrixMode(GL_MODELVIEW);
    
    GLfloat light_position[] = {0.0, 20.0, 0.0};
    GLfloat light_diffuse[] = {0.6, 0.6, 0.6, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, light_diffuse);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat diffuse[] = {0.7, 0.6, 0.6};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuse);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  
    glutPostRedisplay();
}

int judge_shoot(float x, float y, float z) {
    float dx = x - xd, dz = z - zd;
    if(sqrt(dx * dx + dz * dz) >= 1.8) return 0;
    if(sqrt(dx * dx + dz * dz) <= 0.3) return 1;
    if(ra && rs && dx < 0 && dz > 0) return 1;
    if(rd && rs && dx > 0 && dz > 0) return 1;
    if(rw && ra && dx < 0 && dz < 0) return 1;
    if(rw && rd && dx > 0 && dz < 0) return 1;
    if(ra && fabs(dz) < 0.1 && dx < 0) return 1;
    if(rs && fabs(dx) < 0.1 && dz > 0) return 1;
    if(rw && fabs(dx) < 0.1 && dz < 0) return 1;
    if(rd && fabs(dz) < 0.1 && dx > 0) return 1;
    return 0;
}

void draw_shoot() {
    if(player == 0) {
    	float sx =- 0.2, sy = 0.55, sz = 0.8;
    	glLineWidth(4);
    	material[0] = 0; material[1] = 0.2; material[2] = 0.5; material[3] = 1;
    	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    	//glColor3f(0, 0.2, 0.5);
    	glBegin(GL_LINES);
    	if(ra && rs) {
    		sx =- sqrt(sx * sx + sz * sz) * sin(atan( - sx / sz) + PI / 4);
    		sz = sqrt(sx * sx + sz * sz) * cos(atan(-sx / sz) + PI / 4) + 0.25;
    		glVertex3f(sx, sy, sz);glVertex3f(sx-0.2/sqrt(2), sy, sz+0.2/sqrt(2));
    		glVertex3f(sx - 0.4 / sqrt(2), sy, sz + 0.4 / sqrt(2)); glVertex3f(sx - 0.6 / sqrt(2), sy, sz + 0.6 / sqrt(2));
    		glVertex3f(sx, sy, sz); glVertex3f(sx - 0.05 * sqrt(2), sy, sz);
    		glVertex3f(sx - 0.1 * sqrt(2), sy, sz); glVertex3f(sx - 0.15 * sqrt(2), sy, sz);
    		glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz + 0.05 * sqrt(2));
    		glVertex3f(sx, sy, sz + 0.1 * sqrt(2)); glVertex3f(sx, sy, sz + 0.15 * sqrt(2));
    	} else if(rd && rs) {
    		sx += 0.55; sz += -0.05;
        	glVertex3f(sx, sy, sz);glVertex3f(sx + 0.2 / sqrt(2), sy, sz + 0.2 / sqrt(2));
        	glVertex3f(sx + 0.4 / sqrt(2), sy, sz + 0.4 / sqrt(2)); glVertex3f(sx + 0.6 / sqrt(2), sy, sz + 0.6 / sqrt(2));
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.05 * sqrt(2), sy, sz);
        	glVertex3f(sx + 0.1 * sqrt(2), sy, sz); glVertex3f(sx + 0.15 * sqrt(2), sy, sz);
        	glVertex3f(sx, sy, sz);glVertex3f(sx, sy, sz + 0.05 * sqrt(2));
        	glVertex3f(sx, sy, sz + 0.1 * sqrt(2)); glVertex3f(sx, sy, sz + 0.15 * sqrt(2));
    	} else if(rw && ra) {
        	sz -= 1.5; sx -= 0.15;
        	glVertex3f(sx, sy, sz); glVertex3f(sx - 0.2 / sqrt(2), sy, sz - 0.2 / sqrt(2));
        	glVertex3f(sx - 0.4 / sqrt(2), sy, sz - 0.4 / sqrt(2)); glVertex3f(sx - 0.6 / sqrt(2), sy, sz - 0.6 / sqrt(2));
        	glVertex3f(sx, sy, sz);glVertex3f(sx - 0.05 * sqrt(2), sy, sz);
        	glVertex3f(sx - 0.1 * sqrt(2), sy, sz); glVertex3f(sx - 0.15 * sqrt(2), sy, sz);
        	glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz - 0.05 * sqrt(2));
        	glVertex3f(sx, sy, sz - 0.1 * sqrt(2)); glVertex3f(sx, sy, sz - 0.15 * sqrt(2));
    	} else if(rw && rd) {
        	sx =- sqrt(sx * sx + sz * sz) * sin(atan(-sx / sz) + PI / 4); sx =- sx; sx -= 0.05;
        	sz = sqrt(sx * sx + sz * sz) * cos(atan(-sx / sz) + PI / 4) + 0.25; sz =- sz; sz += 0.95;
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.2 / sqrt(2), sy, sz - 0.2 / sqrt(2));
        	glVertex3f(sx + 0.4 / sqrt(2), sy, sz - 0.4 / sqrt(2)); glVertex3f(sx + 0.6 / sqrt(2), sy, sz - 0.6 / sqrt(2));
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.05 * sqrt(2), sy, sz);
        	glVertex3f(sx + 0.1 * sqrt(2), sy, sz); glVertex3f(sx + 0.15 * sqrt(2), sy, sz);
        	glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz - 0.05 * sqrt(2));
        	glVertex3f(sx, sy, sz - 0.1 * sqrt(2)); glVertex3f(sx, sy, sz - 0.15 * sqrt(2));
    	} else if(ra) {
        	float temp = sx; sx = sz; sz = temp; sx =- sx;
        	glVertex3f(sx, sy, sz);glVertex3f(sx - 0.2, sy, sz);
        	glVertex3f(sx - 0.4, sy, sz);glVertex3f(sx - 0.6, sy, sz);
        	glVertex3f(sx, sy, sz);glVertex3f(sx - 0.05, sy, sz + 0.05);
        	glVertex3f(sx - 0.1, sy, sz + 0.1);glVertex3f(sx - 0.15, sy, sz + 0.15);
        	glVertex3f(sx, sy, sz);glVertex3f(sx - 0.05, sy, sz - 0.05);
        	glVertex3f(sx - 0.1, sy, sz - 0.1);glVertex3f(sx - 0.15, sy, sz - 0.15);
        } else if(rd) {
        	float temp = sx; sx = sz; sz = temp; sz =- sz;
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.2, sy, sz);
        	glVertex3f(sx + 0.4, sy, sz); glVertex3f(sx + 0.6, sy, sz);
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.05, sy, sz + 0.05);
        	glVertex3f(sx + 0.1, sy, sz + 0.1); glVertex3f(sx + 0.15, sy, sz + 0.15);
        	glVertex3f(sx, sy, sz);glVertex3f(sx + 0.05, sy, sz - 0.05);
        	glVertex3f(sx + 0.1, sy, sz - 0.1); glVertex3f(sx + 0.15, sy, sz - 0.15);
    	} else if(rw) {
        	sz =- sz; sx =- sx;
        	glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz - 0.2);
        	glVertex3f(sx, sy, sz - 0.4); glVertex3f(sx, sy, sz - 0.6);
        	glVertex3f(sx, sy, sz); glVertex3f(sx - 0.05, sy, sz - 0.05);
        	glVertex3f(sx - 0.1, sy, sz - 0.1); glVertex3f(sx - 0.15, sy, sz - 0.15);
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.05, sy, sz + 0.05);
        	glVertex3f(sx + 0.1, sy, sz - 0.1); glVertex3f(sx + 0.15, sy, sz - 0.15);
    	} else {
        	glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz + 0.2);
        	glVertex3f(sx, sy, sz + 0.4); glVertex3f(sx, sy, sz + 0.6);
        	glVertex3f(sx, sy, sz); glVertex3f(sx - 0.05, sy, sz + 0.05);
        	glVertex3f(sx - 0.1, sy, sz + 0.1); glVertex3f(sx - 0.15, sy, sz + 0.15);
        	glVertex3f(sx, sy, sz); glVertex3f(sx + 0.05, sy, sz + 0.05);
        	glVertex3f(sx + 0.1, sy, sz + 0.1); glVertex3f(sx + 0.15, sy, sz + 0.15);
    	}
    	glEnd();
    } else {
        if(player == 1) {
        	material[0] = 0.7; material[1] = 0; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        } else {
        	material[0] = 0.7; material[1] = 0.7; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        
        float sx = -0.2, sy = 0.55, sz = 0.8;
        glLineWidth(7);
        glBegin(GL_LINES);
        if(ra&&rs) {
            sx = -sqrt(sx * sx + sz * sz) * sin(atan(-sx / sz) + PI / 4) + 0.1;
            sz = sqrt(sx * sx + sz * sz) * cos(atan(-sx / sz) + PI / 4) + 0.25;
            glVertex3f(sx, sy, sz); glVertex3f(sx - 1 / sqrt(2), sy, sz + 1 / sqrt(2));
        } else if(rd && rs) {
            sx += 0.55;
            sz += -0.05;
            glVertex3f(sx, sy, sz); glVertex3f(sx + 1 / sqrt(2), sy, sz + 1 / sqrt(2));
        } else if(rw && ra) {
            sz -= 1.5;
            sx -= 0.15;
            glVertex3f(sx, sy, sz); glVertex3f(sx - 1 / sqrt(2), sy, sz - 1 / sqrt(2));
        } else if(rw && rd) {
            sx =- sqrt(sx * sx + sz * sz) * sin(atan(-sx / sz) + PI / 4); sx =- sx; sx -= 0.05;
            sz = sqrt(sx * sx + sz * sz) * cos(atan(-sx / sz) + PI / 4) + 0.25; sz =- sz; sz += 0.95;
            glVertex3f(sx, sy, sz); glVertex3f(sx + 1 / sqrt(2), sy, sz - 1 / sqrt(2));
        } else if(ra) {
            float temp = sx; sx = sz; sz = temp; sx =- sx;
            glVertex3f(sx, sy, sz); glVertex3f(sx - 1, sy, sz);
        } else if(rd) {
            float temp = sx; sx = sz; sz = temp; sz =- sz;
            glVertex3f(sx, sy, sz); glVertex3f(sx + 1, sy, sz);
        } else if(rw) {
            sz =- sz; sx =- sx;
            glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz - 1);
        } else glVertex3f(sx, sy, sz); glVertex3f(sx, sy, sz + 1);
        glEnd();
    }
}

void draw_player() {
    if(blood <= 0) glRotatef(90, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    //glColor3f(243/255.0, 208/255.0, 177/255.0);
    material[0] = 243 / 255.0;
    material[1] = 208 / 255.0;
    material[2] = 177 / 255.0;
    material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    for(int i = 0; i <= vertices[2].size(); i++) {
        if(i == 132000) {
            //glColor3f(0, 0, 0.9);
            material[0] = 0; material[1] = 0; material[2] = 0.9; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        if(i == 145350) {
            //glColor3f(0, 0, 0);
            material[0] = 79 / 255.0; material[1] = 79 / 255.0; material[2] = 79 / 255.0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        glNormal3f(normals[2][i].x / 500, normals[2][i].y / 500, normals[2][i].z / 500);
        glVertex3f(vertices[2][i].x / 500, vertices[2][i].y / 500, vertices[2][i].z / 500);
    }
    glEnd();
    if(blood <= 0) glRotatef(-90, 0, 0, 1);
}

void draw_player2() {
    if(blood <= 0) glRotatef(90, 0, 0, 1);
    glRotatef(45, 0, 1, 0);
    material[0] = 243 / 255.0; material[1] = 208 / 255.0; material[2] = 177 / 255.0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glBegin(GL_TRIANGLES);
    
    for(int i = 0; i <= vertices[4].size(); i++) {
        if(i == 132000) {
            material[0] = 0; material[1] = 0; material[2] = 0.9; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        if(i == 145350) {
            material[0] = 180 / 255.0; material[1] = 133 / 255.0; material[2] = 83 / 255.0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        if(i == 272000) {
            material[0] = 79 / 255.0; material[1] = 79 / 255.0; material[2] = 79 / 255.0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        glNormal3f(normals[4][i].x / 500, normals[4][i].y / 500, normals[4][i].z / 500);
        glVertex3f(vertices[4][i].x / 500, vertices[4][i].y / 500, vertices[4][i].z / 500);
    }
    glEnd();
    glRotatef(-45, 0, 1, 0);
    if(blood <= 0) glRotatef(-90, 0, 0, 1);
}

void draw_player3() {
    if(blood <= 0) glRotatef(90, 0, 0, 1);
    glRotatef(45, 0, 1, 0);
    material[0] = 243 / 255.0; material[1] = 208 / 255.0; material[2] = 177 / 255.0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glBegin(GL_TRIANGLES);
    
    for(int i = 0; i <= vertices[5].size(); i++) {
        if(i == 132000) {
            material[0] = 79 / 255.0; material[1] = 79 / 255.0; material[2] = 79 / 255.0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        if(i == 133000) {
            material[0] = 98 / 255.0; material[1] = 48 / 255.0; material[2] = 124 / 255.0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        if(i == 146000) {
            material[0] = 0.95; material[1] = 0.95; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }
        glNormal3f(normals[5][i].x / 500, normals[5][i].y / 500, normals[5][i].z / 500);
        glVertex3f(vertices[5][i].x / 500, vertices[5][i].y / 500, vertices[5][i].z / 500);
    }
    glEnd();
    glRotatef(-45, 0, 1, 0);
    if(blood <= 0) glRotatef(-90, 0, 0, 1);
}

void draw_solid_circle(float x, float y, float radius) {
    int count;
    int sections = 200;
    
    GLfloat TWOPI = 2.0f * 3.14159f;
    //glColor3f(0.7, 0, 0);
    material[0] = 0.85; material[1] = 0; material[2] = 0; material[3] = 1;
    glNormal3f(0, 1, 0);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x, 0, y);
    
    for(count = 0; count <= sections; count++)
        glVertex3f(x + radius * cos(count * TWOPI / sections), 0, y + radius * sin(count * TWOPI / sections));
    glEnd();
}

void draw_evil(float x, float y, float z, int k) {
    glTranslatef(x, y, z);
    glRotatef(-90, 0, 1, 0);
    
    float dx = xd - x, dz = zd - z;
    if(fabs(dx) < 0.25 && fabs(dz) < 0.25) blood -= 1;
    float delta = atan(dx / dz);
    if(dx > 0 && dz > 0) glRotatef(delta / PI * 180.0, 0, 1, 0); else
        if(dx > 0 && dz == 0) glRotatef(90, 0, 1, 0); else
            if(dx < 0 && dz == 0) glRotatef(-90, 0, 1, 0); else
                if(dx == 0 && dz < 0) glRotatef(-180.0, 0, 1, 0); else
                    if(dx > 0 && dz < 0) glRotatef(delta / PI * 180.0 + 180, 0, 1, 0); else
                        if(dx < 0 && dz > 0) glRotatef(delta / PI * 180.0, 0, 1, 0); else
                            if(dx < 0 && dz < 0) glRotatef(-180 + delta / PI * 180.0, 0, 1, 0);
    
    material[0] = 1; material[1] = 0; material[2] = 0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    //glColor3f(1, 0, 0);
    glBegin(GL_TRIANGLES);
    for(int i = 0; i <= vertices[0].size(); i++) {
        if(i == 65985) {
            material[0] = 0; material[1] = 0; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(0, 0, 0);
        if(i == 66800) {
            material[0] = 1; material[1] = 165 / 255.0; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(1.0,165/255.0,0.0);
        if(i == 83800) {
            material[0] = 0; material[1] = 0; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(0, 0, 0);
        glNormal3f(normals[0][i].x / 500, normals[0][i].y / 500, normals[0][i].z / 500);
        glVertex3f( vertices[0][i].x / 500, vertices[0][i].y / 500, vertices[0][i].z / 500);
    }
    glEnd();
    if(shoot && judge_shoot(x, y, z)) {
        num_blood = num_blood + 1;
        if(num_blood >= 1000) {
        	num_blood -= 1000;
        	kk = 1;
        }
        blood_x[num_blood] = x + (rand() % 40 - 20) / 100.0;
        blood_z[num_blood] = z + (rand() % 40 - 20) / 100.0;
        blood_r[num_blood] = rand() % 25 / 100.0;
        
        if(player == 0) evil_h[k]++;
        if(player == 1) evil_h[k] += 2;
        if(player == 2) evil_h[k] += 5;
        if(evil_h[k] >= 20) {
            evil_h[k] = 0;
            score_int += 500;
            while(1) {
                int xx = rand() % 8 - 4;
                int zz = rand() % 8 - 4;
                int yy = 0;
                if(judge(xx, yy, zz, -1)) {
                    evil_x[k] = xx; evil_y[k] = yy; evil_z[k] = zz;
                    break;
                }
            }
        }
    }
    
    if(dx > 0 && dz > 0) glRotatef(-delta / PI * 180.0, 0, 1, 0); else
        if(dx > 0 && dz == 0) glRotatef(-90, 0, 1, 0); else
            if(dx < 0 && dz == 0) glRotatef(90, 0, 1, 0); else
                if(dx == 0 && dz < 0) glRotatef(180.0, 0, 1, 0); else
                    if(dx > 0 && dz < 0) glRotatef(-delta / PI * 180.0 - 180, 0, 1, 0); else
                        if(dx < 0 && dz > 0) glRotatef(-delta / PI * 180.0, 0, 1, 0); else
                            if(dx < 0 && dz < 0) glRotatef(180 - delta / PI * 180.0, 0, 1, 0); glRotatef(90, 0, 1, 0);
    glTranslatef(-x, -y, -z);
}

void draw_zoomb(float x, float y, float z, int k)
{
    glTranslatef(x, y, z);
    glRotatef(-45, 0, 1, 0);
    
    float dx = xd - x, dz = zd - z;
    if(fabs(dx) < 0.25 && fabs(dz) < 0.25) blood -= 1;
    
    float delta = atan(dx / dz);
    if(dx > 0 && dz > 0) glRotatef(delta / PI * 180.0, 0, 1, 0); else
        if(dx > 0 && dz == 0) glRotatef(90, 0, 1, 0); else
            if(dx < 0 && dz == 0) glRotatef(-90, 0, 1, 0); else
                if(dx == 0 && dz < 0) glRotatef(-180.0, 0, 1, 0); else
                    if(dx > 0 && dz < 0) glRotatef(delta / PI * 180.0 + 180, 0, 1, 0); else
                        if(dx < 0 && dz > 0) glRotatef(delta / PI * 180.0, 0, 1, 0); else
                            if(dx < 0 && dz < 0) glRotatef(-180 + delta / PI * 180.0, 0, 1, 0);
    
    glBegin(GL_TRIANGLES);
    material[0] = 0; material[1] = 87 / 255.0; material[2] = 55 / 255.0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    //glColor3f(0, 87/255.0, 55/255.0);
    for(int i = 0; i <= vertices[1].size(); i++) {
        if(i == 13400) {
            material[0] = 0; material[1] = 0.5; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(0, 0.5, 0);
        if(i == 17000) {
            material[0] = 0; material[1] = 87 / 255.0; material[2] = 55 / 255.0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(0, 87/255.0, 55/255.0);
        if(i==80000) {
            material[0]=1;    material[1]=1;    material[2]=1; material[3]=1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(1, 1, 1);
        if(i == 140000) {
            material[0] = 0; material[1] = 0; material[2] = 0; material[3] = 1;
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
        }//glColor3f(0, 0, 0);
        glNormal3f(normals[1][i].x / 500, normals[1][i].y / 500, normals[1][i].z / 500);
        glVertex3f(vertices[1][i].x / 500, vertices[1][i].y / 500, vertices[1][i].z / 500);
    }
    glEnd();
    if(shoot && judge_shoot(x, y, z)) {
        if(player == 0) zoom_h[k]++;
        if(player == 1) zoom_h[k] += 2;
        if(player == 2) zoom_h[k] += 5;
        
        num_blood = num_blood + 1;
        if(num_blood >= 1000) {
        	num_blood -= 1000;
        	kk = 1;
        }
        blood_x[num_blood] = x + (rand() % 40 - 20) / 100.0;
        blood_z[num_blood] = z + (rand() % 40 - 20) / 100.0;
        blood_r[num_blood] = rand() % 25 / 100.0;
        if(zoom_h[k] >= 5) {
            zoom_h[k] = 0;
            score_int += 100;
            while(1) {
                int xx = rand() % 8 - 4;
                int zz = rand() % 8 - 4;
                int yy = 0;
                if(judge(xx, yy, zz, -1)) {
                    zoom_x[k] = xx;
                    zoom_y[k] = yy;
                    zoom_z[k] = zz;
                    break;
                }
            }
        }
    }
    if(dx > 0 && dz > 0) glRotatef(-delta / PI * 180.0, 0, 1, 0); else
        if(dx > 0 && dz == 0) glRotatef(-90, 0, 1, 0); else
            if(dx < 0 && dz == 0) glRotatef(90, 0, 1, 0); else
                if(dx == 0 && dz < 0) glRotatef(180.0, 0, 1, 0); else
                    if(dx > 0 && dz < 0) glRotatef(-delta / PI * 180.0 - 180, 0, 1, 0); else
                        if(dx < 0 && dz > 0) glRotatef(-delta / PI * 180.0, 0, 1, 0); else
                            if(dx < 0 && dz < 0) glRotatef(180 - delta / PI * 180.0, 0, 1, 0);
    glRotatef(45, 0, 1, 0);
    glTranslatef(-x, -y, -z);
}

void draw_cube(float x, float z, int kind) {
    glTranslatef(x, 0.1, z);
    glRotatef(-45, 0, 1, 0);
    
    glBegin(GL_TRIANGLES);
    if(kind <= 45) {
    	material[0] = 1; material[1] = 1; material[2] = 0; material[3] = 1;
    	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    } else {
        material[0] = 0; material[1] = 1; material[2] = 0; material[3] = 1;
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    }
    glutSolidCube(0.25);
    glRotatef(45, 0, 1, 0);
    glTranslatef(-x, -0.1, -z);

}

void draw_boom(float x, float y, float z) {
    if(fabs(x - xd) < 0.1 && fabs(z - zd) < 0.1) blood -= 5;
    glTranslatef(x, 0.5, z);
    //glColor3f(0.95, 0.95, 0);
    material[0] = 0.95; material[1] = 0.95; material[2] = 0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glutSolidSphere(0.1, 20, 20);
    glTranslatef(-x, -0.5, -z);
}

static void RenderScene(void) {
    if(score_int >= 5000 && flag3 == 0) {num_zoom = 5; num_evil = 2; random_boss(); flag3 = 1;}
    if(score_int >= 50000 && flag4 == 0) {num_zoom = 3; num_evil = 5; random_boss(); flag4 = 1;}
    if(score_int >= 100000 && flag5 == 0) {num_evil = 8; num_zoom = 0; random_boss(); flag5 = 1;}
    
    static bool init = 0;
    static GLuint haze; //Texture for sky
    if(!init) {
        init = 1;
        haze = maketex(BKGRD_FILE, 512, 512);
    }
    
    if(rw&&judge(xd, 0, zd - 0.1, -num_zoom - num_evil)) {
    	zd = zd - 0.1;
        //z = z - 0.1;
    }
    if(rs&&judge(xd, 0, zd + 0.1, -num_zoom - num_evil)) {
    	zd = zd + 0.1;
        //z = z + 0.1;
    }
    if(ra && judge(xd - 0.1, 0, zd, -num_zoom - num_evil)) {
    	xd = xd - 0.1;
        //x = x - 0.1;
    }
    if(rd && judge(xd + 0.1, 0, zd, -num_zoom - num_evil)) {
    	xd = xd + 0.1;
        //x = x + 0.1;
    }
    if(cube == 1 && fabs(cx - xd) <= 0.3 && fabs(cy - zd) <= 0.3) {
        if(kind <= 45) tot_num += 1;
        if(tot_num > 3) tot_num = 3;
        if(kind > 45) blood += 20;
        if(blood > 100) blood = 100;
        cube = 0;
    }
    
    glClearColor(1.0f, 1.0f, 1.0f, 0);
    glColor3f(1.0, 1.0, 1.0);
    glLoadIdentity();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    sky(haze); //Draw sky
    glDisable(GL_TEXTURE_2D);
    gluLookAt(x, y, z, 0, 0, 0, 0.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    
    glTranslatef(xd, yd, zd);
    if(shoot && pshoot) draw_shoot();
    printf("%d\n", blood);

    glNormal3f(0, 0, 1);
    material[0] = 0; material[1] = 0.9; material[2] = 0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glBegin(GL_QUADS); //Draw blood
    glVertex3f(-0.3f, 1.0, 0.0f);
    glVertex3f(blood / 100.0 * 0.6 - 0.3, 1.0f, 0.0f);
    glVertex3f(blood / 100.0 * 0.6 - 0.3, 0.9f, 0.0f);
    glVertex3f(-0.3f, 0.9f, 0.0f);
    glEnd();
    material[0] = 0; material[1] = 0; material[2] = 0; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    glBegin(GL_QUADS);
    glVertex3f(blood / 100.0 * 0.6 - 0.3, 1.0f, 0.0f);
    glVertex3f( 0.3f, 1.0f, 0.0f);
    glVertex3f( 0.3f, 0.9f, 0.0f);
    glVertex3f(blood / 100.0 * 0.6 - 0.3, 0.9f, 0.0f);
    glEnd();
    
    glRotatef(-45, 0, 1, 0);
    if(ra && rs) glRotatef(-45, 0, 1, 0);
    else if(rd && rs) glRotatef(45, 0, 1, 0);
    else if(rw && ra) glRotatef(-135, 0, 1, 0);
    else if(rw && rd) glRotatef(135, 0, 1, 0);
    else if(ra) glRotatef(-90, 0, 1, 0);
    else if(rd) glRotatef(90, 0, 1, 0);
    else if(rw) glRotatef(-180, 0, 1, 0);
    
    if (player == 0) draw_player();
    if (player == 1) draw_player2();
    if (player == 2) draw_player3();
    
    if(ra && rs) glRotatef(45, 0, 1, 0);
    else if(rd && rs) glRotatef(-45, 0, 1, 0);
    else if(rw && ra) glRotatef(135, 0, 1, 0);
    else if(rw && rd) glRotatef(-135, 0, 1, 0);
    else if(ra) glRotatef(90, 0, 1, 0);
    else if(rd) glRotatef(-90, 0, 1, 0);
    else if(rw) glRotatef(180, 0, 1, 0);
    glRotatef(45, 0, 1, 0);
    
    glTranslatef(-xd, -yd, -zd);
    
    for(int i = 0; i < num_zoom; i++) draw_zoomb(zoom_x[i], zoom_y[i], zoom_z[i], i);
    for(int i = 0; i < num_evil; i++) draw_evil(evil_x[i], evil_y[i], evil_z[i], i);
    for(int i = 0; i < num_evil; i++) draw_boom(boom_x[i],0,boom_z[i]);
    if(kk == 0) {
    	for(int i = 0; i <= num_blood; i++) draw_solid_circle(blood_x[i], blood_z[i], blood_r[i]);
    } else {
    	for(int i = 0; i <= 1000; i++) draw_solid_circle(blood_x[i], blood_z[i], blood_r[i]);
    }
    
    if (cube == 1) draw_cube(cx, cy, kind);

    material[0] = 1; material[1] = 1; material[2] = 1; material[3] = 1;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
    
    score = "";
    char p;
    int k = score_int;
    while(k > 0) {
    	p = '0' + (k % 10);
    	k /= 10;
    	score = p + score;
    }
    score = "YOUR SHIT SCORE: " + score;
    printf("%s\n", score.c_str());
    char* s = (char*)malloc(sizeof(score));
    int i;
    for(i = 0; score[i] != '\0'; i++) s[i] = score[i];
    s[i] = '\0';
    output(-4, -5, s);
    
    if (blood <= 0) output(2, -5, "GAME OVER");

    glFlush();
    glutSwapBuffers();
}

static void Timer(int value) {
    if(tot_num == 3) super_time++;
    if(super_time >= 200) {
    	tot_num = 2;
    	player = 1;
    	super_time = 0;
    }
    tot_time = (tot_time + 1) % 200;
    if(tot_time == 199 && cube == 0) {
    	cube = 1;
    	cx = rand() % 5 - 2;
    	cy = rand() % 5 - 2;
    	kind = rand() % 100;
    }
    if(shoot) pshoot = pshoot ^ 1;
    float xx, yy, zz, dx, dz;
    for(int i = 0; i < num_evil; i++) {
        if(boom_x[i] == -100 && boom_z[i] == -100 &&
           sqrt((xd - evil_x[i]) * (xd - evil_x[i]) + (zd - evil_z[i]) * (zd - evil_z[i])) < 8) {
        	boom_x[i] = evil_x[i];
            boom_z[i] = evil_z[i];
            bdx[i] = xd - boom_x[i];
            bdz[i] = zd - boom_z[i];
        }
        if(!(boom_x[i] == 100 && boom_z[i] == -100)){
            dx = boom_x[i] - xd; dz = boom_z[i] - zd;
            boom_x[i] = boom_x[i] + bdx[i] / sqrt(bdx[i] * bdx[i] + bdz[i] * bdz[i]) * 0.2;
            boom_z[i] = boom_z[i] + bdz[i] / sqrt(bdx[i] * bdx[i] + bdz[i] * bdz[i]) * 0.2;
            if(sqrt(dx * dx + dz * dz) > 8) {
            	boom_x[i] =- 100;
            	boom_z[i] =- 100;
            }
        }
    }
    for(int i = 0; i < num_zoom; i++) {
        dx = xd - zoom_x[i]; dz = zd - zoom_z[i];
        xx = zoom_x[i] + dx / sqrt(dx * dx + dz * dz) * 0.05;
        zz = zoom_z[i] + dz / sqrt(dx * dx + dz * dz) * 0.05;
        yy = 0;
        if(judge(xx, yy, zz, i)) {
            zoom_x[i] = xx;
            zoom_y[i] = yy;
            zoom_z[i] = zz;
        }
    }
    for(int i = 0; i < num_evil; i++) {
        dx = xd - evil_x[i]; dz = zd - evil_z[i];
        xx = evil_x[i] + dx / sqrt(dx * dx + dz * dz) * 0.03;
        zz = evil_z[i] + dz / sqrt(dx * dx + dz * dz) * 0.03;
        yy = 0;
        if(judge(xx, yy, zz, i + num_zoom)) {
            evil_x[i]=xx;
            evil_y[i]=yy;
            evil_z[i]=zz;
        }
    }
    glutPostRedisplay();
    if(blood > 0 && pause == 0) glutTimerFunc(33, Timer, value);
    else glutPostRedisplay();
}