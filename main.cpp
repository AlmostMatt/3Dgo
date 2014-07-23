/* ***************************************************************
 *
 * File :  main.cpp
 *
 * Author : Tiberiu Popa
 *          J. Alexander Clarke
 * Date   : June 18th, 2002
 *
 * Modified:
 *
 * Purpose: Simple OpenGL program to illustrate the use of SDL with OpenGL
 *
 * Notes: Based on an SDL tutorial from SDL's web site
 *
 * ****************************************************************/
#define GLEW_STATIC
#include <GL/glew.h>
#include "SDL_OGL.h"
#include "SoundManager.h"
#include "scene.hpp"
#include "particles.h"
#include <string>

#define TEXTURE_LOAD_ERROR 0

const Uint32 fps = 60;
const Uint32 frameDuration = 1000 / fps;

double xpos = 0, ypos = 0, zpos = 0;
int oldx = 0, oldy = 0;

int music_on = 0;
bool done = false;
std::list<Object*> objects;
Object* selected;
Object* hover;

Matrix4x4 viewTransform;
Material* table;

int windowWidth = 1200;
int windowHeight = 900;

emitter* particles;

// shader texture / framebuffer related variables
// that need to be accessible during initialization and render
GLuint fbo, fbo_texture, rbo_depth;
GLuint program, attribute_v_coord, uniform_fbo_texture;
GLuint vbo_fbo_vertices;
// bump mapping shader info
GLuint bumpProgram;

// taken from A5 starter sample code, reads a text file to a string
char* readShaderToString(char *fileName)
{
  FILE *fp;
  long len;
  char *buf;

  fp = fopen(fileName,"rb");
  if ( fp == NULL )
  {
    printf("Cannot read the file %s\n", fileName);
    return NULL;
  }

  fseek(fp, 0, SEEK_END); //go to end of file

  len = ftell(fp); //get position at end (length)

  if ( len == -1 )
  {
      printf("Cannot determine size of the shader %s\n", fileName);
      return NULL;
  }

  fseek(fp, 0, SEEK_SET); //go to beginning

  buf = new char[len+1];

  fread(buf, len, 1, fp);

  buf[len] = '\0';
  fclose(fp);

  for(unsigned i=0; i<len; i++) printf("%c",buf[i]);
  printf("\n");

  return buf;
}

void update(double dt) {
  particles->update(dt);
  double spread = 0.5;
  double rx = (spread * rand() / RAND_MAX) - spread/2.0;
  double rz = (spread * rand() / RAND_MAX) - spread/2.0;
  double area = 0.5;
  double ox = (area * rand() / RAND_MAX) - area/2.0;
  double oz = (area * rand() / RAND_MAX) - area/2.0;
  std::cerr<< "rx is " << rx << std::endl;
  if (selected == NULL && hover != NULL) {
    Colour col = hover->m_material->get_colour();
    particles->emit(Point3D(xpos + ox, ypos, zpos + oz), 5 * Vector3D(sin(rx) * cos(rz), cos(rx) * cos(rz), cos(rx) * sin(rz)), 1.0, 1.0, col);
  }
  if (selected != NULL) {
    //Point3D hoverpos = selected->m_pos;
    //hoverpos[1] = 3;
    Point3D hoverpos = Point3D(xpos, ypos + 3 + 0.5, zpos);
    selected->seek(hoverpos, 10 * dt);
    //if ((selected->m_pos[1] < 3 && selected->m_vel[1] < 4.0) || selected->m_vel[1] < -1.0) {
      //selected->m_vel[1] -= 2 * GRAVITY * dt; // fall upwards twice as fast as downwards
    //}
  }
  for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
    Object* obj = *I;
    obj->move(dt, objects);
  }
}

