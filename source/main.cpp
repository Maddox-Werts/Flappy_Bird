// For Windows
#define SDL_MAIN_HANDLED

// Includes
// --SYSTEMS
#include <iostream>
#include <time.h>
// --SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// Constants
#define W_WIDTH 640
#define W_HEIGHT 640 

#define SCN_PIPEWIDTH 75
#define SCN_PIPESPACE 85
#define SCN_PIPESPEED 0.04f;

#define PHY_GRAVITY 0.00006f

// Variables
SDL_Window* window;
SDL_Renderer* renderer;
bool running;

int SCN_POINTS = 0;
int SCN_FLOORHEIGHT;

float groundOffset = 0;

Mix_Chunk* samples[2];

SDL_Texture* groundTex;

// Structures
typedef struct Vector2{
    float x,y;

    Vector2(): x(0), y(0){

    }
    Vector2(float x, float y): x(x), y(y){

    }

    Vector2 operator+(Vector2 b){
        return Vector2(x + b.x, y + b.y);
    }
    void operator+=(Vector2 b){
        x += b.x; y += b.y;
    }
    Vector2 operator*(float b){
        return Vector2(x * b, y * b);
    }
    void operator*=(float b){
        x *= b; y *= b;
    }
} Vector2;

// Functions
int init(){
    // Start SDL2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        throw(SDL_GetError());
        return -1;
    }

    // Create Window
    window = SDL_CreateWindow(
        "Flappy Bird",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        W_WIDTH, W_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if(window == NULL){
        throw(SDL_GetError());
        return -1;
    }

    // Create Renderer
    renderer = SDL_CreateRenderer(window, 1, SDL_RENDERER_ACCELERATED);

    if(renderer == NULL){
        throw(SDL_GetError());
        return -1;
    }

    // Start SDL2_Image
    if(!IMG_Init(IMG_INIT_PNG)){
        throw(IMG_GetError());
    }

    // Start audio
    // --VARIABLES
    const Uint8 s_format = AUDIO_S16SYS;
    const int s_buffers = 4096;
    const int s_channels = 2;
    const int s_rate = 22050;

    // --SYSTEMS
    if(Mix_OpenAudio(s_rate, s_format, s_channels, s_buffers) == 0){
        printf("Audio systems are good.\n");
    }
    else{
        throw(Mix_GetError());
    }

    // Loading ground texture
    SDL_Surface* gsurf;

    if(!(gsurf = IMG_Load("res/img/ground.png"))){
        throw(IMG_GetError());
    }

    if(!(groundTex = SDL_CreateTextureFromSurface(renderer, gsurf))){
        throw(SDL_GetError());
    }

    SDL_FreeSurface(gsurf);

    // We're running!
    running = true;
}
int init_scn(){
    // Floor height
    SCN_FLOORHEIGHT = W_HEIGHT / 10;
}
int update(){
    // Hold events
    SDL_Event ev;

    // Poll them
    while(SDL_PollEvent(&ev)){
        switch(ev.type){
        case SDL_QUIT:
            running = false;
            break;
        }
    }

    return 0;
}
int clear(){
    SDL_SetRenderDrawColor(renderer, 0,0,0, 1);
    SDL_RenderClear(renderer);
    return 0;
}
int display(){
    SDL_RenderPresent(renderer);
    return 0;
}
int draw_floor(){
    // Making the color
    SDL_SetRenderDrawColor(renderer, 50,255,50, 1);

    groundOffset -= SCN_PIPESPEED;

    // Making a rectangle
    SDL_Rect rect, viewport;

    // Ready?
    viewport.x = 0; viewport.y = 0;
    viewport.w = 336; viewport.h = 114;

    // Making transform
    rect.x = 0; rect.y = W_HEIGHT - SCN_FLOORHEIGHT;
    rect.w = W_WIDTH; rect.h = SCN_FLOORHEIGHT;

    // Rendering
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderCopy(renderer, groundTex, &viewport, &rect);

    // Quit
    return 0;
}
int playSound(const char* name, int channel){
    Mix_FreeChunk(samples[channel]);

    // Finding file
    samples[channel] = Mix_LoadWAV(name);

    if(samples[channel] == NULL){
        throw("Failed to find Audio File ( wing.wav ).\n");
        return -1;
    }

    // Playing the sound
    Mix_PlayChannel(-1, samples[channel], 0);

    // Good!
    return 1;
}
float lerp(float a, float b, float t){
    return a + t * (b - a);
}

// Classes
class Pipe{
private:
    SDL_Texture* texture;
    bool passed;

    // Functions
    void _wrap(){
        // Random
        srand((unsigned)time(NULL));

        // Passed!
        passed = false;

        // Random shit
        int randomPos = rand() % 8 + 1;

        position.x = W_WIDTH; position.y = randomPos * (W_WIDTH / 10);
    }

public:
    // Variables
    Vector2 position;

    // Constructors
    Pipe(){
        // Setting random stuff
        _wrap();

        SDL_Delay(1000);

        // Loading texture
        SDL_Surface* surf;

        if(!(surf = IMG_Load("res/img/pipe.png"))){
            throw(IMG_GetError());
        }

        if(!(texture = SDL_CreateTextureFromSurface(renderer, surf))){
            throw(SDL_GetError());
        }

        SDL_FreeSurface(surf);
    }

