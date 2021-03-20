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
    layout(location = 1) in vec3 vertexColor;

    out vec3 color;

	void main() {
        color = vec3(1,0,0);
		gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, sqrt(vertexPosition.x*vertexPosition.x+vertexPosition.y*vertexPosition.y+1));		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char *const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

    in vec3 color;		// uniform variable, the color of the primitive
	out vec4  fragmentColor;		// computed color of the current pixel

	void main() {
		fragmentColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders

constexpr float PI = (float) M_PI;
const vec3 ORIGIN(0, 0, 1);

// Initialization, create an OpenGL context


vec3 segmentEq(vec3 p1, vec3 dv, float distance) {
    return p1 * std::cosh(distance) + dv * std::sinh(distance);
}

vec3 directionVector(vec3 p1, vec3 m, float distance) {
    return (m - p1 * std::cosh(distance)) / sinh(distance);
}

float lorentzEq(vec3 p1, vec3 p2) {
    return p1.x * p2.x + p1.y * p2.y - p1.z * p2.z;
}

float distance(vec3 p1, vec3 p2) {
    return acosh(-lorentzEq(p1, p2));
}

vec3 mirrorPoint(vec3 point, vec3 symmetryPoint) {
    float d = distance(point, symmetryPoint);
    if (d < 0.001) {
        return point;
    }
    vec3 directionVec = directionVector(point, symmetryPoint, d);
    return segmentEq(point, directionVec, 2 * d);
}
//tükrözés lépései:
//p és m pontok távolsága -> distance
//v vector meghatarozása
//r helyere m et
// segmentEqba nem 1 hanem ket dvel havando

vec3 hiperbolicTranslate(vec3 p1, vec3 q1, vec3 q2) {
    //felezopont meghatározása
    //tükrözés

    vec3 mirroredP1 = mirrorPoint(p1, q1);
    float d = distance(q1, q2);
    if (d < 0.001) {
        return p1;
    }
    vec3 directionVec = directionVector(q1, q2, d);
    vec3 symmetryPoint = segmentEq(q1, directionVec, d / 2);
    return mirrorPoint(mirroredP1, symmetryPoint);
}

class Circle {
    unsigned int vao, vbo;
    vec3 center;
    const float radius = 0.04;


public:
    vec3 getCenter() { return center; }

    void create(vec2 center) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);


        this->center = vec3(center.x, center.y, sqrt(center.x * center.x + center.y * center.y + 1));


        generateData();


        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }


    void generateData() {
        float points[40]; //(20)*2

        for (int i = 0; i < 20; i++) {

            float x = (radius * std::cos(i * 2 * PI / 20));
            float y = (radius * std::sin(i * 2 * PI / 20));
            float z = sqrt(x * x + y * y + 1);
            vec3 translatedP = hiperbolicTranslate(vec3(x, y, z), ORIGIN, center);

            points[i * 2] = translatedP.x;
            points[i * 2 + 1] = translatedP.y;
        }


        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW); //CPU-ra


    }

    void updateCenter(vec3 newCenter) {
        center= newCenter;
       generateData();
    }

    void Draw() {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 20);
    }

};

class Edge {
    unsigned int vao, vbo;


public:
    void create(vec3 center1, vec3 center2) {
        float centers[6];
        centers[0] = center1.x;
        centers[1] = center1.y;
        centers[2] = center1.z;
        centers[3] = center2.x;
        centers[4] = center2.y;
        centers[6] = center2.z;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);


        glBufferData(GL_ARRAY_BUFFER, sizeof(centers), centers, GL_DYNAMIC_DRAW); //CPU-ra

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


    }

    void updateCenter(vec3 center1, vec3 center2){
        float centers[6];
        centers[0] = center1.x;
        centers[1] = center1.y;
        centers[2] = center1.z;
        centers[3] = center2.x;
        centers[4] = center2.y;
        centers[6] = center2.z;

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(centers), centers, GL_DYNAMIC_DRAW); //CPU-ra

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

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

float randomFloat() {
    return (float) rand() / RAND_MAX;
}


void GLAPIENTRY messageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam) {
    if (type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "ERROR, severity = 0x%x, message = %s\n", severity, message);
    }
}

void onInitialization() {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, nullptr);

    glViewport(0, 0, windowWidth, windowHeight);
    glLineWidth(2.0f);


    for (int i = 0; i < 50; ++i) {
        circles[i].create(vec2(randomFloat(), randomFloat()) * 2 - vec2(1, 1));
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

    for (int i = 0; i < 50; ++i) {
        circles[i].Draw();
    }

    for (int i = 0; i < 50; ++i) {
        edges[i].Draw();
    }


    // Set color to (0, 1, 0) = green


    glutSwapBuffers(); // exchange buffers for double buffering


}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

vec3 convertCoordinates(float x, float y) {
    return vec3(x / sqrt(1 - x * x - y * y), y / sqrt(1 - x * x - y * y), 1 / sqrt(1 - x * x - y * y));
}

void translateAll(vec3 q1, vec3 q2) {

    for (int i = 0; i < 61; ++i) {
        vec3 translatedCenter1 = hiperbolicTranslate(circles[(int) edgePairs[i].x].getCenter(), q1, q2);
        circles[(int) edgePairs[i].x].updateCenter(translatedCenter1);

        vec3 translatedCenter2 = hiperbolicTranslate(circles[(int) edgePairs[i].y].getCenter(), q1, q2);
        circles[(int)edgePairs[i].y].updateCenter(translatedCenter2);

        edges[i].updateCenter(translatedCenter1, translatedCenter2);
    }

}

vec3 pos;
vec3 oldPos(0,0,1);
// Move mouse with key pressed
void onMouseMotion(int pX,
                   int pY) {    // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    pos = convertCoordinates(cX, cY);
    translateAll(oldPos, pos);
    oldPos=pos;
    printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
    onDisplay();
}


// Mouse click event
void onMouse(int button, int state, int pX,
             int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    oldPos = convertCoordinates(cX, cY);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

    }


    char *buttonStat;
    switch (state) {
        case GLUT_DOWN:
            buttonStat = "pressed";
            break;
        case GLUT_UP:
            buttonStat = "released";
            break;
    }

    switch (button) {
        case GLUT_LEFT_BUTTON:
            printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);

            break;
        case GLUT_MIDDLE_BUTTON:
            printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
            break;
        case GLUT_RIGHT_BUTTON:
            printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
            break;
    }
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}

