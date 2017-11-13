#include <iostream>
#include "glm/glm.hpp"
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <math.h>

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// sturctures

struct Intersection
{
    vec3 position;
    float distance;
    int triangleIndex;
};

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

vector<Triangle> triangles;
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 400;
SDL_Surface* screen;
int t;
vec3 start(0, 0, -4);
float yaw = 0;
mat3 R;
int skip = -1;
vec3 lightPos( 0, -0.5, -0.7 );
vec3 lightColor = 14.f * vec3( 1, 1, 1 );
vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );

Intersection intersect;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

vec3 DirectLight( const Intersection& i );

bool ClosestIntersection(vec3 start, vec3 dir, int skip, const vector<Triangle>& triangles, Intersection& closestIntersection);


int main( int argc, char* argv[] )
{
    //cout << M_PI;
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    t = SDL_GetTicks();	// Set start value for timer.
    LoadTestModel(triangles);
    float m = std::numeric_limits<float>::max();
    
    while( NoQuitMessageSDL() )
    {
        Update();
        Draw();
    }
    
    SDL_SaveBMP( screen, "screenshot.bmp" );
    return 0;
}

void Update()
{
    // Compute frame time:
    int t2 = SDL_GetTicks();
    float dt = float(t2-t);
    t = t2;
    cout << "Render time: " << dt << " ms." << endl;
    
    Uint8* keystate = SDL_GetKeyState( 0 );
    if( keystate[SDLK_UP] )
    {
        // Move camera forward
        start.z += 0.1;
    }
    if( keystate[SDLK_DOWN] )
    {
        // Move camera backward
        start.z -= 0.1;
    }
    if( keystate[SDLK_LEFT] )
    {
        // Move camera to the left
        //start = R*start;
        yaw -= 0.1;
    }
    if( keystate[SDLK_RIGHT] )
    {
        // Move camera to the right
        //start = glm::transpose(R)*start;
        yaw += 0.1;
    }
    R = mat3(glm::cos(yaw), 0, glm::sin(yaw), 0, 1, 0, -glm::sin(yaw), 0, glm::cos(yaw));
    
    if( keystate[SDLK_w] )
        lightPos.z += 0.1;
    
    if( keystate[SDLK_s] )
        lightPos.z -= 0.1;
    
    if( keystate[SDLK_a] )
        lightPos.x -= 0.1;
    
    if( keystate[SDLK_d] )
        lightPos.x += 0.1;
    
    if( keystate[SDLK_q] )
        lightPos.y -= 0.1;
    
    if( keystate[SDLK_e] )
        lightPos.y += 0.1;
}

void Draw()
{
    if( SDL_MUSTLOCK(screen) )
        SDL_LockSurface(screen);
    
    float focalLength = SCREEN_HEIGHT*1.5;
    vec3 black(0,0,0);
    for(int i = 0; i < SCREEN_HEIGHT; i++){
        for(int j = 0; j < SCREEN_WIDTH; j++){
            vec3 d(j-SCREEN_WIDTH/2, i-SCREEN_HEIGHT/2, focalLength);
            //vec3 start(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, -4);
            
            
            if(ClosestIntersection(start, d, skip, triangles, intersect)){
                //cout << triangles[intersect.triangleIndex].color[0];
                //
                vec3 color = triangles[intersect.triangleIndex].color;
                vec3 light = DirectLight(intersect);
                PutPixelSDL(screen, j, i, color * (light+indirectLight));
            }else{
                PutPixelSDL(screen, j, i, black);
            }
        }
    }
    
    
    if( SDL_MUSTLOCK(screen) )
        SDL_UnlockSurface(screen);
    
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}


bool ClosestIntersection(
                         vec3 start,
                         vec3 dir,
                         int skip,
                         const vector<Triangle>& triangles,
                         Intersection& closestIntersection) {
    bool returnBool = false;
    intersect.position;
    intersect.distance = 24234274238;
    intersect.triangleIndex = 0;
    
    for(int i=0; i<triangles.size(); i++){
        using glm::vec3;
        using glm::mat3;
        
        vec3 v0 = triangles[i].v0*R;
        vec3 v1 = triangles[i].v1*R;
        vec3 v2 = triangles[i].v2*R;
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        vec3 b = start - v0;
        mat3 A( -dir, e1, e2 );
        vec3 x = glm::inverse( A ) * b;
        //cout << "x: " << x[0] << "y: " << x[1]  << "z: " << x[2];
        //distance calculation
        //vec3 t = (x-start)/dir;
        vec3 intersectionPoint = v0 + x.y*e1 + x.z*e2;
        if(x.z >= 0 && x.y >= 0 && x.z+x.y <= 1 && x.x >= 0  && skip != i){
            
            //object intersect
            if(intersect.distance > x.x){
                intersect.position = intersectionPoint;
                intersect.distance = x.x;
                intersect.triangleIndex = i;
                returnBool = true;
            }
        }
    }
    skip = -1;
    return returnBool;
}


vec3 DirectLight( const Intersection& i ){
    Intersection temp = i;
    vec3 normal = glm::normalize(triangles[temp.triangleIndex].normal);
    vec3 directionToLight = lightPos - temp.position;
    float distance = glm::distance(lightPos, temp.position);
    float dotProduct = glm::dot(normal, glm::normalize(directionToLight));
    vec3 D = (lightColor*max(dotProduct, 0.0f)) / (4 * 3.14f * distance*distance);
    
    
    if(ClosestIntersection(lightPos, -directionToLight, i.triangleIndex, triangles, intersect)){
        if(glm::distance(intersect.position, lightPos) < distance){
            //cout << intersect.position[0] << intersect.position[1] << intersect.position[2];
            //cout << i.position[0] << i.position[1] << i.position[2];
            vec3 black(0,0,0);
            return black;
        }
    }
    return D;
    
    
}




