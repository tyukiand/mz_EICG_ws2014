#ifndef MODERNGL_H
#define MODERNGL_H

#include <QActionGroup>
#include <QApplication>
#include <QFrame>
#include <QKeyEvent>
#include <QLocale>
#include <QMainWindow>
#include <QStatusBar>
#include <QVector3D>
#include <QMenu>
#include <QMenuBar>
#include <QBoxLayout>

#include <QtCore>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <QOpenGLTexture>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include "vecmath.h"
class CGView;

class CGMainWindow : public QMainWindow {
    Q_OBJECT

public:

    CGMainWindow (QWidget* parent = 0); 
    ~CGMainWindow ();
    CGView *ogl;

public slots:

protected:
   void keyPressEvent(QKeyEvent*);
};

class CGView : public QGLWidget, public QGLFunctions {
    Q_OBJECT

public:

    CGView(CGMainWindow*,QWidget*);
    void initShaders();
    void initializeGL();

    void initSolidSphereVBO();
    void refineSolidSphere(const std::vector<QVector3D>&, std::vector<QVector3D>&);
    GLuint vboSolidSphereId;
    int vboSolidSphereSize;
    
    qreal zoom;

    QVector3D lightPos;
    QVector3D lightDir;   

protected:

    void paintGL();
    void resizeGL(int,int);

    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    void drawSolidSphere(const QVector3D&,qreal);
    void mouseToTrackball(int x, int y, int W, int H, QVector3D &v);
    QQuaternion trackball(const QVector3D& u, const QVector3D& v);

    CGMainWindow *main;
    int oldX,oldY;
    QQuaternion q_now;

private:

    QGLShaderProgram program;
    QMatrix4x4 projection,modelView;
    int width, height;

};

#endif
