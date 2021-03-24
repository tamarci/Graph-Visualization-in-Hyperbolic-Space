//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Tárnok Márton
// Neptun : GGDVB2
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================

//törött voanl video
// hazifeladat video
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char *const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers


	layout(location = 0) in vec2 vertexPosition;	// Varying input: vp = vertex position is expected in attrib array 0
    layout(location = 1) in vec2 vertexUV;

    out vec2 texCoord;

	void main() {
        texCoord= vertexUV;
		gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, sqrt(vertexPosition.x*vertexPosition.x+vertexPosition.y*vertexPosition.y+1));		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char *const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

    uniform sampler2D textureUnit;

    in vec2 texCoord;		// uniform variable, the color of the primitive
	out vec4  fragmentColor;		// computed color of the current pixel



	void main() {
		fragmentColor = texture(textureUnit, texCoord);

	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders

constexpr float PI = (float) M_PI;
const vec3 ORIGIN(0, 0, 1);

vec2 movingVector(0, 0);

// Initialization, create an OpenGL context


vec3 segmentEq(vec3 p1, vec3 dv, float distance) {
    return p1 * coshf(distance) + dv * sinhf(distance);
}

vec3 directionVector(vec3 p1, vec3 m, float distance) {
    return (m - p1 * coshf(distance)) /sinhf(distance);
}

float lorentzEq(vec3 p1, vec3 p2) {
    return p1.x * p2.x + p1.y * p2.y - p1.z * p2.z;
}

float distance(vec3 p1, vec3 p2) {
    return acosh(-lorentzEq(p1, p2));
}

vec3 mirrorPoint(vec3 point, vec3 symmetryPoint) {
    float d = distance(point, symmetryPoint);
    if (d < 0.00001f) {

        return point;
    }
    vec3 directionVec = directionVector(point, symmetryPoint, d);
    return segmentEq(point, directionVec, 2 * d);
}

vec3 hiperbolicTranslate(vec3 p1, vec3 q1, vec3 q2, float multiplier=1) {

    vec3 mirroredP1 = mirrorPoint(p1, q1);
    float d = distance(q1, q2);
    if (d < 0.00001f) {
        return p1;
    }
    vec3 directionVec = directionVector(q1, q2, d);
    vec3 symmetryPoint = segmentEq(q1, directionVec, (d / 2)*multiplier);
    return mirrorPoint(mirroredP1, symmetryPoint);
}

float ranFloat() {
    return (float) rand() / RAND_MAX;
}

float calcZ(float x, float y){
    return sqrtf(x *x + y* y + 1);
}

class Circle {
    unsigned int vao, vbo[2];
    vec3 center;
    const float radius = 0.04;
    Texture myTexture;


public:
    vec3 getCenter() { return center; }
    void setCenter(vec2 c) {center=c;
        center.z= calcZ(c.x,c.y);

        }

    void create(vec2 center) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        this->center = vec3(center.x, center.y, calcZ(center.x,center.y));

        generateData();

        generateTexture();
    }

    void generateTexture() {

        vec2 textureUVCoordinates[20];
        for (int i = 0; i < 20; ++i) {
            float x = (cosf(i * 2 * PI / 20) + 1) / 2;
            float y = (sinf(i * 2 * PI / 20) + 1) / 2;
            textureUVCoordinates[i] = vec2(x, y);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(vec2), textureUVCoordinates, GL_STATIC_DRAW);

        std::vector<vec4> textureOutlook;
        textureOutlook.resize(50 * 50);

        vec4 color0(ranFloat(), ranFloat(), ranFloat(), 1);
        vec4 color1(ranFloat(), ranFloat(), ranFloat(), 1);
        vec4 color2(ranFloat(), ranFloat(), ranFloat(), 1);

        textureOutlook.assign(50 * 50, color0);
        textureOutlook.assign(50 * 33, color1);
        textureOutlook.assign(50 * 16, color2);

        myTexture.create(50, 50, textureOutlook);
    }


    void generateData() {
        float points[40]; //(20)*2
        vec3 mv3{movingVector.x, movingVector.y,
                 calcZ(movingVector.x,movingVector.y)};

        for (int i = 0; i < 20; i++) {

            float x = (radius * cosf(i * 2 * PI / 20));
            float y = (radius * sinf(i * 2 * PI / 20));
            float z = calcZ(x,y);
            vec3 translatedP = hiperbolicTranslate(vec3(x, y, z), ORIGIN, center);

            translatedP = hiperbolicTranslate(translatedP, ORIGIN, mv3, 1);

            points[i * 2] = translatedP.x;
            points[i * 2 + 1] = translatedP.y;
        }


        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW); //CPU-ra

    }


    void Draw() {
        glBindVertexArray(vao);
        gpuProgram.setUniform(myTexture, "textureUnit");
        glDrawArrays(GL_TRIANGLE_FAN, 0, 20);
    }

};
//dögöljön meg a tulipán, hulljanak le a ...
class Edge {
    unsigned int vao, vbo;
//velocity állandó
//erõ változik
//olyan függvény kell ami szimmetrikus és metszi az x tengely
//sinh(x-1)
//tanhx-1
public:
    void create(vec3 center1, vec3 center2) {
        float centers[4];
        centers[0] = center1.x;
        centers[1] = center1.y;
        centers[2] = center2.x;
        centers[3] = center2.y;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(centers), centers, GL_DYNAMIC_DRAW); //CPU-ra

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);


    }

    void updateCenter(vec3 center1, vec3 center2) {
        vec3 mouseOffset3{movingVector.x, movingVector.y,
                          sqrtf(movingVector.x * movingVector.x + movingVector.y * movingVector.y + 1)};

        vec3 translatedCenter1 = hiperbolicTranslate(center1, ORIGIN, mouseOffset3,1);
        vec3 translatedCenter2 = hiperbolicTranslate(center2, ORIGIN, mouseOffset3,1);

        float centers[4];
        centers[0] = translatedCenter1.x;
        centers[1] = translatedCenter1.y;
        centers[2] = translatedCenter2.x;
        centers[3] = translatedCenter2.y;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(centers), centers, GL_DYNAMIC_DRAW); //CPU-ra
    }

    void Draw() {
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, 2);
    }

};

