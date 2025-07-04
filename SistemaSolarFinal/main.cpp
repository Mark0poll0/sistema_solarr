/*  Enrique Julca Delgado - Proyecto Sistema Solar
    Descripción:    Todas estará definido en el propio main.cpp para evitar complicaciones a la hora de la revisión
*/

// NOTA: Este proyecto se compila y ejecuta desde Visual Studio.
// Si va a compilar en otro entorno, asegúrese de:
// - Copiar freeglut.dll al mismo lugar donde se genera el .exe
// - Que el .exe cargue texturas desde la carpeta /assets
// - Cambiar LoadBMP("../assets/...") a "assets/..." si el .exe se genera en raíz

#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <iostream>

#include <vector>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ------------------------
// CONSTANTES DE CÁMARA
// ------------------------

float camX = 0.0f, camY = 5.0f, camZ = 30.0f;
float centerX = 0.0f, centerY = 5.0f, centerZ = 0.0f;
float yaw = -90.0f, pitch = 0.0f;
float moveSpeed = 0.5f;
float sensitivity = 0.1f;
bool ignoringWarp = false;
int windowWidth = 1280, windowHeight = 720;

// ------------------------
// ROTACIÓN DE PLANETAS
// ------------------------
float anguloSol = 0.0f;
float anguloTierra = 0.0f;

float rotacionPropia = 0.0f;

// ------------------------
// CARGA DE TEXTURAS
// ------------------------
GLuint texturaFondo = 0;
GLuint texturaSol = 0;
GLuint texturaTierra = 0;



struct Satelite {
    float distancia;     // respecto al planeta
    float radio;
    float velocidad;
    float angulo;
    GLuint textura;
};

struct Planeta {
    std::string nombre;
    float distancia;
    float radius;
    float velocidad;
    GLuint textura;
    float angulo;
    float rotacionPropia;
    std::vector<Satelite> satelites;
};


std::vector<Planeta> planetas;

Satelite luna = { 2.0f, 0.3f, 0.5f, 0.0f, 0 };         // Tierra
Satelite fobos = { 1.2f, 0.2f, 0.8f, 0.0f, 0 };        // Marte
Satelite deimos = { 2.0f, 0.15f, 0.5f, 0.0f, 0 };      // Marte



GLuint LoadBMP(const char* filename) {
    FILE* file;
    fopen_s(&file, filename, "rb");
    if (!file) return 0;

    unsigned char header[54];
    fread(header, 1, 54, file);
    unsigned int dataPos = *(int*)&(header[0x0A]);
    unsigned int imageSize = *(int*)&(header[0x22]);
    unsigned int width = *(int*)&(header[0x12]);
    unsigned int height = *(int*)&(header[0x16]);

    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;

    unsigned char* data = new unsigned char[imageSize];
    fread(data, 1, imageSize, file);
    fclose(file);

    for (unsigned int i = 0; i < imageSize; i += 3) {
        std::swap(data[i], data[i + 2]);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return textureID;
}

// ------------------------
// CÁMARA FPS
// ------------------------
void updateCenter() {
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;
    centerX = camX + cos(radPitch) * cos(radYaw);
    centerY = camY + sin(radPitch);
    centerZ = camZ + cos(radPitch) * sin(radYaw);
}

void mouseMotion(int xpos, int ypos) {
    if (ignoringWarp) {
        ignoringWarp = false;
        return;
    }
    int cx = windowWidth / 2;
    int cy = windowHeight / 2;
    float dx = xpos - cx;
    float dy = cy - ypos;

    dx *= sensitivity;
    dy *= sensitivity;

    yaw += dx;
    pitch += dy;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCenter();
    ignoringWarp = true;
    glutWarpPointer(cx, cy);
    glutPostRedisplay();
}

void moverCamara(int) {
    float dx = centerX - camX;
    float dy = centerY - camY;
    float dz = centerZ - camZ;
    float len = sqrt(dx * dx + dy * dy + dz * dz);
    dx /= len; dy /= len; dz /= len;

    float rightX = dz;
    float rightZ = -dx;

    if (GetAsyncKeyState('W') & 0x8000) { camX += dx * moveSpeed; camZ += dz * moveSpeed; }
    if (GetAsyncKeyState('S') & 0x8000) { camX -= dx * moveSpeed; camZ -= dz * moveSpeed; }
    if (GetAsyncKeyState('A') & 0x8000) { camX += rightX * moveSpeed; camZ += rightZ * moveSpeed; }
    if (GetAsyncKeyState('D') & 0x8000) { camX -= rightX * moveSpeed; camZ -= rightZ * moveSpeed; }

    
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) { camY += 0.3f; }  // Subir
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { camY -= 0.3f; }  // Bajar

    updateCenter();
    glutPostRedisplay();
    glutTimerFunc(16, moverCamara, 0);
}


