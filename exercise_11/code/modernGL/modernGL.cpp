#include "modernGL.h"

CGMainWindow *w;

CGMainWindow::CGMainWindow (QWidget* parent) 
	: QMainWindow (parent) {
    resize (720, 720);

    // Create a nice frame to put around the OpenGL widget
    QFrame* f = new QFrame (this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(2);

    // Create our OpenGL widget
    ogl = new CGView (this,f);
    // Create a menu
    QMenu *file = new QMenu("&File",this);
    file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);

    menuBar()->addMenu(file);

    // Put the GL widget inside the frame
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(ogl);
    layout->setMargin(0);
    f->setLayout(layout);

    setCentralWidget(f);
}

CGMainWindow::~CGMainWindow () {}

void CGMainWindow::keyPressEvent(QKeyEvent* event) {

    switch(event->key()) {
    case Qt::Key_Down: break;
    case Qt::Key_Up: break;
    }

    ogl->updateGL();
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
		main = mainwindow;
        setFocusPolicy(Qt::StrongFocus);
}

void CGView::initShaders() {
    setlocale(LC_NUMERIC, "C");

    if (!program.addShaderFromSourceFile(QGLShader::Vertex, ":/vshader.glsl"))
        close();

    if (!program.addShaderFromSourceFile(QGLShader::Fragment, ":/fshader.glsl"))
        close();

    if (!program.link())
        close();

    if (!program.bind())
        close();

    setlocale(LC_ALL, "");
}


void CGView::initializeGL() {
    initializeGLFunctions();
    initShaders();
    initSolidSphereVBO();

    qglClearColor(Qt::black);
    glEnable(GL_DEPTH_TEST);
    zoom = 0.5;
    q_now = QQuaternion(1.0,0.0,0.0,0.0);


    lightPos = QVector3D(0.0,0.0,15.0);
    lightDir = QVector3D(0,0,-1.0);   
}

void CGView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 R = QMatrix4x4();
    R.rotate(q_now);

    modelView.setToIdentity();
    modelView.translate(0.0,0.5,-3.0);
    modelView *= R;
    modelView.scale(zoom,zoom,zoom);

    program.bind();
    program.setUniformValue("uPMat", projection);
    program.setUniformValue("uMVMat", modelView);
    program.setUniformValue("uNMat", modelView.normalMatrix());
    program.setUniformValue("Light.ambient", QVector3D(0.1,0.1,0.1));
    program.setUniformValue("Light.specular", QVector3D(0.5,0.5,0.5));
    program.setUniformValue("Light.diffuse", QVector3D(0.5,0.5,0.5));
    program.setUniformValue("Light.position", lightPos);
    program.setUniformValue("Material.emission",QVector3D(0.2,0.0,0.0));
    program.setUniformValue("Material.ambient",QVector3D(1.0,1.0,1.0));
    program.setUniformValue("Material.diffuse",QVector3D(1.0,1.0,1.0));
    program.setUniformValue("Material.specular",QVector3D(1.0,1.0,1.0));
    program.setUniformValue("Material.shininess", 10.0f);
    program.setUniformValue("uColor", QVector4D(0.5,0.5,0.5,1.0));

    drawSolidSphere(QVector3D(0.5,0.0,0.0),1.0);

}

void CGView::resizeGL(int w, int h) {
    width = w;
    height = h;
    glViewport(0,0,width,height);
    projection.setToIdentity();
    if (width > height) {
        qreal ratio = width/(qreal) height;
        //projection.ortho(-ratio,ratio,-1.0,1.0,-10.0,10.0);
        projection.perspective(45,ratio,1.0,1000.0);
    } else {
        qreal ratio = height/(qreal) width;
        //projection.ortho(-ratio,ratio,-1.0,1.0,-10.0,10.0);
        projection.perspective(45,ratio,1.0,1000.0);
    }
}

void CGView::mousePressEvent(QMouseEvent *event) {
    oldX = event->x();
    oldY = event->y();
    update();
}

void CGView::mouseReleaseEvent(QMouseEvent*) {
   update();
}

void CGView::wheelEvent(QWheelEvent* event) {
    if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
    update();
}

