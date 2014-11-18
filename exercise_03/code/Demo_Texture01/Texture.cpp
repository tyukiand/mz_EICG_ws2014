#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "Texture.h"
#include "glext.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

const int offset = 5;
double dx_old, dy_old;

int pickX,pickY;

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);

	// Create a menu
	QMenu *file = new QMenu("&File",this);
	file->addAction ("Load picture", this, SLOT(loadPicture()), Qt::CTRL+Qt::Key_L);
	file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);

    // Notice that we have added a bunch of additional commands here that
    // were not explicitly required. Ctrl+R is such an example.
    file->addAction ("Show grid", this, SLOT(showGrid()), Qt::CTRL+Qt::Key_G);
    file->addAction ("Larger brush", this, SLOT(largerBrush()), Qt::Key_Plus);
    file->addAction ("Smaller brush", this, SLOT(smallerBrush()), Qt::Key_Minus);
    file->addAction ("Reset grid", this, SLOT(resetGrid()), Qt::CTRL+Qt::Key_R);

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

    // mouseMoveEvents are only registered when `mouseTracking` is enabled.
    // otherwise mouseMoveEvents behave like mouseDragEvents.
    // Yeah, this makes tons of sense...
    ogl->setMouseTracking(true);
	setCentralWidget(f);

	statusBar()->showMessage("Ready",1000);
}

CGMainWindow::~CGMainWindow () { }

void CGMainWindow::loadPicture() {
	QString fn = QFileDialog::getOpenFileName(this, "Load picture ...", QString(), "*.jpg *.png" );

	if (fn.isEmpty()) return;
	statusBar()->showMessage ("Loading picture ...");
	QImage img(fn);

//	if (img.depth() == 8)
//		gluBuild2DMipmaps(GL_TEXTURE_2D,3,img.width(),img.height(),GL_BGRA_EXT,GL_UNSIGNED_BYTE,img.mirrored(false,true).convertToFormat(QImage::Format_RGB32).bits());

	if (img.depth() == 32)
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,img.width(),img.height(),0,GL_BGRA_EXT,GL_UNSIGNED_BYTE,img.mirrored(false,true).bits());
//		gluBuild2DMipmaps(GL_TEXTURE_2D,3,img.width(),img.height(),GL_BGRA_EXT,GL_UNSIGNED_BYTE,img.mirrored(false,true).bits());

	ogl->updateGL();
	statusBar()->showMessage ("Loading picture done.",3000);
}

/**
 * SLOT-method that simply redirects orders to CGView
 */
void CGMainWindow::showGrid() {
  ogl->toggleGrid();
}

void CGMainWindow::resetGrid() {
  ogl->resetGrid();
}

void CGMainWindow::largerBrush() {
  ogl->scaleBrush(1.125);
}

void CGMainWindow::smallerBrush() {
  ogl->scaleBrush(1/1.125);
}

void CGMainWindow::keyPressEvent(QKeyEvent* event) {
	switch(event->key()) {
	case Qt::Key_I: std::cout << "I" << std::flush; break;
	case Qt::Key_M: std::cout << "M" << std::flush; break;
	}
	ogl->updateGL();
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;
	gridOn = false;

	// generating grids
	int row, col;
	float dg = 2.0 / (N - 1); // step width for grid coordinates
	float dt = 1.0 / (N - 1); // step width for texture coordinates
	float gridX = -1;
	float gridY = -1;
	float textureX = 0;
	float textureY = 0;
	// fill `grid` and `T` by uniform grid coordinates
	for (row = 0; row < N; row++) {
		textureX = 0;
		gridX = -1;
	    for (col = 0; col < N; col++) {
            grid[row][col][0] = gridX;
            grid[row][col][1] = gridY;
            T[row][col][0] = textureX;
            T[row][col][1] = textureY;
            textureX += dt;
            gridX += dg;
	    }
	    textureY += dt;
	    gridY += dg;
	}

	// setting initial properties of the "brush"
	brushRadius = 0.1;
	brushX = 0.0;
	brushY = 0.0;

	// setting initial properties of the "dragging"-tool
	draggingOn = false;
	draggedRow = 0;
	draggedCol = 0;
	draggedX = 0.0;
	draggedY = 0.0;
}

