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
int crosshairSize = 15;
int soundVolume = 50;
float duckSpeedMultiplier = 1.0f;

// Animation frames
const int DUCK_FRAMES = 11;
SDL_Texture* duckFrames[DUCK_FRAMES + 1];
int currentFrame = 1;
int frameDelay = 100;
Uint32 lastFrameTime = 0;
int bulletslelf =3;

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

// Khai báo hàm loadTexture và showMainMenu trước khi sử dụng
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer);
bool showMainMenu(SDL_Renderer* renderer);

// Hàm loadTexture để tải ảnh từ file
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (newTexture == nullptr) {
        cerr << "Failed to load texture: " << path << " SDL_image Error: " << IMG_GetError() << "\n";
    }
    return newTexture;
}
SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer);

// Hàm tải hình ảnh biểu tượng đạn
SDL_Texture* loadBulletIcon(SDL_Renderer* renderer) {
    return loadTexture("C:/Users/bebiu/Documents/GitHub/puckdorn/puckdorn.com/bullet.png", renderer);
}
void drawText(SDL_Renderer* renderer, const string& text, int x, int y, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, static_cast<int>(text.size() * 10), 30};
    SDL_RenderFillRect(renderer, &rect);
}
bool showSettingsMenu(SDL_Renderer* renderer, int& crosshairSize, int& soundVolume, float& duckSpeedMultiplier) {
    SDL_ShowCursor(SDL_ENABLE); // Hiển thị con trỏ chuột trong menu cài đặt

    // Tạo nền cho menu cài đặt
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect settingsRect = {100, 100, 600, 400};
    SDL_RenderFillRect(renderer, &settingsRect);

    // Các vùng nhấn cho các cài đặt
    SDL_Rect crosshairRect = {150, 150, 500, 50};
    SDL_Rect soundVolumeRect = {150, 250, 500, 50};
    SDL_Rect duckSpeedRect = {150, 350, 500, 50};
    SDL_Rect saveRect = {300, 450, 200, 50};

    SDL_Event e;
    bool inSettings = true;

    while (inSettings) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) return false;
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                // Điều chỉnh kích thước crosshair
                if (mouseX >= crosshairRect.x && mouseX <= crosshairRect.x + crosshairRect.w &&
                    mouseY >= crosshairRect.y && mouseY <= crosshairRect.y + crosshairRect.h) {
                    crosshairSize += 5;
                    if (crosshairSize > 50) crosshairSize = 15; // Reset nếu quá lớn
                }

                // Điều chỉnh âm lượng
                if (mouseX >= soundVolumeRect.x && mouseX <= soundVolumeRect.x + soundVolumeRect.w &&
                    mouseY >= soundVolumeRect.y && mouseY <= soundVolumeRect.y + soundVolumeRect.h) {
                    soundVolume += 10;
                    if (soundVolume > 100) soundVolume = 0; // Reset nếu quá lớn
                }

                // Điều chỉnh tốc độ vịt
                if (mouseX >= duckSpeedRect.x && mouseX <= duckSpeedRect.x + duckSpeedRect.w &&
                    mouseY >= duckSpeedRect.y && mouseY <= duckSpeedRect.y + duckSpeedRect.h) {
                    duckSpeedMultiplier += 0.5f;
                    if (duckSpeedMultiplier > 3.0f) duckSpeedMultiplier = 1.0f; // Reset nếu quá lớn
                }

                // Lưu và thoát
                if (mouseX >= saveRect.x && mouseX <= saveRect.x + saveRect.w &&
                    mouseY >= saveRect.y && mouseY <= saveRect.y + saveRect.h) {
                    inSettings = false; // Thoát menu cài đặt
                }
            }
        }

        // Vẽ giao diện menu cài đặt
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &settingsRect);

        // Hiển thị các tùy chọn cài đặt
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Crosshair size
        SDL_Rect optionRect = crosshairRect;
        SDL_RenderFillRect(renderer, &optionRect);
        drawText(renderer, "Crosshair Size: " + to_string(crosshairSize), crosshairRect.x + 10, crosshairRect.y + 10, {0, 0, 0, 255});

        // Sound volume
        optionRect = soundVolumeRect;
        SDL_RenderFillRect(renderer, &optionRect);
        drawText(renderer, "Sound Volume: " + to_string(soundVolume), soundVolumeRect.x + 10, soundVolumeRect.y + 10, {0, 0, 0, 255});

        // Duck speed
        optionRect = duckSpeedRect;
        SDL_RenderFillRect(renderer, &optionRect);
        drawText(renderer, "Duck Speed Multiplier: " + to_string(duckSpeedMultiplier), duckSpeedRect.x + 10, duckSpeedRect.y + 10, {0, 0, 0, 255});

        // Save button
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &saveRect);
        drawText(renderer, "Save and Exit", saveRect.x + 40, saveRect.y + 10, {0, 0, 0, 255});

        SDL_RenderPresent(renderer);
    }

    SDL_ShowCursor(SDL_DISABLE); // Ẩn con trỏ sau khi thoát menu
    return true;
}

