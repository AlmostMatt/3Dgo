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

#include "SDL_OGL.h"
#include "SoundManager.h"
#include "scene.hpp"
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

void update(double dt) {
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

void DrawGLScene(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity(); // Reset the view

    glMultMatrixd(viewTransform.transpose().begin());

    glPushMatrix();
    // draw floor opaque without depth info.
    glDepthMask(GL_FALSE);
    table->apply_gl(false, 1.0);
    glBegin(GL_QUADS);
      glNormal3f(0.0f,1.0f,0.0f);
      glVertex3d(-100, 0, 100);
      glVertex3d(100, 0, 100);
      glVertex3d(100, 0, -100);
      glVertex3d(-100, 0, -100);
    glEnd();
    glDepthMask(GL_TRUE);
    // draw reflected scene (about y = 0)
    // should reflect lights as well?
    Matrix4x4 reflection = Matrix4x4();
    reflection[1][1] = -1.0;
    glMultMatrixd(reflection.transpose().begin());
    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      obj->render();
    }
    glPopMatrix();
    // draw transparent floor with transparency on top of the reflection
    glEnable(GL_BLEND);
    //glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    table->apply_gl(false, 0.6);
    glBegin(GL_QUADS);
      glNormal3f(0.0f,1.0f,0.0f);
      glVertex3d(-100, 0, 100);
      glVertex3d(100, 0, 100);
      glVertex3d(100, 0, -100);
      glVertex3d(-100, 0, -100);
    glEnd();
    //glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // draw normal scene
    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      obj->render();
    }

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
    double width = 640;
    double height = 480;
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
    double w = h * (width / height);
    Vector3D camX = (view.cross(up));
    Vector3D camZ = view;
    Vector3D camY = (camX.cross(camZ));
    camX.normalize();
    camY.normalize();
    camZ.normalize();

    Point3D worldPixel = eye + ((w * ((motion.x/width) - 0.5)) * camX)
                             + ((h * (0.5 - (motion.y/height))) * camY) // invert y to not be upside down
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
        //SM.PlaySound(2);
      }
	  } else {
      //SM.StopSound(2);
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
GLuint loadTexture(const char* filename, int &width, int &height)
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

 //update width and height based on png info
 width = twidth;
 height = theight;

 // Update the png info struct.
 png_read_update_info(png_ptr, info_ptr);

 // Row size in bytes.
 int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

 // Allocate the image_data as a big block, to be given to opengl
 png_byte *image_data = new png_byte[rowbytes * height];
 if (!image_data) {
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   fclose(fp);
   return TEXTURE_LOAD_ERROR;
 }

 //row_pointers is for pointing to image_data for reading the png with libpng
 png_bytep *row_pointers = new png_bytep[height];
 if (!row_pointers) {
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   delete[] image_data;
   fclose(fp);
   return TEXTURE_LOAD_ERROR;
 }
 // set the individual row_pointers to point at the correct offsets of image_data
 for (int i = 0; i < height; ++i)
   row_pointers[height - 1 - i] = image_data + i * rowbytes;

 //read the png into image_data through row_pointers
 png_read_image(png_ptr, row_pointers);

 //Now generate the OpenGL texture object
 GLuint texture;
 glGenTextures(1, &texture);
 glBindTexture(GL_TEXTURE_2D, texture);
 glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, width, height, 0,
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
  if(CreateGLWindow("SDL & OpenGL", 640, 480, 16, 0) == 0){
      printf("Could not initalize OpenGL :(\n\n");
      KillGLWindow();
      return 0;
  }


  // Hide the mouse cursor
  //SDL_ShowCursor(0);

  // This is the main loop for the entire program and it will run until done==TRUE
  //int changeit = 1;
  int reset = 1;
  Uint32 frametime;

  //objects
  double width = 20;
  double border = 1.0;
  double piece_size = 0.9; // 1.4
  double piece_depth = 0.5; // 0.8
  double depth = 1.0;
  //SceneNode* root = new SceneNode("root", false);

  // textures

  //int imgWidth = 512, imgHeight = 512;
  //unsigned* imageData = malloc(imgWidth * imgHeight * 3);

  int imgWidth;
  int imgHeight;
  std::cerr << "Loading texture" << std::endl;
  GLuint boardID = loadTexture("board.png", imgWidth, imgHeight);
  /*
  char* filename = "board.bmp";
  FILE* File = fopen(filename, "rb");
  if (File) {
    png_byte header[8];
    fread(header, 1, 8 ,File);
    int is_png =
    //fclose(File);
    //AUX_RGBImageRec* imageData = auxDIBImageLoad(filename);
    // create texture
    glBindTexture(GL_TEXTURE_2D, BOARD);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageData->sizeX, imageData->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->data);
  } else {
    std::cerr << "Failed to load image " << filename << std::endl;
  }
  free(imageData);
  */

  // materials
  table = new PhongMaterial(Colour(0.8, 0.8, 0.8), Colour(1.0, 1.0, 1.0), 1);
  Material* wood = new PhongMaterial(Colour(1.0, 0.6, 0.2), Colour(1.0, 1.0, 1.0), 1);
  Material* black = new PhongMaterial(Colour(0.0, 0.0, 0.0), Colour(1.0, 1.0, 1.0), 20);
  Material* white = new PhongMaterial(Colour(1.0, 1.0, 1.0), Colour(1.0, 1.0, 1.0), 20);
  Material* bowl = new PhongMaterial(Colour(0.3, 0.3, 0.3), Colour(1.0, 1.0, 1.0), 3);

  // primitives
  NonhierBox* boardprimitive = new NonhierBox(Point3D(0,0,0), Vector3D(width, depth, width), boardID);
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
  objects.push_back(new Object("Rbowl1", Point3D(width + offsetX, 0, offsetY), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Rbowl2", Point3D(width + offsetX, 0, offsetY + boxH - boxBorder), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Rbowl3", Point3D(width + offsetX, 0, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Rbowl4", Point3D(width + offsetX + boxW - boxBorder, 0, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Rbowl5", Point3D(width + offsetX + boxBorder, 0, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxBottom, false));
  // bowl on the left
  objects.push_back(new Object("Lbowl1", Point3D(- boxW - offsetX, 0, offsetY), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Lbowl2", Point3D(- boxW - offsetX, 0, offsetY + boxH - boxBorder), Vector3D(1,1,1), bowl, boxWide, false));
  objects.push_back(new Object("Lbowl3", Point3D(- boxW - offsetX, 0, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Lbowl4", Point3D(- boxW - offsetX + boxW - boxBorder, 0, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxTall, false));
  objects.push_back(new Object("Lbowl5", Point3D(- boxW - offsetX + boxBorder, 0, offsetY + boxBorder), Vector3D(1,1,1), bowl, boxBottom, false));

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
  //root->getobjectlist(objects, Matrix4x4(), Matrix4x4(), Matrix4x4());

  viewTransform = translation(Point3D(-10.0, 9.0, -40.0)) * rotation('x', 65.0);
  //viewTransform = translation(Point3D(-10.0, 5.0, -40.0)) * rotation('x', 40.0);

  while(!done){
    frametime = SDL_GetTicks();

    // fixed update
    double dt = frameDuration/1000.0;
    update(dt);

    // Draw the scene
    //if (changeit == 1)
    //   {
    DrawGLScene();
    //   changeit = 0;
    //   }

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
      if (reset == 1)
      {
        xpos = 0;
        ypos = 0;
        reset = 0;
      }
      //changeit = 1;
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
  return 0;
}
