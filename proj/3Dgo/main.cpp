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

const Uint32 fps = 60;
const Uint32 frameDuration = 1000 / fps;

float xpos = 0, ypos = 0;
int oldx = 0, oldy = 0;

int music_on = 0;
bool done = false;
std::list<Object*> objects;
Object* selected;
Object* hover;


void DrawGLScene(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity(); // Reset the view

    // instead of transforming the scene as follows, do the inverse to the camera
    //root->translate(Vector3D(-width/2.0, 5.0, -50.0));
    //root->rotate('x', 40);
    glTranslated(-10.0, 5.0, -50.0);
    glRotated(40.0, 1.0, 0.0, 0.0);


    //glTranslated(0,0, -50.0);

    std::cout << "Drawing Scene\n";

    // move the scene
    //glTranslatef(xpos, ypos, -150);
    // draw s simple sphere
    //glCallList(MICKEY);

    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      obj->render();
      std::cout << "Object " << obj->m_name << " has transform:\n" << obj->transform << "\n";
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
    Point3D eye = Point3D(10.0, -5.0, 50);
    Vector3D up = Vector3D(0.0, -1.0, 0.0);
    Vector3D view = Point3D(0.0, 0.0, 0.0) - eye;

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
        SM.StopSound(0);
      }
      if (closestObj != NULL) {
        // moused onto something
        SM.PlaySound(0);
      }
      hover = closestObj;
    }

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
        SM.PlaySound(2);
      }
	  } else {
      SM.StopSound(2);
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
  double piece_size = 1.4;
  double piece_depth = 0.8;
  double depth = 1.0;
  //SceneNode* root = new SceneNode("root", false);

  Material* wood = new PhongMaterial(Colour(1.0, 0.6, 0.2), Colour(1.0, 1.0, 1.0), 1);
  Material* black = new PhongMaterial(Colour(0.0, 0.0, 0.0), Colour(1.0, 1.0, 1.0), 3);
  Material* white = new PhongMaterial(Colour(1.0, 1.0, 1.0), Colour(1.0, 1.0, 1.0), 3);

  // primitives
  NonhierBox* boardprimitive = new NonhierBox(Point3D(0,0,0), Vector3D(width, depth, width));
  Sphere* pieceprimitive = new Sphere();

  // objects
  objects.push_back(new Object("board", Point3D(0, 0, 0), Vector3D(1,1,1), wood, boardprimitive, false));

  //NonhierBox* pieceprimitive = new NonhierBox(Point3D(0,0,0), Vector3D(piece_size, 0.4, piece_size));
  for (int n = 0; n < 20; n++) {
    double x = border + ((width - 2 * border - piece_size) * rand() / RAND_MAX);
    double y = border + ((width - 2 * border - piece_size) * rand() / RAND_MAX);
    //GeometryNode* piece = new GeometryNode("piece", pieceprimitive, true);
    Material* piece_mat;
    if (n%2 == 0)
      piece_mat = (black);
    else
      piece_mat = (white);
    objects.push_back(new Object("piece", Point3D(x, x/4 + 5 + depth + piece_depth/2, y), Vector3D(piece_size/2.0, piece_depth/2.0, piece_size/2.0), piece_mat, pieceprimitive, true));
  }
  //root->getobjectlist(objects, Matrix4x4(), Matrix4x4(), Matrix4x4());

  while(!done){
    frametime = SDL_GetTicks();

    // fixed update
    double dt = frameDuration/1000.0;
    for (std::list<Object*>::const_iterator I = objects.begin(); I != objects.end(); ++I) {
      Object* obj = *I;
      obj->move(dt, objects);
      //std::cout << "Object " << obj->m_name << " has transform:\n" << obj->transform << "\n";
    }

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
/*
#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif

#include <SDL/SDL.h>

int main ( int argc, char** argv )
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(640, 480, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

    // load an image
    SDL_Surface* bmp = SDL_LoadBMP("cb.bmp");
    if (!bmp)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
    }

    // centre the bitmap on screen
    SDL_Rect dstrect;
    dstrect.x = (screen->w - bmp->w) / 2;
    dstrect.y = (screen->h - bmp->h) / 2;

    // program main loop
    bool done = false;
    while (!done)
    {
        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
                {
                    // exit if ESCAPE is pressed
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = true;
                    break;
                }
            } // end switch
        } // end of message processing

        // DRAWING STARTS HERE

        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

        // draw bitmap
        SDL_BlitSurface(bmp, 0, screen, &dstrect);

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);
    } // end main loop

    // free loaded bitmap
    SDL_FreeSurface(bmp);

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
*/
