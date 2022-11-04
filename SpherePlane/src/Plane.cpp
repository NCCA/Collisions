#include "Plane.h"
#include <ngl/ShaderLib.h>
#include <ngl/Util.h>
#include <ngl/Vec3.h>
#include <ngl/VAOFactory.h>
#include "MultiBufferIndexVAO.h"
#include <ngl/SimpleVAO.h>
Plane::Plane(const ngl::Vec3 &_center, GLfloat _w, GLfloat _d)
{
  // store the values
  m_center = _center;
  m_width = _w;
  m_depth = _d;
  // now create the vertices based on the values passed in
  m_oVerts[0].m_x = m_center.m_x - (m_width / 2.0f);
  m_oVerts[0].m_y = m_center.m_y;
  m_oVerts[0].m_z = m_center.m_z + (m_depth / 2.0f);

  m_oVerts[1].m_x = m_center.m_x + (m_width / 2.0f);
  m_oVerts[1].m_y = m_center.m_y;
  m_oVerts[1].m_z = m_center.m_z + (m_depth / 2.0f);

  m_oVerts[2].m_x = m_center.m_x + (m_width / 2.0f);
  m_oVerts[2].m_y = m_center.m_y;
  m_oVerts[2].m_z = m_center.m_z - (m_depth / 2.0f);

  m_oVerts[3].m_x = m_center.m_x - (m_width / 2.0f);
  m_oVerts[3].m_y = m_center.m_y;
  m_oVerts[3].m_z = m_center.m_z - (m_depth / 2.0f);
  // finally calculate the surface m_normal
  m_normal = calcNormal(m_oVerts[3], m_oVerts[2], m_oVerts[1]);
  m_xrot = m_zrot = 0;
}

Plane::Plane()
{
  // default ctor set 0.0 as origin and unit size
  m_center.set(0, 0, 0);
  m_width = 1.0;
  m_depth = 1.0;

  m_oVerts[0].m_x = m_center.m_x - (m_width / 2.0f);
  m_oVerts[0].m_y = m_center.m_y;
  m_oVerts[0].m_z = m_center.m_z + (m_depth / 2.0f);
  m_oVerts[1].m_x = m_center.m_x + (m_width / 2.0f);
  m_oVerts[1].m_y = m_center.m_y;
  m_oVerts[1].m_z = m_center.m_z + (m_depth / 2.0f);
  m_oVerts[2].m_x = m_center.m_x + (m_width / 2.0f);
  m_oVerts[2].m_y = m_center.m_y;
  m_oVerts[2].m_z = m_center.m_z - (m_depth / 2.0f);
  m_oVerts[3].m_x = m_center.m_x - (m_width / 2.0f);
  m_oVerts[3].m_y = m_center.m_y;
  m_oVerts[3].m_z = m_center.m_z - (m_depth / 2.0f);
  m_normal = ngl::calcNormal(m_oVerts[3], m_oVerts[2], m_oVerts[1]);
}

Plane::~Plane()
{
  // do nothing for now
}

void Plane::loadMatricesToShader(const ngl::Mat4 &_view, const ngl::Mat4 &_project) const
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV = _view * m_mouseRot;
  MVP = _project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
}

void Plane::draw(const std::string &_shaderName, const ngl::Mat4 &_view, const ngl::Mat4 &_project, const ngl::Mat4 &_rotMat)
{
  m_mouseRot = _rotMat;
  // create the m_points array for drawing the quad as a tri
  std::vector<ngl::Vec3> normals(4);

  ngl::Vec3 normal = ngl::calcNormal(m_verts[0], m_verts[2], m_verts[1]);

  for (size_t i = 0; i < 4; ++i)
  {
    normals[i] = normal;
  }
  GLubyte indices[] = {0, 1, 3, 3, 2, 1};
  ngl::ShaderLib::use(_shaderName);
  ngl::ShaderLib::setUniform("Colour", 1.0f, 1.0f, 0.0f, 0.0f);
  std::unique_ptr<ngl::AbstractVAO> vao(ngl::VAOFactory::createVAO("multiBufferIndexVAO", GL_TRIANGLES));

  vao->bind();
  vao->setData(MultiBufferIndexVAO::VertexData(4 * sizeof(ngl::Vec3), m_verts[0].m_x));
  vao->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(ngl::Vec3), 0);
  vao->setData(MultiBufferIndexVAO::VertexData(4 * sizeof(ngl::Vec3), normals[0].m_x));

  vao->setVertexAttributePointer(1, 3, GL_FLOAT, sizeof(ngl::Vec3), 0);

  dynamic_cast<MultiBufferIndexVAO *>(vao.get())->setIndices(sizeof(indices), &indices[0], GL_UNSIGNED_BYTE);

  vao->setNumIndices(6);
  loadMatricesToShader(_view, _project);
  vao->draw();

  // now draw normal

  std::vector<ngl::Vec3> lines(2);
  lines[0] = m_center;
  lines[1] = (m_normal * 4.0);
  vao = ngl::VAOFactory::createVAO("simpleVAO", GL_LINES);
  vao->bind();
  vao->setData(ngl::SimpleVAO::VertexData(2 * sizeof(ngl::Vec3), lines[0].m_x));
  vao->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(ngl::Vec3), 0);
  vao->setNumIndices(2);
  vao->draw();
}

// modify the verts based on the dt and a flag to indicate which to tilt
void Plane::tilt(GLfloat dt, bool X, bool Z)
{

  // create a matrix for the m_xrotation and set to ident
  ngl::Mat4 m_xrotMatrix = 1;
  // create a m_zrotation matrix and set to ident
  ngl::Mat4 m_zrotMatrix = 1;

  if (X)
  {
    m_xrot += dt;
  }

  if (Z)
  {
    m_zrot += dt;
  }

  m_xrotMatrix = ngl::Mat4::rotateX(m_xrot);
  m_zrotMatrix = ngl::Mat4::rotateZ(m_zrot);

  // combine them together for the final rotation
  ngl::Mat4 rotMatrix = m_zrotMatrix * m_xrotMatrix;
  // now modify the verts by multiplying the points with the matrix
  for (int i = 0; i < 4; ++i)
  {
    m_verts[i] = rotMatrix * m_oVerts[i];
  }
  m_normal = calcNormal(m_verts[3], m_verts[2], m_verts[1]);
}