// Hàm hiển thị menu
bool showMainMenu(SDL_Renderer* renderer) {
    // Tải hình ảnh nền cho menu
    SDL_ShowCursor(SDL_ENABLE);

    SDL_Texture* menuBackground = loadTexture("C:/Users/bebiu/Desktop/puckdorn.com/main_menu_background.png", renderer);
    if (menuBackground == nullptr) {
        cerr << "Failed to load menu background.\n";
        return false;
    }

    SDL_Rect option1Rect = {210, 330, 180, 100};
    SDL_Rect option2Rect = {420, 330, 180, 100};
    SDL_Event e;
    bool inMenu = true;
    int selectedOption = -1;

    while (inMenu) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyTexture(menuBackground);
                return false;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                if (mouseX >= option1Rect.x && mouseX <= option1Rect.x + option1Rect.w &&
                    mouseY >= option1Rect.y && mouseY <= option1Rect.y + option1Rect.h) {
                    selectedOption = 1;
                    inMenu = false;
                }

                if (mouseX >= option2Rect.x && mouseX <= option2Rect.x + option2Rect.w &&
                    mouseY >= option2Rect.y && mouseY <= option2Rect.y + option2Rect.h) {
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Notice", "Function will be updated soon", nullptr);
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, menuBackground, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(menuBackground);
    return (selectedOption == 1);
}
// Định nghĩa hàm showPauseMenu trước hàm main
bool showPauseMenu(SDL_Renderer* renderer) {
    SDL_ShowCursor(SDL_ENABLE); // Hiển thị con trỏ chuột khi ở menu tạm dừng

    // Tạo background cho menu tạm dừng
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect pauseMenuRect = {100, 150, 600, 300};
    SDL_RenderFillRect(renderer, &pauseMenuRect);

    // Định nghĩa các vùng nhấn cho các nút
    SDL_Rect continueRect = {200, 200, 400, 50};
    SDL_Rect settingsRect = {200, 270, 400, 50};
    SDL_Rect backToMenuRect = {200, 340, 400, 50};

    SDL_Event e;
    bool inPauseMenu = true;
    int selectedOption = -1;

    while (inPauseMenu) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) return false;
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                // Kiểm tra nếu nhấn vào "Continue"
                if (mouseX >= continueRect.x && mouseX <= continueRect.x + continueRect.w &&
                    mouseY >= continueRect.y && mouseY <= continueRect.y + continueRect.h) {
                    selectedOption = 1;
                    inPauseMenu = false;
                }
                // Kiểm tra nếu nhấn vào "Settings"
                if (mouseX >= settingsRect.x && mouseX <= settingsRect.x + settingsRect.w &&
                    mouseY >= settingsRect.y && mouseY <= settingsRect.y + settingsRect.h) {
                    if (!showSettingsMenu(renderer, crosshairSize, soundVolume, duckSpeedMultiplier)) {
                        inPauseMenu = false;
                    }
                }
                // Kiểm tra nếu nhấn vào "Back to Menu"
                if (mouseX >= backToMenuRect.x && mouseX <= backToMenuRect.x + backToMenuRect.w &&
                    mouseY >= backToMenuRect.y && mouseY <= backToMenuRect.y + backToMenuRect.h) {
                    selectedOption = 3;
                    inPauseMenu = false;
                }
            }
        }
        SDL_RenderPresent(renderer);
    }

    SDL_ShowCursor(SDL_DISABLE); // Ẩn con trỏ khi quay lại trò chơi
    return (selectedOption == 1);
}