void CGView::resetGrid() {
	int row, col;
	float dg = 2.0 / (N - 1); // step width for grid coordinates
	float gridX = -1;
	float gridY = -1;
	for (row = 0; row < N; row++) {
		gridX = -1;
	    for (col = 0; col < N; col++) {
            grid[row][col][0] = gridX;
            grid[row][col][1] = gridY;
            gridX += dg;
	    }
	    gridY += dg;
	}
	update();
}

void CGView::initializeGL() {
	glLoadExtensions();

	qglClearColor(Qt::white);

	zoom = 1.0;
	centerX = 0.0;
	centerY = 0.0;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glGenTextures(1,&texname);
	glBindTexture(GL_TEXTURE_2D,texname);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

 	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
}

void CGView::paintGL() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScaled(zoom,zoom,1.0);
	glTranslated(-centerX,-centerY,0.0);

	glClear(GL_COLOR_BUFFER_BIT);

    // The grid by itself looks somewhat empty and boring. 
    // So we decided that we render the image in both cases.
    // However, the image is rendered as almost transparent when we
    // go into the grid-modus.
	if (gridOn) {  
	  glEnable(GL_BLEND);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glColor4f(1,1,1,0.35);
	}

	// Rendering the image (in both modes: the settings are changed for
	// the grid-mode slightly)
    glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glBindTexture(GL_TEXTURE_2D,texname);
    // rendering the textured image
    // GL_QUADS are deprecated, use GL_TRIANGLES instead
	glBegin(GL_TRIANGLES);
	int row, col;
	float* t;
	float g[2] = {0, 0};
	#define ADD_POINT(i, j) \
	  t = T[i][j]; \
	  dragPoint(i, j, g); \
	  glTexCoord2d(t[0], t[1]); \
	  glVertex2d(g[0], g[1]);
	for (row = 0; row < (N - 1); row ++) {
    	for (col = 0; col < (N - 1); col ++) {
            ADD_POINT(row  , col  );
            ADD_POINT(row+1, col  );
            ADD_POINT(row+1, col+1);
            ADD_POINT(row  , col+1);
            ADD_POINT(row  , col  );
            ADD_POINT(row+1, col+1);
    	}
	}
	#undef ADD_POINT
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Now render the grid if necessary
    if (gridOn) {
      glColor3f(0.0, 0.6, 0.5);
      // rendering only the horizontal and vertical lines of the
      // grid
      // GL_QUADS are deprecaded, use GL_LINES instead
      glBegin(GL_LINES);
      int row, col;
	  float g[2] = {0, 0};
	  #define ADD_POINT(i, j) dragPoint(i, j, g); glVertex2d(g[0], g[1]);
	  for (row = 0; row < (N - 1); row ++) {
      	for (col = 0; col < (N - 1); col ++) {
              ADD_POINT(row  , col  );
              ADD_POINT(row+1, col  );
              ADD_POINT(row  , col  );
              ADD_POINT(row  , col+1);
      	}
	  }
	  // render the bottom and the right boundary separately
	  col = (N - 1);
	  for (row = 0; row < (N - 1); row ++) {
	  	ADD_POINT(row, col);
	  	ADD_POINT(row +1, col);
	  	ADD_POINT(col, row);
	  	ADD_POINT(col, row+1);
	  }
	  #undef ADD_POINT
      glEnd();
    }

    // draw the brush
    glColor3f(0.2, 0.1, 0.3);
    glBegin(GL_LINE_LOOP);
    int i;
    int numLoopVertices = 100;
    double factor = 2 * M_PI / numLoopVertices;
    for (i = 0; i < numLoopVertices; i++) {
      glVertex2d(
      	brushX + brushRadius * cos(i * factor), 
      	brushY + brushRadius * sin(i * factor)
      );
    }
    glEnd();
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

void CGView::toggleGrid() {
  gridOn = !gridOn;
  update();
}

