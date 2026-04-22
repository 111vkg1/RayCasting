#define NODEBUG
#define DRAWGUI
#define NOSLOWMOTION

#ifndef WINDOWW
    #define WINDOWW 512
#endif // WINDOWW
#ifndef WINDOWH
    #define WINDOWH 256
#endif // WINDOWH
#ifndef PI
    #define PI 3.141592
#endif // WINDOWH

#define MAXRAYS 512
#define MAXSTPS 800
#define RAYSTEP 0.37
#define FOV 75

#include<iostream>
#include<cmath>
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<windows.h>
#include<vector>
#include<map>
#include<fstream>

using namespace std;

// Объявляем окно, рендер и события
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event event;
SDL_Surface *surf;

short frame = 0;
bool isFired = 0;
double sizW = 1, sizH = 1;

// Создаём структуры векторов и цвета
struct doubleVector2
{
    double __x, __y;
};

struct intVector2
{
    int __x, __y;
};

struct u32ColorA
{
    short r = 255, g = 255, b = 255, a = 255;
};

// Создаём структуру стены
struct object
{
    intVector2 pos = {0, 0};
    intVector2 siz = {0, 0};
    u32ColorA color = {255, 255, 255, 255};
    string filepath = "";
};

// Создаём структуру стены
struct cam
{
    doubleVector2 pos = {0, 0};
    double angle = 0;
    int health = 100, maxHealth = 100;
};

// Создаём текстурный атлас и массив стен
map<string, SDL_Texture*> texturesMap;
vector<object> world;

