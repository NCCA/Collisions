// note NGL uses PLANE_H_
#ifndef MYPLANE_H_
#define MYPLANE_H_

#include <ngl/Vec3.h>
#include <ngl/Transformation.h>

/*! \brief a simple class to store a plane */

class Plane
{
public :
  // ctor
  Plane(const ngl::Vec3 &_center,GLfloat _w, GLfloat _d );
  Plane();
  ~Plane();
  // method to draw the plane
  void draw(const std::string &_shaderName,  const ngl::Mat4 &_view, const ngl::Mat4 &_project , const ngl::Mat4 &_rotMat	);
	// Method to tilt the plane
	void tilt( GLfloat _dt,	 bool _x, bool _z	);

  // method to reset the rotation values
  void reset(){
                      m_xrot=0;
                      m_zrot=0;
                      tilt(0,0,0);
                      }
  ngl::Vec3 getNormal()const {return m_normal;}
  ngl::Vec3 getCenter() const {return m_center;}
  GLfloat getDepth() const {return m_depth;}
  GLfloat getWidth() const {return m_width;}

private :
  // and array of vectors for the vertices
  ngl::Vec3 m_verts[4];
  // the original verts
  ngl::Vec3 m_oVerts[4];
  // Xrotation
  ngl::Real m_xrot;
  // Yroation
  ngl::Real m_zrot;
  // Vector for the center of the plane
  ngl::Vec3 m_center;
  // Width of the plane
  GLfloat m_width;
  // Depth ofthe plane
  GLfloat m_depth;
  // The surface normal
  ngl::Vec3 m_normal;
  // mouse rotation
  ngl::Mat4 m_mouseRot;
  void loadMatricesToShader(const ngl::Mat4 &_view , const ngl::Mat4 &_project) const;
};

#endif