void dibujarAnillosSaturnoAvanzado() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int numCapas = 3;
    float rMin = 4.5f, rMax = 5.5f;

    for (int i = 0; i < numCapas; ++i) {
        float t = (float)i / (numCapas - 1);
        float r1 = rMin + t * (rMax - rMin);
        float r2 = r1 + 0.4f;
        float alpha = 1.0f - t * 0.35f;

        glColor4f(0.95f, 0.85f, 0.6f, alpha);
        glBegin(GL_TRIANGLE_STRIP);
        for (float ang = 0; ang <= 360; ang += 2.0f) {
            float rad = ang * M_PI / 180.0f;
            glVertex3f(cos(rad) * r1, 0.0f, sin(rad) * r1);
            glVertex3f(cos(rad) * r2, 0.0f, sin(rad) * r2);
        }
        glEnd();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    for (float r = 6.5f, alpha = 0.12f; r <= 8.0f; r += 0.4f, alpha -= 0.008f) {
        if (alpha <= 0.0f) break;
        for (float yShift = -0.2f; yShift <= 0.2f; yShift += 0.1f) {
            glColor4f(0.95f, 0.85f, 0.5f, alpha * (1.0f - fabs(yShift)));
            glBegin(GL_TRIANGLE_STRIP);
            for (float ang = 0; ang <= 360; ang += 2.0f) {
                float rad = ang * M_PI / 180.0f;
                float c = cos(rad), s = sin(rad);
                glVertex3f(c * (r - 0.3f), yShift, s * (r - 0.3f));
                glVertex3f(c * r, yShift, s * r);
            }
            glEnd();
        }
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void dibujarAnillosUranoAvanzado() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int numCapas = 2;
    float rMin = 3.0f, rMax = 3.5f;

    for (int i = 0; i < numCapas; ++i) {
        float t = (float)i / (numCapas - 1);
        float r1 = rMin + t * (rMax - rMin);
        float r2 = r1 + 0.3f;
        float alpha = 1.0f - t * 0.35f;

        glColor4f(0.6f, 0.85f, 1.0f, alpha);  // azul tenue
        glBegin(GL_TRIANGLE_STRIP);
        for (float ang = 0; ang <= 360; ang += 2.0f) {
            float rad = ang * M_PI / 180.0f;
            glVertex3f(cos(rad) * r1, 0.0f, sin(rad) * r1);
            glVertex3f(cos(rad) * r2, 0.0f, sin(rad) * r2);
        }
        glEnd();
    }

    // Glow externo
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    for (float r = 4.0f, alpha = 0.08f; r <= 4.5f; r += 0.3f, alpha -= 0.008f) {
        if (alpha <= 0.0f) break;
        for (float yShift = -0.15f; yShift <= 0.15f; yShift += 0.1f) {
            glColor4f(0.6f, 0.85f, 1.0f, alpha * (1.0f - fabs(yShift)));
            glBegin(GL_TRIANGLE_STRIP);
            for (float ang = 0; ang <= 360; ang += 2.0f) {
                float rad = ang * M_PI / 180.0f;
                float c = cos(rad), s = sin(rad);
                glVertex3f(c * (r - 0.2f), yShift, s * (r - 0.2f));
                glVertex3f(c * r, yShift, s * r);
            }
            glEnd();
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

// ------------------------
// DIBUJAR SISTEMA
// ------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---------- PROYECCIÓN ----------
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)windowWidth / windowHeight, 0.1, 500.0);

    // ---------- CÁMARA FPS ----------
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, centerX, centerY, centerZ, 0, 1, 0);

    // ---------- FONDO ESTRELLADO ----------
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturaFondo);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor3f(1, 1, 1);

    GLUquadric* qFondo = gluNewQuadric();
    gluQuadricTexture(qFondo, GL_TRUE);
    gluQuadricNormals(qFondo, GLU_NONE);
    gluQuadricOrientation(qFondo, GLU_INSIDE);
    gluSphere(qFondo, 380.0f, 80, 80);
    gluDeleteQuadric(qFondo);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glPopMatrix();

    // ---------- ILUMINACIÓN ----------
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // ---------- EMISIÓN DEL SOL ----------
    GLfloat mat_emission[] = { 0.8f, 0.8f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);

    // ---------- SOL ----------
    glPushMatrix();
    glRotatef(anguloSol, 0, 1, 0);
    glBindTexture(GL_TEXTURE_2D, texturaSol);
    glColor3f(1, 1, 1);
    GLUquadric* qSol = gluNewQuadric();
    gluQuadricTexture(qSol, GL_TRUE);
    gluQuadricNormals(qSol, GLU_SMOOTH);
    gluQuadricOrientation(qSol, GLU_OUTSIDE);
    gluSphere(qSol, 10.0f, 50, 50);
    gluDeleteQuadric(qSol);
    glPopMatrix();

    GLfloat no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);

    // ---------- PLANETAS Y SATÉLITES ----------
    for (Planeta& p : planetas) {
        glPushMatrix();
        glRotatef(p.angulo, 0, 1, 0);
        glTranslatef(p.distancia, 0, 0);
        glRotatef(p.rotacionPropia, 0, 1, 0);

        glBindTexture(GL_TEXTURE_2D, p.textura);
        glColor3f(1, 1, 1);

        GLUquadric* q = gluNewQuadric();
        gluQuadricTexture(q, GL_TRUE);
        gluQuadricNormals(q, GLU_SMOOTH);
        gluQuadricOrientation(q, GLU_OUTSIDE);
        gluSphere(q, p.radius, 30, 30);
        gluDeleteQuadric(q);

        if (p.nombre == "Saturno") {
            glPushMatrix();
            glRotatef(27.6f, 0, 0, 1);     // inclinación axial realista

            glDisable(GL_CULL_FACE);           // Visible desde atrás
            dibujarAnillosSaturnoAvanzado();
            glEnable(GL_CULL_FACE);
            glPopMatrix();
        }

        if (p.nombre == "Urano") {
            glPushMatrix();
            glRotatef(90, 0, 0, 1);             // Giro vertical
            glDisable(GL_CULL_FACE);           // Visible desde atrás
            dibujarAnillosUranoAvanzado();     // Llama la nueva función
            glEnable(GL_CULL_FACE);
            glPopMatrix();
        }

        // === SATÉLITES DEL PLANETA ===
        for (const Satelite& s : p.satelites) {
            glPushMatrix();
            glRotatef(s.angulo, 0, 1, 0);
            glTranslatef(s.distancia, 0, 0);
            glBindTexture(GL_TEXTURE_2D, s.textura);
            GLUquadric* qs = gluNewQuadric();
            gluQuadricTexture(qs, GL_TRUE);
            gluQuadricNormals(qs, GLU_SMOOTH);
            gluQuadricOrientation(qs, GLU_OUTSIDE);
            gluSphere(qs, s.radio, 20, 20);
            gluDeleteQuadric(qs);
            glPopMatrix();
        }

        glPopMatrix();
    }

    // ---------- HUD ----------
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    std::vector<std::string> instrucciones = {
        "Controles:",
        "WASD - Mover",
        "Mouse - Girar",
        "ESPACIO / SHIFT - Subir / Bajar",
        "ESC - Salir"
    };

    int startY = windowHeight - 20;
    for (const std::string& linea : instrucciones) {
        glRasterPos2i(20, startY);
        for (char c : linea)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        startY -= 22;
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}




