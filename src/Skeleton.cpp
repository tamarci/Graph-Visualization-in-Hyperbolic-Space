//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
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
// Nev    : T�rnok M�rton
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

//t�r�tt voanl video
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

Texture myTexture;
vec2 movingVector(0, 0);

// Initialization, create an OpenGL context


vec3 segmentEq(vec3 p1, vec3 dv, float distance) {
    return p1 * std::cosh(distance) + dv * std::sinh(distance);
}

vec3 directionVector(vec3 p1, vec3 m, float distance) {
    return (m - p1 * std::cosh(distance)) / std::sinh(distance);
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
//t�kr�z�s l�p�sei:
//p �s m pontok t�vols�ga -> distance
//v vector meghataroz�sa
//r helyere m et
// segmentEqba nem 1 hanem ket dvel havando

vec3 hiperbolicTranslate(vec3 p1, vec3 q1, vec3 q2) {
    //felezopont meghat�roz�sa
    //t�kr�z�s

    vec3 mirroredP1 = mirrorPoint(p1, q1);
    float d = distance(q1, q2);
    if (d < 0.00001f) {
        return p1;
    }
    vec3 directionVec = directionVector(q1, q2, d);
    vec3 symmetryPoint = segmentEq(q1, directionVec, d / 2);
    return mirrorPoint(mirroredP1, symmetryPoint);
}

class Circle {
    unsigned int vao, vbo[2];
    vec3 center;
    const float radius = 0.04;


public:
    vec3 getCenter() { return center; }

    void create(vec2 center) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        this->center = vec3(center.x, center.y, sqrt(center.x * center.x + center.y * center.y + 1));

        generateData();

        generateTexture();
    }

    void generateTexture() {

        vec2 textureUVCoordinates[20];
        for (int i = 0; i < 20; ++i) {
            float x = (std::cos(i * 2 * PI / 20) + 1) / 2;
            float y = (std::sin(i * 2 * PI / 20) + 1) / 2;
            textureUVCoordinates[i] = vec2(x, y);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(vec2), textureUVCoordinates, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        std::vector<vec4> colors;
        colors.resize(50 * 50);
        colors.assign(50 * 50, vec4(1, 0.5, 0, 1));
        colors.assign(50 * 25, vec4(1, 1, 0, 1));


        myTexture.create(50, 50, colors);
    }

    void generateData() {
        float points[40]; //(20)*2
        vec3 mouseOffset3{movingVector.x, movingVector.y,
                          sqrt(movingVector.x * movingVector.x + movingVector.y * movingVector.y + 1)};

        for (int i = 0; i < 20; i++) {

            float x = (radius * std::cos(i * 2 * PI / 20));
            float y = (radius * std::sin(i * 2 * PI / 20));
            float z = sqrt(x * x + y * y + 1);
            vec3 translatedP = hiperbolicTranslate(vec3(x, y, z), ORIGIN, center);

            translatedP = hiperbolicTranslate(translatedP, ORIGIN, mouseOffset3);

            points[i * 2] = translatedP.x;
            points[i * 2 + 1] = translatedP.y;
        }


        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW); //CPU-ra

    }

    void updateCenter(vec3 newCenter) {
        center = newCenter;
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
                          sqrt(movingVector.x * movingVector.x + movingVector.y * movingVector.y + 1)};

        vec3 translatedCenter1 = hiperbolicTranslate(center1, ORIGIN, mouseOffset3);
        vec3 translatedCenter2 = hiperbolicTranslate(center2, ORIGIN, mouseOffset3);

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
        circles[i].create(vec2(randomFloat(), randomFloat()) * 4 - vec2(1, 1));
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

    gpuProgram.setUniform(myTexture, "textureUnit");
    for (int i = 0; i < 50; ++i) {
        circles[i].Draw();
    }

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

void updateAll() {

    for (int i = 0; i < 50; ++i) {
        // vec3 translatedCenter = hiperbolicTranslate(circles[i].getCenter(), q1, q2);
        circles[i].generateData();
    }

    for (int i = 0; i < 61; ++i) {
        edges[i].updateCenter(circles[(int) edgePairs[i].x].getCenter(), circles[(int) edgePairs[i].y].getCenter());

    }

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
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}