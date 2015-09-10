#include "modernGL.h"

void CGView::initSolidCubeVBO() {
    std::vector<QVector3D> vertexWithNormal;
    GLuint id;
    glGenBuffers(1,&id);

    vertexWithNormal.push_back(QVector3D(-1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 0,-1, 0));
    vertexWithNormal.push_back(QVector3D( 1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 0,-1, 0));
    vertexWithNormal.push_back(QVector3D( 1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 0,-1, 0));
    vertexWithNormal.push_back(QVector3D( 1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 0,-1, 0));
    vertexWithNormal.push_back(QVector3D(-1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 0,-1, 0));
    vertexWithNormal.push_back(QVector3D(-1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 0,-1, 0));

    vertexWithNormal.push_back(QVector3D(-1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 1, 0));
    vertexWithNormal.push_back(QVector3D(-1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 1, 0));
    vertexWithNormal.push_back(QVector3D( 1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 1, 0));
    vertexWithNormal.push_back(QVector3D( 1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 1, 0));
    vertexWithNormal.push_back(QVector3D( 1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 1, 0));
    vertexWithNormal.push_back(QVector3D(-1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 1, 0));

    vertexWithNormal.push_back(QVector3D(-1, 1, 1));
    vertexWithNormal.push_back(QVector3D(-1, 0, 0));
    vertexWithNormal.push_back(QVector3D(-1, 1,-1));
    vertexWithNormal.push_back(QVector3D(-1, 0, 0));
    vertexWithNormal.push_back(QVector3D(-1,-1,-1));
    vertexWithNormal.push_back(QVector3D(-1, 0, 0));
    vertexWithNormal.push_back(QVector3D(-1,-1,-1));
    vertexWithNormal.push_back(QVector3D(-1, 0, 0));
    vertexWithNormal.push_back(QVector3D(-1,-1, 1));
    vertexWithNormal.push_back(QVector3D(-1, 0, 0));
    vertexWithNormal.push_back(QVector3D(-1, 1, 1));
    vertexWithNormal.push_back(QVector3D(-1, 0, 0));

    vertexWithNormal.push_back(QVector3D( 1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 1, 0, 0));
    vertexWithNormal.push_back(QVector3D( 1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 1, 0, 0));
    vertexWithNormal.push_back(QVector3D( 1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 1, 0, 0));
    vertexWithNormal.push_back(QVector3D( 1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 1, 0, 0));
    vertexWithNormal.push_back(QVector3D( 1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 1, 0, 0));
    vertexWithNormal.push_back(QVector3D( 1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 1, 0, 0));

    vertexWithNormal.push_back(QVector3D( 1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 0,-1));
    vertexWithNormal.push_back(QVector3D(-1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 0,-1));
    vertexWithNormal.push_back(QVector3D(-1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 0,-1));
    vertexWithNormal.push_back(QVector3D(-1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 0,-1));
    vertexWithNormal.push_back(QVector3D( 1, 1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 0,-1));
    vertexWithNormal.push_back(QVector3D( 1,-1,-1));
    vertexWithNormal.push_back(QVector3D( 0, 0,-1));

    vertexWithNormal.push_back(QVector3D(-1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 0, 1));
    vertexWithNormal.push_back(QVector3D(-1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 0, 1));
    vertexWithNormal.push_back(QVector3D( 1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 0, 1));
    vertexWithNormal.push_back(QVector3D( 1,-1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 0, 1));
    vertexWithNormal.push_back(QVector3D( 1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 0, 1));
    vertexWithNormal.push_back(QVector3D(-1, 1, 1));
    vertexWithNormal.push_back(QVector3D( 0, 0, 1));

    glBindBuffer(GL_ARRAY_BUFFER,id);
    glBufferData(GL_ARRAY_BUFFER,vertexWithNormal.size()*sizeof(QVector3D),vertexWithNormal.data(),GL_STATIC_DRAW);

    vboSolidCubeId = id;
    vboSolidCubeSize = 3*12;
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
    glBufferData(GL_ARRAY_BUFFER,vertexWithNormal.size()*sizeof(QVector3D),vertexWithNormal.data(),GL_STATIC_DRAW);

    vboSolidSphereId = id;
    vboSolidSphereSize = static_cast<int>(ico.size());
}