int prepareShaders() {
  // texture for render to texture
  // Reference: http://en.wikibooks.org/wiki/OpenGL_Programming/Post-Processing
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &fbo_texture);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
  // depth buffer
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  /* Framebuffer to link everything together */
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
  GLenum status;
  if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "glCheckFramebufferStatus: error %p", status);
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);



  // create shader objects (and get their ID)
  GLuint vertShader, fragShader;
  vertShader = glCreateShader(GL_VERTEX_SHADER);
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  // load the source and pass it to the shader obj
  const char* vertShaderSource = readShaderToString("shader.vert");
  const char* fragShaderSource = readShaderToString("shader.frag");
  std::cerr << "read source to string" << std::endl;
  glShaderSource(vertShader, 1, &vertShaderSource, NULL);
  glShaderSource(fragShader, 1, &fragShaderSource, NULL);
  std::cerr << "Shaders loaded" << std::endl;
  // compile the shaders
  GLint vertCompiled, fragCompiled;
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &vertCompiled);
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragCompiled);
  if (!vertCompiled || !fragCompiled)
  {
    std::cerr << "Failed to compile a shader." << std::endl;
    return -1;
  }
  // link the shaders
  program = glCreateProgram();
  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    std::cerr << "Failed to link the shader program" << std::endl;
    return -1;
  }

  char * attribute_name = "v_coord";
  attribute_v_coord = glGetAttribLocation(program, attribute_name);
  if (attribute_v_coord == -1) {
    fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
    return 0;
  }
  char * uniform_name = "fbo_texture";
  uniform_fbo_texture = glGetUniformLocation(program, uniform_name);
  if (uniform_fbo_texture == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }

  GLfloat fbo_vertices[] = {
    -1, -1,
     1, -1,
    -1,  1,
     1,  1,
  };
  glGenBuffers(1, &vbo_fbo_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // create shader objects (and get their ID)
  GLuint vertShader2, fragShader2;
  vertShader2 = glCreateShader(GL_VERTEX_SHADER);
  fragShader2 = glCreateShader(GL_FRAGMENT_SHADER);
  // load the source and pass it to the shader obj
  const char* vertShaderSource2 = readShaderToString("bump.vert");
  const char* fragShaderSource2 = readShaderToString("bump.frag");
  std::cerr << "read source to string" << std::endl;
  glShaderSource(vertShader2, 1, &vertShaderSource2, NULL);
  glShaderSource(fragShader2, 1, &fragShaderSource2, NULL);
  std::cerr << "Second Shader loaded" << std::endl;
  // compile the shaders
  GLint vertCompiled2, fragCompiled2;
  glCompileShader(vertShader2);
  glGetShaderiv(vertShader2, GL_COMPILE_STATUS, &vertCompiled2);
  glCompileShader(fragShader2);
  glGetShaderiv(fragShader2, GL_COMPILE_STATUS, &fragCompiled2);
  if (!vertCompiled2 || !fragCompiled2)
  {
    std::cerr << "Failed to compile 2nd shader." << std::endl;
    return -1;
  }
  // link the shaders
  bumpProgram = glCreateProgram();
  glAttachShader(bumpProgram, vertShader2);
  glAttachShader(bumpProgram, fragShader2);
  glLinkProgram(bumpProgram);
  GLint linked2;
  glGetProgramiv(bumpProgram, GL_LINK_STATUS, &linked2);
  if (!linked2)
  {
    std::cerr << "Failed to link the shader program" << std::endl;
    return -1;
  }
  return 0;
}


