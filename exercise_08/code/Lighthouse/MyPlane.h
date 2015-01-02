#ifndef MYPOLYGON_H
#define MYPOLYGON_H

#include "vecmath.h"
#include "Perspective.h"
#include <vector>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>

#include <QtOpenGL>

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
  #define RETINA_DISPLAY 0
#else
	#include <GL/glu.h>
#endif

class MyPlane
{

  public:

    MyPlane (char* as_FileName): origin_(0,0,0), nose_(Vector3d(1,0,0)), up_(Vector3d(0,1,0))
  {
    std::cout << "Lade Off File(\"" << as_FileName << "\");" << std::endl;

    int v_num = 0;
    int f_num = 0;

    std::ifstream is(as_FileName);
    if(!is)
    {
      std::cout << "Off-Datei nicht gefunden!" << std::endl;
      return;
    }
    std::string is_header;
    std::getline(is, is_header);
    if (is_header.length() > 0 && is_header[is_header.length()-1] == 0xd) 
      is_header.erase (is_header.length()-1,1);

    if(is_header.compare ("OFF") == 0)
    {
      std::cout << "Lese OFF-Datei..." << std::endl;
    }
    else 
    {
      std::cout << "HEADER: " << is_header << std::endl;
      std::cout << "Keine gueltige OFF-Datei!" << std::endl;
      return;
    }

    is >> v_num;
    std::cout << "Knoten: " << v_num << std::endl;
    is >> f_num;
    std::cout << "Flaechen: " << f_num << std::endl;
    int e_num;;
    is >> e_num;
    std::cout << "Kanten: " << e_num << std::endl;

    for(int i = 0; i < v_num; i++) 
    {
      double x, y, z;
      is >> x;
      is >> y;
      is >> z;
      vertices_.push_back(Vector3d(x, y, z));
    }

    faces_.resize (f_num);

    for (int i = 0; i < f_num; i++) 
    {
      int knoten;
      is >> knoten;
      for(int j = 0; j < knoten; j++)
      {
        int index;
        is >> index;
        faces_[i].push_back (index);
      }
    }

    set_into_mass_center ();
    scale_into_sphere (0.1); 
    map_axes_to (Vector3d(0,0,1), Vector3d(0,1,0));
  }

    MyPlane () : origin_(0,0,0), nose_(Vector3d(1,0,0)), up_(Vector3d(0,1,0))
  {
  };

    ///Malt das Plane
    void draw(Vector3d color = Vector3d(1,1,1),  bool fill=false)
    {
      glPushMatrix();

      Vector3d noxup = nose_%up_;
      Matrix4d R(
          nose_.x(),   nose_.y(),   nose_.z(),   0,
          noxup.x(),   noxup.y(),   noxup.z(),   0,
          up_.x(),     up_.y(),     up_.z(),     0,
          origin_.x(), origin_.y(), origin_.z(), 1);

      glMultMatrixd (R.ptr());

      glColor3dv (color.ptr());
      if (fill) glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
      else glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
      for (unsigned int i=0;i<faces_.size();i++)
      {
        glBegin(GL_POLYGON);

        ///ADD YOUR CODE HERE!
        Vector3d centerTriangle=(vertices_[faces_[i][0]]+vertices_[faces_[i][1]]+vertices_[faces_[i][2]])/3;
        Vector3d normal;
        normal.cross(vertices_[faces_[i][0]],vertices_[faces_[i][1]]);
        normal=normal+centerTriangle;
        normal.normalize();
        glNormal3d(normal.x(),normal.y(),normal.z());

        for (unsigned int j=0;j<faces_[i].size();j++)
        {
          const Vector3d& v = vertices_[faces_[i][j]];
          glVertex3dv (v.ptr());
        }
        glEnd();
      }

      glPopMatrix();

      glPrintErrors ("MyPlane::draw()");

    };

    void placeMe (Vector3d p)
    {
      origin_ = p;
    };

    void flyMe (double step)
    {
      origin_ += nose_*step;
    };

    void rollMe (double alpha) 
    { 
      Matrix4d R;
      R.makeRotate (alpha, nose_);

      up_ = R*up_;
    }

    void yawMe (double alpha) 
    { 
      Matrix4d R;
      R.makeRotate (alpha, up_);

      nose_ = R*nose_;
    }

    void pitchMe (double alpha)
    {
      Matrix4d R;
      R.makeRotate (alpha, up_%nose_);

      up_ = R*up_;
      nose_=R*nose_;
    }



    int size () {return points_.size();};

    Vector3d nose (){return nose_;};
    Vector3d up (){return up_;};
    Vector3d origin (){return origin_;};

  private:

    /// Achtung! Diese Sch"one Funktion hilft Euch beim Debuggen von OpenGL-Code!
    /// Einfach am Ende jeder Funktion, die OpenGL verwendet, aufrufen. Das Argument
    /// sollte der Funktionsnamen sein, das hilft dann beim Fehlerfinden!
    void glPrintErrors(const char* as_Function)
    {
      GLenum lr_Error = GL_NO_ERROR;
      bool lb_ErrorFound = false;
      int li_ErrorCount = 5;
      do
      {
        lr_Error = glGetError();
        if (lr_Error != GL_NO_ERROR)
        {
          lb_ErrorFound = true;
          li_ErrorCount--;
          char* ls_ErrorString = (char*)gluErrorString(lr_Error);
          std::cerr << "OpenGL Error (" << as_Function << "): " << ls_ErrorString << std::endl;
        }
        if (li_ErrorCount == 0)
        {
          std::cerr << "OPENGL :: ERROR Too many errors!" << std::endl;
          break;
        }
      } while (lr_Error != GL_NO_ERROR);
      if (lb_ErrorFound) exit(0);
    };

    void set_into_mass_center ()
    {
      Vector3d c = Vector3d (0,0,0);
      for (unsigned int i=0;i<vertices_.size();i++)
        c += vertices_[i];

      c/= vertices_.size ();

      for (unsigned int i=0;i<vertices_.size();i++)
        vertices_[i] -= c;

      origin_ = c;
    }

    void scale_into_sphere (double radius)
    {	
      assert (radius > 1e-10);	
      double abs = 0;
      for (unsigned int i = 0;i<vertices_.size();i++)
      {
        const double& abs_tmp = vertices_[i].lengthSquared();
        if (abs_tmp>abs) abs = abs_tmp;
      }
      abs = sqrt(abs)/radius;

      for (unsigned int i = 0;i<vertices_.size();i++)
        vertices_[i] /= abs;
    }

    void map_axes_to (Vector3d map_nose, Vector3d map_up)
    {
      ///Sind vektoren wirklich ortogonal?
      assert ((map_nose*map_up)*(map_nose*map_up) < 1e-10);
      Vector3d nxu = map_nose % map_up;

      Matrix4d R(
          map_nose.x(),   map_nose.y(),   map_nose.z(),   0,
          nxu.x(),        nxu.y(),        nxu.z(),        0,
          map_up.x(),     map_up.y(),     map_up.z(),     0,
          0,              0,              0,              1);

      for (unsigned int i = 0;i<vertices_.size();i++)
        vertices_[i] = R * vertices_[i];



    }

    std::vector<Vector3d> points_;

    //Ursprung
    Vector3d origin_;

    //Nasenvektor
    Vector3d nose_;

    //Upvector
    Vector3d up_;

    //Knoten
    std::vector <Vector3d> vertices_;

    //Faces
    std::vector <std::vector <int> > faces_;


};

#endif
