#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"

#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <algorithm>
#include <iostream>
//----------------------------------------------------------------------------------------------------------------------
/// @brief extents of the bbox
//----------------------------------------------------------------------------------------------------------------------
const static int s_extents = 20;

NGLScene::NGLScene(int _numSpheres)
{
  setTitle("Sphere Bounding Box Collisions");
  m_animate = true;
  m_checkSphereSphere = false;
  // create vectors for the position and direction
  m_numSpheres = _numSpheres;
  resetSpheres();
}

void NGLScene::resetSpheres()
{
  m_sphereArray.resize(m_numSpheres);

  std::generate(std::begin(m_sphereArray), std::end(m_sphereArray), [this]()
                { return Sphere(ngl::Random::getRandomPoint(s_extents, s_extents, s_extents),
                                ngl::Random::getRandomVec3(),
                                ngl::Random::randomPositiveNumber(2) + 0.5f); });
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
  ngl::Vec3 from(0.0f, 80.0f, 80.0f);
  ngl::Vec3 to(0.0f, 0.0f, 0.0f);
  ngl::Vec3 up(0.0f, 1.0f, 0.0f);
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45.0f, 720.0f / 576.0f, 0.5f, 150.0f);
  // now to load the shader and set the values

  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::ShaderLib::setUniform("Colour", 1.0f, 1.0f, 0.0f, 1.0f);
  ngl::ShaderLib::setUniform("lightPos", 1.0f, 1.0f, 1.0f);
  ngl::ShaderLib::setUniform("lightDiffuse", 1.0f, 1.0f, 1.0f, 1.0f);

  ngl::ShaderLib::use("nglColourShader");
  ngl::ShaderLib::setUniform("Colour", 1.0f, 1.0f, 1.0f, 1.0f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

  ngl::VAOPrimitives::createSphere("sphere", 1.0f, 40.0f);
  // create our Bounding Box, needs to be done once we have a gl context as we create VAO for drawing
  m_bbox = std::make_unique<ngl::BBox>(ngl::Vec3(0.0f, 0.0f, 0.0f), 80.0f, 80.0f, 80.0f);
  m_bbox->setDrawMode(GL_LINE);
  m_sphereUpdateTimer = startTimer(40);
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("nglDiffuseShader");
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV = m_view * m_mouseGlobalTX;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
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

  ngl::ShaderLib::use("nglColourShader");
  loadMatricesToColourShader();
  m_bbox->draw();

  ngl::ShaderLib::use("nglDiffuseShader");

  for (Sphere s : m_sphereArray)
  {
    s.draw("nglDiffuseShader", m_mouseGlobalTX, m_view, m_project);
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::updateScene()
{
  for (Sphere &s : m_sphereArray)
  {
    s.move();
  }
  checkCollisions();
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
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  case Qt::Key_Space:
    m_animate ^= true;
    break;
  case Qt::Key_S:
    m_checkSphereSphere ^= true;
    break;
  case Qt::Key_R:
    resetSpheres();
    break;
  case Qt::Key_Minus:
    removeSphere();
    break;
  case Qt::Key_Plus:
    addSphere();
    break;

  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  if (_event->timerId() == m_sphereUpdateTimer)
  {
    if (m_animate != true)
    {
      return;
    }
  }
  updateScene();
  update();
}

bool NGLScene::sphereSphereCollision(ngl::Vec3 _pos1, GLfloat _radius1, ngl::Vec3 _pos2, GLfloat _radius2)
{
  // the relative position of the spheres
  ngl::Vec3 relPos;
  // min an max distances of the spheres
  GLfloat dist;
  GLfloat minDist;
  GLfloat len;
  relPos = _pos1 - _pos2;
  // and the distance
  len = relPos.length();
  dist = len * len;
  minDist = _radius1 + _radius2;
  // if it is a hit
  if (dist <= (minDist * minDist))
  {
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::BBoxCollision()
{
  // create an array of the extents of the bounding box
  float ext[6];
  ext[0] = ext[1] = (m_bbox->height() / 2.0f);
  ext[2] = ext[3] = (m_bbox->width() / 2.0f);
  ext[4] = ext[5] = (m_bbox->depth() / 2.0f);
  // Dot product needs a Vector so we convert The Point Temp into a Vector so we can
  // do a dot product on it
  ngl::Vec3 p;
  // D is the distance of the Agent from the Plane. If it is less than ext[i] then there is
  // no collision
  GLfloat D;
  // Loop for each sphere in the vector list
  for (Sphere &s : m_sphereArray)
  {
    p = s.getPos();
    // Now we need to check the Sphere agains all 6 planes of the BBOx
    // If a collision is found we change the dir of the Sphere then Break
    for (int i = 0; i < 6; ++i)
    {
      // to calculate the distance we take the dotporduct of the Plane Normal
      // with the new point P
      D = m_bbox->getNormalArray()[i].dot(p);
      // Now Add the Radius of the sphere to the offsett
      D += s.getRadius();
      // If this is greater or equal to the BBox extent /2 then there is a collision
      // So we calculate the Spheres new direction
      if (D >= ext[i])
      {
        // We use the same calculation as in raytracing to determine the
        //  the new direction
        GLfloat x = 2 * (s.getDirection().dot((m_bbox->getNormalArray()[i])));
        ngl::Vec3 d = m_bbox->getNormalArray()[i] * x;
        s.setDirection(s.getDirection() - d);
        s.setHit();
      } // end of hit test
    }   // end of each face test
  }     // end of for
}

void NGLScene::checkSphereCollisions()
{
  bool collide;

  unsigned int size = m_sphereArray.size();

  for (unsigned int ToCheck = 0; ToCheck < size; ++ToCheck)
  {
    for (unsigned int Current = 0; Current < size; ++Current)
    {
      // don't check against self
      if (ToCheck == Current)
        continue;

      else
      {
        // cout <<"doing check"<<endl;
        collide = sphereSphereCollision(m_sphereArray[Current].getPos(), m_sphereArray[Current].getRadius(),
                                        m_sphereArray[ToCheck].getPos(), m_sphereArray[ToCheck].getRadius());
        if (collide == true)
        {
          m_sphereArray[Current].reverse();
          m_sphereArray[Current].setHit();
        }
      }
    }
  }
}

void NGLScene::checkCollisions()
{

  if (m_checkSphereSphere == true)
  {
    checkSphereCollisions();
  }
  BBoxCollision();
}

void NGLScene::removeSphere()
{
  std::vector<Sphere>::iterator end = m_sphereArray.end();
  if (--m_numSpheres == 0)
  {
    m_numSpheres = 1;
  }
  else
  {
    m_sphereArray.erase(end - 1, end);
  }
}

void NGLScene::addSphere()
{

  // add the spheres to the end of the particle list
  m_sphereArray.push_back(Sphere(ngl::Random::getRandomPoint(s_extents, s_extents, s_extents), ngl::Random::getRandomVec3(), ngl::Random::randomPositiveNumber(2) + 0.5));
  ++m_numSpheres;
}