void DrawGLScene(){
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity(); // Reset the view

    glMultMatrixd(viewTransform.transpose().begin());

    // draw floor opaque without depth info.
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    table->apply_gl(false, 1.0);
    glBegin(GL_QUADS);
      glNormal3f(0.0f,1.0f,0.0f);
      glVertex3d(-100, 0, 100);
      glVertex3d(100, 0, 100);
      glVertex3d(100, 0, -100);
      glVertex3d(-100, 0, -100);
    glEnd();
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // draw reflected scene (about y = 0)
    // should reflect lights as well?
    glCullFace(GL_FRONT); // my reflection causes clockwise to become counterclockwise
    Matrix4x4 reflection = Matrix4x4();
    //reflection[1][1] = -1.0;
    //glMultMatrixd(reflection.transpose().begin());
    glPushMatrix();
    glScaled(1.0, -1.0, 1.0);
    //glDisable(GL_DEPTH_TEST);
    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      obj->render();
    }
    //glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK); // the particles seem to always face the camera
    particles->render();
    glPopMatrix();
    // draw transparent floor with transparency on top of the reflection
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    table->apply_gl(false, 0.6);
    glBegin(GL_QUADS);
      glNormal3f(0.0f,1.0f,0.0f);
      glVertex3d(-100, 0, 100);
      glVertex3d(100, 0, 100);
      glVertex3d(100, 0, -100);
      glVertex3d(-100, 0, -100);
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // draw normal scene
    glCullFace(GL_BACK);
    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      obj->render();
    }
    particles->render();

    //Post Processing
    // done drawing the scene to the buffer.
    // draw the texture to the screen using the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, fbo_texture);
    glUniform1i(uniform_fbo_texture, /*GL_TEXTURE*/0);
    glEnableVertexAttribArray(attribute_v_coord);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
    glVertexAttribPointer(
      attribute_v_coord,  // attribute
      2,                  // number of elements per vertex, here (x,y)
      GL_FLOAT,           // the type of each element
      GL_FALSE,           // take our values as-is
      0,                  // no extra data between each position
      0                   // offset of first element
    );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attribute_v_coord);

    SDL_GL_SwapBuffers(); // Swap the buffers
}


void handleKey(SDL_KeyboardEvent key) {
  switch(key.keysym.sym) {
  case SDLK_m:
    if(key.state == SDL_PRESSED) {
	SM.PlaySound(0);
    } else {
	SM.StopSound(0);
    }// if
    break;
  case SDLK_n:
    if(key.state == SDL_PRESSED) {
	SM.PlaySound(1);
    } else {
	SM.StopSound(1);
    }// if
    break;
  case SDLK_b:
    if(key.state == SDL_PRESSED) {
	SM.PlaySound(2);
    } else {
	SM.StopSound(2);
    }// if
    break;
  case SDLK_q:
    done = true;
    break;
  case SDLK_t:
    if(key.state == SDL_PRESSED) {
       if (music_on == 0) {
          SM.PlayMusic(0);
          music_on = 1;
       } else {
	   SM.StopMusic(0);
	   music_on = 0;
       }
    }// if
    break;
  }// switch

}

void handleMouseMotion(SDL_MouseMotionEvent motion){
    // TODO: determine this info based on the view's matrix transformation and the opengl viewport size
    // Picking (see what the cursor is pointing at)
    double fov = 40.0;
    //glTranslated(-10.0, 5.0, -50.0);
    Matrix4x4 inverseViewTransform = viewTransform.invert();
    Point3D eye = inverseViewTransform * Point3D(0.0, 0.0, 0.0);
    Vector3D up = Vector3D(0.0, 1.0, 0.0);
    //Vector3D view = Point3D(0.0, 0.0, 0.0) - eye;
    Vector3D view = inverseViewTransform * Vector3D(0.0, 0.0, -1.0);


    double z = 1; // the depth of the projection plane, doesn't really matter
    // w and h are the size of the 3d viewing plane, width and height are the 2d screen size
    double h = z * 2 * tan((fov/2) * M_PI / 180); // assume fov is vertical fov
    double w = h * (windowWidth / (double) windowHeight);
    Vector3D camX = (view.cross(up));
    Vector3D camZ = view;
    Vector3D camY = (camX.cross(camZ));
    camX.normalize();
    camY.normalize();
    camZ.normalize();

    Point3D worldPixel = eye + ((w * ((motion.x/ (double) windowWidth) - 0.5)) * camX)
                             + ((h * (0.5 - (motion.y/ (double) windowHeight))) * camY) // invert y to not be upside down
                             + ((z) * camZ);
    Vector3D ray = worldPixel - eye;

    RayHit* hit = NULL;
    Object* closestObj = NULL;
    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      if (obj->dynamic) {
        RayHit* objhit = obj->raycast(eye, ray);
        if (objhit != NULL && (hit == NULL || hit->dist > objhit->dist)) {
            if (hit != NULL) {
              free(hit);
            }
            hit = objhit;
            closestObj = obj;
        }
      }
    }
    if (hit != NULL) {
      free(hit);
    }
    // closest obj and hvoer are both pointers to an object or NULL
    if (closestObj != hover) {
      if (hover != NULL) {
        // moused off of something
        //SM.StopSound(0);
      }
      if (closestObj != NULL) {
        // moused onto something
        //SM.PlaySound(0);
      }
      hover = closestObj;
    }

  // intersect the ray and the y=1 plane
  std::cerr << "The eye is " << eye << " and the ray is " << ray << std::endl;
  ypos = 1.0;
  double t = (ypos - eye[1]) / ray[1];
  xpos = eye[0] + t * ray[0];
  zpos = eye[2] + t * ray[2];

  //xpos += (float)(motion.x-oldx)/10.0f;
  //ypos += -(float)(motion.y-oldy)/10.0f;
  oldx = motion.x; oldy = motion.y;
}

