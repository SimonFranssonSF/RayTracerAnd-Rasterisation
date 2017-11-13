// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include <SDL.h>
#include <iostream>
#include "glm/glm.hpp"
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;
vector<vec3> interpolationTop(SCREEN_WIDTH);
vector<vec3> interpolationBottom(SCREEN_WIDTH);
vector<vec3> interpolationY(SCREEN_HEIGHT);

//variables for the starfield
vector<vec3> stars (1000);
int t;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();
void Interpolate(vec3 a, vec3 b, vector<vec3>& result);

void Draw2();
void Update();

// --------------------------------------------------------
// FUNCTION DEFINITIONS

int main( int argc, char* argv[] )
{
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    
    for(int i=0; i<(int)stars.size();i++){
        
        vec3 star(0,0,0);
        
        //random float between -1 and 1
        star[0] = (float)(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/2)))-1;
        star[1] = (float)(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/2)))-1;
        //random float between 0 and 1
        star[2] = (float)(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
        stars[i] = star;
        
    }
    
    t = SDL_GetTicks();
    while( NoQuitMessageSDL() )
    {
        
        //CHANGE BETWEEN DRAW 1 AND DRAW 2
        Draw();
        Update();
        //Draw2();
        
    }
    SDL_SaveBMP( screen, "screenshot.bmp" );
    return 0;
}




void Draw()
{
    
    vec3 topLeft(1,0,0); // red
    vec3 topRight(0,0,1); // blue
    vec3 bottomLeft(1,1,0); // yellow
    vec3 bottomRight(0,1,0); // green
    
    Interpolate( topLeft, topRight, interpolationTop );
    Interpolate( bottomLeft, bottomRight, interpolationBottom );
    
    //interpolate the top and bottom of the image
    for( int x=0; x<SCREEN_WIDTH; ++x )
    {
        vec3 color(interpolationTop[x]);
        PutPixelSDL( screen, x, 0, color );
        vec3 color2(interpolationBottom[x]);
        PutPixelSDL( screen, x, SCREEN_HEIGHT-1, color2 );
    }
    
    //interpolate between the top and the bottom, the y values
    for( int x=0; x<SCREEN_WIDTH; ++x )
    {
        Interpolate(interpolationTop[x], interpolationBottom[x], interpolationY);
        for( int y=0; y<SCREEN_HEIGHT; ++y )
        {
            vec3 color(interpolationY[y]);
            PutPixelSDL( screen, x, y, color );
        }
    }
    
    if( SDL_MUSTLOCK(screen) )
        SDL_UnlockSurface(screen);
    
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void Draw2()
{
    
    SDL_FillRect( screen, 0, 0 );
    if( SDL_MUSTLOCK(screen) )
        SDL_LockSurface(screen);
    for( size_t s=0; s<stars.size(); ++s )
    {
        int u = (SCREEN_HEIGHT/2) * (stars[s][0]/stars[s][2]) + (SCREEN_WIDTH/2);
        int v = (SCREEN_HEIGHT/2) * (stars[s][1]/stars[s][2]) + (SCREEN_HEIGHT/2);
        vec3 color = 0.2f * vec3(1,1,1) / (stars[s].z*stars[s].z);
        PutPixelSDL( screen, u, v, color );
        
    }
    if( SDL_MUSTLOCK(screen) )
        SDL_UnlockSurface(screen);
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void Interpolate(vec3 a, vec3 b, vector<vec3>& result)
{
    int index = 0;
    
    float stepX = (b[0] - a[0])/(result.size()-1);
    float stepY = (b[1] - a[1])/(result.size()-1);
    float stepZ = (b[2] - a[2])/(result.size()-1);
    for( int i=0; i<result.size(); ++i)
    {
        result[i][0] = a[0]+stepX*i;
        result[i][1] = a[1]+stepY*i;
        result[i][2] = a[2]+stepZ*i;
        
    }
}

void Update(){
    
    int t2 = SDL_GetTicks();
    float dt = float(t2-t);
    t = t2;
    
    
    
    
    for( int s=0; s<stars.size(); ++s )
    {
        
        
        // Add code for update of stars
        stars[s][2] = stars[s][2]-0.0001*dt;
        
        if( stars[s].z <= 0 )
            stars[s].z += 1;
        if( stars[s].z > 1 )
            stars[s].z -= 1;
    }
    
    
    
}