std::vector<vec2> shuffledEdgeGen() {
    std::vector<vec2> temp;
    for (int i = 0; i < 50; ++i) {
        for (int j = i + 1; j < 50; ++j) {
            vec2 newEdge(i, j);
            temp.push_back(newEdge);
        }
    }

    for (int i = 0; i < 1225; i++) {
        int j = i + rand() % (1225 - i);
        vec2 save = temp[i];
        temp[i] = temp[j];
        temp[j] = save;

    }

    printf("\n");
    std::vector<vec2> finalEdges;
    for (int k = 0; k < (1225 * 0.05); ++k) {
        finalEdges.push_back(temp[k]);

    }

    return finalEdges;
}


Circle circles[50];
std::vector<vec2> edgePairs = shuffledEdgeGen();
Edge edges[61];
std::vector<vec3> forceVector;
std::vector<vec3> speedVector;
bool forceC=false;


bool parban(int a, int b) {

    for (int i = 0; i < edgePairs.size(); ++i) {
        if ((a == edgePairs.at(i).x && b == edgePairs.at(i).y) || (a == edgePairs.at(i).y && b == edgePairs.at(i).x)) {
            return true;
        }
    }
    return false;

}
void updateAll() {

    for (int i = 0; i < 50; ++i) {
        // vec3 translatedCenter = hiperbolicTranslate(circles[i].getCenter(), q1, q2);
        circles[i].generateData();
    }

    for (int i = 0; i < 61; ++i) {
        edges[i].updateCenter(circles[(int) edgePairs[i].x].getCenter(), circles[(int) edgePairs[i].y].getCenter());

    }

}
void resetForce(){
    forceVector.resize(50);
    for (int i = 0; i < forceVector.size(); ++i) {
        forceVector.at(i)=vec3(0,0,1);
    }
}
void calculateAllForce(){
    float force;
    for (int i = 0; i < 50; ++i) {
        for (int j = i + 1; j < 50; ++j) {

            float dist = distance(circles[i].getCenter(),circles[j].getCenter());
            if(dist<0.0001)
                printf("baj van %d %d",i,j);

           if(parban(i,j)){
               force = sinhf(dist-0.5f);
           }
           else{
               force= -1/dist/8;
           }
           float multiplier = force/dist/8; //ez nem biztos h jo igy

           forceVector.at(i) = hiperbolicTranslate(forceVector.at(i),circles[i].getCenter(),circles[j].getCenter(), multiplier);
           forceVector.at(j) = hiperbolicTranslate(forceVector.at(j),circles[j].getCenter(),circles[i].getCenter(), multiplier);
           forceVector[i].z=calcZ( forceVector[i].x, forceVector[i].y);
            forceVector[j].z=calcZ( forceVector[j].x, forceVector[j].y);

        }
    }
}

