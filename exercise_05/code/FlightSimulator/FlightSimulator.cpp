#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>

#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <string>
#include <iostream>

#include "Jet.h"
#include "FlightSimulator.h"
#include "OffReader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double townColor = 0.35;
const double buildingColor = 0.66;

CGMainWindow::CGMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {
    resize(604, 614);

    // Create a nice frame to put around the OpenGL widget
    QFrame *f = new QFrame(this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(3);

    // Create our OpenGL widget
    ogl = new CGView(this, f);

    QMenu *file = new QMenu("&Plane", this);
    file->addAction("&Reset plane", this->ogl, SLOT(reset_plane()), Qt::CTRL + Qt::Key_R);
    file->addAction("Toggle &Follow view", this->ogl, SLOT(toggle_follow()), Qt::CTRL + Qt::Key_F);
    file->addAction("Toggle &Wireframe Landscape", this->ogl, SLOT(toggle_wire()), Qt::CTRL + Qt::Key_W);
    file->addAction("Quit", this, SLOT(close()), Qt::CTRL + Qt::Key_Q);

    menuBar()->addMenu(file);

    // Put the GL widget inside the frame
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(ogl);
    layout->setMargin(0);
    f->setLayout(layout);

    setCentralWidget(f);

    statusBar()->showMessage("Ready", 1000);
}

CGMainWindow::~CGMainWindow() {}


CGView::CGView(CGMainWindow *mainwindow, QWidget *parent) : QGLWidget(parent) {
    main = mainwindow;
    fov = 0.4;
    maxHeight = 1;

    phi = 1.5;
    theta = 3.14;

    plane_fly = false;
    follow  = false;

    // start in the middle, 2km above the ground
    place = Vector3d(0, 0, 2000);

    /// Diskreter Schritt bei der Bewegung!
    step = 0.01;

    // rendering mode of the terrain: true for wire only, false for a mixture
    // of wireframe + monochrome triangles.
    wire = false;

    // load the town geometry and compute the "blurred footprint"
    init_town();

    /// Um das H"ohenfeld zu erzeugen
    init_height();

    // generating geometry of a plane (wing span 14m, length 16m)
    double wingLength = 7;
    double bodyWidth = 2.2;
    double noseLength = 1.3;
    double wingWidth = 2.3;
    double tailWidth = 1.7;
    double bodyLength = 12;
    double rearWingLength = 2.5;
    double rearWingWidth = 1.7;

    std::vector<Vector3d> p;
    p.push_back(Vector3d(0, bodyWidth / 2, 0));
    p.push_back(Vector3d(-noseLength, bodyWidth / 2,  0));
    p.push_back(Vector3d(-noseLength, wingLength, 0));
    p.push_back(Vector3d(-noseLength - wingWidth, wingLength, 0));
    p.push_back(Vector3d(-noseLength - wingWidth, bodyWidth / 2, 0));
    p.push_back(Vector3d(-bodyLength + rearWingWidth, tailWidth / 2, 0));
    p.push_back(Vector3d(-bodyLength + rearWingWidth, rearWingLength, 0));
    p.push_back(Vector3d(-bodyLength, rearWingLength, 0));
    int numVertices = p.size();
    int i;

    for (i = numVertices - 1; i >= 0; i--) {
        Vector3d right = p[i];
        Vector3d left = Vector3d(right.x(), -right.y(), 0);
        p.push_back(left);
    }

    plane.setPoints(p);
    plane.placeMe(place);



    /// Um Keyboard-Events durchzulassen
    setFocusPolicy(Qt::StrongFocus);
}


/// Initialisiert das H"ohenfeld
void CGView::init_height() {
    double s = 0.25;
    double t = 0.15;
    double a = sizeX * sizeY;

    double landscapeMaxHeight = 5000;
    double terrainFunction;
    double weight;

    for (int i = 0; i < sizeX; i++)
        for (int j = 0; j < sizeY; j++) {
            terrainFunction = (1 / a) * (j * i) + 0.05 * (sin(s * i) + cos(t * j));
            terrainFunction *= landscapeMaxHeight * terrainFunction;
            weight = town_vs_terrain[i][j];
            height_vector[i][j] =
                weight * townPlateauHeight + (1 - weight) * terrainFunction;

            ///Hier die normalen!
            Vector3d dx(1, j / a + s * 0.05 * cos(s * i), 0);
            Vector3d dy(0, i / a - t * 0.05 * sin(t * j), 1);
            normal_vector[i][j] = (dy % dx);
            normal_vector[i][j].normalize();

        }

    return;
}

double townWeight(double radius) {
    if (radius <= townFlatRadius) {
        return 1.0;
    } else if (radius >= townMaxRadius) {
        return 0.0;
    } else {
        return 0.5 * (cos(
                          M_PI * (radius - townFlatRadius) / (townMaxRadius - townFlatRadius)
                      ) + 1);
    }
}

void gridPointToWorld(int x, int y, double &wX, double &wY) {
    wX = (x * landscapeSizeX / sizeX) - 0.5 * landscapeSizeX;
    wY = (y * landscapeSizeY / sizeY) - 0.5 * landscapeSizeY;
}

void closestWorldToGridCoord(double wx, double wy, int &x, int &y) {
    x = std::max(0, std::min(sizeX - 1, (int)floor(0.5 + sizeX * (wx / landscapeSizeX + 0.5))));
    y = std::max(0, std::min(sizeY - 1, (int)floor(0.5 + sizeY * (wy / landscapeSizeY + 0.5))));
}

// returns a rectangular range of integer indices of terrain
// points that can be affected by the world-coordinate point (x, y)
void affectedGridPointRange(
    double x, double y, int &minX, int &maxX, int &minY, int &maxY
) {
    closestWorldToGridCoord(x - townMaxRadius, y - townMaxRadius, minX, minY);
    closestWorldToGridCoord(x + townMaxRadius, y + townMaxRadius, maxX, maxY);
}

// Load the geometry of the town and rescale it accordingly.
// This function also initializes the town_vs_terrain weight function,
// by drawing a "blurred footprint" of the town into the town_vs_terrain
void CGView::init_town() {

    LoadOffFile("Houston.off", town_vertices, town_triangles);

    // rescale the town (the tallest building in houston is 305m high)
    double minZ = 100000000;
    double maxZ = -minZ;
    double minX = 100000000;
    double maxX = -minX;
    double minY = 100000000;
    double maxY = -minY;
    int i;

    for (i = 0; i < town_vertices.size(); i++) {
        Vector3d v = town_vertices[i];
        minX = fmin(minX, v.x()); maxX = fmax(maxX, v.x());
        minY = fmin(minY, v.y()); maxY = fmax(maxY, v.y());
        minZ = fmin(minZ, v.z()); maxZ = fmax(maxZ, v.z());
    }

    // initialize town_vs_terrain to 0
    for (i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            town_vs_terrain[i][j] = 0;
        }
    }

    // scale and compute the "town footprint"
    // "305" is a magic number: it's the height of the highest skyscraper in
    // Houston
    double scaleFactor = 305.0 / (maxZ - minZ);

    for (i = 0; i < town_vertices.size(); i++) {
        Vector3d v = town_vertices[i];
        Vector3d scaled(
            (v.x() - (minX + maxX) / 2) * scaleFactor,
            (v.y() - (minY - maxY) / 2) * scaleFactor,
            (v.z() - minZ) * scaleFactor + townPlateauHeight + 0.1
        );
        town_vertices[i] = scaled;

        // the footprint part
        int minX, maxX, minY, maxY;
        double wx, wy, r, weight;
        affectedGridPointRange(scaled.x(), scaled.y(), minX, maxX, minY, maxY);

        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                gridPointToWorld(x, y, wx, wy);
                r = hypot(wx - scaled.x(), wy - scaled.y());
                weight = townWeight(r);
                town_vs_terrain[x][y] = fmax(weight, town_vs_terrain[x][y]);
            }
        }
    }
}

