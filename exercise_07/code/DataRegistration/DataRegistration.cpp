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
#include "vecmath.h"
#include "OffReader.h"

std::vector<Vector3d> loadVertices(char* filename) {

  std::ifstream file(filename);
  int vn,fn,en;

  std::string s;
  file >> s;

  file >> vn >> fn >> en;
  std::cout << "number of vertices : " << vn << std::endl;
  std::cout << "number of faces    : " << fn << std::endl;
  std::cout << "number of edges    : " << en << std::endl;

  std::vector<Vector3d> coord;
  coord.resize(vn);
  
  for(int i=0;i<vn;i++) {
    file >> coord[i][0] >> coord[i][1] >> coord[i][2];
  }
  file.close();

  return coord;
}

int main(int argc, char** args) {
  if (argc != 3) {
    std::cout << "Need two arguments: data file and model file" << std::endl;
  } else {
    char* dataFile = args[1];
    char* modelFile = args[2];
    std::vector<Vector3d> data = loadVertices(dataFile);
    std::vector<Vector3d> model = loadVertices(modelFile);
    
    if (data.size() != model.size()) {
      std::cout << "Different number of vertices in model and data, correspondence unclear." << std::endl;
      return 1;
    }

    Vector3d modelCenter;
    Vector3d dataCenter;
    int i; 
    for (i = 0; i < data.size(); i++) {
      modelCenter += model[i];
      dataCenter += data[i];
    }
    modelCenter /= data.size();
    dataCenter /= data.size();
    Matrix4d sumUV;
          
    for (i = 0; i < data.size(); i++) {
      Vector3d du = data[i] - dataCenter;
      Vector3d dv = model[i] - modelCenter;
      Matrix4d u(
        0,       -du.v[0], -du.v[1], -du.v[2],
        du.v[0],        0,  du.v[2], -du.v[1],
        du.v[1], -du.v[2],        0,  du.v[0],
        du.v[2],  du.v[1], -du.v[0],        0
      );
      Matrix4d v(
        0,       -dv.v[0], -dv.v[1], -dv.v[2],
        dv.v[0],        0, -dv.v[2],  dv.v[1],
        dv.v[1],  dv.v[2],        0, -dv.v[0],
        dv.v[2], -dv.v[1],  dv.v[0],        0
      );
      sumUV = sumUV + (u.transpose() * v);
    }

    // Here one should somehow compute the eigenvector for
    // the largest eigenvalue... But besides being incapable
    // of adding matrices, "vecmath.h" is incapable to compute
    // eigenvectors (at least there does not seem to be anything
    // with name "*eigen*")...

  }
}