void calculateAllCenters(){

    for (int i = 0; i < 50; ++i) {

        vec3 centerForce=hiperbolicTranslate(ORIGIN,circles[i].getCenter(),ORIGIN,1);


      // speedVector.at(i)= hiperbolicTranslate(speedVector.at(i),ORIGIN,forceVector.at(i),0.01);
       speedVector.at(i).z= calcZ(speedVector.at(i).x,speedVector.at(i).y);

        speedVector.at(i)= hiperbolicTranslate(speedVector.at(i),ORIGIN,centerForce,0.01);
        speedVector.at(i).z= calcZ(speedVector.at(i).x,speedVector.at(i).y);


       vec3 center= hiperbolicTranslate(circles[i].getCenter(),ORIGIN,speedVector.at(i),0.01);
       circles[i].setCenter(vec2(center.x,center.y));





    }
    updateAll();

}
void forceControll(){
    resetForce();
    calculateAllForce();
    calculateAllCenters();
}



vec2 heuristicGen(std::vector<vec2> centers) {
    float x;
    float y;
    float nev = 0;
    float szamx = 0;
    float szamy = 0;
    for (int i = 0; i < centers.size(); ++i) {
        float m;
        if (parban(i, centers.size())) {
            m = 1;
        } else {
            m = 0.01;
        }
        szamx += centers.at(i).x*m;
        szamy += centers.at(i).y*m;
        nev += m;
      //  if(nev <0){
        //    nev= -nev;
        //}

    }
    if (centers.size() == 0) {
        x = ranFloat();
        y = ranFloat();
    } else {
        x = szamx / nev;
        y = szamy / nev;
    }

    return vec2(x, y);
}


void onInitialization() {
  //  glEnable(GL_DEBUG_OUTPUT);

    srand(0);

    glViewport(0, 0, windowWidth, windowHeight);
    glLineWidth(2.0f);

    std::vector<vec2> Centers;
    forceVector.resize(50);
    speedVector.resize(50);

    for (int i = 0; i < speedVector.size(); ++i) {
        speedVector.at(i)=vec3(0,0,1);
    }

    for (int i = 0; i < 50; ++i) {
        vec2 newCenter = heuristicGen(Centers);
        Centers.push_back(newCenter);
        circles[i].create(vec2(ranFloat(),ranFloat()) * 6 - vec2(3, 3));
    }

    for (int i = 0; i < 61; ++i) {
        vec3 center1 = circles[(int) edgePairs[i].x].getCenter();
        vec3 center2 = circles[(int) edgePairs[i].y].getCenter();
        edges[i].create(center1, center2);
    }


    gpuProgram.create(vertexSource, fragmentSource, "fragmentSource");
    glutPostRedisplay();
}

// Window has become invalid: Redraw
void onDisplay() {
    glClearColor(0, 0, 0, 0);     // background color
    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer
    gpuProgram.Use();

    for (int i = 0; i < 61; ++i) {
        edges[i].Draw();
    }

    //gpuProgram.setUniform(myTexture, "textureUnit");
    for (int i = 0; i < 50; ++i) {
        circles[i].Draw();
    }

    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
  forceC=!forceC;// if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}



vec2 pos;
vec2 oldPos(0, 0);

// Move mouse with key pressed
void onMouseMotion(int pX,
                   int pY) {    // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;

    movingVector = movingVector + vec2{cX, cY} - oldPos;

    pos = vec2(cX, cY);

    oldPos = pos;
    printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);

    updateAll();
    onDisplay();
}


// Mouse click event
void onMouse(int button, int state, int pX,
             int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    oldPos = vec2(cX, cY);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

    }
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME);
    if(forceC){
    for (int i = 0; i < 10; ++i) {
        forceControll();

    }
    }
    glutPostRedisplay();
}