#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int CROSSHAIR_SIZE = 15;  // Half-length of each crosshair line
const int DUCK_WIDTH = 60;      // Duck width (adjust based on image)
const int DUCK_HEIGHT = 60;     // Duck height (adjust based on image)
const float BASE_SPEED = 3.0f;  // Base speed of the duck
const float FALL_SPEED = 5.0f;  // Speed at which the duck falls when hit
const int GROUND_LEVEL_Y = SCREEN_HEIGHT - 150; // "Ground level" where the duck appears to emerge from

enum DuckState {
    FLYING,
    FALLING
};

// Function to load an image as a texture
SDL_Texture* loadTexture(const std::string &path, SDL_Renderer *renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (newTexture == nullptr) {
        std::cerr << "Failed to load texture " << path << " SDL_image Error: " << IMG_GetError() << "\n";
    }
    return newTexture;
}

// Function to set the target for the duck to move through the center first, then to a corner
void setDuckTargets(SDL_Rect &duckRect, float &duckSpeedX, float &duckSpeedY, SDL_Point &centerTarget, SDL_Point &finalTarget, float speedMultiplier, bool &spawnedFromSky) {
    if (rand() % 2 == 0) { // Randomly decide spawn location
        // Spawn from the sky (top of the screen)
        duckRect.x = rand() % (SCREEN_WIDTH - DUCK_WIDTH); // Random horizontal position
        duckRect.y = 0 - DUCK_HEIGHT; // Just above the screen
        centerTarget = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        finalTarget = (rand() % 2 == 0) ? SDL_Point{SCREEN_WIDTH, SCREEN_HEIGHT} : SDL_Point{0, SCREEN_HEIGHT}; // Bottom corners
        spawnedFromSky = true;
    } else {
        // Spawn from below the "ground level" (bottom part of the screen)
        duckRect.x = rand() % (SCREEN_WIDTH - DUCK_WIDTH); // Random horizontal position
        duckRect.y = GROUND_LEVEL_Y + DUCK_HEIGHT; // Just below the ground level
        centerTarget = { SCREEN_WIDTH / 2, GROUND_LEVEL_Y - 100 }; // Fly up towards a point above ground level
        finalTarget = (rand() % 2 == 0) ? SDL_Point{SCREEN_WIDTH, GROUND_LEVEL_Y - 100} : SDL_Point{0, GROUND_LEVEL_Y - 100}; // Move left or right above ground level
        spawnedFromSky = false;
    }

    // Set initial speed towards the center, scaled by the speed multiplier
    int dx = centerTarget.x - duckRect.x;
    int dy = centerTarget.y - duckRect.y;
    float distance = sqrt(dx * dx + dy * dy);
    duckSpeedX = speedMultiplier * BASE_SPEED * (dx / distance);
    duckSpeedY = speedMultiplier * BASE_SPEED * (dy / distance);
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    // Initialize SDL_image for PNG/JPG loading
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Duck Hunt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer) {
        std::cerr << "Window or renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // Hide the default system cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Load textures for duck and background
    SDL_Texture* duckTextureSky = loadTexture("C:/Users/bebiu/Desktop/Prj/duck_in_sky.png", renderer);
    SDL_Texture* duckTextureGround = loadTexture("C:/Users/bebiu/Desktop/Prj/duck_in_ground.png", renderer);
    SDL_Texture* duckFallTexture = loadTexture("C:/Users/bebiu/Desktop/Prj/duck_fall.png", renderer); // Falling duck texture
    SDL_Texture* backgroundTexture = loadTexture("C:/Users/bebiu/Desktop/Prj/background.png", renderer); // Full background texture

    if (!duckTextureSky || !duckTextureGround || !duckFallTexture || !backgroundTexture) {
        std::cerr << "Error: Some textures not loaded. Check the file paths and formats.\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Set up the duck position and movement
    SDL_Rect duckRect = {0, 0, DUCK_WIDTH, DUCK_HEIGHT};
    float duckSpeedX, duckSpeedY;
    SDL_Point centerTarget, finalTarget;
    bool headingToCenter = true; // Indicates if the duck is currently heading to the center target
    bool spawnedFromSky;         // True if the duck spawned from the sky, false if from ground
    DuckState duckState = FLYING; // Duck starts in flying state
    srand(static_cast<unsigned>(time(0)));

    float currentSpeedMultiplier = 1.0f;
    setDuckTargets(duckRect, duckSpeedX, duckSpeedY, centerTarget, finalTarget, currentSpeedMultiplier, spawnedFromSky); // Initialize the duck's target

    int score = 0;
    int crosshairX = SCREEN_WIDTH / 2;
    int crosshairY = SCREEN_HEIGHT / 2;
    bool quit = false;
    SDL_Event e;

    std::cout << "Duck Hunt Game starting..." << std::endl;

    // Define the full-screen rectangle for the background
    SDL_Rect backgroundRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // Main game loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEMOTION) {
                // Update crosshair position based on mouse
                crosshairX = e.motion.x;
                crosshairY = e.motion.y;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && duckState == FLYING) {
                // Check if crosshair overlaps with the duck on click
                if (crosshairX >= duckRect.x && crosshairX <= duckRect.x + DUCK_WIDTH &&
                    crosshairY >= duckRect.y && crosshairY <= duckRect.y + DUCK_HEIGHT) {
                    score++;
                    std::cout << "Hit! Score: " << score << "\n";

                    // Set duck to falling state
                    duckState = FALLING;
                    duckSpeedX = 0;         // Stop horizontal movement
                    duckSpeedY = FALL_SPEED; // Set vertical fall speed
                }
            }
        }

        // Update duck position based on its state
        if (duckState == FLYING) {
            if (headingToCenter && abs(duckRect.x - centerTarget.x) < 5 && abs(duckRect.y - centerTarget.y) < 5) {
                headingToCenter = false;

                // Calculate new speed towards the final target
                int dx = finalTarget.x - duckRect.x;
                int dy = finalTarget.y - duckRect.y;
                float distance = sqrt(dx * dx + dy * dy);
                duckSpeedX = currentSpeedMultiplier * BASE_SPEED * (dx / distance);
                duckSpeedY = currentSpeedMultiplier * BASE_SPEED * (dy / distance);
            }

            // Update duck position
            duckRect.x += static_cast<int>(duckSpeedX);
            duckRect.y += static_cast<int>(duckSpeedY);

            // Check if the duck has gone off the screen
            if (duckRect.y > SCREEN_HEIGHT || duckRect.y < -DUCK_HEIGHT || duckRect.x > SCREEN_WIDTH || duckRect.x < -DUCK_WIDTH) {
                setDuckTargets(duckRect, duckSpeedX, duckSpeedY, centerTarget, finalTarget, currentSpeedMultiplier, spawnedFromSky);
                headingToCenter = true;
            }
        } else if (duckState == FALLING) {
            // Update duck position to fall
            duckRect.y += static_cast<int>(duckSpeedY);

            // Respawn duck if it falls off the screen
            if (duckRect.y > SCREEN_HEIGHT) {
                duckState = FLYING; // Reset to flying state
                setDuckTargets(duckRect, duckSpeedX, duckSpeedY, centerTarget, finalTarget, currentSpeedMultiplier, spawnedFromSky);
                headingToCenter = true;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255); // Sky blue background for "Duck Hunt" feel
        SDL_RenderClear(renderer);

        // Render the background
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &backgroundRect);

        // Choose the appropriate texture based on duck state and position
        SDL_Texture* currentDuckTexture;
        if (duckState == FALLING) {
            currentDuckTexture = duckFallTexture;
        } else {
            currentDuckTexture = (duckRect.y < SCREEN_HEIGHT / 2) ? duckTextureSky : duckTextureGround;
        }

        // Render the duck
        SDL_RenderCopy(renderer, currentDuckTexture, nullptr, &duckRect);

        // Draw crosshair at current mouse position
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, crosshairX - CROSSHAIR_SIZE, crosshairY, crosshairX + CROSSHAIR_SIZE, crosshairY);
        SDL_RenderDrawLine(renderer, crosshairX, crosshairY - CROSSHAIR_SIZE, crosshairX, crosshairY + CROSSHAIR_SIZE);

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay to control frame rate
        SDL_Delay(16); // ~60 FPS
    }

    std::cout << "Game Over! Final Score: " << score << "\n";

    // Clean up resources
    SDL_DestroyTexture(duckTextureSky);
    SDL_DestroyTexture(duckTextureGround);
    SDL_DestroyTexture(duckFallTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