void CGView::mouseMoveEvent(QMouseEvent* event) {
    QVector3D p1, p2;

    //trackball
    mouseToTrackball(oldX,oldY,width,height,p1);
    mouseToTrackball(event->x(),event->y(),width,height,p2);
    QQuaternion q = trackball(p1,p2);
    q_now = q * q_now;
    q_now.normalize();

    oldX = event->x();
    oldY = event->y();

    updateGL();
}


void CGView::mouseToTrackball(int x, int y, int W, int H, QVector3D &v) {
    if (W > H) {
                 //v.setX((2.0*x-W)/H);
                 v.setX((2.0*x-W)/W);
                 v.setY(1.0-y*2.0/H);
             } else {
                 v.setX((2.0*x-W)/W);
                 //v.setY((H-2.0*y)/W);
                 v.setY(1.0-y*2.0/H);
             }
             double d = v.x()*v.x()+v.y()*v.y();
             if (d > 1.0) {
                 v.setZ(0.0);
                 v /= sqrt(d);
             } else v.setZ(sqrt(1.0-d));
}

QQuaternion CGView::trackball(const QVector3D& u, const QVector3D& v) {
    QVector3D uxv = QVector3D::crossProduct(u, v);
    QQuaternion ret = QQuaternion(1.0+ QVector3D::dotProduct(u,v),uxv.x(),uxv.y(),uxv.z());
    ret.normalize();
    return ret;
}

int main (int argc, char **argv) {
		QApplication app(argc, argv);

		if (!QGLFormat::hasOpenGL()) {
				qWarning ("This system has no OpenGL support. Exiting.");
				return 1;
		}

		w = new CGMainWindow(NULL);

		w->show();

		return app.exec();
}


void CGView::refineSolidSphere(const std::vector<QVector3D>& sphere,std::vector<QVector3D>& refined) {
    for(size_t i=0;i<sphere.size()/3;i++) {
        const QVector3D& a = sphere[3*i+0];
        const QVector3D& b = sphere[3*i+1];
        const QVector3D& c = sphere[3*i+2];

        QVector3D ab = a+b;
        QVector3D bc = b+c;
        QVector3D ca = c+a;

        ab.normalize();
        bc.normalize();
        ca.normalize();

        refined.push_back(a);
        refined.push_back(ab);
        refined.push_back(ca);

        refined.push_back(ab);
        refined.push_back(b);
        refined.push_back(bc);

        refined.push_back(bc);
        refined.push_back(c);
        refined.push_back(ca);

        refined.push_back(ab);
        refined.push_back(bc);
        refined.push_back(ca);
    }
}