// Load duck animation frames
void loadDuckFrames(SDL_Renderer* renderer) {
    for (int i = 1; i <= DUCK_FRAMES; ++i) {
        string path = "C:/Users/bebiu/Desktop/puckdorn.com/frame" + to_string(i) + ".png";
        duckFrames[i] = loadTexture(path, renderer);
    }
}
void drawBullets(SDL_Renderer* renderer, SDL_Texture* bulletTexture, int bulletsLeft) {
    const int bulletWidth = 30;      // Chiều rộng biểu tượng đạn
    const int bulletHeight = 30;     // Chiều cao biểu tượng đạn
    const int spacing = 5;           // Khoảng cách giữa các viên đạn

    for (int i = 0; i < bulletsLeft; ++i) {
        SDL_Rect bulletRect = {10 + i * (bulletWidth + spacing), 10, bulletWidth, bulletHeight};
        SDL_RenderCopy(renderer, bulletTexture, nullptr, &bulletRect);
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
void checkGameOver(SDL_Renderer* renderer) {
    if (bulletsLeft <= 0 && duckState != FALLING) {
        // Display a "Game Over" message box
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", "You are out of bullets and missed the duck!", nullptr);
        
        // Set the `quit` flag to true to exit the game loop
        exit(0);
    }
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

// Function to draw the game statistics
void drawStats(SDL_Renderer* renderer) {
    drawText(renderer, "Score: " + to_string(score), 10, 10, {255, 255, 0, 255});
    drawText(renderer, "Level: " + to_string(level), 10, 40, {255, 255, 0, 255});
    drawText(renderer, "Bullets: " + to_string(bulletsLeft), 10, 70, {255, 255, 0, 255});
}


int main(int argc, char* args[]) {
    // Khởi tạo SDL và SDL_image
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    // Tạo cửa sổ và renderer
    SDL_Window* window = SDL_CreateWindow("Duck Hunt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Tải tài nguyên hình ảnh
    loadDuckFrames(renderer);
    SDL_Texture* backgroundTexture = loadTexture("C:/Users/bebiu/Desktop/puckdorn.com/background.png", renderer);
    SDL_Texture* foregroundTexture = loadTexture("C:/Users/bebiu/Desktop/puckdorn.com/foreground.png", renderer);
    SDL_Texture* bulletTexture = loadBulletIcon(renderer);
    if (bulletTexture == nullptr) {
    cerr << "Failed to load bullet icon.\n";
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
    return -1;
    }
    if (backgroundTexture == nullptr || foregroundTexture == nullptr) {
        cerr << "Failed to load game textures.\n";
        freeDuckFrames();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    bool inGame = false;

    // Hiển thị menu chính
    if (showMainMenu(renderer)) {
        inGame = true;
    } else {
        // Người chơi đã chọn thoát trò chơi
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    // Ẩn con trỏ chuột khi bắt đầu trò chơi
    SDL_ShowCursor(SDL_DISABLE);

    // Khởi tạo các thuộc tính của con vịt
    int bulletsleft = 3;
    bool duckHit = false;
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
        } 
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                // Khi nhấn phím ESC, mở menu tạm dừng
                if (!showPauseMenu(renderer)) {
                    quit = true; // Thoát trò chơi nếu chọn "Back to Menu"
                }
            }
        else if (e.type == SDL_MOUSEBUTTONDOWN && bulletsLeft > 0 && duckState != FALLING) {
            bulletsLeft--; // Giảm số đạn khi bắn
            // Kiểm tra nếu người chơi bắn trúng con vịt
            if (crosshairX >= duckRect.x && crosshairX <= duckRect.x + DUCK_WIDTH &&
                crosshairY >= duckRect.y && crosshairY <= duckRect.y + DUCK_HEIGHT) {
                duckState = POP;
                currentFrame = 7;
                popStartTime = SDL_GetTicks();
                duckHit = true;
                
            }
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN && duckState != FALLING) {
            if (crosshairX >= duckRect.x && crosshairX <= duckRect.x + DUCK_WIDTH &&
                crosshairY >= duckRect.y && crosshairY <= duckRect.y + DUCK_HEIGHT) {
                duckState = POP;
                currentFrame = 7;
                popStartTime = SDL_GetTicks();
            }
        }
    }
    if (bulletsLeft == 0 && !duckHit) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", "You're out of bullets!", nullptr);
            quit = true;
    }
    updateDuckFrame();
 // Kiểm tra nếu hết đạn mà không bắn trúng con vịt
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
            duckHit = false;
            bulletsLeft =3;
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


    // Dọn dẹp tài nguyên trước khi thoát
freeDuckFrames();
SDL_DestroyTexture(bulletTexture);
SDL_DestroyTexture(backgroundTexture);
SDL_DestroyTexture(foregroundTexture);
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
IMG_Quit();
SDL_Quit();

    return 0;
}