void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

void idle() {
    anguloSol += 0.05f;
    anguloTierra += 0.2f;

    if (anguloSol >= 360.0f) anguloSol -= 360.0f;
    if (anguloTierra >= 360.0f) anguloTierra -= 360.0f;

    for (auto& p : planetas) {
        p.angulo += p.velocidad;
        if (p.angulo >= 360.0f) p.angulo -= 360.0f;

        p.rotacionPropia += 1.5f;  // velocidad de rotación diaria
        if (p.rotacionPropia >= 360.0f) p.rotacionPropia -= 360.0f;

        for (auto& s : p.satelites) {
            s.angulo += s.velocidad;
            if (s.angulo >= 360.0f) s.angulo -= 360.0f;
        }
    }

    glutPostRedisplay();
}


// KEYDOWN
void keyDown(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        exit(0);     // Cierra el programa
    }
}

// ------------------------
// MAIN
// ------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Sistema Solar FPS - Enrique");

    glEnable(GL_DEPTH_TEST);

    texturaSol = LoadBMP("../assets/Sol.bmp");
    texturaTierra = LoadBMP("../assets/PTierra.bmp");
    texturaFondo = LoadBMP("../assets/stars_milky_way_24bit_clean.bmp");

    luna.textura = LoadBMP("../assets/SLuna.bmp");
    fobos.textura = LoadBMP("../assets/SPhobos.bmp");
    deimos.textura = LoadBMP("../assets/SDeimos.bmp");


    planetas.push_back({ "Mercurio", 12.0f, 0.7f, 0.45f, LoadBMP("../assets/PMercurio.bmp"), 0.0f, 0.5f });
    planetas.push_back({ "Venus", 22.4f, 1.2f, 0.25f, LoadBMP("../assets/PVenus.bmp"), 0.0f, 0.3f });
    planetas.push_back({ "Tierra", 31.0f, 1.2f, 0.3f, texturaTierra, 0.0f, 1.0f });
    planetas.push_back({ "Marte", 47.2f, 0.9f, 0.35f, LoadBMP("../assets/PMarte.bmp"), 0.0f, 0.8f });
    planetas.push_back({ "Jupiter", 100.3f, 5.0f, 0.1f, LoadBMP("../assets/PJupiter.bmp"), 0.0f, 1.3f });
    planetas.push_back({ "Saturno", 150.2f, 4.2f, 0.07f, LoadBMP("../assets/PSaturno.bmp"), 0.0f, 1.0f });
    planetas.push_back({ "Urano", 260.7f, 2.3f, 0.05f, LoadBMP("../assets/PUrano.bmp"), 0.0f, 0.6f });
    planetas.push_back({ "Neptuno", 280.0f, 2.2f, 0.04f, LoadBMP("../assets/PNeptuno.bmp"), 0.0f, 0.7f });
    planetas.push_back({ "Pluton", 290.0f, 0.5f, 0.02f, LoadBMP("../assets/PPluton.bmp"), 0.0f, 0.2f });

    // === ASIGNAR SATÉLITES A PLANETAS ===
    for (auto& p : planetas) {
        if (p.nombre == "Tierra") {
            p.satelites.push_back(luna);
        }
        if (p.nombre == "Marte") {
            p.satelites.push_back(fobos);
            p.satelites.push_back(deimos);
        }
    }

    updateCenter();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMotion);
    glutTimerFunc(0, moverCamara, 0);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyDown);

    glutWarpPointer(windowWidth / 2, windowHeight / 2);
    glutSetCursor(GLUT_CURSOR_NONE);

    glutMainLoop();
    return 0;
}
