#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>
using namespace std;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int CROSSHAIR_SIZE = 15;  // độ dài crosshair 
const int DUCK_WIDTH = 60;      // chiều rộng của con vịt 
const int DUCK_HEIGHT = 60;     // chiều rộng của con vịt
const float BASE_SPEED = 3.0f;  // tốc độ đi của con vịt
const float FALL_SPEED = 5.0f;  // tốc độ vịt rơi khi bị bắn 
const int GROUND_LEVEL_Y = SCREEN_HEIGHT - 150; // nơi mà con vịt được spawn ra theo cột y 2D game có nghĩa là chỉ có x,y

enum DuckState {
    FLYING,
    FALLING
};

// load image fucntion r
SDL_Texture* loadTexture(const string &path, SDL_Renderer *renderer) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (newTexture == nullptr) {
        cerr << "Failed to load texture " << path << " SDL_image Error: " << IMG_GetError() << "\n";
    }
    return newTexture;
}

// Function to set the target for the duck to move through the center first, then to a corner
void setDuckTargets(SDL_Rect &duckRect, float &duckSpeedX, float &duckSpeedY, SDL_Point &centerTarget, SDL_Point &finalTarget, float speedMultiplier, bool &spawnedFromSky) {
    if (rand() % 2 == 0) { // random vị trí spawn 
        // spawn ở trên xuống từ trên màn hình 
        duckRect.x = rand() % (SCREEN_WIDTH - DUCK_WIDTH); // vị trí nằm ngang ngẫu nhiên 
        duckRect.y = 0 - DUCK_HEIGHT; // cho nó spawn trên không 
        centerTarget = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        finalTarget = (rand() % 2 == 0) ? SDL_Point{SCREEN_WIDTH, SCREEN_HEIGHT} : SDL_Point{0, SCREEN_HEIGHT}; // đây là các góc dưới của màn hình 
        spawnedFromSky = true;
    } else {
        // spawn ở dưới màn hình
        duckRect.x = rand() % (SCREEN_WIDTH - DUCK_WIDTH); // giống như phần spwan ở trên 
        duckRect.y = GROUND_LEVEL_Y + DUCK_HEIGHT; // phần chắc chắn spawn ở dưới 
        centerTarget = { SCREEN_WIDTH / 2, GROUND_LEVEL_Y - 100 }; // cho con vịt bay lên trời 
        finalTarget = (rand() % 2 == 0) ? SDL_Point{SCREEN_WIDTH, GROUND_LEVEL_Y - 100} : SDL_Point{0, GROUND_LEVEL_Y - 100}; // đi trái hoặc phải lên không trung 
        spawnedFromSky = false;
    }

    // khởi tạo tốc độ ban đầu của con vịt hướng tới trung tâm, được cân bằng bởi hệ số nhân của tốc độ.
    int dx = centerTarget.x - duckRect.x;
    int dy = centerTarget.y - duckRect.y;
    float distance = sqrt(dx * dx + dy * dy);
    duckSpeedX = speedMultiplier * BASE_SPEED * (dx / distance);
    duckSpeedY = speedMultiplier * BASE_SPEED * (dy / distance);
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // khởi tạo sdl
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    // khởi tạo sdl.image dùng cho load các file có đuôi là jpg hay là png
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Duck Hunt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer) {
        cerr << "Window or renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // ẩn con trỏ chuột 
    SDL_ShowCursor(SDL_DISABLE);

    // dùng để gọi các tài nguyên bằng cách dùng path để chỉ dẫn tới các file 
    SDL_Texture* duckTextureSky = loadTexture("C:/Users/bebiu/Desktop/Prj/duck_in_sky.png", renderer);
    SDL_Texture* duckTextureGround = loadTexture("C:/Users/bebiu/Desktop/Prj/duck_in_ground.png", renderer);
    SDL_Texture* duckFallTexture = loadTexture("C:/Users/bebiu/Desktop/Prj/duck_fall.png", renderer); // Falling duck texture
    SDL_Texture* backgroundTexture = loadTexture("C:/Users/bebiu/Desktop/Prj/background.png", renderer); // Full background texture

    if (!duckTextureSky || !duckTextureGround || !duckFallTexture || !backgroundTexture) {
        cerr << "Error: Some textures not loaded. Check the file paths and formats.\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // cái này dùng để set up vị trí và tốc độ cho nó(vịt)
    SDL_Rect duckRect = {0, 0, DUCK_WIDTH, DUCK_HEIGHT};
    float duckSpeedX, duckSpeedY;
    SDL_Point centerTarget, finalTarget;
    bool headingToCenter = true; // cho biết con vịt có hướng đến trung tâm hay không 
    bool spawnedFromSky;         // trường hợp này nó sẽ đúng khi con vịt được spawn từ trời nếu ở dưới đất giá trị nó nhận được sẽ là false
    DuckState duckState = FLYING; // đặt trạng thái của vịt đang bay 
    srand(static_cast<unsigned>(time(0)));

    float currentSpeedMultiplier = 1.0f;
    setDuckTargets(duckRect, duckSpeedX, duckSpeedY, centerTarget, finalTarget, currentSpeedMultiplier, spawnedFromSky); // khởi tạo các con vịt 

    int score = 0;
    int crosshairX = SCREEN_WIDTH / 2;
    int crosshairY = SCREEN_HEIGHT / 2;
    bool quit = false;
    SDL_Event e;

    cout << "Duck Hunt Game starting..." << endl;

    // đạt cho back ground sẽ là hình chữ nhật 
    SDL_Rect backgroundRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // tạo vòng lặp 
    while (!quit) {
        // xử lý các event
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEMOTION) {
                // cập nhật vị trí crosshair dựa vào con trỏ chuột 
                crosshairX = e.motion.x;
                crosshairY = e.motion.y;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && duckState == FLYING) {
                // kiểm tra xem crosshair có click được mấy con vịt không 
                if (crosshairX >= duckRect.x && crosshairX <= duckRect.x + DUCK_WIDTH &&
                    crosshairY >= duckRect.y && crosshairY <= duckRect.y + DUCK_HEIGHT) {
                    score++;
                    cout << "Hit! Score: " << score << "\n";

                    // đặt con vịt vào trạng thái rơi nếu bấm trúng 
                    duckState = FALLING;
                    duckSpeedX = 0;         // dừng trạng thái bay của con vịt 
                    duckSpeedY = FALL_SPEED; // cho nó rơi theo vector x và y hiện tại 
                }
            }
        }

        // cập nhật vị trí của vịt dựa vào trạng thái của nó
        if (duckState == FLYING) {
            if (headingToCenter && abs(duckRect.x - centerTarget.x) < 5 && abs(duckRect.y - centerTarget.y) < 5) {
                headingToCenter = false;

                // phép toán làm cho con vịt bay nhanh hơn sau khi đạt ngưỡng 25 con 
                int dx = finalTarget.x - duckRect.x;
                int dy = finalTarget.y - duckRect.y;
                float distance = sqrt(dx * dx + dy * dy);
                duckSpeedX = currentSpeedMultiplier * BASE_SPEED * (dx / distance);
                duckSpeedY = currentSpeedMultiplier * BASE_SPEED * (dy / distance);
            }

            // cập nhật vị trí của vịt
            duckRect.x += static_cast<int>(duckSpeedX);
            duckRect.y += static_cast<int>(duckSpeedY);

            // thuật toán check xem con vị đã rơi khỏi nền hay chưa 
            if (duckRect.y > SCREEN_HEIGHT || duckRect.y < -DUCK_HEIGHT || duckRect.x > SCREEN_WIDTH || duckRect.x < -DUCK_WIDTH) {
                setDuckTargets(duckRect, duckSpeedX, duckSpeedY, centerTarget, finalTarget, currentSpeedMultiplier, spawnedFromSky);
                headingToCenter = true;
            }
        } else if (duckState == FALLING) {
            // cập nhật vị trí của vịt để rơi
            duckRect.y += static_cast<int>(duckSpeedY);

            // hồi sinh lại con vịt nếu nó bay ra khỏi nền và chưa bị bắn
            if (duckRect.y > SCREEN_HEIGHT) {
                duckState = FLYING; // về lại chế độ bay 
                setDuckTargets(duckRect, duckSpeedX, duckSpeedY, centerTarget, finalTarget, currentSpeedMultiplier, spawnedFromSky);
                headingToCenter = true;
            }
        }

        // clear the screen
        SDL_SetRenderDrawColor(renderer, 135, 206, 250, 255); // render nền trời
        SDL_RenderClear(renderer);

        // render nền 
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &backgroundRect);

        // chọn kết cấu phù hợp dựa trên trạng thái và vị trí của vịt
        SDL_Texture* currentDuckTexture;
        if (duckState == FALLING) {
            currentDuckTexture = duckFallTexture;
        } else {
            currentDuckTexture = (duckRect.y < SCREEN_HEIGHT / 2) ? duckTextureSky : duckTextureGround;
        }

        // render ra con vịt
        SDL_RenderCopy(renderer, currentDuckTexture, nullptr, &duckRect);

        // đặt vị trí của crosshair vào vị trí x.y mà con chuột đang chỉ 
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, crosshairX - CROSSHAIR_SIZE, crosshairY, crosshairX + CROSSHAIR_SIZE, crosshairY);
        SDL_RenderDrawLine(renderer, crosshairX, crosshairY - CROSSHAIR_SIZE, crosshairX, crosshairY + CROSSHAIR_SIZE);

        // cập nhật màn hình 
        SDL_RenderPresent(renderer);

        // điều chỉnh frame rate 
        SDL_Delay(16); // ~60 FPS
    }

    cout << "Game Over! Final Score: " << score << "\n";

    // xóa các resource 
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
