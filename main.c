#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include "glm.h"

#define bool int
#define true 1
#define false 0

double pas = 1.6;
int selected = -1;
int moved = -1;

int ww;
int hw;
int j,i;
GLMmodel* chessBoard;
GLMmodel  *bRoi, *bRenne, *bTour[2], *bFou[2], *bCheval[2], *bPion[8];
GLMmodel  *wRoi, *wRenne, *wTour[2], *wFou[2], *wCheval[2], *wPion[8];

int deplacements[32]={0};
int jeu[8][8]=
{
    {5,13,9,1,3,11,15,7},
    {17,19,21,23,25,27,29,31},
    {99,99,99,99,99,99,99,99},
    {99,99,99,99,99,99,99,99},
    {99,99,99,99,99,99,99,99},
    {99,99,99,99,99,99,99,99},
    {16,18,20,22,24,26,28,30},
    {4,12,8,0,2,10,14,6}

};
int grille[8][8];
int grilleFirst=51;


/* Utile si on utilse une projection Ortho */
double _left = 0.0;
double _right = 0.0;
double _bottom = 1.0;
double _top = -1.0;

/* Utilisation de la souris  */
int _mouseX = 0;
int _mouseY = 0;
bool _mouseLeft = false;
bool _mouseMiddle = false;
bool _mouseRight = false;

double _dragPosX = 0.0;
double _dragPosY = 0.0;
double _dragPosZ = 0.0;

/* Pour lire les matrices Modelview  */
double _matrix[16];
double _matrixI[16];


/* variables d'illumination  */
GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_position[] = { 0.0, 10.0, 10, 0.0 };


// Propriete materielle Or
GLfloat or_amb[] = {0.24725, 0.1995, 0.0745, 1.0};
GLfloat or_diff[] = {0.75164, 0.60648, 0.22648, 1.0};
GLfloat or_spec[] = {0.628281, 0.555802, 0.366065, 1.0};
GLfloat or_shin = 51.2;

// Propriete materielle Aluminium
GLfloat alu_amb[] = { 0.0,   0.0,     0.0,  1.0};
GLfloat alu_diff[] = {  0.55,  0.55,    0.55, 1.0};
GLfloat alu_spec[] = {0.70,  0.70,    0.70, 1.0};
GLfloat alu_shin = 32;




double fovy = 60;
double _zNear = 4, _zFar=100;
int translatePion = 0;







void display(void);
void init(void);
void reshape(int w, int h);
void clavier(unsigned char key, int x, int y);
void souris(int button, int state, int x, int y);
void picking(int x, int y);
void list_hits(GLint hits, GLuint *names);
void gl_selall(GLint hits, GLuint *buff);
void motion(int x, int y);
double module(double x, double y, double z);
void pos(double *px, double *py, double *pz, const int x, const int y,const int *viewport);
void getMatrix();
void invertMatrix(const GLdouble * m, GLdouble * out);
void highlight(GLMmodel *model);
void selection(GLMmodel *model, int name);




int main(int argc, char **argv){


    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(284, 80);


    glutCreateWindow("[OpenGL] Application X");


    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(souris);
    glutMotionFunc(motion);
    glutKeyboardFunc(clavier);
    glutIdleFunc(NULL);


    glutMainLoop();

    return 0;
}

