#include "Sphere.h"
#include <ngl/VAOPrimitives.h>


Sphere::Sphere(ngl::Vec3 _pos,  GLfloat _rad)
{
  // set values from params
  m_pos=_pos;
  m_radius=_rad;
  m_hit=false;
}

Sphere::Sphere()
{
  m_hit=false;
}

void Sphere::loadMatricesToShader( ngl::Transformation  &_tx,  const ngl::Mat4 &_globalMat, const ngl::Mat4 &_view, const ngl::Mat4 &_project) const
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV=_view*_globalMat *_tx.getMatrix()  ;
  MVP=_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse();
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
}



void Sphere::draw(const std::string &_shaderName, const ngl::Mat4 &_globalMat, ngl::Mat4 &_view , const ngl::Mat4 &_project)const

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
  ngl::Transformation t;
  t.setPosition(m_pos);
  t.setScale(m_radius,m_radius,m_radius);
  loadMatricesToShader(t,_globalMat,_view,_project);
  ngl::VAOPrimitives::draw("sphere");

  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}