void CGView::scaleLocally(float factor) {
  int row,col;
  float dx, dy, r;
  float transformationScaling;
  for (row = 0; row < N; row++) {
  	for (col = 0; col < N; col++) {
  	 
  	  // subtract the brushX/Y offset to obtain relative coords
  	  dx = grid[row][col][0] - brushX;
  	  dy = grid[row][col][1] - brushY;
      r = hypot(dx, dy);

      if (r < brushRadius) {
      	transformationScaling = 1;

      	// transform from disk to plane
      	transformationScaling /= (brushRadius - r);
        r /=  (brushRadius - r);

        // scale
        r *= factor;
        transformationScaling *= factor;

        // transform back into disk
        transformationScaling *= brushRadius / (1 + r);

        // scale the relative coords and add the original offset
        grid[row][col][0] = brushX + dx * transformationScaling;
        grid[row][col][1] = brushY + dy * transformationScaling;
      }
  	}
  }
}

void CGView::scaleBrush(float factor) {
  brushRadius *= factor;
  update();
}

void CGView::pick(int px, int py, int &pickedRow, int &pickedCol) {
  double x, y;
  worldCoord(px, py, x, y);
  std::cout << "picked at " << x << ", " << y << std::endl;
  int bestRow = 0;
  int bestCol = 0;
  float minDist = 10E10; // Where is FLT_MAX? hmm...
  int row;
  int col;
  float dx, dy, r;
  for (row = 0; row < N; row++) {
  	for (col = 0; col < N; col++) {
  	  dx = grid[row][col][0] - x;
  	  dy = grid[row][col][1] - y;
          r = hypot(dx, dy);
          if (r < minDist) {
          	minDist = r;
          	bestRow = row;
          	bestCol = col;
          }
  	}
  }
  std::cout << "Picked: row = " << bestRow << " col = " << bestCol << std::endl;
  pickedRow = bestRow;
  pickedCol = bestCol;
}

void CGView::dragPoint(int row, int col, float* arr2f) {
	if (draggingOn) {
        int dr = row - draggedRow;
        int dc = col - draggedCol;
        float gridRadiusSq = (dr * dr + dc * dc) * 4.0 / (N * N);
        float weight = exp(-gridRadiusSq / (brushRadius * brushRadius));
        float offsetX = (brushX - draggedX) * weight;
        float offsetY = (brushY - draggedY) * weight;
        arr2f[0] = grid[row][col][0] + offsetX;
        arr2f[1] = grid[row][col][1] + offsetY;
	} else {
		arr2f[0] = grid[row][col][0];
		arr2f[1] = grid[row][col][1];
	}
}

void CGView::mousePressEvent(QMouseEvent *event) {
	if (event->buttons() == Qt::RightButton) {
		if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
		  scaleLocally(1/1.1);
		} else {
          scaleLocally(1.1);
		}
		update();
	} else if (event->buttons() == Qt::LeftButton) {
		// pick a point of the grid and start dragging it around
		pick(event->x(), event->y(), draggedRow, draggedCol);
		draggedX = grid[draggedRow][draggedCol][0];
		draggedY = grid[draggedRow][draggedCol][1];
		draggingOn = true;
	}

}

void CGView::mouseReleaseEvent(QMouseEvent*) {
	if (draggingOn) {
	  // save the changes after dragging
  	  int row, col;
  	  for (row = 0; row < N; row++) {
  	    for (col = 0; col < N; col++) {
          dragPoint(row, col, grid[row][col]);
	    }
	  }
    }
	// stop dragging
	draggingOn = false;
	update();
}

void CGView::wheelEvent(QWheelEvent* event) {
  double px,py,dz;
  worldCoord(event->x(),event->y(),px,py);
  if (event->delta() < 0) dz = 1.1; else dz = 1/1.1;
  zoom *= dz;
  //	centerX = px + (centerX-px)/dz;
  //	centerY = py + (centerY-py)/dz;

  update();
}

void CGView::mouseMoveEvent(QMouseEvent* event) {
  double x, y;
  worldCoord(event->x(),event->y(), x, y);
  brushX = (float)x;
  brushY = (float)y;
  update();
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

