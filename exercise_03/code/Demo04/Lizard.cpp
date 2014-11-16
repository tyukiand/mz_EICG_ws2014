#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "Lizard.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

#ifdef max
#undef max
#endif

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);

	// Create a menu
	QMenu *file = new QMenu("&File",this);
	file->addAction ("Load polygon", this, SLOT(loadPolygon()), Qt::CTRL+Qt::Key_L);
	file->addAction ("Duplicate active polygon", this, SLOT(duplicatePolygon()), Qt::CTRL+Qt::Key_D);
	file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);
	menuBar()->addMenu(file);

	// Create a nice frame to put around the OpenGL widget
	QFrame* f = new QFrame (this);
	f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	f->setLineWidth(2);

	// Create our OpenGL widget
	ogl = new CGView (this,f);

	// Put the GL widget inside the frame
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(ogl);
	layout->setMargin(0);
	f->setLayout(layout);

	setCentralWidget(f);

	statusBar()->showMessage("Ready",1000);
}

CGMainWindow::~CGMainWindow () { }


void CGMainWindow::loadPolygon() {
	QString fn = QFileDialog::getOpenFileName(this, "Load polygon ...", QString(), "POL files (*.pol)" );

	if (fn.isEmpty()) return;
	statusBar()->showMessage ("Loading polygon ...");
	std::ifstream file(fn.toLatin1());
	double x,y;
	int m,n;

	ogl->minX = ogl->minY = std::numeric_limits<double>::max();
	ogl->maxX = ogl->maxY = -ogl->minX;

	file >> n;
	ogl->poly.resize(n);
	for(int i=0;i<n;i++) {
		file >> m;
		ogl->poly[i].resize(3*m);
		for(int j=0;j<m;j++) {
			file >> x >> y;
			if (x < ogl->minX) ogl->minX = x;
			else if (x > ogl->maxX) ogl->maxX = x;
			if (y < ogl->minY) ogl->minY = y;
			else if (y > ogl->maxY) ogl->maxY = y;
			ogl->poly[i][3*j+0] = x;
			ogl->poly[i][3*j+1] = y;
			ogl->poly[i][3*j+2] = 0.0;
		}
        //int transSize=(int)ogl->trans.size();
        ogl->trans.resize(n);
        //int centerSize=(int)ogl->center.size();
        ogl->center.resize(n);
	}
	file.close();

	std::cout << "minX = " << ogl->minX << std::endl;
	std::cout << "maxX = " << ogl->maxX << std::endl;
	std::cout << "minY = " << ogl->minY << std::endl;
	std::cout << "maxY = " << ogl->maxY << std::endl;

	ogl->zoom = 2.0/std::max(ogl->maxX-ogl->minX,ogl->maxY-ogl->minY);

	// this is center of the scene
	ogl->centerX = (ogl->minX+ogl->maxX)/2;
	ogl->centerY = (ogl->minY+ogl->maxY)/2;
	
	ogl->i_picked = 0;


    for(int i=0;i<(int) ogl->center.size();i++){
    // this is center of ith polygon
        ogl->center[i].resize(2);
        ogl->center[i][0]=ogl->centerX;
        ogl->center[i][0]=ogl->centerY;
    }
    for(int i=0;i<(int) ogl->trans.size();i++){
        ogl->trans[i].resize(3);
        ogl->trans[i][0] = 0; // x
        ogl->trans[i][1] = 0; // y
        ogl->trans[i][2] = 0; // phi
     }

	ogl->updateGL();
	statusBar()->showMessage ("Loading polygon done.",3000);
}



void CGMainWindow::duplicatePolygon() {
        //duplicates the picked polygon and rotates the copy by
        // 30 degree and translates it by 1 in x
        for(int i=0;i<ogl->poly.size();i++){
            if(i==ogl->i_picked){
                int N = ogl->poly.size();
                ogl->poly.resize(N+1);
                ogl->poly[N].resize(ogl->poly[i].size());
                for (int j=0;j<ogl->poly[i].size();j++)
                                ogl->poly[N][j] = ogl->poly[i][j];

                N = ogl->center.size();
                ogl->center.resize(N+1);
                ogl->center[N].resize(2);
                ogl->center[N][0] = ogl->center[i][0];
                ogl->center[N][1] = ogl->center[i][1];

                N = ogl->trans.size();
                ogl->trans.resize(N+1);
                ogl->trans[N].resize(3);
                ogl->trans[N][0] = ogl->trans[i][0]+1.0;
                ogl->trans[N][1] = ogl->trans[i][1]+1.0;
                ogl->trans[N][2] = ogl->trans[i][2]+30;
                ogl->i_picked = N;
                break;
            }
        }
ogl->updateGL();
}