void CGView::initSolidSphereVBO() {
    std::vector<QVector3D> ico;
    qreal gr = 0.5*(1.0+sqrt(5.0));

    ico.push_back( QVector3D(gr,1.0,0.0));
    ico.push_back( QVector3D(1.0,0.0,gr));
    ico.push_back( QVector3D(gr,-1.0,0.0));

    ico.push_back( QVector3D(gr,1.0,0.0));
    ico.push_back( QVector3D(gr,-1.0,0.0));
    ico.push_back( QVector3D(1.0,0.0,-gr));

    ico.push_back( QVector3D(gr,1.0,0.0));
    ico.push_back( QVector3D(0.0,gr,-1.0));
    ico.push_back( QVector3D(0.0,gr,1.0));

    ico.push_back( QVector3D(gr,1.0,0.0));
    ico.push_back( QVector3D(0.0,gr,1.0));
    ico.push_back( QVector3D(1.0,0.0,gr));

    ico.push_back( QVector3D(gr,1.0,0.0));
    ico.push_back( QVector3D(1.0,0.0,-gr));
    ico.push_back( QVector3D(0.0,gr,-1.0));

    ico.push_back( QVector3D(-gr,-1.0,0.0));
    ico.push_back( QVector3D(-1.0,0.0,gr));
    ico.push_back( QVector3D(-gr,1.0,0.0));

    ico.push_back( QVector3D(-gr,-1.0,0.0));
    ico.push_back( QVector3D(-gr,1.0,0.0));
    ico.push_back( QVector3D(-1.0,0.0,-gr));

    ico.push_back( QVector3D(-gr,-1.0,0.0));
    ico.push_back( QVector3D(0.0,-gr,-1.0));
    ico.push_back( QVector3D(0.0,-gr,1.0));

    ico.push_back( QVector3D(-gr,-1.0,0.0));
    ico.push_back( QVector3D(0.0,-gr,1.0));
    ico.push_back( QVector3D(-1.0,0.0,gr));

    ico.push_back( QVector3D(-gr,-1.0,0.0));
    ico.push_back( QVector3D(-1.0,0.0,-gr));
    ico.push_back( QVector3D(0.0,-gr,-1.0));

    ico.push_back( QVector3D(1.0,0.0,gr));
    ico.push_back( QVector3D(-1.0,0.0,gr));
    ico.push_back( QVector3D(0.0,-gr,1.0));

    ico.push_back( QVector3D(1.0,0.0,gr));
    ico.push_back( QVector3D(0.0,gr,1.0));
    ico.push_back( QVector3D(-1.0,0.0,gr));

    ico.push_back( QVector3D(0.0,gr,1.0));
    ico.push_back( QVector3D(-gr,1.0,0.0));
    ico.push_back( QVector3D(-1.0,0.0,gr));

    ico.push_back( QVector3D(0.0,gr,1.0));
    ico.push_back( QVector3D(0.0,gr,-1.0));
    ico.push_back( QVector3D(-gr,1.0,0.0));

    ico.push_back( QVector3D(0.0,gr,-1.0));
    ico.push_back( QVector3D(-1.0,0.0,-gr));
    ico.push_back( QVector3D(-gr,1.0,0.0));

    ico.push_back( QVector3D(-1.0,0.0,-gr));
    ico.push_back( QVector3D(0.0,gr,-1.0));
    ico.push_back( QVector3D(1.0,0.0,-gr));

    ico.push_back( QVector3D(-1.0,0.0,-gr));
    ico.push_back( QVector3D(1.0,0.0,-gr));
    ico.push_back( QVector3D(0.0,-gr,-1.0));

    ico.push_back( QVector3D(0.0,-gr,-1.0));
    ico.push_back( QVector3D(1.0,0.0,-gr));
    ico.push_back( QVector3D(gr,-1.0,0.0));

    ico.push_back( QVector3D(0.0,-gr,-1.0));
    ico.push_back( QVector3D(gr,-1.0,0.0));
    ico.push_back( QVector3D(0.0,-gr,1.0));

    ico.push_back( QVector3D(0.0,-gr,1.0));
    ico.push_back( QVector3D(gr,-1.0,0.0));
    ico.push_back( QVector3D(1.0,0.0,gr));

    for(size_t i=0;i<ico.size();i++) ico[i].normalize();

    for(int i=0;i<3;i++) {
        std::vector<QVector3D> ico_refined;
        refineSolidSphere(ico,ico_refined);
        ico = ico_refined;
    }

    std::vector<QVector3D> vertexWithNormal;
    GLuint id;
    glGenBuffers(1,&id);

    for(size_t i=0;i<ico.size();i++) {
    vertexWithNormal.push_back(ico[i]);
    vertexWithNormal.push_back(ico[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER,id);
    glBufferData(GL_ARRAY_BUFFER,vertexWithNormal.size()*sizeof(QVector3D),
                 vertexWithNormal.data(),GL_STATIC_DRAW);

    vboSolidSphereId = id;
    vboSolidSphereSize = static_cast<int>(ico.size());
}


void CGView::drawSolidSphere(const QVector3D& c, qreal r) {
    QMatrix4x4 M(modelView);
    M.translate(c);
    M.scale(r);
    program.setUniformValue("uMVMat", M);
    program.setUniformValue("uNMat", M.normalMatrix());
    glBindBuffer(GL_ARRAY_BUFFER, vboSolidSphereId);

    int vertexLocation = program.attributeLocation("a_position");
    program.enableAttributeArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), 0);

    int normalLocation = program.attributeLocation("a_normal");
    program.enableAttributeArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), (const void*) sizeof(QVector3D));

    glDrawArrays(GL_TRIANGLES,0,vboSolidSphereSize);
}