    // Functions
    void Pass(){
        // Passed already?
        if(passed) {return;}

        passed = true;
        playSound("res/snd/point.wav", 1);

        SCN_POINTS += 1;
    }
    void Update(){
        // Moving to the left
        position.x -= SCN_PIPESPEED;

        // Ready to wrap?
        if(position.x + SCN_PIPEWIDTH < 0){
            _wrap();
        }
    }
    void Draw(){
        // Set draw color
        SDL_SetRenderDrawColor(renderer, 0,0,0, 1);

        // Make rect variables
        SDL_Rect rect;

        // Rendering parts
        // --TOP
        // ----FLIP
        SDL_Point centre = {0,0};

        rect.x = position.x; rect.y = 0;
        rect.w = SCN_PIPEWIDTH; rect.h = position.y - SCN_PIPESPACE;

        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, 0, &centre, SDL_FLIP_VERTICAL);

        // --BOTTOM
        rect.x = position.x; rect.y = position.y + SCN_PIPESPACE;
        rect.w = SCN_PIPEWIDTH; rect.h = W_HEIGHT - (position.y - SCN_PIPESPACE);

        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }
};
class Bird{
private:
    // Variables
    SDL_Texture* texture;
    Vector2 velocity;
    bool pressingUp, canFlap;

    // Functions
    void _flap(){
        velocity.y = -0.1f;

        // Play Sound
        playSound("res/snd/wing.wav", 0);

        return;
    }
    void _checkFlap(){
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_SPACE]){
            pressingUp = true;
            
            if(canFlap){
                canFlap = false;
                
                _flap();
            }
        }
        else{
            pressingUp = false;

            if(!canFlap){
                canFlap = true;
            }
        }
    }
    bool _floor_death(){
        if(position.y + scale.y >= W_HEIGHT - SCN_FLOORHEIGHT){
            is_Dead = true;
        }
    }

public:
    // Variables
    Vector2 position, scale;
    bool is_Dead;
    float rotation;

    // Constructors
    Bird(): scale(50,30), velocity(0,0){
        // Setting position
        position = Vector2(
            (W_WIDTH / 4),
            (W_HEIGHT / 2) 
        );

        // Loading the texture
        SDL_Surface* surf;

        if(!(surf = IMG_Load("res/img/bird.png"))){
            throw(IMG_GetError());
        }

        if(!(texture = SDL_CreateTextureFromSurface(renderer, surf))){
            throw(SDL_GetError());
        }

        SDL_FreeSurface(surf);
    }

    // Functions
    void Update(){
        // Gravity
        velocity.y += PHY_GRAVITY;

        // Applying position
        position += velocity;
        
        // Set rotation
        rotation = lerp(rotation, velocity.y * 150.0f, 0.125f);

        // Can we Flap?
        _checkFlap();

        // Dead?
        _floor_death();
    }
    void Fall(){
        velocity.y += PHY_GRAVITY;
        position += velocity;
    }
    void Draw(){
        // Set draw color
        SDL_SetRenderDrawColor(renderer, 0,0,0, 1);

        // Flipping BS
        SDL_Point centre = {64,32};

        // Making a rectangle
        SDL_Rect rect;
        
        // Setting a position and shit
        rect.x = position.x; rect.y = position.y;
        rect.w = scale.x; rect.h = scale.x;

        // Drawing the player
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, rotation, &centre, SDL_FLIP_NONE);
    }
    void Collide(Pipe& pipe){
        // Checking..
        if(position.x + scale.x > pipe.position.x
        && position.x < pipe.position.x + SCN_PIPEWIDTH){
            // Uh oh.. Big moment..
            // Are we safe?
            if(position.y < pipe.position.y - SCN_PIPESPACE
            || position.y + scale.y > pipe.position.y + SCN_PIPESPACE){
                is_Dead = true;
            }
            else{
                pipe.Pass();
            }
        }
    }
};

// Entry Point
int main(int argc, char* argv[]){
    // Start stuff
    init();
    init_scn();

    // Making objects
    Bird* player = new Bird();
    Pipe pipe_A;
    Pipe pipe_B;

    // Setting stuff
    pipe_B.position.x = W_WIDTH * 1.5f;

    // Game loop
    while(running){
        // Update window
        update();

        // Clear display
        clear();

        // Game code..
        // --Dead or not?
        if(!player->is_Dead){
            player->Update();
            pipe_A.Update();
            pipe_B.Update();

            // --Collisions
            player->Collide(pipe_A);
            player->Collide(pipe_B);

            // --Are we dead?
            if(player->is_Dead) {playSound("res/snd/hit.wav", 1); SDL_Delay(500); playSound("res/snd/die.wav", 1);}
        }
        else{
            player->Fall();

            if(player->position.y + player->scale.y > W_HEIGHT - SCN_FLOORHEIGHT){
                SDL_Delay(1000);
                break;
            }
        }

        // Render code..
        player->Draw();
        pipe_A.Draw();
        pipe_B.Draw();
        draw_floor();

        // Show display
        display();
    }

    // Stop SDL
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    // Say points
    printf("You ended with %i points.\n", SCN_POINTS);
    SDL_Delay(3000);

    // Resume
    SDL_Quit();

    // Quit app.
    return 0;
}