/// Malt das H"ohenfeld
void CGView::drawLandscape(bool wire, Vector3d color) {

    glPushMatrix();
    glTranslated(-landscapeSizeX / 2, -landscapeSizeY / 2, 0);
    glScaled(
        landscapeSizeX / (sizeX - 1),
        landscapeSizeY / (sizeY - 1),
        1
    );
    glLineWidth(3);

    if (!wire) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    double w, wc; // "w complement = 1 - w"

    for (int i = 0; i < (sizeX - 1); i++) {
        glBegin(GL_TRIANGLE_STRIP);

        for (int j = 0; j < (sizeY); j++) {
            w = town_vs_terrain[i][j];

            if (w < 0.95) w = 0;

            wc = 1 - w;
            glColor3f(
                w * townColor + wc * color.x(),
                w * townColor + wc * color.y(),
                w * townColor + wc * color.z()
            );
            glVertex3d(i, j, height_vector[i][j]);
            glColor3f(
                w * townColor + wc * color.x(),
                w * townColor + wc * color.y(),
                w * townColor + wc * color.z()
            );
            glVertex3d(i + 1, j, height_vector[i + 1][j]);
        }

        glEnd();
    }

    glPopMatrix();
}

#define COLORED(i) v = town_vertices[town_triangles[i]]; \
  if (v.z() > 10 + townPlateauHeight) \
    glColor3f(buildingColor, buildingColor, buildingColor); \
  else glColor3f(townColor, townColor, townColor); \
  glVertex3dv(town_vertices[town_triangles[i]].ptr());


