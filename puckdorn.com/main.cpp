#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

using namespace std;

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Duck and game settings
const int CROSSHAIR_SIZE = 15;
const int DUCK_WIDTH = 60;
const int DUCK_HEIGHT = 60;
const float BASE_SPEED = 3.0f;
const float FALL_SPEED = 5.0f;
const int GRASS_HEIGHT = 150;
const int DUCK_SPAWN_Y = SCREEN_HEIGHT - GRASS_HEIGHT - DUCK_HEIGHT;
int bulletsLeft = 3; // Limit the player to 3 bullets per level
int score = 0; // Player's score
int level = 1; // Game level
float speedMultiplier = 1.0f; // Speed multiplier for higher levels
const int SCORE_THRESHOLD = 5; // Ducks needed to level up

// Animation frames
const int DUCK_FRAMES = 11;
SDL_Texture* duckFrames[DUCK_FRAMES + 1];
int currentFrame = 1;
int frameDelay = 100;
Uint32 lastFrameTime = 0;

// Duck states
enum DuckState {
    FLYING_HORIZONTAL,
    FLYING_UPWARDS,
    POP,
    FALLING
};

// Menu options
enum MenuOption {
    START_GAME,
    QUIT_GAME,
    TOTAL_OPTIONS
};

DuckState duckState = FLYING_HORIZONTAL;
Uint32 popStartTime = 0;
int selectedOption = 0; // For the menu

// Function to draw menu text (placeholder since we aren't using SDL_ttf)
void drawMenuText(SDL_Renderer* renderer, const string& text, int x, int y, bool isSelected) {
    SDL_SetRenderDrawColor(renderer, isSelected ? 255 : 200, isSelected ? 255 : 200, 0, 255);
    SDL_Rect rect = {x, y, static_cast<int>(text.size() * 15), 30};
    SDL_RenderFillRect(renderer, &rect);
}

// Function to show the main menu
bool showMainMenu(SDL_Renderer* renderer) {
    SDL_Event e;
    bool inMenu = true;

    while (inMenu) {
        // Handle menu input
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) return false;

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        selectedOption = (selectedOption - 1 + TOTAL_OPTIONS) % TOTAL_OPTIONS;
                        break;
                    case SDLK_DOWN:
                        selectedOption = (selectedOption + 1) % TOTAL_OPTIONS;
                        break;
                    case SDLK_RETURN:
                    case SDLK_SPACE:
                        if (selectedOption == START_GAME) return true;
                        if (selectedOption == QUIT_GAME) return false;
                }
            }
        }

        // Render the menu
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawMenuText(renderer, "Start Game", 300, 200, selectedOption == START_GAME);
        drawMenuText(renderer, "Quit Game", 300, 250, selectedOption == QUIT_GAME);

        SDL_RenderPresent(renderer);
    }

    return false;
}

// Function to load textures
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (newTexture == nullptr) {
        cerr << "Failed to load texture: " << path << " SDL_image Error: " << IMG_GetError() << "\n";
    }
    return newTexture;
}

// Load duck animation frames
void loadDuckFrames(SDL_Renderer* renderer) {
    for (int i = 1; i <= DUCK_FRAMES; ++i) {
        string path = "C:/Users/bebiu/Desktop/puckdorn.com/frame" + to_string(i) + ".png";
        duckFrames[i] = loadTexture(path, renderer);
    }
}

// Free loaded textures
void freeDuckFrames() {
    for (int i = 1; i <= DUCK_FRAMES; ++i) {
        SDL_DestroyTexture(duckFrames[i]);
    }
}
void spawnDuck(SDL_Rect& duckRect) {
    duckRect.x = rand() % (SCREEN_WIDTH - DUCK_WIDTH);
    duckRect.y = DUCK_SPAWN_Y;
}

// Update duck animation frame
void updateDuckFrame() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastFrameTime + frameDelay) {
        switch (duckState) {
            case FLYING_HORIZONTAL:
                currentFrame = (currentFrame < 3) ? currentFrame + 1 : 1;
                break;
            case FLYING_UPWARDS:
                currentFrame = (currentFrame < 6) ? currentFrame + 1 : 4;
                break;
            case POP:
                currentFrame = 7;
                break;
            case FALLING:
                currentFrame = (currentFrame < 11) ? currentFrame + 1 : 8;
                break;
        }
        lastFrameTime = currentTime;
    }
}

