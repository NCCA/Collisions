#include "Sphere.h"
#include <ngl/VAOPrimitives.h>


Sphere::Sphere(const ngl::Vec3 &_pos,const ngl::Vec3 &_dir, GLfloat _rad)
{
  // set values from params
  m_pos=_pos;
  m_dir=_dir;
  m_radius=_rad;
  m_hit=false;
}

Sphere::Sphere()
{
  m_hit=false;
}

void Sphere::loadMatricesToShader( ngl::Transformation &_transform,const ngl::Mat4 &_globalTx, const ngl::Mat4 &_view, const ngl::Mat4 &_project ) const
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=_globalTx*_transform.getMatrix();
  MV=_view *M;
  MVP=_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
  ngl::ShaderLib::setUniform("Colour",m_colour);

}


void Sphere::draw( const std::string &_shaderName, const ngl::Mat4 &_globalTx, const ngl::Mat4 &_view, const ngl::Mat4 &_project )const
{

  // draw wireframe if hit
  if(m_hit)
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  }


  ngl::ShaderLib::use(_shaderName);
  // grab an instance of the primitives for drawing
  ngl::Transformation transform;
  transform.reset();
  {

    transform.setPosition(m_pos);
    transform.setScale(m_radius,m_radius,m_radius);
    loadMatricesToShader(transform,_globalTx,_view,_project);
    ngl::VAOPrimitives::draw("sphere");
  }
glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}



void Sphere :: set(const ngl::Vec3 &_pos,   const ngl::Vec3 &_dir,  GLfloat _rad )
{
  m_pos=_pos;
  m_dir=_dir;
  m_radius=_rad;
}

void Sphere::move()
{
  // store the last position
  m_lastPos=m_pos;
  // update the current position
  m_pos+=m_dir;
  // get the next position
  m_nextPos=m_pos+m_dir;
  m_hit=false;
}