void CGView::drawTown() {
    glPushMatrix();

    glColor3f(townColor, townColor, townColor);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    Vector3d v;

    for (int i = 0; i < town_triangles.size(); i += 3) {
        COLORED(i);
        COLORED(i + 1);
        COLORED(i + 2);
    }

    glEnd();

    glColor3f(0, 0, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);

    for (int i = 0; i < town_triangles.size(); i += 3) {
        glVertex3dv(town_vertices[town_triangles[i + 0]].ptr());
        glVertex3dv(town_vertices[town_triangles[i + 1]].ptr());
        glVertex3dv(town_vertices[town_triangles[i + 2]].ptr());
    }

    glEnd();

    glPopMatrix();
}

void CGView::drawOcean() {
    glPushMatrix();
    double seaLevel = townPlateauHeight - 60;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0.1, 0.6, 0.5);
    glPolygonMode(GL_FRONT, GL_FILL);
    glBegin(GL_TRIANGLES);
    glVertex3d(landscapeSizeX / 2, landscapeSizeY / 2, seaLevel);
    glVertex3d(-landscapeSizeX / 2, landscapeSizeY / 2, seaLevel);
    glVertex3d(-landscapeSizeX / 2, -landscapeSizeY / 2, seaLevel);
    glVertex3d(-landscapeSizeX / 2, -landscapeSizeY / 2, seaLevel);
    glVertex3d(landscapeSizeX / 2, -landscapeSizeY / 2, seaLevel);
    glVertex3d(landscapeSizeX / 2, landscapeSizeY / 2, seaLevel);
    glEnd();

    glPopMatrix();
}