void CGMainWindow::keyPressEvent(QKeyEvent* event) {


    // add your code to rotate the picked lizard
    double phi;
    phi=30;
    switch(event->key()) {
            case Qt::Key_I: std::cout << "I" << std::flush; break;
            case Qt::Key_M: std::cout << "M" << std::flush; break;
            case Qt::Key_Q: ogl->trans[ogl->i_picked][2]=ogl->trans[ogl->i_picked][2]+phi; break;
            case Qt::Key_W: ogl->trans[ogl->i_picked][2]=ogl->trans[ogl->i_picked][2]-phi; break;

    }

    ogl->updateGL();
}







CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
				main = mainwindow;
}

void CGView::initializeGL() {
				qglClearColor(Qt::white);
				zoom = 1.0;
				fillMode = 0;

				tobj = gluNewTess();
#ifdef _MSC_VER
				gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(APIENTRY*)()) glBegin);
				gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(APIENTRY*)()) glVertex3dv);
				gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(APIENTRY*)()) glEnd);
#else
				gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(*)()) glBegin);
				gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(*)()) glVertex3dv);
				gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(*)()) glEnd);
#endif
				gluTessProperty(tobj,GLU_TESS_BOUNDARY_ONLY,GL_FALSE);
}

void CGView::paintGL() {
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT);

    for(int i=0;i<(int) poly.size();i++) {

            // View
            glLoadIdentity();
            glScaled(zoom,zoom,1.0);
            glTranslated(-centerX,-centerY,0.0);

            // Model
            glTranslated(trans[i][0],trans[i][1],0);
            glTranslated(center[i][0],center[i][1],0);
            glRotated(trans[i][2],0,0,1);
            glTranslated(-center[i][0],-center[i][1],0);
            glColor3f(0.0f,1.0f,0.0f);
            if (i == i_picked)
                glColor3f(1.0f,0.0f,0.0f);

            // draw ith polygons interior
            gluTessBeginPolygon(tobj,NULL);
            gluTessBeginContour(tobj);
            for(int j=0;j<(int) poly[i].size()/3;j++)
                            gluTessVertex(tobj,&poly[i][3*j],&poly[i][3*j]);
            gluTessEndContour(tobj);
            gluTessEndPolygon(tobj);

            // draw ith polygons polyline
            glColor3f(0.0f,0.0f,0.0f);
            glBegin(GL_LINE_STRIP);
            for(int j=0;j<(int) poly[i].size()/3;j++){

                double polyX=poly[i][3*j+0];
                double polyy=poly[i][3*j+1];
                glVertex2d(poly[i][3*j+0],poly[i][3*j+1]);

            }
            if ((int) poly[i].size()/3>0)
                glVertex2d(poly[i][0],poly[i][1]);
            glEnd();

    }
}

void CGView::resizeGL(int width, int height) {
				glViewport(0,0,width,height);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				if (width > height) {
								double ratio = width/(double) height;
								gluOrtho2D(-ratio,ratio,-1.0,1.0);
				}
				else {
								double ratio = height/(double) width;
								gluOrtho2D(-1.0,1.0,-ratio,ratio);
				}
				glMatrixMode (GL_MODELVIEW);
}

void CGView::worldCoord(int x, int y, double &dx, double &dy) {
				if (width() > height()) {
								dx = (2.0*x-width())/height();
								dy = 1.0-2.0*y/(double) height();
				} else {
								dx = 2.0*x/(double) width()-1.0;
								dy = (height()-2.0*y)/width();
				}
				dx /= zoom;
				dy /= zoom;
				dx += centerX;
				dy += centerY;
}