void handleMouseButtons(SDL_MouseButtonEvent button){

    if(button.type == SDL_MOUSEBUTTONDOWN){
	switch (button.button){
	case 1/*LMB*/:
	  if (selected == NULL) {
      selected = hover;
      if (selected != NULL) {
        // do something
        SM.PlaySound(0);
      }
	  } else {
      SM.StopSound(0);
      selected = NULL;
    }
	  break;
	case 2/*MMB*/:SM.PlaySound(1);break;
	case 3/*RMB*/: SM.PlaySound(2);break;
	}// switch
    } else { /* SDL_MOUSEBUTTONUP */
	switch (button.button){
	case 1:
	  //SM.StopSound(0);
	  break;
	case 2: SM.StopSound(1);break;
	case 3:SM.StopSound(2);break;
	} // switch
    }// if ... else

}

// The following function to laod a file using libpng is taken from the following tutorial
// http://en.wikibooks.org/wiki/OpenGL_Programming/Intermediate/Textures#A_simple_libpng_example
GLuint loadTexture(const char* filename, bool alpha)
{
 //header for testing if it is a png
 png_byte header[8];

 //open file as binary
 FILE *fp = fopen(filename, "rb");
 if (!fp) {
   return TEXTURE_LOAD_ERROR;
 }

 //read the header
 fread(header, 1, 8, fp);

 //test if png
 int is_png = !png_sig_cmp(header, 0, 8);
 if (!is_png) {
   fclose(fp);
   return TEXTURE_LOAD_ERROR;
 }

 //create png struct
 png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
     NULL, NULL);
 if (!png_ptr) {
   fclose(fp);
   return (TEXTURE_LOAD_ERROR);
 }

 //create png info struct
 png_infop info_ptr = png_create_info_struct(png_ptr);
 if (!info_ptr) {
   png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
   fclose(fp);
   return (TEXTURE_LOAD_ERROR);
 }

 //create png info struct
 png_infop end_info = png_create_info_struct(png_ptr);
 if (!end_info) {
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
   fclose(fp);
   return (TEXTURE_LOAD_ERROR);
 }

 //png error stuff, not sure libpng man suggests this.
 if (setjmp(png_jmpbuf(png_ptr))) {
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   fclose(fp);
   return (TEXTURE_LOAD_ERROR);
 }

 //init png reading
 png_init_io(png_ptr, fp);

 //let libpng know you already read the first 8 bytes
 png_set_sig_bytes(png_ptr, 8);

 // read all the info up to the image data
 png_read_info(png_ptr, info_ptr);

 //variables to pass to get info
 int bit_depth, color_type;
 png_uint_32 twidth, theight;

 // get info about png
 png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
     NULL, NULL, NULL);

 // Update the png info struct.
 png_read_update_info(png_ptr, info_ptr);

 // Row size in bytes.
 int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

 // Allocate the image_data as a big block, to be given to opengl
 png_byte *image_data = new png_byte[rowbytes * theight];
 if (!image_data) {
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   fclose(fp);
   return TEXTURE_LOAD_ERROR;
 }

 //row_pointers is for pointing to image_data for reading the png with libpng
 png_bytep *row_pointers = new png_bytep[theight];
 if (!row_pointers) {
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   delete[] image_data;
   fclose(fp);
   return TEXTURE_LOAD_ERROR;
 }
 // set the individual row_pointers to point at the correct offsets of image_data
 for (int i = 0; i < theight; ++i)
   row_pointers[theight - 1 - i] = image_data + i * rowbytes;

 //read the png into image_data through row_pointers
 png_read_image(png_ptr, row_pointers);

 //Now generate the OpenGL texture object
 GLuint texture;
 glGenTextures(1, &texture);
 glBindTexture(GL_TEXTURE_2D, texture);
 if (alpha)
  glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, twidth, theight, 0,
     GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
 else
  glTexImage2D(GL_TEXTURE_2D,0, GL_RGB, twidth, theight, 0,
     GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) image_data);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

 //clean up memory and close stuff
 png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
 delete[] image_data;
 delete[] row_pointers;
 fclose(fp);

 return texture;
}

