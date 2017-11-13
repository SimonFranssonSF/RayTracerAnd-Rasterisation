#include <iostream>
#include "glm/glm.hpp"
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
float f = 490;
vec3 cameraPos(0,0,-3.001);
mat3 R;
float yaw = 0; // Yaw angle controlling camera rotation around y-axis
vec3 currentColor;
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
vec3 lightPos(0,-0.5,-0.7);
vec3 lightPower = 14.0f*vec3( 1, 1, 1 );
vec3 indirectLightPowerPerArea = 0.5f*vec3( 1, 1, 1 );
vec3 currentNormal;
vec3 currentReflectance;

struct Pixel
{
    int x;
    int y;
    float zinv;
    //vec3 illumination;
    vec3 pos3d;
    float zindex;
};

struct Vertex
{
    vec3 position;
    //vec3 normal;
    //vec3 reflectance;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();
//void VertexShader(const vec3& v, ivec2& p);
void VertexShader( const Vertex& v, Pixel& p );
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL( SDL_Surface* surface, Pixel a, Pixel b, vec3 color );
void DrawPolygonEdges( const vector<Vertex>& vertices );
//void ComputePolygonRows(const vector<ivec2>& vertexPixels,vector<ivec2>& leftPixels,vector<ivec2>& rightPixels );
void ComputePolygonRows(const vector<Pixel>& vertexPixels,vector<Pixel>& leftPixels,vector<Pixel>& rightPixels );
//void DrawRows( const vector<ivec2>& leftPixels,const vector<ivec2>& rightPixels );
void DrawRows(const vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels);
void DrawPolygon( const vector<Vertex>& vertices );
void PixelShader( const Pixel& p );

int main( int argc, char* argv[] )
{
    /*
     vector<ivec2> vertexPixels(3);
     vertexPixels[0] = ivec2(10, 5);
     vertexPixels[1] = ivec2( 5,10);
     vertexPixels[2] = ivec2(15,15);
     vector<ivec2> leftPixels;
     vector<ivec2> rightPixels;
     ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
     for( int row=0; row<leftPixels.size(); ++row ){
     cout << "Start: ("<< leftPixels[row].x << ","<< leftPixels[row].y << "). "<< "End: ("<< rightPixels[row].x << ","<< rightPixels[row].y << "). " << endl;
     }
     */
    
    LoadTestModel( triangles );
    screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
    t = SDL_GetTicks();	// Set start value for timer.
    
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
    //cout << "Render time: " << dt << " ms." << endl;
    
    Uint8* keystate = SDL_GetKeyState(0);
    
    if( keystate[SDLK_UP] )
        cameraPos.z += 0.1;
    
    if( keystate[SDLK_DOWN] )
        cameraPos.z -= 0.1;
    
    if( keystate[SDLK_RIGHT] )
        yaw -= 0.01;
    
    if( keystate[SDLK_LEFT] )
        yaw += 0.01;
    
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
    
    R = mat3(glm::cos(yaw), 0, glm::sin(yaw), 0, 1, 0, -glm::sin(yaw), 0, glm::cos(yaw));
}

void Draw()
{
    SDL_FillRect( screen, 0, 0 );
    
    if( SDL_MUSTLOCK(screen) )
        SDL_LockSurface(screen);
    
    for( int y=0; y<SCREEN_HEIGHT; ++y )
        for( int x=0; x<SCREEN_WIDTH; ++x )
            depthBuffer[y][x] = 0;
    
    for( int i=0; i<triangles.size(); ++i)
    {
        currentColor = triangles[i].color;
        vector<Vertex> vertices(3);
        
        vertices[0].position = triangles[i].v0;
        //vertices[0].normal = triangles[i].normal;
        //vertices[0].reflectance = triangles[i].color;
        
        vertices[1].position = triangles[i].v1;
        //vertices[1].normal = triangles[i].normal;
        //vertices[1].reflectance = triangles[i].color;
        
        vertices[2].position = triangles[i].v2;
        //vertices[2].normal = triangles[i].normal;
        //vertices[2].reflectance = triangles[i].color;
        
        //DrawPolygonEdges(vertices);
        // Add drawing
        currentNormal = triangles[i].normal;
        currentReflectance = triangles[i].color;
        DrawPolygon(vertices);
        
        
        /*
         for(int v=0; v<3; ++v)
         {
         ivec2 projPos;
         VertexShader( vertices[v], projPos );
         //cout << projPos.x << " y: "<< projPos.y << "\n";
         vec3 color(1,1,1);
         PutPixelSDL( screen, projPos.x, projPos.y, color );
         }
         */
    }
    
    if ( SDL_MUSTLOCK(screen) )
        SDL_UnlockSurface(screen);
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void VertexShader(const Vertex& v, Pixel& p)
{
    vec3 vPrim = (v.position - cameraPos)*R;
    p.x = (f*(vPrim.x/vPrim.z)) + (SCREEN_WIDTH/2);
    p.y = (f*(vPrim.y/vPrim.z)) + (SCREEN_HEIGHT/2);
    p.zinv = 1.0f / vPrim.z;
    p.pos3d = v.position;
    
    /*
     float radius = glm::distance(v.position, lightPos);
     vec3 r = glm::normalize(lightPos - v.position);
     float dotProduct = glm::dot(r, v.normal);
     
     if(dotProduct > 0){
     p.illumination = v.reflectance*((lightPower * dotProduct / (4*3.14f*radius*radius)) + indirectLightPowerPerArea);
     }else{
     p.illumination = v.reflectance* (indirectLightPowerPerArea);
     }
     */
}

/*
 void Interpolate( ivec2 a, ivec2 b, vector<ivec2>& result )
 {
	int N = result.size();
	glm::vec2 step = glm::vec2(b-a) / float(max(N-1,1));
	glm::vec2 current( a );
	for( int i=0; i<N; ++i )
	{
 result[i] = current;
 vec3 color(1,1,1);
 current += step;
	}
 }
 */

void Interpolate( Pixel a, Pixel b, vector<Pixel>& result )
{
    int N = result.size();
    
    vec3 step(b.x-a.x, b.y-a.y, b.zinv - a.zinv);
    step = step/float(max(N-1,1));
    vec3 current(a.x, a.y, a.zinv);
    
    //vec3 illuminationStep = (b.illumination-a.illumination)/float(max(N-1,1));
    //vec3 currentIllumination(a.illumination);
    
    vec3 pos3dstep = (b.pos3d*b.zinv-a.pos3d*a.zinv)/float(max(N-1,1));
    //pos3dstep = pos3dstep/pos3dstep.z;
    vec3 currentPos3d(a.pos3d*a.zinv);
    //currentPos3d = currentPos3d/currentPos3d.z;
    
    Pixel ip;
    
    for( int i=0; i<N; ++i )
    {
        ip.x = current[0];
        ip.y = current[1];
        ip.zinv = current[2];
        //ip.illumination = currentIllumination;
        ip.zindex = currentPos3d.z;
        //currentPos3d = currentPos3d/currentPos3d.z;
        ip.pos3d = currentPos3d/current[2];
        result[i] = ip;
        current += step;
        //currentIllumination += illuminationStep;
        currentPos3d += pos3dstep;
        
    }
}


void DrawLineSDL( SDL_Surface* surface, Pixel a, Pixel b, vec3 color ){
    //ivec2 delta = glm::abs( a - b );
    //int pixels = glm::max( delta.x, delta.y ) + 1;
    
    float deltaX = b.x - a.x;
    float deltaY = b.y - a.y;
    int pixels = glm::max( deltaX, deltaY ) + 1;
    
    vector<Pixel> result(pixels);
    Interpolate( a, b, result );
    for(int i = 0 ; i < result.size(); i++){
        PutPixelSDL( screen, result[i].x, result[i].y, color );
    }
}


void DrawPolygonEdges( const vector<Vertex>& vertices )
{
    int V = vertices.size();
    // Transform each vertex from 3D world position to 2D image position:
    vector<Pixel> projectedVertices( V );
    for( int i=0; i<V; ++i )
    {
        VertexShader( vertices[i], projectedVertices[i] );
    }
    // Loop over all vertices and draw the edge from it to the next vertex:
    for( int i=0; i<V; ++i )
    {
        int j = (i+1)%V;
        //The next vertex
        vec3 color( 1, 1, 1 );
        DrawLineSDL( screen, projectedVertices[i], projectedVertices[j], color );
    }
}

void ComputePolygonRows(const vector<Pixel>& vertexPixels,vector<Pixel>& leftPixels,vector<Pixel>& rightPixels )
{
    int maxY = glm::max(vertexPixels[2].y, glm::max(vertexPixels[0].y, vertexPixels[1].y));
    int minY = glm::min(vertexPixels[2].y, glm::min(vertexPixels[0].y, vertexPixels[1].y));
    int rows = abs(maxY - minY) + 1;
    
    leftPixels.resize(rows);
    rightPixels.resize(rows);
    
    for( int i=0; i<rows; ++i )
    {
        leftPixels[i].x = +numeric_limits<int>::max();
        rightPixels[i].x = -numeric_limits<int>::max();
        leftPixels[i].y = i + minY;
        rightPixels[i].y = i + minY;
        //cout << rightPixels[i].y << " ";
    }
    
    for(int i=0; i<vertexPixels.size(); ++i){
        vector<Pixel> result;
        
        if(i == 2){
            //ivec2 delta = glm::abs( vertexPixels[2] - vertexPixels[0] );
            float deltaX = vertexPixels[2].x - vertexPixels[0].x;
            float deltaY = vertexPixels[2].y - vertexPixels[0].y;
            //int pixels = glm::max( delta.x, delta.y ) + 1;
            int pixels = glm::max( deltaX, deltaY ) + 1;
            result.resize(rows);
            Interpolate(vertexPixels[2], vertexPixels[0], result);
        }else{
            //ivec2 delta = glm::abs( vertexPixels[i] - vertexPixels[i+1] );
            float deltaX = vertexPixels[i].x - vertexPixels[i+1].x;
            float deltaY = vertexPixels[i].y - vertexPixels[i+1].y;
            //int pixels = glm::max( delta.x, delta.y ) + 1;
            int pixels = glm::max( deltaX, deltaY ) + 1;
            result.resize(rows);
            Interpolate(vertexPixels[i], vertexPixels[i+1] , result);
        }
        for(int j =0; j < result.size(); ++j){
            //cout << result[j].zinv << "\n";
            if(result[j].y-minY < 0 || result[j].y-minY > maxY ){
                continue;
            }
            if(result[j].x < leftPixels[result[j].y-minY].x){
                leftPixels[result[j].y-minY] = result[j];
                
            }
            if(result[j].x > rightPixels[result[j].y-minY].x){
                rightPixels[result[j].y-minY] = result[j];
            }
        }
    }
    
    
}

void DrawRows( const vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels ){
    
    for(int i=0; i<leftPixels.size(); i++){
        vector<Pixel> result(rightPixels[i].x-leftPixels[i].x+1);
        Interpolate(leftPixels[i], rightPixels[i], result);
        
        for(int j = 0 ; j < result.size(); j++){
            PixelShader(result[j]);
            
        }
    }
}

void PixelShader( const Pixel& p ){
    if(p.zinv > depthBuffer[p.x][p.y]){
        vec3 finalColor;
        //vec3 temp = p.pos3d;
        //temp = temp*p.zindex;
        
        float radius = glm::distance(p.pos3d, lightPos);
        vec3 r = glm::normalize(lightPos - p.pos3d);
        float dotProduct = glm::dot(r, currentNormal);
        
        if(dotProduct > 0){
            finalColor = currentReflectance*((lightPower * dotProduct / (4*3.14f*radius*radius)) + indirectLightPowerPerArea);
        }else{
            finalColor = currentReflectance* (indirectLightPowerPerArea);
        }
        
        PutPixelSDL( screen, p.x, p.y, finalColor );
        depthBuffer[p.x][p.y] = p.zinv;
    }
}

void DrawPolygon( const vector<Vertex>& vertices )
{
    int V = vertices.size();
    vector<Pixel> vertexPixels( V );
    for( int i=0; i<V; ++i ){
        //cout << vertexPixels[i][0] << " " << vertexPixels[i][1] << "\n";
        VertexShader( vertices[i], vertexPixels[i] );
    }
    
    vector<Pixel> leftPixels;
    vector<Pixel> rightPixels;
    ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
    DrawRows( leftPixels, rightPixels );
}

/*
 * DEPTHBUFFER FUNKAR, FORTSÄTT MED ILLUMINATION.
 * DRAW, DRAWPOLYGON OCH VERTEXSHADER SKA TA VERTEX IST FÖR VEC3
 */
