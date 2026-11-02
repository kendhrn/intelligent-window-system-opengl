#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Rotation and Camera
float xRotation = 0.0f, yRotation = 0.0f, zRotation = 0.0f;
float cameraDistance = 150.0f;
int lastMouseX, lastMouseY;
bool isMouseDragging = false;

// Component States
bool lightEnabled = true;
bool topFanRotate = false;
bool bottomFanRotate = false;
bool ventAnimate = false;
float topFanAngle = 0.0f;
float bottomFanAngle = 0.0f;
float ventAngle = 0.0f;
bool ventOpening = true;

// Glass and Climate States
float glassTintLevel = 0.3f;
int climateMode = 3; // Hot, Cold, Normal

// Exploded View
bool explodedView = false;
float explosionDistance = 0.0f;
float maxExplosion = 80.0f;

// Animation
float fanSpeedMultiplier = 1.0f;
float animationTime = 0.0f;

// Texture IDs
GLuint metalTexture, glassTexture, displayTexture;

// Function to create metal texture
GLuint createMetalTexture() {
    const int texSize = 256;
    unsigned char* data = new unsigned char[texSize * texSize * 3];

    for(int i = 0; i < texSize; i++) {
        for(int j = 0; j < texSize; j++) {
            int idx = (i * texSize + j) * 3;
            // Create brushed metal effect
            float noise = (rand() % 30) / 255.0f;
            float horizontal = sin(j * 0.5f) * 0.1f;
            float baseColor = 0.4f + noise + horizontal;
            data[idx] = (unsigned char)(baseColor * 200);
            data[idx + 1] = (unsigned char)(baseColor * 205);
            data[idx + 2] = (unsigned char)(baseColor * 210);
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return texID;
}

// Function to create shiny glass texture
GLuint createGlassTexture() {
    const int texSize = 256;
    unsigned char* data = new unsigned char[texSize * texSize * 4];

    for(int i = 0; i < texSize; i++) {
        for(int j = 0; j < texSize; j++) {
            int idx = (i * texSize + j) * 4;
            // Create smooth shiny glass
            float baseNoise = (rand() % 8) / 255.0f;
            float gradient = 0.95f + (float)(i + j) / (texSize * 4) * 0.05f;
            float brightness = gradient + baseNoise;
            if(brightness > 1.0f) brightness = 1.0f;
            data[idx] = (unsigned char)(235 * brightness);
            data[idx + 1] = (unsigned char)(245 * brightness);
            data[idx + 2] = (unsigned char)(255 * brightness);
            data[idx + 3] = 85;
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return texID;
}

// Function to create display screen texture
GLuint createDisplayTexture() {
    const int texSize = 128;
    unsigned char* data = new unsigned char[texSize * texSize * 3];

    for(int i = 0; i < texSize; i++) {
        for(int j = 0; j < texSize; j++) {
            int idx = (i * texSize + j) * 3;
            // Create digital display pattern
            float grid = ((i % 4 == 0) || (j % 4 == 0)) ? 0.9f : 0.7f;
            data[idx] = (unsigned char)(20 * grid);
            data[idx + 1] = (unsigned char)(100 * grid);
            data[idx + 2] = (unsigned char)(80 * grid);
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return texID;
}

void init() {
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Setup lighting
    GLfloat lightPos[] = {50.0f, 100.0f, 100.0f, 1.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);

    // Create textures
    metalTexture = createMetalTexture();
    glassTexture = createGlassTexture();
    displayTexture = createDisplayTexture();
}

// Draw glass panel
void drawGlassPanel(float width, float height, float depth, float r, float g, float b, float alpha, bool useTexture) {
    glPushMatrix();
    glColor4f(r, g, b, alpha);

    if(useTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, glassTexture);
    }

    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-width/2, -height/2, depth/2);
    glTexCoord2f(1, 0); glVertex3f(width/2, -height/2, depth/2);
    glTexCoord2f(1, 1); glVertex3f(width/2, height/2, depth/2);
    glTexCoord2f(0, 1); glVertex3f(-width/2, height/2, depth/2);
    glEnd();

    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glTexCoord2f(0, 0); glVertex3f(-width/2, -height/2, -depth/2);
    glTexCoord2f(0, 1); glVertex3f(-width/2, height/2, -depth/2);
    glTexCoord2f(1, 1); glVertex3f(width/2, height/2, -depth/2);
    glTexCoord2f(1, 0); glVertex3f(width/2, -height/2, -depth/2);
    glEnd();

    if(useTexture) glDisable(GL_TEXTURE_2D);

    // Edges with glow effect
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.4f, 0.6f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-width/2, -height/2, depth/2);
    glVertex3f(width/2, -height/2, depth/2);
    glVertex3f(width/2, height/2, depth/2);
    glVertex3f(-width/2, height/2, depth/2);
    glEnd();
    if(lightEnabled) glEnable(GL_LIGHTING);

    glPopMatrix();
}

// Draw Outer Smart Glass Layer
void drawOuterSmartGlass() {
    glPushMatrix();
    if(explodedView) glTranslatef(0, 0, explosionDistance * 1.5f);
    glScalef(1.04f, 1.04f, 1.0f);
    float tintAlpha = 0.6f - (glassTintLevel * 0.4f);
    drawGlassPanel(60, 80, 1, 0.3f + glassTintLevel*0.3f, 0.6f, 0.9f - glassTintLevel*0.3f, tintAlpha, true);
    glPopMatrix();
}

// Draw Solar Coating Layer
void drawSolarCoating() {
    glPushMatrix();
    if(explodedView) {
        glTranslatef(0, 0, explosionDistance * 1.2f);
    } else {
        glTranslatef(0, 0, -1.5f);
    }
    drawGlassPanel(59, 79, 0.3f, 0.8f, 0.7f, 0.4f, 0.3f, false);
    glPopMatrix();
}

// Airflow visualization
void drawAirflowChamber() {
    glPushMatrix();
    if(explodedView) {
        glTranslatef(0, 0, explosionDistance * 0.5f);
    } else {
        glTranslatef(0, 0, -5);
    }

    glDisable(GL_LIGHTING);
    glColor4f(0.2f, 0.25f, 0.3f, 0.4f);
    float w = 58, h = 78, d = 8;

    // Top wall
    glBegin(GL_QUADS);
    glVertex3f(-w/2, h/2, d/2);
    glVertex3f(w/2, h/2, d/2);
    glVertex3f(w/2, h/2, -d/2);
    glVertex3f(-w/2, h/2, -d/2);
    glEnd();

    // Bottom wall
    glBegin(GL_QUADS);
    glVertex3f(-w/2, -h/2, d/2);
    glVertex3f(-w/2, -h/2, -d/2);
    glVertex3f(w/2, -h/2, -d/2);
    glVertex3f(w/2, -h/2, d/2);
    glEnd();

    // Left wall
    glBegin(GL_QUADS);
    glVertex3f(-w/2, -h/2, d/2);
    glVertex3f(-w/2, h/2, d/2);
    glVertex3f(-w/2, h/2, -d/2);
    glVertex3f(-w/2, -h/2, -d/2);
    glEnd();

    // Right wall
    glBegin(GL_QUADS);
    glVertex3f(w/2, -h/2, d/2);
    glVertex3f(w/2, -h/2, -d/2);
    glVertex3f(w/2, h/2, -d/2);
    glVertex3f(w/2, h/2, d/2);
    glEnd();

    // Simple airflow visualization (curved lines)
    if(climateMode == 1) {
        glColor3f(1.0f, 0.3f, 0.2f); // Hot mode - red airflow
    } else if(climateMode == 2) {
        glColor3f(0.2f, 0.4f, 0.8f); // Cold mode - blue
    } else {
        glColor3f(0.3f, 0.7f, 0.5f);
    }

    glLineWidth(2.0f);
    for(int i = 0; i < 5; i++) {
        float offset = (i - 2) * 10.0f;
        glBegin(GL_LINE_STRIP);
        for(float t = -h/2; t <= h/2; t += 2.0f) {
            float x = offset + sin(t * 0.1f) * 3.0f;
            glVertex3f(x, t, 0);
        }
        glEnd();
    }

    if(lightEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();
}

// Draw Micro Vents
void drawMicroVents() {
    glPushMatrix();
    if(explodedView) {
        glTranslatef(0, explosionDistance * 0.0f, explosionDistance * 1.0f);
    } else {
        glTranslatef(0, 0, -5);
    }

    glColor3f(0.4f, 0.4f, 0.45f);

    // Top vents
    glPushMatrix();
    glTranslatef(0, 39, 0);
    for(int i = 0; i < 8; i++) {
        glPushMatrix();
        glTranslatef((i - 3.5f) * 7, 0, 0);
        glRotatef(ventAngle, 1, 0, 0);
        glBegin(GL_QUADS);
        glVertex3f(-3, 0, -1);
        glVertex3f(3, 0, -1);
        glVertex3f(3, 0, 1);
        glVertex3f(-3, 0, 1);
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();

    // Bottom vents
    glPushMatrix();
    glTranslatef(0, -39, 0);
    for(int i = 0; i < 8; i++) {
        glPushMatrix();
        glTranslatef((i - 3.5f) * 7, 0, 0);
        glRotatef(ventAngle, 1, 0, 0);
        glBegin(GL_QUADS);
        glVertex3f(-3, 0, -1);
        glVertex3f(3, 0, -1);
        glVertex3f(3, 0, 1);
        glVertex3f(-3, 0, 1);
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();

    glPopMatrix();
}

// Draw Mini Cooling Fan with metal texture
void drawMiniCoolingFan(bool isTop, float angle) {
    glPushMatrix();

    float yPos = isTop ? 38 : -38;
    float explodeOffset = isTop ? explosionDistance * 1.0f : -explosionDistance * 1.0f;

    if(explodedView) {
        glTranslatef(0, explodeOffset, explosionDistance * 0.5f);
    } else {
        glTranslatef(0, yPos, -5);
    }

    // Fan housing
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, metalTexture);
    glColor3f(0.8f, 0.8f, 0.85f);

    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-8, -4, 4);
    glTexCoord2f(1, 0); glVertex3f(8, -4, 4);
    glTexCoord2f(1, 1); glVertex3f(8, 4, 4);
    glTexCoord2f(0, 1); glVertex3f(-8, 4, 4);
    glEnd();

    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glTexCoord2f(0, 0); glVertex3f(-8, -4, -4);
    glTexCoord2f(1, 0); glVertex3f(8, -4, -4);
    glTexCoord2f(1, 1); glVertex3f(8, 4, -4);
    glTexCoord2f(0, 1); glVertex3f(-8, 4, -4);
    glEnd();

    // Side faces
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-8, 4, -4);
    glTexCoord2f(1, 0); glVertex3f(8, 4, -4);
    glTexCoord2f(1, 1); glVertex3f(8, 4, 4);
    glTexCoord2f(0, 1); glVertex3f(-8, 4, 4);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord2f(0, 0); glVertex3f(-8, -4, -4);
    glTexCoord2f(0, 1); glVertex3f(-8, -4, 4);
    glTexCoord2f(1, 1); glVertex3f(8, -4, 4);
    glTexCoord2f(1, 0); glVertex3f(8, -4, -4);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(-8, -4, -4);
    glTexCoord2f(1, 0); glVertex3f(-8, 4, -4);
    glTexCoord2f(1, 1); glVertex3f(-8, 4, 4);
    glTexCoord2f(0, 1); glVertex3f(-8, -4, 4);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(8, -4, -4);
    glTexCoord2f(0, 1); glVertex3f(8, -4, 4);
    glTexCoord2f(1, 1); glVertex3f(8, 4, 4);
    glTexCoord2f(1, 0); glVertex3f(8, 4, -4);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Add ventilation grill pattern on front
    glDisable(GL_LIGHTING);
    glColor3f(0.15f, 0.15f, 0.18f);
    glLineWidth(2.0f);
    for(int i = 1; i < 7; i++) {
        float yOffset = -4.0f + i * (8.0f / 7.0f);
        glBegin(GL_LINES);
        glVertex3f(-7.5f, yOffset, 4.01f);
        glVertex3f(7.5f, yOffset, 4.01f);
        glEnd();
    }

    for(int i = 1; i < 5; i++) {
        float xOffset = -8.0f + i * (16.0f / 5.0f);
        glBegin(GL_LINES);
        glVertex3f(xOffset, -3.5f, 4.01f);
        glVertex3f(xOffset, 3.5f, 4.01f);
        glEnd();
    }

    if(lightEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();

    // Fan blades
    glPushMatrix();
    glTranslatef(0, 0, -1);
    glRotatef(angle, 0, 0, 1);
    glDisable(GL_LIGHTING);

    for(int i = 0; i < 4; i++) {
        glPushMatrix();
        glRotatef(i * 90, 0, 0, 1);

        glColor3f(0.7f, 0.7f, 0.75f);
        glBegin(GL_QUADS);
        glVertex3f(0.5f, -1.5f, 0.3f);
        glVertex3f(5.5f, -1.2f, 0.3f);
        glVertex3f(6.0f, 1.2f, 0.3f);
        glVertex3f(1.0f, 1.5f, 0.3f);
        glEnd();

        glColor3f(0.5f, 0.5f, 0.55f);
        glBegin(GL_QUADS);
        glVertex3f(0.5f, -1.5f, 0.3f);
        glVertex3f(0.5f, -1.5f, -0.3f);
        glVertex3f(5.5f, -1.2f, -0.3f);
        glVertex3f(5.5f, -1.2f, 0.3f);
        glEnd();

        glBegin(GL_QUADS);
        glVertex3f(5.5f, -1.2f, 0.3f);
        glVertex3f(5.5f, -1.2f, -0.3f);
        glVertex3f(6.0f, 1.2f, -0.3f);
        glVertex3f(6.0f, 1.2f, 0.3f);
        glEnd();

        glColor3f(0.6f, 0.6f, 0.65f);
        glBegin(GL_QUADS);
        glVertex3f(0.5f, -1.5f, -0.3f);
        glVertex3f(1.0f, 1.5f, -0.3f);
        glVertex3f(6.0f, 1.2f, -0.3f);
        glVertex3f(5.5f, -1.2f, -0.3f);
        glEnd();

        glPopMatrix();
    }

    glColor3f(0.3f, 0.3f, 0.35f);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 1.5, 1.5, 1.2, 16, 1);
    glPushMatrix();
    glTranslatef(0, 0, 1.2);
    gluDisk(quad, 0, 1.5, 16, 1);
    glPopMatrix();
    gluDeleteQuadric(quad);

    if(lightEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();
}

// Draw Inner Insulated Glass Layer
void drawInnerInsulatedGlass() {
    glPushMatrix();
    if(explodedView) {
        glTranslatef(0, 0, -explosionDistance * 0.8f);
    } else {
        glTranslatef(0, 0, -10);
    }
    drawGlassPanel(60, 80, 1.5f, 0.7f, 0.75f, 0.8f, 0.5f, true);
    glPopMatrix();
}

// Draw sensor module
void drawSensor(float x, float y, float z, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);
    glutSolidSphere(1.5, 16, 16);
    glPopMatrix();
}

// Draw Intelligent Frame System with metal texture
void drawIntelligentFrame() {
    glPushMatrix();
    if(explodedView) {
        glScalef(1.0f + explosionDistance * 0.01f, 1.0f + explosionDistance * 0.01f, 1.0f);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, metalTexture);
    glColor3f(0.8f, 0.8f, 0.85f);

    float h = 84, d = 15, thick = 4;

    // Top frame
    glPushMatrix();
    glTranslatef(0, h/2, -d/2);
    glScalef(68, thick, d);
    glutSolidCube(1);
    glPopMatrix();

    // Bottom frame
    glPushMatrix();
    glTranslatef(0, -h/2, -d/2);
    glScalef(68, thick, d);
    glutSolidCube(1);
    glPopMatrix();

    // Left frame
    glPushMatrix();
    glTranslatef(-64/2, 0, -d/2);
    glScalef(thick, h, d);
    glutSolidCube(1);
    glPopMatrix();

    // Right frame
    glPushMatrix();
    glTranslatef(64/2, 0, -d/2);
    glScalef(thick, h, d);
    glutSolidCube(1);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // Sensors
    if(climateMode == 1) {
        drawSensor(32.5, 0, 1, 1.0f, 0.2f, 0.2f); // Hot - red
    } else if(climateMode == 2) {
        drawSensor(32.5, 0, 1, 0.2f, 0.4f, 1.0f); // Cold - blue
    } else {
        drawSensor(32.5, 0, 1, 0.3f, 1.0f, 0.3f); // Normal - green
    }

    // Light sensor
    drawSensor(32.5, 40, 1, 0.9f, 0.9f, 0.3f);

    // Microcontroller box
    glPushMatrix();
    glTranslatef(28, -35, 0);
    glColor3f(0.15f, 0.15f, 0.2f);
    glScalef(6, 8, 4);
    glutSolidCube(1);
    glPopMatrix();

    // Display panel with texture
    glPushMatrix();
    glTranslatef(-28, -38, 2);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, displayTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-3, -2, 0);
    glTexCoord2f(1, 0); glVertex3f(3, -2, 0);
    glTexCoord2f(1, 1); glVertex3f(3, 2, 0);
    glTexCoord2f(0, 1); glVertex3f(-3, 2, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // LED indicators with glow effect
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Pulsating glow effect
    float glowIntensity = 0.5f + 0.5f * sin(animationTime * 3.0f);

    glColor4f(0.0f, 1.0f, 0.0f, glowIntensity * 0.8f);
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    glVertex3f(-1, 0, 0.1f);
    glVertex3f(1, 0, 0.1f);
    glEnd();

    glColor4f(0.0f, 1.0f, 0.0f, glowIntensity * 0.3f);
    glPointSize(12.0f);
    glBegin(GL_POINTS);
    glVertex3f(-1, 0, 0.1f);
    glVertex3f(1, 0, 0.1f);
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(lightEnabled) glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();
}

void drawCompleteIWS() {
    drawIntelligentFrame();
    drawOuterSmartGlass();
    drawSolarCoating();
    drawAirflowChamber();
    drawMicroVents();
    drawMiniCoolingFan(true, topFanAngle);
    drawMiniCoolingFan(false, bottomFanAngle);
    drawInnerInsulatedGlass();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0, 0, cameraDistance, 0, 0, 0, 0, 1, 0);

    glRotatef(xRotation, 1, 0, 0);
    glRotatef(yRotation, 0, 1, 0);
    glRotatef(zRotation, 0, 0, 1);

    drawCompleteIWS();

    // Draw text info
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(10, 580);
    char info[100];
    snprintf(info, sizeof(info), "Mode: %s | Exploded: %s | Press 'H' for help",
            climateMode == 1 ? "HOT" : (climateMode == 2 ? "COLD" : "NORMAL"),
            explodedView ? "YES" : "NO");
    for(char* c = info; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if(lightEnabled) glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

// Timer callback for animations
void timer(int /*value*/) {
    animationTime += 0.016f; // ~16ms per frame

    // Fan animations
    if(topFanRotate) {
        topFanAngle += 12.0f * fanSpeedMultiplier;
        if(topFanAngle > 360.0f) topFanAngle -= 360.0f;
    }

    if(bottomFanRotate) {
        bottomFanAngle += 12.0f * fanSpeedMultiplier;
        if(bottomFanAngle > 360.0f) bottomFanAngle -= 360.0f;
    }

    // Vent animation
    if(ventAnimate) {
        if(ventOpening) {
            ventAngle += 2.0f;
            if(ventAngle >= 90.0f) {
                ventAngle = 90.0f;
                ventOpening = false;
            }
        } else {
            ventAngle -= 2.0f;
            if(ventAngle <= 0.0f) {
                ventAngle = 0.0f;
                ventOpening = true;
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// Apply climate mode settings
void applyClimateMode(int mode) {
    climateMode = mode;

    if(mode == 1) { // HOT
        glassTintLevel = 0.8f;
        ventAngle = 90.0f;
        topFanRotate = true;
        bottomFanRotate = true;
        fanSpeedMultiplier = 2.0f;
    } else if(mode == 2) { // COLD
        glassTintLevel = 0.1f;
        ventAngle = 0.0f;
        topFanRotate = false;
        bottomFanRotate = false;
        fanSpeedMultiplier = 0.0f;
    } else { // NORMAL
        glassTintLevel = 0.4f;
        ventAngle = 45.0f;
        topFanRotate = true;
        bottomFanRotate = true;
        fanSpeedMultiplier = 1.0f;
    }
}

// Keyboard callback
void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch(key) {
        case 27: exit(0); break;
        case 'z': case 'Z':
            cameraDistance -= 5.0f;
            if(cameraDistance < 50.0f) cameraDistance = 50.0f;
            break;
        case 'x': case 'X':
            cameraDistance += 5.0f;
            if(cameraDistance > 300.0f) cameraDistance = 300.0f;
            break;
        case 'a': case 'A': zRotation += 5.0f; break;
        case 'd': case 'D': zRotation -= 5.0f; break;
        case 'l': case 'L':
            lightEnabled = true;
            glEnable(GL_LIGHTING);
            printf("Light enabled\n");
            break;
        case 'k': case 'K':
            lightEnabled = false;
            glDisable(GL_LIGHTING);
            printf("Light disabled\n");
            break;
        case 'f': case 'F': topFanRotate = !topFanRotate; break;
        case 'g': case 'G': bottomFanRotate = !bottomFanRotate; break;
        case 'v': case 'V': ventAnimate = !ventAnimate; break;
        case 't': case 'T':
            glassTintLevel += 0.1f;
            if(glassTintLevel > 1.0f) glassTintLevel = 1.0f;
            break;
        case 'y': case 'Y':
            glassTintLevel -= 0.1f;
            if(glassTintLevel < 0.0f) glassTintLevel = 0.0f;
            break;
        case '1':
            applyClimateMode(1); // HOT
            printf("Mode 1 (HOT): Dark glass, open vents, fast fans\n");
            break;
        case '2':
            applyClimateMode(2); // COLD
            printf("Mode 2 (COLD): Clear glass, closed vents, fans off\n");
            break;
        case '3':
            applyClimateMode(3); // NORMAL
            printf("Mode 3 (NORMAL): Balanced settings\n");
            break;
        case 'e': case 'E':
            explodedView = !explodedView;
            explosionDistance = explodedView ? maxExplosion : 0.0f;
            printf("Exploded view: %s\n", explodedView ? "ON" : "OFF");
            break;
        case '+': case '=':
            explosionDistance += 5.0f;
            if(explosionDistance > 150.0f) explosionDistance = 150.0f;
            if(explosionDistance > 0) explodedView = true;
            break;
        case '-': case '_':
            explosionDistance -= 5.0f;
            if(explosionDistance < 0.0f) explosionDistance = 0.0f;
            if(explosionDistance == 0) explodedView = false;
            break;
        case 'h': case 'H':
            printf("\n=== ENHANCED IWS CONTROLS ===\n");
            printf("Arrow Keys: Rotate view\n");
            printf("Z/X: Camera zoom\n");
            printf("A/D: Rotate around Z-axis\n");
            printf("L/K: Light on/off\n");
            printf("F: Toggle top fan\n");
            printf("G: Toggle bottom fan\n");
            printf("V: Animate vents\n");
            printf("T/Y: Adjust glass tint\n");
            printf("1: HOT mode\n");
            printf("2: COLD mode\n");
            printf("3: NORMAL mode\n");
            printf("E: Toggle exploded view\n");
            printf("+/=: Increase explosion distance\n");
            printf("-/_: Decrease explosion distance\n");
            printf("ESC: Exit\n");
            break;
    }
    glutPostRedisplay();
}

// Special keys callback
void specialKeys(int key, int /*x*/, int /*y*/) {
    switch(key) {
        case GLUT_KEY_UP: xRotation += 5.0f; break;
        case GLUT_KEY_DOWN: xRotation -= 5.0f; break;
        case GLUT_KEY_LEFT: yRotation -= 5.0f; break;
        case GLUT_KEY_RIGHT: yRotation += 5.0f; break;
    }
    glutPostRedisplay();
}

// Mouse callback
void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON) {
        if(state == GLUT_DOWN) {
            isMouseDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            isMouseDragging = false;
        }
    }
}

// Mouse motion callback
void mouseMotion(int x, int y) {
    if(isMouseDragging) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;
        yRotation += dx * 0.5f;
        xRotation += dy * 0.5f;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Intelligent Window System (IWS) - Adaptive Climate & Light Control");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, timer, 0);

    printf("=== Intelligent Window System (IWS) ===\n");
    printf("Press 'H' for help\n");
    printf("\nStarting in NORMAL mode...\n\n");

    applyClimateMode(3); // Start in normal mode

    glutMainLoop();
    return 0;
}