void rendererDraw(cam *camera)
{
    // Очищаем рендер
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    // Рисуем задний фон
    SDL_SetRenderDrawColor(renderer, 153,221,255, 255);
    SDL_Rect __f = {0, 0, WINDOWW, WINDOWH};
    SDL_RenderFillRect(renderer, &__f);
    __f = {0, WINDOWH/2-25, WINDOWW, WINDOWH/2+25};
    SDL_SetRenderDrawColor(renderer, 28,28,28, 255);
    SDL_RenderFillRect(renderer, &__f);

    int rayNum = 0; // Счётчик лучей
    for(double ang = camera->angle + FOV/2; ang > camera->angle - FOV/2; ang -= 0.14){
        doubleVector2 ray = camera->pos; // Задаём позицию из камеры
        int yDisSize;
        double rayLeght;
        bool __c = 0; // Флаг для колизии
        for(int i = 0; i < MAXSTPS; i++){ // Идём в диапазоне от 0 до макс. кл-во шагов
            ray = {ray.__x + RAYSTEP * sin(ang*PI/180), ray.__y + RAYSTEP * cos(ang*PI/180)}; // Двигаем луч
            for(int j = 0; j < world.size(); j++){
                // Если пересекается
                if(ray.__x > world[j].pos.__x && ray.__x < world[j].pos.__x + world[j].siz.__x && ray.__y > world[j].pos.__y && ray.__y < world[j].pos.__y + world[j].siz.__y){
                    __c = 1;
                    // Итоговая длинна
                    rayLeght = sqrt(pow(ray.__x - camera->pos.__x, 2) + pow(ray.__y - camera->pos.__y, 2)) * cos((ang - camera->angle) * PI / 180);
                    double yRatio = 100-(MAXSTPS*RAYSTEP/rayLeght)*10;
                    yDisSize = (100 - yRatio) * 2;
                    int yDisPos = yRatio;
                    // Позиция на экране
                    SDL_Rect rect = {rayNum * (WINDOWW/MAXRAYS), yDisPos, (WINDOWW/MAXRAYS), yDisSize};
                    if(world[j].filepath != "no"){ // Если есть текстура
                        SDL_Rect srect;
                        int tw, th;
                        // Получаем размеры текстуры
                        SDL_QueryTexture(texturesMap[world[j].filepath], NULL, NULL, &tw, &th);
                        double hit;
                        double dx = ray.__x - world[j].pos.__x;
                        double dy = ray.__y - world[j].pos.__y;
                        double textureRepeats;
                        // Вычисляем стенка вертикальная или горизонтальная
                        if(ray.__x > world[j].pos.__x + RAYSTEP && ray.__x < world[j].pos.__x - RAYSTEP + world[j].siz.__x){
                            hit = dx / world[j].siz.__x;
                            textureRepeats = world[j].siz.__x / 20;
                        }
                        else if(ray.__y > world[j].pos.__y && ray.__y < world[j].pos.__y + world[j].siz.__y){
                            hit = dy / world[j].siz.__y;
                            textureRepeats = world[j].siz.__y / 20;
                        }
                        // Дублируем текстуру, если поверхность больше
                        hit = fmod(fabs(hit * (textureRepeats)), 1.0);
                        srect =
                        {
                            hit * tw,
                            0,
                            ((WINDOWW)/MAXRAYS),
                            th
                        };
                        rect =
                        {
                            rect.x,
                            rect.y,
                            rect.w,
                            rect.h
                        };
                        // Растягиваем текстуру и копируем в рендер
                        SDL_SetTextureScaleMode(texturesMap[world[j].filepath], SDL_ScaleModeNearest);
                        SDL_RenderCopy(renderer, texturesMap[world[j].filepath], &srect, &rect);
                    }
                    else{ // Иначе заполняем пустышкой
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
            if(__c)
                break; // Если была колизия останавливаем луч
        }
        rayNum++;
    }
    SDL_RenderPresent(renderer); // Отрисовываем рендер
}

bool init() // Инициализация
{
    SDL_Init(SDL_INIT_EVERYTHING);
    cout << "SDL\n";
    cout << SDL_GetError();
    IMG_Init(IMG_INIT_PNG);
    cout << "IMG\n";
    return 1;
}

bool load()
{
    // Создаём окно и рендер
    window = SDL_CreateWindow("Ray Casting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWW, WINDOWH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, 0, NULL);

    // Загружаем текстуры
    surf = IMG_Load("textures/test1.jpg");
    texturesMap["textures/test.png"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/test.jpg");
    texturesMap["textures/test.jpg"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/violet.jpg");
    texturesMap["textures/violet.jpg"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/wall.jpg");
    texturesMap["textures/wall.jpg"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/hand-pistol.png");
    texturesMap["__hand"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/pistol-fire.png");
    texturesMap["__piFire"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/monster.png");
    texturesMap["textures/monster.png"] = SDL_CreateTextureFromSurface(renderer, surf);
    surf = IMG_Load("textures/wht-debug.png");
    texturesMap["__--blck"] = SDL_CreateTextureFromSurface(renderer, surf);
    return 1;
}

int main(int argc, char* argv[])
{
    init();
    ShowCursor(FALSE); // Скрываем курсор (для красоты)
    load();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Задаём сглаживание текстур

    //Создаём камеру
    cam camera;
    camera.pos = {100.0, 100.0};

    // Добавляем объекты в массив
    world.push_back({60, 20, 220, 10, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({60, 230, 220, 10, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({270, 30, 10, 80, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({270, 150, 10, 80, 255, 0, 255, 255, "textures/wall.jpg"});
    world.push_back({60, 30, 10, 200, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({110, 70, 120, 120, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({160, 70, 20, 20, 255, 0, 255, 255, "textures/violet.jpg"});
    world.push_back({280, 150, 100, 10, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({280, 100, 100, 10, 255, 0, 255, 255, "textures/test.png"});
    world.push_back({120, 50, 20, 20, 255, 0, 255, 255, "textures/test.jpg"});

    bool canMove = 1;
    while(1){
        rendererDraw(&camera); // Рисуем изображение
        SDL_Delay(16);
        while(SDL_PollEvent(&event)){ // Если произошло действие
            canMove = 1;
            // Передвижение
            if(event.key.keysym.scancode == SDL_SCANCODE_W){
                camera.pos = {camera.pos.__x + 5*sin(camera.angle*PI/180), camera.pos.__y + 5*cos(camera.angle*PI/180)};
                for(int i = 0; i < world.size(); i++){
                    if(camera.pos.__x > world[i].pos.__x && camera.pos.__x < world[i].pos.__x + world[i].siz.__x && camera.pos.__y > world[i].pos.__y && camera.pos.__y < world[i].pos.__y + world[i].siz.__y){
                        canMove = 0;
                    }
                }
                // Если нет колизии двигаем камеру
                camera.pos = {camera.pos.__x - 5*sin(camera.angle*PI/180), camera.pos.__y - 5*cos(camera.angle*PI/180)};
                if(canMove)
                    camera.pos = {camera.pos.__x + sin(camera.angle*PI/180), camera.pos.__y + cos(camera.angle*PI/180)};
            }
            else if(event.key.keysym.scancode == SDL_SCANCODE_S){
                camera.pos = {camera.pos.__x - 5*sin(camera.angle*PI/180), camera.pos.__y - 5*cos(camera.angle*PI/180)};
                for(int i = 0; i < world.size(); i++){
                    if(camera.pos.__x > world[i].pos.__x && camera.pos.__x < world[i].pos.__x + world[i].siz.__x && camera.pos.__y > world[i].pos.__y && camera.pos.__y < world[i].pos.__y + world[i].siz.__y){
                        canMove = 0;
                    }
                }
                // Если нет колизии двигаем камеру
                camera.pos = {camera.pos.__x + 5*sin(camera.angle*PI/180), camera.pos.__y + 5*cos(camera.angle*PI/180)};
                if(canMove)
                    camera.pos = {camera.pos.__x - sin(camera.angle*PI/180), camera.pos.__y - cos(camera.angle*PI/180)};
            }
            // Поворот камеры клавиатурой
            if(event.key.keysym.scancode == SDL_SCANCODE_D){
                if(camera.angle > 0)
                    camera.angle -= 1;
                else
                    camera.angle = 360;
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_A){
                if(camera.angle < 360)
                    camera.angle += 1;
                else
                    camera.angle = 0;
            }
            // Если окно изменило размер
            if(event.window.event == SDL_WINDOWEVENT_RESIZED){
                int __w, __h;
                // Получаем текущий размер
                SDL_GetWindowSize(window, &__w, &__h);
                // Получаем коофициэнт к стандартному
                sizW = (double(__w) / double(WINDOWW));
                sizH = (double(__h) / double(WINDOWH));
                // Растягиваем
                SDL_RenderSetScale(renderer, sizW, sizH);
            }
        }
    }
}