int CGView::intersect(
  double px, double py,
  double ax, double ay,
  double bx, double by
) {

  // we use a macro, becase we do not want to evaluate this expression
  // if it's not necessary, but we also do not want to write it out twice
  // or introduce additional helper function.
  #define DET bx * py - by * px - ax * py + ay * px - bx * ay + by * ax
  return
    (py <= ay && py > by && DET < 0) ||
    (py > ay && py <= by && DET > 0);
  #undef DET
}

bool CGView::insidePolygonI(int i,double px, double py) {
  int numIntersections = 0; // collides with the required `intersect` method, renamed
  for(int j=0;j<(int) poly[i].size()/3-1;j++){
    int j3=3*j;
    double polyX1,polyY1,polyX2,polyY2;
    double phi=M_PI*trans[i][3]/180.0;
    double cPhi=cos(phi);
    double sPhi=sin(phi);
    polyX1=cPhi*poly[i][j3]-sPhi*poly[i][j3+1]+cPhi*trans[i][0]-sPhi*trans[i][1];
    polyY1=sPhi*poly[i][j3]+cPhi*poly[i][j3+1]+sPhi*trans[i][0]+cPhi*trans[i][1];
    polyX2=cPhi*poly[i][j3+3]-sPhi*poly[i][j3+4]+cPhi*trans[i][0]-sPhi*trans[i][1];
    polyY2=sPhi*poly[i][j3+3]+cPhi*poly[i][j3+4]+sPhi*trans[i][0]+cPhi*trans[i][1];
    numIntersections += intersect(
      px, py,
      polyX1, polyY1,
      polyX2, polyY2
    );
  }
  return numIntersections % 2 == 1;
}

void CGView::mousePressEvent(QMouseEvent *event) {
        double dx, dy,px,py;
        worldCoord(event->x(),event->y(),dx,dy);
        std::cout << "Mouse pressed at (" << dx << "," << dy <<")" << std::endl;
        px=dx;
        py=dy;
        bool inI;
        if(event->button() == Qt::LeftButton)
            clicked=true;
        clickedPoint = event->pos();
    // add your intersect code here
    // also set the new i_picked
        for(int i=0;i<poly.size();i++){
            inI=insidePolygonI(i,px,py);
            if(inI){
                i_picked=i;
                std::cout << "Polygon " << i << " selected" << std::endl;
                clickedPolyTrans.clear();
                clickedPolyTrans.push_back(trans[i_picked][0]);
                clickedPolyTrans.push_back(trans[i_picked][1]);
            }
        }
        update();

}

void CGView::mouseReleaseEvent (QMouseEvent* event) {
				std::cout << "Mouse released" << std::endl;
                if(event->button() == Qt::LeftButton)
                    clicked=false;
}

void CGView::wheelEvent(QWheelEvent* event) {
                if (event->delta() < 0) zoom *= 1.1; else zoom *= 1/1.1;
				update();
}

void CGView::mouseMoveEvent (QMouseEvent* event) {
				QPoint current = event->pos();


				int x = current.x();
                int y = current.y();
                int xClick=clickedPoint.x();
                int yClick=clickedPoint.y();

                double diffX,diffY;
                double dx,dy,dxClick,dyClick;
                worldCoord(x,y,dx,dy);
                worldCoord(xClick,yClick,dxClick,dyClick);
                diffX=dx-dxClick;
                diffY=dy-dyClick;

				// add code here to move the picked lizard

                trans[i_picked][0]=clickedPolyTrans[0]+diffX;
                trans[i_picked][1]=clickedPolyTrans[1]+diffY;

                //if (event->button() == Qt::LeftButton)

                std::cout << "clickedPoint (" << diffX << "," << diffY << "," << dxClick << "," << dyClick << ")" << std::endl;

                if(clicked)
                    updateGL();
}



int main (int argc, char **argv) {
				QApplication app(argc, argv);

				if (!QGLFormat::hasOpenGL()) {
								qWarning ("This system has no OpenGL support. Exiting.");
								return 1;
				}

				CGMainWindow *w = new CGMainWindow(NULL);

				w->show();

				return app.exec();
}