void CGView::initSolidCylinderVBO() {
    std::vector<QVector3D> vertexWithNormal;
    const int n = 64;
    std::vector<QVector3D> circle(n);

    circle[0]     = QVector3D( 1.0, 0.0, 0.0);
    circle[n/4]   = QVector3D( 0.0, 1.0, 0.0);
    circle[n/2]   = QVector3D(-1.0, 0.0, 0.0);
    circle[3*n/4] = QVector3D( 0.0,-1.0, 0.0);

    for(int r=n/4;r>1;r/=2)
       for(int i=0;i<n;i+=r) {
          circle[i+r/2] = circle[i]+circle[(i+r)%n];
          circle[i+r/2].normalize(); 
       }

    QVector3D ez(0.0,0.0,1.0);

    for(int i=0;i<n;i++) {
       vertexWithNormal.push_back(circle[i]);
       vertexWithNormal.push_back(circle[i]);
       vertexWithNormal.push_back(circle[(i+1)%n]);
       vertexWithNormal.push_back(circle[(i+1)%n]);
       vertexWithNormal.push_back(circle[i]+ez);
       vertexWithNormal.push_back(circle[i]);

       vertexWithNormal.push_back(circle[(i+1)%n]+ez);
       vertexWithNormal.push_back(circle[(i+1)%n]);
       vertexWithNormal.push_back(circle[i]+ez);
       vertexWithNormal.push_back(circle[i]);
       vertexWithNormal.push_back(circle[(i+1)%n]);
       vertexWithNormal.push_back(circle[(i+1)%n]);
    }

    GLuint id;
    glGenBuffers(1,&id);

    glBindBuffer(GL_ARRAY_BUFFER,id);
    glBufferData(GL_ARRAY_BUFFER,vertexWithNormal.size()*sizeof(QVector3D),vertexWithNormal.data(),GL_STATIC_DRAW);

    vboSolidCylinderId = id;
    vboSolidCylinderSize = static_cast<int>(vertexWithNormal.size()/2);
}

QMatrix4x4 orthonormalSystem(QVector3D e3) {
    qreal u[3],v[3];
        e3.normalize();
    v[0] = e3.x();
    v[1] = e3.y();
    v[2] = e3.z();
    int imax = 0;
    if (fabs(v[1]) > fabs(v[imax])) imax = 1;
    if (fabs(v[2]) > fabs(v[imax])) imax = 2;
    u[imax] = v[(imax+1)%3];
    u[(imax+1)%3] = -v[imax];
    u[(imax+2)%3] = 0.0;
    QVector3D e2(u[0],u[1],u[2]);
    e2.normalize();
    QVector3D e1 = QVector3D::crossProduct(e2,e3);
    e1.normalize();

    return QMatrix4x4(e1.x(),e2.x(),e3.x(),0.0,
                      e1.y(),e2.y(),e3.y(),0.0,
                      e1.z(),e2.z(),e3.z(),0.0,
                      0.0,   0.0,   0.0,   1.0);
}

void CGView::drawSolidCylinder(const QVector3D& a, const QVector3D& b, qreal radius) {
    QMatrix4x4 M(model);
    M.translate(a);
    M *= orthonormalSystem(b-a);
    M.scale(radius,radius,(b-a).length());
    glBindBuffer(GL_ARRAY_BUFFER, vboSolidCylinderId);
    int vertexLocation;

        program.setUniformValue("model_matrix", M);
        program.setUniformValue("normal_matrix", (view*M).normalMatrix());
        vertexLocation = program.attributeLocation("a_position");
        program.enableAttributeArray(vertexLocation);
        glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), 0);
        int normalLocation = program.attributeLocation("a_normal");
        program.enableAttributeArray(normalLocation);
        glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), (const void*) sizeof(QVector3D));
    glDrawArrays(GL_TRIANGLES,0,vboSolidCylinderSize);
}