int main(int argc, char *argv[])
{
  Uint8* keys;

  SM.LoadSound("card.wav");
  SM.LoadSound("OBS.wav");
  SM.LoadSound("ghost.wav");
  SM.LoadMusic("UNREAL.S3M");


  // Create a new OpenGL window with the title "Cone3D Basecode" at
  // 640x480x32, fullscreen and check for errors along the way
  if(CreateGLWindow("SDL & OpenGL", windowWidth, windowHeight, 16, 0) == 0){
      printf("Could not initalize OpenGL :(\n\n");
      KillGLWindow();
      return 0;
  }

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Glew init failed" << std::endl;
    exit(1); // or handle the error in a nicer way
  } else if (!GLEW_VERSION_2_1) { // check that the machine supports the 2.1 API.
    std::cerr << "Version 2.1 not supported" << std::endl;
    exit(1); // or handle the error in a nicer way
  }
  // Hide the mouse cursor
  //SDL_ShowCursor(0);

  // This is the main loop for the entire program and it will run until done==TRUE
  Uint32 frametime;

  //objects
  double width = 20;
  double border = 1.0;
  double piece_size = 0.9; // 1.4
  double piece_depth = 0.5; // 0.8
  double depth = 1.0;

  std::cerr << "Loading textures" << std::endl;
  GLuint boardID = loadTexture("board.png", false);
  GLuint bumpMap = loadTexture("bumpmap.png", false);
  GLuint particleID = loadTexture("circle_soft.png", true);
  std::cerr << "Textures loaded" << std::endl;

  particles = new emitter(particleID);

  int error = prepareShaders();
  if (error != 0) return error;

  // initialize shaders
  std::cerr << "Loading shaders" << std::endl;

  // materials
  table = new PhongMaterial(Colour(0.8, 0.8, 0.8), Colour(1.0, 1.0, 1.0), 1);
  Material* wood = new PhongMaterial(Colour(1.0, 0.6, 0.2), Colour(1.0, 1.0, 1.0), 1);
  Material* black = new PhongMaterial(Colour(0.0, 0.0, 0.0), Colour(1.0, 1.0, 1.0), 128);
  Material* white = new PhongMaterial(Colour(1.0, 1.0, 1.0), Colour(1.0, 1.0, 1.0), 128);
  Material* bowl = new PhongMaterial(Colour(0.3, 0.3, 0.3), Colour(1.0, 1.0, 1.0), 3);

  // primitives
  NonhierBox* boardprimitive = new NonhierBox(Point3D(0,0,0), Vector3D(width, depth, width), boardID, bumpProgram, bumpMap);
  Sphere* pieceprimitive = new Sphere();

  // objects
  objects.push_back(new Object("board", Point3D(0, 0, 0), Vector3D(1,1,1), wood, boardprimitive, false));

  double boxW = 6.0;
  double boxH = 8.0;
  double boxBorder = 0.4;
  double boxDepth = 3.5;
  NonhierBox* boxWide = new NonhierBox(Point3D(0,0,0), Vector3D(boxW, boxDepth, boxBorder));
  NonhierBox* boxTall = new NonhierBox(Point3D(0,0,0), Vector3D(boxBorder, boxDepth, boxH - 2 * boxBorder));
  NonhierBox* boxBottom = new NonhierBox(Point3D(0,0,0), Vector3D(boxW - 2 * boxBorder, boxBorder, boxH - 2 * boxBorder));
  double offsetX = 1.0;
  double offsetY = 4.0;

  // bowl on the right
  double boxZ = 0.0;
  objects.push_back(new Object("Rbowl1", Point3D(width + offsetX, boxZ, offsetY), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Rbowl2", Point3D(width + offsetX, boxZ, offsetY + boxH - boxBorder), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Rbowl3", Point3D(width + offsetX, boxZ, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Rbowl4", Point3D(width + offsetX + boxW - boxBorder, boxZ, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Rbowl5", Point3D(width + offsetX + boxBorder, boxZ, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxBottom, false));
  // bowl on the left
  objects.push_back(new Object("Lbowl1", Point3D(- boxW - offsetX, boxZ, offsetY), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Lbowl2", Point3D(- boxW - offsetX, boxZ, offsetY + boxH - boxBorder), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Lbowl3", Point3D(- boxW - offsetX, boxZ, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Lbowl4", Point3D(- boxW - offsetX + boxW - boxBorder, boxZ, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Lbowl5", Point3D(- boxW - offsetX + boxBorder, boxZ, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxBottom, false));


  //NonhierBox* pieceprimitive = new NonhierBox(Point3D(0,0,0), Vector3D(piece_size, 0.4, piece_size));
  for (int n = 0; n < 100; n++) {
    //double x = border + ((width - 2 * border - piece_size) * rand() / RAND_MAX);
    //double y = border + ((width - 2 * border - piece_size) * rand() / RAND_MAX);
    Material* piece_mat;
    double border = boxBorder + piece_size / 2.0;
    double areax = width + offsetX + border;
    double areay = offsetY + border;
    if (n%2 == 0) {
      piece_mat = (black);
    } else {
      piece_mat = (white);
      areax = -offsetX - boxW;
    }
    double x = areax + ((boxW - 2 * border) * rand() / RAND_MAX);
    double y = areay + ((boxH - 2 * border) * rand() / RAND_MAX);
    objects.push_back(new Object("piece", Point3D(x, boxBorder + (piece_depth) * (1 + n / 2), y), Vector3D(piece_size/2.0, piece_depth/2.0, piece_size/2.0), piece_mat, pieceprimitive, true));
  }

  viewTransform = translation(Point3D(-10.0, 9.0, -40.0)) * rotation('x', 65.0);
  //viewTransform = translation(Point3D(-10.0, 5.0, -40.0)) * rotation('x', 40.0);

  while(!done){
    frametime = SDL_GetTicks();

    // fixed update
    double dt = frameDuration/1000.0;
    update(dt);

    // Draw the scene
    DrawGLScene();

    // And poll for events
    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {
      switch (event.type) {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
	  handleKey(event.key);
	  break;

      case SDL_MOUSEMOTION:
      SDL_PeepEvents (&event,9,SDL_GETEVENT,SDL_MOUSEMOTION);
      handleMouseMotion(event.motion);
	  break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	  handleMouseButtons(event.button);
	  break;
        case SDL_QUIT:
          // then we're done and we'll end this program
          done=true;
          break;
        default:
          break;
      }// switch

    }// while

    // Get the state of the keyboard keys
    keys = SDL_GetKeyState(NULL);

    // and check if ESCAPE has been pressed. If so then quit
    if (keys[SDLK_ESCAPE])
        done = 1;
    else if (SDL_GetTicks() - frametime < frameDuration)
      SDL_Delay (frameDuration - (SDL_GetTicks () - frametime));
  }// while done

  // Kill the GL & SDL screens

  KillGLWindow();
  // And quit
  glDeleteProgram(program);
  return 0;
}
