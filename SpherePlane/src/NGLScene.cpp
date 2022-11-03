#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include "MultiBufferIndexVAO.h"
#include <algorithm>

NGLScene::NGLScene(int _numSpheres)
{
  m_numSpheres = _numSpheres;

  setTitle("Sphere -> Plane Collision");
  m_sphereUpdateTimer = startTimer(130);
  m_animate = true;
  // now create the actual spheres for our program

  m_plane = new Plane(ngl::Vec3(0, 0, 0), 5, 5);
  ngl::Vec3 pos;
  // now create the actual spheres for our program

  m_sphereArray.resize(_numSpheres);
  std::generate(std::begin(m_sphereArray), std::end(m_sphereArray), [this]()
                { return Sphere(ngl::Vec3(ngl::Random::randomNumber(6), 8,
                                          ngl::Random::randomNumber(6)),
                                ngl::Vec3(0.0f, -1.0f, 0.0f), 0.2f); });
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f, 0.0f, 15.0f);
  ngl::Vec3 to(0.0f, 0.0f, 0.0f);
  ngl::Vec3 up(0.0f, 1.0f, 0.0f);
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45.0f, 720.0f / 576.0f, 0.05f, 350.0f);

  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::ShaderLib::setUniform("Colour", 1.0f, 1.0f, 0.0f, 1.0f);
  ngl::ShaderLib::setUniform("lightPos", 1.0f, 1.0f, 1.0f);
  ngl::ShaderLib::setUniform("lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);

  ngl::ShaderLib::use("nglColourShader");
  ngl::ShaderLib::setUniform("Colour", 1.0f, 1.0f, 1.0f, 1.0f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces
  ngl::VAOPrimitives::createSphere("sphere", 1.0f, 40.0f);
  ngl::VAOFactory::registerVAOCreator("multiBufferIndexVAO", MultiBufferIndexVAO::create);
  ngl::VAOFactory::listCreators();
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_mouseGlobalTX;
  MV = m_view * M;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MV", MV);
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  ngl::ShaderLib::setUniform("M", M);
}

void NGLScene::loadMatricesToColourShader()
{
  ngl::ShaderLib::use("nglColourShader");
  ngl::Mat4 MVP;
  MVP = m_project * m_view * m_mouseGlobalTX;
  ngl::ShaderLib::setUniform("MVP", MVP);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);
  // Rotation based on the mouse position for our global
  // transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  m_plane->draw("nglDiffuseShader", m_view, m_project, m_mouseGlobalTX);
  for (Sphere s : m_sphereArray)
  {
    s.draw("nglDiffuseShader", m_mouseGlobalTX, m_view, m_project);
  }
}

void NGLScene::updateScene()
{
  static int updateCount = 0;
  for (Sphere &s : m_sphereArray)
  {
    s.move();
  }
  spherePlaneCollide();

  if (++updateCount == 20)
  {
    updateCount = 0;
    ngl::Vec3 pos;
    for (Sphere &s : m_sphereArray)
    {
      pos.set(ngl::Random::randomNumber(6.0f), 8.0f, ngl::Random::randomNumber(6.0f));
      s.set(pos, ngl::Vec3(0.0f, -1.0f, 0.0f), 0.2f);
    }
  }
}
void NGLScene::timerEvent(QTimerEvent *_event)
{
  if (_event->timerId() == m_sphereUpdateTimer)
  {
    if (m_animate != true)
    {
      return;
    }
    updateScene();
    update();
  }
}
void NGLScene::spherePlaneCollide()
{
  ngl::Vec3 p;
  GLfloat D;
  for (Sphere &s : m_sphereArray)
  {
    p = s.getPos();

    // If a collision is found we change the m_dir of the Sphere then Break
    // with the new point P
    D = m_plane->getNormal().dot(p);
    // Now Add the radius of the sphere to the offsett
    D += s.getRadius();
    // If this is greater or equal to the BBox extent /2 then there is a collision
    // So we calculate the Spheres new direction
    if (D <= 0.0f)
    {

      // we on the plane now see if we hit it or not
      if (s.getPos().m_x > m_plane->getCenter().m_x - (m_plane->getWidth() / 2.0f) &&
          s.getPos().m_x < m_plane->getCenter().m_x + (m_plane->getWidth() / 2.0f) &&
          s.getPos().m_z > m_plane->getCenter().m_z - (m_plane->getDepth() / 2.0f) &&
          s.getPos().m_z < m_plane->getCenter().m_z + (m_plane->getDepth() / 2.0f))
      {

        s.setDirection(m_plane->getNormal());
        s.setHit();
      } // end of hit test
    }
  } // end of each face test
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent(QMouseEvent *_event)
{
// note the method buttons() is the button state when event was called
// that is different from button() which is used to check which button was
// pressed when the mousePress/Release event is generated
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (m_win.rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx = position.x() - m_win.origX;
    int diffy = position.y() - m_win.origY;
    m_win.spinXFace += static_cast<int>(0.5f * diffy);
    m_win.spinYFace += static_cast<int>(0.5f * diffx);
    m_win.origX = position.x();
    m_win.origY = position.y();
    update();
  }
  // right mouse translate code
  else if (m_win.translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = static_cast<int>(position.x() - m_win.origXPos);
    int diffY = static_cast<int>(position.y() - m_win.origYPos);
    m_win.origXPos = position.x();
    m_win.origYPos = position.y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent(QMouseEvent *_event)
{
// that method is called when the mouse button is pressed in this case we
// store the value where the maouse was clicked (x,y) and set the Rotate flag to true
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (_event->button() == Qt::LeftButton)
  {
    m_win.origX = position.x();
    m_win.origY = position.y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if (_event->button() == Qt::RightButton)
  {
    m_win.origXPos = position.x();
    m_win.origYPos = position.y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent(QMouseEvent *_event)
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if (_event->angleDelta().x() > 0)
  {
    m_modelPos.m_z += ZOOM;
  }
  else if (_event->angleDelta().x() < 0)
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  case Qt::Key_Up:
    m_plane->tilt(1.0, 1, 0);
    break;
  case Qt::Key_Down:
    m_plane->tilt(-1.0, 1, 0);
    break;
  case Qt::Key_Left:
    m_plane->tilt(-1.0, 0, 1);
    break;
  case Qt::Key_Right:
    m_plane->tilt(1.0, 0, 1);
    break;
  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}