/// Das kennt Ihr aus der Vorlesung
void CGView::resizeGL(int width, int height) {
    double ratio;

    int W = width;
    int H = height;
    glViewport(0, 0, W, H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (W > H) {
        ratio = (double) W / (double) H;
        gluPerspective(180.0 * fov, ratio, 0.1, 10000.0);
    } else {
        ratio = (double) H / (double) W;
        gluPerspective(180.0 * fov, 1.0 / ratio, 0.1, 10000.0);
    }

    glMatrixMode(GL_MODELVIEW);
}




void CGView::initializeGL() {

    glClearColor(0.0, 0.0, 0.0, 0.0);

    qglClearColor(Qt::black);
    zoom = 100; // this is now distance to the plane... Unclear what else it
    // should be...

    ///Make timer
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(20);
}


void CGView::paintGL() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Das ist zur korrekten Darstelung des Fliegers, wenn er
    /// hinter oder vor dem H"ohenfeld fliegt!
    /// Bitte vorerst ignorieren, das lernt ihr sp"ater!
    glEnable(GL_DEPTH_TEST);

    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (follow) {
        Vector3d cameraPos = plane.origin() + plane.up() * 5 - plane.nose() * 20;
        Vector3d lookAtPos = plane.origin() + plane.up() * 5;
        gluLookAt(
            cameraPos.x(), cameraPos.y(), cameraPos.z(),
            lookAtPos.x(), lookAtPos.y(), lookAtPos.z(),
            // we add a constant offset into the z-direction to make it look
            // a little more dynamic and give a feeling of the current roll
            plane.up().x(), plane.up().y(), plane.up().z() + 0.8
        );
    } else {
        double sT = sin(theta);
        double cT = cos(theta);
        double sP = sin(phi);
        double cP = cos(phi);
        gluLookAt(
            plane.origin().x() + zoom * cP * cT,
            plane.origin().y() + zoom * cP * sT,
            plane.origin().z() + zoom * sP,
            plane.origin().x(),
            plane.origin().y(),
            plane.origin().z(),
            0, 0, 1
        );
    }


    Vector3d landscapeColor;

    if (!wire) {
        landscapeColor = Vector3d(237 / 256.0, 201 / 256.0, 175 / 256.0);
        drawLandscape(false, landscapeColor);
    }

    if (wire) {
        landscapeColor = Vector3d(0.0, 1, 0.0);
    } else {
        landscapeColor = Vector3d(120 / 256.0, 110 / 256.0, 90 / 256.0);
    }

    drawLandscape(true, landscapeColor);
    drawTown();
    drawOcean();
    /*
    drawYachts();
    drawSunrise();
    ...
    // ok, weekend over... That was kind of relaxing^^
    */

    plane.draw(Vector3d(0.9, 0.4, 0.3), true);
}

void CGView::mousePressEvent(QMouseEvent *event) {
    oldX = event->x();
    oldY = event->y();
    oldTheta = theta;
    oldPhi = phi;
}

void CGView::mouseReleaseEvent(QMouseEvent *) {}

void CGView::wheelEvent(QWheelEvent *event) {
    if (event->delta() < 0) zoom /= 1.2; else zoom *= 1.2;

    update();
}

void CGView::mouseMoveEvent(QMouseEvent *event) {
    theta = oldTheta + 0.002 * (event->x() - oldX);
    phi = fmax(-1.5, fmin(1.5, oldPhi + 0.002 * (event->y() - oldY)));
    // oldX = event->x();
    // oldY = event->y();
    updateGL();
}

/// Hier der timer-Slot zur Animation!
void CGView::timer() {
    if (plane_fly) plane.flyMe(step);

    updateGL();
}

/// Ganz viele Funktionen zum steuern des Fliegers!
void CGView::keyPressEvent(QKeyEvent *event) {

    switch (event->key()) {
    case Qt::Key_Up    : plane.pitchMe(0.03); break;

    case Qt::Key_Down  : plane.pitchMe(-0.03); break;

    case Qt::Key_Left  : plane.rollMe(-0.05); break;

    case Qt::Key_Right : plane.rollMe(0.05); break;

    case Qt::Key_Space : plane_fly = !plane_fly; break;

    case Qt::Key_N     : plane.yawMe(0.01); break;

    case Qt::Key_M     : plane.yawMe(-0.01); break;

    case Qt::Key_W     : wire = !wire; break;
    }
}

int main(int argc, char **argv) {

    QApplication app(argc, argv);

    if (!QGLFormat::hasOpenGL()) {
        qWarning("This system has no OpenGL support. Exiting.");
        return 1;
    }

    CGMainWindow *w = new CGMainWindow(NULL);

    w->show();

    return app.exec();
}