// Function to draw the crosshair
void drawCrosshair(SDL_Renderer* renderer, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color for the crosshair
    // Draw horizontal line
    SDL_RenderDrawLine(renderer, x - CROSSHAIR_SIZE, y, x + CROSSHAIR_SIZE, y);
    // Draw vertical line
    SDL_RenderDrawLine(renderer, x, y - CROSSHAIR_SIZE, x, y + CROSSHAIR_SIZE);
}
// Function to draw text on the screen (for score, level, and bullets)
void drawText(SDL_Renderer* renderer, const string& text, int x, int y, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, static_cast<int>(text.size() * 10), 30};
    SDL_RenderFillRect(renderer, &rect);
}

// Function to draw the game statistics
void drawStats(SDL_Renderer* renderer) {
    drawText(renderer, "Score: " + to_string(score), 10, 10, {255, 255, 0, 255});
    drawText(renderer, "Level: " + to_string(level), 10, 40, {255, 255, 0, 255});
    drawText(renderer, "Bullets: " + to_string(bulletsLeft), 10, 70, {255, 255, 0, 255});
}


int main(int argc, char* args[]) {
    // Initialize SDL and SDL_image
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Duck Hunt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_ShowCursor(SDL_DISABLE);

    loadDuckFrames(renderer);
    SDL_Texture* backgroundTexture = loadTexture("C:/Users/bebiu/Desktop/puckdorn.com/background.png", renderer);
    SDL_Texture* foregroundTexture = loadTexture("C:/Users/bebiu/Desktop/puckdorn.com/foreground.png", renderer);

    // Show the main menu
    if (!showMainMenu(renderer)) {
        freeDuckFrames();
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(foregroundTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    // Initialize duck properties
    SDL_Rect duckRect = {0, DUCK_SPAWN_Y, DUCK_WIDTH, DUCK_HEIGHT};
    spawnDuck(duckRect);
    float duckSpeedX = BASE_SPEED;
    float duckSpeedY = BASE_SPEED;
    int crosshairX = SCREEN_WIDTH / 2;
    int crosshairY = SCREEN_HEIGHT / 2;
    bool quit = false;
    SDL_Event e;

// Game loop
while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) quit = true;
        else if (e.type == SDL_MOUSEMOTION) {
            crosshairX = e.motion.x;
            crosshairY = e.motion.y;
        } else if (e.type == SDL_MOUSEBUTTONDOWN && duckState != FALLING) {
            if (crosshairX >= duckRect.x && crosshairX <= duckRect.x + DUCK_WIDTH &&
                crosshairY >= duckRect.y && crosshairY <= duckRect.y + DUCK_HEIGHT) {
                duckState = POP;
                currentFrame = 7;
                popStartTime = SDL_GetTicks();
            }
        }
    }

    updateDuckFrame();

    if (duckState == POP) {
        if (SDL_GetTicks() - popStartTime > 500) {
            duckState = FALLING;
            currentFrame = 8;
        }
    } else if (duckState == FALLING) {
        duckRect.y += FALL_SPEED;
        if (duckRect.y > SCREEN_HEIGHT) {
            duckState = FLYING_HORIZONTAL;
            spawnDuck(duckRect);
            currentFrame = 1;
        }
    } else {
        duckRect.x += duckSpeedX;
        duckRect.y -= duckSpeedY;

        if (duckRect.x > SCREEN_WIDTH - DUCK_WIDTH || duckRect.x < 0) duckSpeedX = -duckSpeedX;
        if (duckRect.y < 0 || duckRect.y > DUCK_SPAWN_Y) duckSpeedY = -duckSpeedY;
    }

    SDL_RendererFlip flip = (duckSpeedX < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // Render everything
    SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    SDL_RenderCopyEx(renderer, duckFrames[currentFrame], nullptr, &duckRect, 0, nullptr, flip);
    SDL_RenderCopy(renderer, foregroundTexture, nullptr, nullptr);

    // Draw the crosshair at the current mouse position
    drawCrosshair(renderer, crosshairX, crosshairY);

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}


    freeDuckFrames();
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(foregroundTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