void CGView::drawSolidSphere(const QVector3D& c, qreal r) {
    QMatrix4x4 M(model);
    M.translate(c);
    M.scale(r);
    glBindBuffer(GL_ARRAY_BUFFER, vboSolidSphereId);
    int vertexLocation;

        program.setUniformValue("model_matrix", M);
        program.setUniformValue("normal_matrix", (view*M).normalMatrix());
        vertexLocation = program.attributeLocation("a_position");
        program.enableAttributeArray(vertexLocation);
        glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), 0);
        int normalLocation = program.attributeLocation("a_normal");
        program.enableAttributeArray(normalLocation);
        glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), (const void*) sizeof(QVector3D));
   glDrawArrays(GL_TRIANGLES,0,vboSolidSphereSize);
}

void CGView::drawSolidCube(const QVector3D& c, qreal r){
    QMatrix4x4 M(model);
    M.translate(c);
    M.scale(r);

    program.setUniformValue("model_matrix", M);
    program.setUniformValue("normal_matrix", (view*M).normalMatrix());
    int vertexLocation;
    glBindBuffer(GL_ARRAY_BUFFER, vboSolidCubeId);
    vertexLocation = program.attributeLocation("a_position");
    int normalLocation = program.attributeLocation("a_normal");
    program.enableAttributeArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), 0);
    program.enableAttributeArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), (const void*) sizeof(QVector3D));
    glDrawArrays(GL_TRIANGLES,0,vboSolidCubeSize);
}



void CGView::initTorusVBO(double R, double r, int N = 100, int n = 100){
    std::vector<QVector3D> vertexWithNormal;
    glGenBuffers(1,&vboTorusId);

    double R_ = R;
    double r_ = r;
    int num_t = N;
    int num_p = n;
    double deltaT = 2.0*M_PI/num_t ;
    double deltaP = 2.0*M_PI/num_p ;
    QVector3D normal;
    for(int i = 0 ; i < num_t ; i++){
        for(int j = 0 ; j < num_p ; j++){

#define TORUS_VERTEX_NORMAL p = QVector3D((R_+r_*cos(deltaP*j))*cos(deltaT*i),(R_+r_*cos(deltaP*j))*sin(deltaT*i),r_*sin(deltaP*j));\
    dTHETA = QVector3D(-(R_+r_*cos(j*deltaP))*sin(i*deltaT),(R_+r_*cos(j*deltaP))*cos(i*deltaT),0.0);\
    dPHI = QVector3D(-r_*sin(j*deltaP)*cos(i*deltaT),-r_*sin(j*deltaP)*sin(i*deltaT), r_*cos(j*deltaP));\
    n = QVector3D::crossProduct(dPHI,dTHETA);\
    n.normalize();\
    vertexWithNormal.push_back(p);\
    vertexWithNormal.push_back(n);

            QVector3D p, dTHETA, dPHI, n;
            TORUS_VERTEX_NORMAL
                            i++;
            TORUS_VERTEX_NORMAL
                            j++;
            TORUS_VERTEX_NORMAL
                            i--;
            TORUS_VERTEX_NORMAL
                            j--;
#undef TORUS_VERTEX_NORMAL


        }
    }
    vboTorusSize = vertexWithNormal.size()/2;
    glBindBuffer(GL_ARRAY_BUFFER,vboTorusId);
    glBufferData(GL_ARRAY_BUFFER,vertexWithNormal.size()*sizeof(QVector3D),vertexWithNormal.data(),GL_STATIC_DRAW);
}

void CGView::drawTorus(){

    QMatrix4x4 M(model);
    program.setUniformValue("model_matrix", M);
    program.setUniformValue("normal_matrix", (view*M).normalMatrix());
    int vertexLocation;
    glBindBuffer(GL_ARRAY_BUFFER, vboTorusId);

    vertexLocation = program.attributeLocation("a_position");
    program.enableAttributeArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), 0);
    int normalLocation = program.attributeLocation("a_normal");
    program.enableAttributeArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(QVector3D), (const void*) sizeof(QVector3D));
    //        unsigned int texLocation = program.uniformLocation("texture");
    //        glUniform1i(texLocation, 1);
    //        glActiveTexture(GL_TEXTURE1);

    glDrawArrays(GL_QUADS,0,vboTorusSize);
}