void init(void){

    //Initialiser la grille
    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            grille[i][j]=grilleFirst;
            printf("%d", grilleFirst);
            grilleFirst++;
        }
        printf("\n");
    }

    glClearColor(0.0,0.0,0.0,0.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDepthRange(0.0,1.0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glMatrixMode(GL_MODELVIEW);





    chessBoard = glmReadOBJ("extends/chessboard.obj");
    wRoi=glmReadOBJ("extends/wRoi.obj");
    wRenne=glmReadOBJ("extends/wRenne.obj");

    bRoi=glmReadOBJ("extends/bRoi.obj");
    bRenne=glmReadOBJ("extends/bRenne.obj");

    int compt;

    for(compt=0;compt<2;compt++){
        wTour[compt] = glmReadOBJ("extends/wTour.obj");
        wFou[compt] = glmReadOBJ("extends/wFou.obj");
        wCheval[compt] = glmReadOBJ("extends/wCheval.obj");

        bTour[compt] = glmReadOBJ("extends/bTour.obj");
        bFou[compt] = glmReadOBJ("extends/bFou.obj");
        bCheval[compt] = glmReadOBJ("extends/bCheval.obj");
    }


    for(compt=0;compt<8;compt++){
        wPion[compt] = glmReadOBJ("extends/wPion.obj");
        bPion[compt] = glmReadOBJ("extends/bPion.obj");
    }







}

void axes(void){
    glPushMatrix();

    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(50,0,0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,50,0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,50);
    glEnd();
    glPopMatrix();

   glClearColor (0.0, 0.0, 0.0, 0.0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_TEXTURE_2D);


}

void display(void){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //axes();
    GLfloat dims[3];
    glmDimensions(chessBoard,dims);
    glTranslatef(-dims[0]/2,-dims[1], dims[2]/2);




    glPushMatrix();

    glTranslatef(2.35,0,-2.15);

    // La table
    GLfloat blackA[3]={0,0,0};
    GLfloat blackD[3]={0.01,0.01,0.01};
    GLfloat blackS[3]={0.5,0.5,0.5};
    GLfloat blackSh=0.1*128;

    GLfloat whiteA[3]={1,1,1};
    GLfloat whiteD[3]={1,1,1};
    GLfloat whiteS[3]={1,1,1};
    GLfloat whiteSh=0.7;

    int caseId=0;

    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            caseId++;
            if((i+j)%2==0){

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blackA);
                //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blackD);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blackS);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &blackSh);
            }
            else{

               glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, whiteA);
               glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &whiteSh);
            }
            glLoadName(caseId+50);
            glutSolidCube(pas);
            glTranslatef(pas,0,0);
        }
        glTranslatef(-8*pas,0,-pas);
    }

    glPopMatrix();

    /*
    glLoadName(99);
    glmDraw(chessBoard, GLM_SMOOTH|GLM_TEXTURE|GLM_COLOR);

    */




    if(selected == 0)
        selection(wRoi, 0);
    else {
        glLoadName(0);
        glmDraw(wRoi, GLM_SMOOTH| GLM_MATERIAL);
    }

    if(selected == 2)
        selection(wRenne, 2);
    else{
        glLoadName(2);
        glmDraw(wRenne, GLM_SMOOTH| GLM_MATERIAL);
    }


    if(selected == 1)
        selection(bRoi, 1);
    else{
        glLoadName(1);
        glmDraw(bRoi, GLM_SMOOTH| GLM_MATERIAL);
    }


    if(selected == 3)
        selection(bRenne, 3);
    else{
        glLoadName(3);
        glmDraw(bRenne, GLM_SMOOTH| GLM_MATERIAL);
    }

    glPushMatrix();
    for(i=0;i<2;i++){

        if(selected == 2*(i+2))
            selection(wTour[i], 2*(i+2));
        else{
            glLoadName(2*(i+2));
            glmDraw(wTour[i], GLM_SMOOTH|GLM_MATERIAL);
        }


        if(selected == (2*(i+2))+1)
            selection(bTour[i], 2*(i+2)+1);
        else{
            glLoadName(2*(i+2)+1);
            glmDraw(bTour[i], GLM_SMOOTH| GLM_MATERIAL);
        }

        glTranslatef((double)7*pas,0,0);
    }
    glPopMatrix();
    glPushMatrix();
    for(i=0;i<2;i++){

        if(selected == (2*(i+4)))
            selection(wFou[i], 2*(i+4));
        else{
            glLoadName(2*(i+4));
            glmDraw(wFou[i], GLM_SMOOTH| GLM_MATERIAL);
        }


        if(selected == (2*(i+4))+1)
            selection(bFou[i], 2*(i+4)+1);
        else{
            glLoadName(2*(i+4)+1);
            glmDraw(bFou[i], GLM_SMOOTH| GLM_MATERIAL);
        }

        glTranslatef((double)3*pas,0,0);
    }
    glPopMatrix();
    glPushMatrix();
    for(i=0;i<2;i++){

        if(selected == 2*(i+6))
            selection(wCheval[i], (2*(i+6)));
        else{
            glLoadName(2*(i+6));
            glmDraw(wCheval[i], GLM_SMOOTH| GLM_MATERIAL);
        }


        if(selected == (2*(i+6))+1)
            selection(bCheval[i], (2*(i+6))+1);
        else{
            glLoadName(2*(i+6)+1);
            glmDraw(bCheval[i], GLM_SMOOTH| GLM_MATERIAL);
        }

        glTranslatef((double)5*pas,0,0);
    }
    glPopMatrix();


    glPushMatrix();
    for(i=0;i<8;i++){

        if(selected == 2*(i+8))
            selection(wPion[i], 2*(i+8));
        else{
            glPushMatrix();
            glLoadName(2*(i+8));
            if(translatePion>0 && moved==2*(i+8))
                glTranslatef(0,0,-translatePion*pas);
            glmDraw(wPion[i], GLM_SMOOTH| GLM_MATERIAL);
            glPopMatrix();
        }


        if(selected == 2*(i+8)+1)
            selection(bPion[i], 2*(i+8)+1);
        else{
            glLoadName(2*(i+8)+1);
            glmDraw(bPion[i], GLM_SMOOTH| GLM_MATERIAL);
        }

        glTranslatef(pas,0,0);
    }
    glPopMatrix();


    glutSwapBuffers();

}



