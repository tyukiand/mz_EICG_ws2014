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

#include "Perspective.h"

#include "MyPlane.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CGMainWindow::CGMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {
    resize(604, 614);

    // Create a nice frame to put around the OpenGL widget
    QFrame *f = new QFrame(this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(2);

    // Create our OpenGL widget
    ogl = new CGView(this, f);

    QMenu *file = new QMenu("&Plane", this);
    file->addAction("&Reset plane", this->ogl, SLOT(reset_plane()), Qt::CTRL + Qt::Key_R);
    file->addAction("&Toggle follow", this->ogl, SLOT(toggle_follow()), Qt::CTRL + Qt::Key_T);
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

    plane_fly = false;
    follow  = false;

    /// Hier startet der Flieger!
    place = Vector3d(0, 0.6, 0);

    /// Diskreter Schritt bei der Bewegung!
    step = 0.01;

    /// Wireframe oder Filled?
    wire = true;

    /// Um das H"ohenfeld zu erzeugen
    init_height();

    /// Hier kommt der Flieger!
    Vector3d p1(0, .1, 0);
    Vector3d p2(-0.05, 0, 0);
    Vector3d p3(0, -.1, 0);
    Vector3d p4(.2, 0, 0);

    std::vector<Vector3d> p;
    p.push_back(p1);
    p.push_back(p2);
    p.push_back(p3);
    p.push_back(p4);

    plane.setPoints(p);
    plane.placeMe(place);

    /// Um Keyboard-Events durchzulassen
    setFocusPolicy(Qt::StrongFocus);
}


/// Initialisiert das H"ohenfeld
void CGView::init_height() {
    double s = .25;
    double t = .15;
    double a = sizeX * sizeY;

    for (int i = 0; i < sizeX; i++)
        for (int j = 0; j < sizeY; j++) {
            height_vector[i][j] = (1 / a) * (j * i) + 0.05 * (sin(s * i) + cos(t * j));
            height_vector[i][j] *= height_vector[i][j];

            ///Hier die normalen!
            Vector3d dx(1, j / a + s * 0.05 * cos(s * i), 0);
            Vector3d dy(0, i / a - t * 0.05 * sin(t * j), 1);
            normal_vector[i][j] = (dy % dx);
            normal_vector[i][j].normalize();

        }

    return;
}

/// Malt das H"ohenfeld
void CGView::drawHeight() {

    glPushMatrix();
    glTranslated(-5.0, 0.0, -5.0);
    glScaled(10.0 / (sizeX - 1), 3.0, 10.0 / (sizeY - 1));
    glLineWidth(1);

    Vector3d c(1, 1, 1);

    if (!wire) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (int i = 0; i < (sizeX - 1); i++) {
        glBegin(GL_TRIANGLE_STRIP);

        for (int j = 0; j < (sizeY); j++) {
            glColor3dv(c.ptr());
            glVertex3d(i, height_vector[i][j], j);
            glColor3dv(c.ptr());
            glVertex3d(i + 1, height_vector[i + 1][j], j);
        }

        glEnd();
    }

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
    zoom = 1.0;
    phi = 0;
    theta = 0;

    center = Vector3d(0.0, 0.0, 0.0);

    ///Make timer
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(100);
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
    glRotated(theta, 1.0, 0.0, 0.0);
    glRotated(phi, 0.0, 1.0, 0.0);

    /*  Falls perspektivische Projektion verwendet wird, Szene etwas nach hinten schieben,
        damit sie in der Sichtpyramide liegt. */
    glTranslated(0.0, 0.0, -3.0);



    /// die nächste Zeile bitte wieder entfernen
    /// dient nur dazu, dass Sie ds Flugzeug sehen können
    /// bevor Sie Ihre Kamerasteuerung implementiert haben ...
    glTranslated(0.0, -1.0, 0.0);




    glScaled(zoom, zoom, zoom);

    drawHeight();

    plane.draw(Vector3d(1, 0, 0), true);
}

void CGView::mousePressEvent(QMouseEvent *event) {
    oldX = event->x();
    oldY = event->y();
    updateGL();
}

void CGView::mouseReleaseEvent(QMouseEvent *) {}

void CGView::wheelEvent(QWheelEvent *event) {
    if (event->delta() < 0) zoom *= 1.2; else zoom *= 1 / 1.2;

    update();
}

void CGView::mouseMoveEvent(QMouseEvent *event) {
    phi += 0.2 * (event->x() - oldX);
    theta += 0.2 * (event->y() - oldY);
    oldX = event->x();
    oldY = event->y();

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
    case Qt::Key_Up    : plane.pitchMe(0.1); break;

    case Qt::Key_Down  : plane.pitchMe(-0.1); break;

    case Qt::Key_Left  : plane.rollMe(-0.1); break;

    case Qt::Key_Right : plane.rollMe(0.1); break;

    case Qt::Key_Space : plane_fly = !plane_fly; break;

    case Qt::Key_N     : plane.yawMe(0.1); break;

    case Qt::Key_M     : plane.yawMe(-0.1); break;

    case Qt::Key_W     : wire = !wire; break;
    }

    updateGL();
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