void reshape(int w, int h){
    glViewport(0, 0, w, h);

    ww = w;
    hw = h;



    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(_left,_right,_bottom,_top,_zNear,_zFar);
    gluPerspective(fovy, (double) w / (double) h, _zNear, _zFar);
    gluLookAt(11.,11.,11.,0.,0.,0.,0.,1.,0.);
    glMatrixMode(GL_MODELVIEW);
}

void clavier(unsigned char key, int x, int y){
    if(key == 'q')
        exit(0);
}

void souris(int button, int state, int x, int y){

    int viewport[4];

    _mouseX = x;
    _mouseY = y;

    if (state == GLUT_UP)
	switch (button) {
	case GLUT_LEFT_BUTTON:
	    _mouseLeft = false;
	    break;
	case GLUT_MIDDLE_BUTTON:
	    _mouseMiddle = false;
	    break;
	case GLUT_RIGHT_BUTTON:
	    _mouseRight = false;
	    break;
    } else
	switch (button) {
	case GLUT_LEFT_BUTTON:
	    _mouseLeft = true;
	    picking(x, hw-y);
	    break;
	case GLUT_MIDDLE_BUTTON:
	    _mouseMiddle = true;
	    break;
	case GLUT_RIGHT_BUTTON:
	    _mouseRight = true;
	    break;
	}

    glGetIntegerv(GL_VIEWPORT, viewport);
    pos(&_dragPosX, &_dragPosY, &_dragPosZ, x, y, viewport);

}
 void picking(int x, int y)
 {
 	GLuint buff[64] = {0};
 	GLint hits, view[4];
 	int id;
 	glSelectBuffer(64, buff);
 	glGetIntegerv(GL_VIEWPORT, view);
 	glRenderMode(GL_SELECT);
 	glInitNames();
 	glPushName(0);
 	glMatrixMode(GL_PROJECTION);
 	glPushMatrix();
        glLoadIdentity();

        gluPickMatrix(x, y, 1.0, 1.0, view);
        gluPerspective(fovy, (double) ww/(double) hw, _zNear, _zFar);
        gluLookAt(11.,11.,11.,0.,0.,0.,0.,1.,0.);


        glMatrixMode(GL_MODELVIEW);
        glutSwapBuffers();
        display();

 		glMatrixMode(GL_PROJECTION);
 	glPopMatrix();

 	hits = glRenderMode(GL_RENDER);
 	list_hits(hits, buff);






 	//gl_selall(hits, buff);//logs


 	glMatrixMode(GL_MODELVIEW);
 }

 void gl_selall(GLint hits, GLuint *buff)
 {
 	GLuint *p;
 	int i;

 	glLoadName(1);
    glmDraw(bRoi, GL_RENDER);
    glLoadName(2);
    glmDraw(wRoi, GL_RENDER);
    glLoadName(3);
    glmDraw(bRenne, GL_RENDER);
    glLoadName(4);
    glmDraw(wRenne, GL_RENDER);

 	p = buff;
 	for (i = 0; i < 6 * 4; i++)
 	{
 		printf("Slot %d: - Value: %d\n", i, p[i]);
 	}

 	printf("Buff size: %x\n", (GLbyte)buff[0]);
 }



  void list_hits(GLint hits, GLuint *names)
 {
 	int i,realOne=999;
 	float zmin=5;
 	printf("%d hits:\n", hits);

 	for (i = 0; i < hits; i++){
 	    if((float)names[i * 4 + 1]/0x7fffffff<zmin){
 	        zmin=(float)names[i * 4 + 1]/0x7fffffff;

                realOne = (int)names[i * 4 + 3];

        }

 		printf(	"Number: %d\n"
 				"Min Z: %f\n"
 				"Max Z: %f\n"
 				"Name on stack: %d\n",
 				(GLubyte)names[i * 4],
 				(float)names[i * 4 + 1]/0x7fffffff,
 				(float)names[i * 4 + 2]/0x7fffffff,
 				(GLubyte)names[i * 4 + 3]
 				);
 	}
    if(selected>0 && selected<51){
        // Deplacement
        printf("deplacement\n");
        //if(selected == 30){
            moved = selected;
            translatePion = 2;
            glutPostRedisplay();

        //}

        //glutSolidCube(2*pas);
        selected = names[3];
    }
    else
        selected = realOne;
 	printf("RealOne: %d \n", selected);
 	glutPostRedisplay();
    printf("\n");


 }
 void motion(int x, int y)
{
    bool changed = false;

    const int dx = x - _mouseX;
    const int dy = y - _mouseY;

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    if (dx == 0 && dy == 0)
	return;

    if (_mouseMiddle || (_mouseLeft && _mouseRight)) {

	glLoadIdentity();
	glTranslatef(0, 0, dy * 0.01);
	glMultMatrixd(_matrix);

	changed = true;
    } else if (_mouseLeft) {
	double ax, ay, az;
	double bx, by, bz;
	double angle;

	ax = dy;
	ay = dx;
	az = 0.0;
	angle = module(ax, ay, az) / (double) (viewport[2] + 1) * 180.0;

	/* determiner les axes de rotation */

	bx = _matrixI[0] * ax + _matrixI[4] * ay + _matrixI[8] * az;
	by = _matrixI[1] * ax + _matrixI[5] * ay + _matrixI[9] * az;
	bz = _matrixI[2] * ax + _matrixI[6] * ay + _matrixI[10] * az;

	glRotatef(angle, bx, by, bz);

	changed = true;
    } else if (_mouseRight) {
	double px, py, pz;

	pos(&px, &py, &pz, x, y, viewport);

	glLoadIdentity();
	glTranslatef(px - _dragPosX, py - _dragPosY, pz - _dragPosZ);
	glMultMatrixd(_matrix);

	_dragPosX = px;
	_dragPosY = py;
	_dragPosZ = pz;

	changed = true;
    }

    _mouseX = x;
    _mouseY = y;

    if (changed) {
	getMatrix();
	glutPostRedisplay();
    }
}
double module(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}
void pos(double *px, double *py, double *pz, const int x, const int y,
	 const int *viewport)
{
    /*
       Informations de projection et de viewport pour
       à partir de la position de la souris.
     */

    *px = (double) (x - viewport[0]) / (double) (viewport[2]);
    *py = (double) (y - viewport[1]) / (double) (viewport[3]);

    *px = _left + (*px) * (_right - _left);
    *py = _top + (*py) * (_bottom - _top);
    *pz = _zNear;
}
void getMatrix()
{
    glGetDoublev(GL_MODELVIEW_MATRIX, _matrix);
    invertMatrix(_matrix, _matrixI);
}
/*  Calcule de la matrice inverse */
void invertMatrix(const GLdouble * m, GLdouble * out)
{

#define MAT(m,r,c) (m)[(c)*4+(r)]
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

    GLdouble det;
    GLdouble d12, d13, d23, d24, d34, d41;
    GLdouble tmp[16];

    d12 = (m31 * m42 - m41 * m32);
    d13 = (m31 * m43 - m41 * m33);
    d23 = (m32 * m43 - m42 * m33);
    d24 = (m32 * m44 - m42 * m34);
    d34 = (m33 * m44 - m43 * m34);
    d41 = (m34 * m41 - m44 * m31);

    tmp[0] = (m22 * d34 - m23 * d24 + m24 * d23);
    tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
    tmp[2] = (m21 * d24 + m22 * d41 + m24 * d12);
    tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);

    det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];


    if (det == 0.0) {
    } else {
	GLdouble invDet = 1.0 / det;


	tmp[0] *= invDet;
	tmp[1] *= invDet;
	tmp[2] *= invDet;
	tmp[3] *= invDet;

	tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
	tmp[5] = (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
	tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
	tmp[7] = (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

	d12 = m11 * m22 - m21 * m12;
	d13 = m11 * m23 - m21 * m13;
	d23 = m12 * m23 - m22 * m13;
	d24 = m12 * m24 - m22 * m14;
	d34 = m13 * m24 - m23 * m14;
	d41 = m14 * m21 - m24 * m11;

	tmp[8] = (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
	tmp[9] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
	tmp[10] = (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
	tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
	tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
	tmp[13] = (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
	tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
	tmp[15] = (m31 * d23 - m32 * d13 + m33 * d12) * invDet;

	memcpy(out, tmp, 16 * sizeof(GLdouble));
    }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}

void highlight(GLMmodel *model){
    glPushMatrix();
    GLfloat emisss[4] = {0.2, 0.2, 0.2,1};
    GLfloat emisssD[4] = {0, 0, 0,1};
    //glColor3f(0,0,0);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisss);
    glTranslatef(0,0.5,0);
    glmDraw(model, GLM_SMOOTH | GLM_COLOR);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisssD);
    glPopMatrix();
}
void selection(GLMmodel *model, int name){
    if(selected>=0 && selected<51)// Si c un objet
        highlight(model);

    // deplacements possibles
    glPushMatrix();

    /*
    printf("PosX Pion == %f\n", model->vertices[model->triangles[model->groups->triangles[10]].vindices[0]+0]);
    printf("PosY Pion == %f\n", model->vertices[model->triangles[model->groups->triangles[10]].vindices[0]+1]);
    printf("PosZ Pion == %f\n", model->vertices[model->triangles[model->groups->triangles[10]].vindices[0]+2]);
    */


    glColor4f(0,0.4,0,0.5);
    glTranslatef(pas/2,pas, pas/2);

    //Pion
    if(name>15 && name<32){
        glPushMatrix();

        int posLigne = 0;
        int posColonne=0;
        int posCaseDevant=0;
        int i,j;

        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                if(name==jeu[i][j]){
                    posLigne=i;
                    posColonne=j;
                }
            }
        }

        if(name%2==0){
            glTranslatef(pas+0.1, -pas/1.5, -3*pas+0.24);
            posCaseDevant=-1;
        }
        else{
            glTranslatef(pas+0.1, -pas/1.5, -8*pas+0.24);
            posCaseDevant=+1;
        }
        // Avancer
        if(jeu[posLigne+posCaseDevant][posColonne]==99){
            glPushMatrix();
            glTranslatef(0,0,posCaseDevant*pas);
            glutSolidCube(pas);
            glPopMatrix();
            if(deplacements[name]==0){ // Deuxieme case si jamais deplacé
                if(jeu[posLigne+2*posCaseDevant][posColonne]==99){
                    glPushMatrix();
                    glTranslatef(0,0,2*posCaseDevant*pas);
                    glutSolidCube(pas);
                    glPopMatrix();
                }
            }
        }
        //Manger
        if(jeu[posLigne+posCaseDevant][posColonne+1]!=99 && (jeu[posLigne+posCaseDevant][posColonne+1]%2 != name%2)){ //Ennemi reperé
            glPushMatrix();
            glTranslatef(pas,0,posCaseDevant*pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(jeu[posLigne+posCaseDevant][posColonne-1]!=99 && (jeu[posLigne+posCaseDevant][posColonne-1]%2 != name%2)){ //Ennemi Blanc reperé
            glPushMatrix();
            glTranslatef(-pas,0,posCaseDevant*pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        glPopMatrix();
    }
    //Cheval
    else if(name>11){
        glPushMatrix();
        if(name%2==0)
            glTranslatef(2*pas+0.1, -pas/1.5, -2*pas+0.24); // Positionnement du curseur sur le cheval
        else
            glTranslatef(2*pas+0.1, -pas/1.5, -9*pas+0.24);

            int posLigne = 0;
            int posColonne=0;
            int i,j;
            for(i=0;i<8;i++){
                for(j=0;j<8;j++){
                    if(name==jeu[i][j]){
                        posLigne=i;
                        posColonne=j;
                    }
                }
            }


            if(posLigne<6 && posColonne <7 && (jeu[posLigne+2][posColonne+1]==99 || jeu[posLigne+2][posColonne+1]%2 != name%2)){
                glPushMatrix();
                glTranslatef(pas,0,2*pas);
                glutSolidCube(pas);
                glPopMatrix();
            }
            if(posLigne<6 && posColonne >0 && (jeu[posLigne+2][posColonne-1]==99 || jeu[posLigne+2][posColonne-1]%2 != name%2)){
                glPushMatrix();
                glTranslatef(-pas,0,2*pas);
                glutSolidCube(pas);
                glPopMatrix();
            }

            if(posLigne>1 && posColonne>0 && (jeu[posLigne-2][posColonne-1]==99 || jeu[posLigne-2][posColonne-1]%2 != name%2)){
                glPushMatrix();
                glTranslatef(-pas,0,-2*pas);
                glutSolidCube(pas);
                glPopMatrix();
            }
            if(posLigne>1 && posColonne<7 && (jeu[posLigne-2][posColonne+1]==99 || jeu[posLigne-2][posColonne+1]%2 != name%2)){
                glPushMatrix();
                glTranslatef(pas,0,-2*pas);
                glutSolidCube(pas);
                glPopMatrix();
            }

            if(posColonne>1 && posLigne>0 && ((jeu[posLigne-1][posColonne-2]==99) || (jeu[posLigne-1][posColonne-2]%2 != name%2))){
                glPushMatrix();
                glTranslatef(-2*pas,0,-pas);
                glutSolidCube(pas);
                glPopMatrix();
            }
            if(posColonne<6 && posLigne>0 && ((jeu[posLigne-1][posColonne+2]==99) || (jeu[posLigne-1][posColonne+2]%2 != name%2))){
                glPushMatrix();
                glTranslatef(2*pas,0,-pas);
                glutSolidCube(pas);
                glPopMatrix();
            }
            if(posLigne<7 && posColonne <6 && (jeu[posLigne+1][posColonne+2]==99 || jeu[posLigne+1][posColonne-2]%2 != name%2)){
                glPushMatrix();
                glTranslatef(2*pas,0,pas);
                glutSolidCube(pas);
                glPopMatrix();
            }
            if(posLigne<7 && posColonne >1 && (jeu[posLigne+1][posColonne-2]==99 || jeu[posLigne+1][posColonne-2]%2 != name%2)){
                glPushMatrix();
                glTranslatef(-2*pas,0,pas);
                glutSolidCube(pas);
                glPopMatrix();
            }

        glPopMatrix();
    }
    //Fou
    else if(name>7){

        int posLigne = 0;
        int posColonne=0;
        int videsBasDroite=0, videsBasGauche=0, videsHautDroite=0, videsHautGauche=0;
        int tmpCol;
        int i,j,c;

        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                if(name==jeu[i][j]){
                    posLigne=i;
                    posColonne=j;
                }
            }
        }


        if(name%2==0)
            glTranslatef(3*pas+0.1, -pas/1.5, -2*pas+0.24);//Positionnement du curseur sur le fou
        else
            glTranslatef(3*pas+0.1, -pas/1.5, -9*pas+0.24);


        // Cases vides en bas a droite
        glPushMatrix();
        tmpCol=posColonne+1;
        for(i=posLigne+1;i<8;i++){
            if(tmpCol<8){
                if(jeu[i][tmpCol]==99)
                    videsBasDroite++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsBasDroite++;
                    break;
                }
                tmpCol++;
            }
            else
                break;
        }
        for(c=0;c<videsBasDroite;c++){
            glTranslatef(pas,0,pas);
            glutSolidCube(pas);
        }
        glPopMatrix();


        // Cases vides en bas a gauche
        glPushMatrix();
        tmpCol=posColonne-1;
        for(i=posLigne+1;i<8;i++){
            if(tmpCol>=0){
                if(jeu[i][tmpCol]==99)
                    videsBasGauche++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsBasGauche++;
                    break;
                }
                tmpCol--;
            }
            else
                break;
        }
        for(c=0;c<videsBasGauche;c++){
            glTranslatef(-pas,0,pas);
            glutSolidCube(pas);
        }
        glPopMatrix();


        // Cases vides en haut a gauche
        glPushMatrix();
        tmpCol=posColonne-1;
        for(i=posLigne-1;i>=0;i--){
            if(tmpCol>=0){
                if(jeu[i][tmpCol]==99)
                    videsHautGauche++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsHautGauche++;
                    break;
                }
                tmpCol--;
            }
            else
                break;
        }
        for(c=0;c<videsHautGauche;c++){
            glTranslatef(-pas,0,-pas);
            glutSolidCube(pas);
        }
        glPopMatrix();

        // Cases vides en haut a droite
        glPushMatrix();
        tmpCol=posColonne+1;
        for(i=posLigne-1;i>=0;i--){
            if(tmpCol<8){
                if(jeu[i][tmpCol]==99)
                    videsHautDroite++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsHautDroite++;
                    break;
                }
                tmpCol++;
            }
            else
                break;
        }
        for(c=0;c<videsHautDroite;c++){
            glTranslatef(pas,0,-pas);
            glutSolidCube(pas);
        }
        glPopMatrix();


    }
    //Tour
    else if(name>3){

            if(name%2==0)//Blanc
                glTranslatef(pas+0.1, -pas/1.5, -3*pas+0.24); // Positionnement du curseur à une case devant la tour
            else//Noir
                glTranslatef(pas+0.1, -pas/1.5, -10*pas+0.24);

            int c;
            int posLigne = 0;
            int posColonne=0;


            int i,j;
            for(i=0;i<8;i++){
                for(j=0;j<8;j++){
                    if(name==jeu[i][j]){
                        posLigne=i;
                        posColonne=j;
                    }
                }
            }

            int videsHaut=0;
            int videsBas=0;
            int videsDroite=0;
            int videsGauche=0;

            int elementHaut = 99;


            glPushMatrix();
            //Dessiner un cube transparent sur les cases possibles en haut
            for(i=posLigne-1;i>=0;i--){
                if(jeu[i][posColonne]==99)
                    videsHaut++;
                else{
                    elementHaut = jeu[i][posColonne];
                    if(elementHaut%2 != name%2)//Si element haut ennemeni
                        videsHaut++;

                    break;
                }
            }
            for(c=0;c<videsHaut;c++){
                glutSolidCube(pas);
                glTranslatef(0,0,-pas);
            }
            glPopMatrix();

            //Dessiner un cube transparent sur les cases possibles en bas
            glPushMatrix();
            glTranslatef(0,0,2*pas);
            for(i=posLigne+1;i<8;i++){
                if(jeu[i][posColonne]==99)
                    videsBas++;
                else{
                    elementHaut = jeu[i][posColonne];
                    if(elementHaut%2 != name%2)//Si element haut ennemeni
                        videsBas++;
                    break;
                }
            }
            //printf("videsBas: %d", videsBas);
            for(c=0;c<videsBas;c++){
                glutSolidCube(pas);
                glTranslatef(0,0,pas);
            }
            glPopMatrix();

            //Dessiner un cube transparent sur les cases possibles a droite
            glPushMatrix();
            glTranslatef(pas,0,pas);
            for(i=posColonne+1;i<8;i++){
                if(jeu[posLigne][i]==99)
                    videsDroite++;
                else{
                    elementHaut = jeu[posLigne][i];
                    if(elementHaut%2 != name%2)//Si element haut ennemeni
                        videsDroite++;
                    break;
                }
            }

            for(c=0;c<videsDroite;c++){
                glutSolidCube(pas);
                glTranslatef(pas,0,0);
            }
            glPopMatrix();

            //Dessiner un cube transparent sur les cases possibles a gauche
            glPushMatrix();
            glTranslatef(-pas,0,pas);
            for(i=posColonne-1;i>=0;i--){
                if(jeu[posLigne][i]==99)
                    videsGauche++;
                else{
                    elementHaut = jeu[posLigne][i];
                    if(elementHaut%2 != name%2)//Si element haut ennemeni
                        videsGauche++;
                    break;
                }
            }

            for(c=0;c<videsGauche;c++){
                glutSolidCube(pas);
                glTranslatef(-pas,0,0);
            }
            glPopMatrix();
    }

    //Renne
    else if(name>1){
        if(name%2==0)
            glTranslatef(5*pas+0.1,-pas/1.5, -2*pas+0.24);//Position du curseur sur la renne
        else
            glTranslatef(5*pas+0.1,-pas/1.5, -9*pas+0.24);//Position du curseur sur la renne

        int posLigne = 0;
        int posColonne=0;
        int videsBasDroite=0, videsBasGauche=0, videsHautDroite=0, videsHautGauche=0;
        int tmpCol;
        int i,j,c;



        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                if(name==jeu[i][j]){
                    posLigne=i;
                    posColonne=j;
                }
            }
        }

        // Cases vides en bas a droite
        glPushMatrix();
        tmpCol=posColonne+1;
        for(i=posLigne+1;i<8;i++){
            if(tmpCol<8){
                if(jeu[i][tmpCol]==99)
                    videsBasDroite++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsBasDroite++;
                    break;
                }
                tmpCol++;
            }
            else
                break;
        }
        for(c=0;c<videsBasDroite;c++){
            glTranslatef(pas,0,pas);
            glutSolidCube(pas);
        }
        glPopMatrix();


        // Cases vides en bas a gauche
        glPushMatrix();
        tmpCol=posColonne-1;
        for(i=posLigne+1;i<8;i++){
            if(tmpCol>=0){
                if(jeu[i][tmpCol]==99)
                    videsBasGauche++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsBasGauche++;
                    break;
                }
                tmpCol--;
            }
            else
                break;
        }
        for(c=0;c<videsBasGauche;c++){
            glTranslatef(-pas,0,pas);
            glutSolidCube(pas);
        }
        glPopMatrix();


        // Cases vides en haut a gauche
        glPushMatrix();
        tmpCol=posColonne-1;
        for(i=posLigne-1;i>=0;i--){
            if(tmpCol>=0){
                if(jeu[i][tmpCol]==99)
                    videsHautGauche++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsHautGauche++;
                    break;
                }
                tmpCol--;
            }
            else
                break;
        }
        for(c=0;c<videsHautGauche;c++){
            glTranslatef(-pas,0,-pas);
            glutSolidCube(pas);
        }
        glPopMatrix();

        // Cases vides en haut a droite
        glPushMatrix();
        tmpCol=posColonne+1;
        for(i=posLigne-1;i>=0;i--){
            if(tmpCol<8){
                if(jeu[i][tmpCol]==99)
                    videsHautDroite++;
                else{
                    if(jeu[i][tmpCol]%2!=name%2)
                        videsHautDroite++;
                    break;
                }
                tmpCol++;
            }
            else
                break;
        }
        for(c=0;c<videsHautDroite;c++){
            glTranslatef(pas,0,-pas);
            glutSolidCube(pas);
        }
        glPopMatrix();




        //glutSolidCube(pas);

    }

    //Roi
    else {
        if(name%2==0)
            glTranslatef(4*pas+0.1, -pas/1.5, -2*pas+0.24);
        else
            glTranslatef(4*pas+0.1, -pas/1.5, -9*pas+0.24);

        int posLigne = 0;
        int posColonne=0;


        int i,j;
        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                if(name==jeu[i][j]){
                    posLigne=i;
                    posColonne=j;
                }
            }
        }


        if(posLigne<7&& (jeu[posLigne+1][posColonne]==99 || jeu[posLigne+1][posColonne]%2!=name%2)){
            glPushMatrix();
            glTranslatef(0,0,pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posLigne<7 && posColonne<7 && (jeu[posLigne+1][posColonne+1]==99 || jeu[posLigne+1][posColonne+1]%2!=name%2)){
            glPushMatrix();
            glTranslatef(pas,0,pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posColonne>0 && posLigne<7 && (jeu[posLigne+1][posColonne-1]==99 || jeu[posLigne+1][posColonne-1]%2!=name%2)){
            glPushMatrix();
            glTranslatef(-pas,0,pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posColonne<7 && (jeu[posLigne][posColonne+1]==99 || jeu[posLigne][posColonne+1]%2!=name%2)){
            glPushMatrix();
            glTranslatef(pas,0,0);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posColonne>0 && (jeu[posLigne][posColonne-1]==99 || jeu[posLigne][posColonne-1]%2!=name%2)){
            glPushMatrix();
            glTranslatef(-pas,0,0);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posLigne>0 && (jeu[posLigne-1][posColonne]==99 || jeu[posLigne-1][posColonne]%2!=name%2)){
            glPushMatrix();
            glTranslatef(0,0,-pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posLigne>0 && posColonne<7 && (jeu[posLigne-1][posColonne+1]==99 || jeu[posLigne-1][posColonne+1]%2!=name%2)){
            glPushMatrix();
            glTranslatef(pas,0,-pas);
            glutSolidCube(pas);
            glPopMatrix();
        }
        if(posLigne>0 && posColonne>0 && (jeu[posLigne-1][posColonne-1]==99 || jeu[posLigne-1][posColonne-1]%2!=name%2)){
            glPushMatrix();
            glTranslatef(-pas,0,-pas);
            glutSolidCube(pas);
            glPopMatrix();
        }

    }



    glPopMatrix();



}


