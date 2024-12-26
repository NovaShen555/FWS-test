#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma execution_character_set("utf-8")

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define CARD_WIDTH 100
#define CARD_HEIGHT 150

typedef struct {
    SDL_Rect rect;
    bool flipped;
    const char* prizeInfo;
} Card;

const char* generatePrizeInfo() {
    int randomIndex = rand() % 5;
    switch (randomIndex) {
    case 0: return "一瓶盒牛奶";
    case 1: return "一包薯片";
    case 2: return "一粒巧克力";
    case 3: return "一份 53 试卷";
    case 4: return "10 个深蹲";
    default: return "神秘奖品";
    }
}

int main(int argc, char* argv[]) {
    int n = 5; // 卡片数量

    // 计算窗口的宽度
    int totalCardWidth = n * CARD_WIDTH + (n - 1) * CARD_WIDTH / 2;
    int windowWidth = totalCardWidth > SCREEN_WIDTH ? totalCardWidth : SCREEN_WIDTH;

    // 初始化SDL视频和音频子系统
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("初始化SDL失败：%s", SDL_GetError());
        return 1;
    }

    // 初始化SDL_ttf字体子系统
    if (TTF_Init() == -1) {
        SDL_Log("初始化TTF失败：%s", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow("卡片游戏", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("创建窗口失败：%s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // 创建渲染器
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("创建渲染器失败：%s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 加载中文字体
    TTF_Font* font = TTF_OpenFont("SimHei.ttf", 24);
    if (!font) {
        SDL_Log("加载字体失败：%s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 创建卡片数组
    Card* cards = (Card*)malloc(n * sizeof(Card));
    if (!cards) {
        SDL_Log("内存分配失败.");
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 初始化随机数生成器
    srand(time(NULL));

    // 初始化卡片
    for (int i = 0; i < n; ++i) {
        cards[i].rect.x = 50 + i * (CARD_WIDTH + CARD_WIDTH / 2); // 调整卡片位置，加入间隔
        cards[i].rect.y = 200;
        cards[i].rect.w = CARD_WIDTH;
        cards[i].rect.h = CARD_HEIGHT;
        cards[i].flipped = false;
        cards[i].prizeInfo = generatePrizeInfo();
    }

    // 加载背景音乐
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;

    if (SDL_LoadWAV("background_music.wav", &wavSpec, &wavBuffer, &wavLength) == NULL) {
        SDL_Log("加载背景音乐失败：%s", SDL_GetError());
        // 处理加载音乐失败的情况
    }

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    if (deviceId == 0) {
        SDL_Log("打开音频设备失败：%s", SDL_GetError());
        // 处理打开音频设备失败的情况
    }

    SDL_PauseAudioDevice(deviceId, 0); // 开始播放音乐
    SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_FreeWAV(wavBuffer);

    // 处理SDL事件
    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                for (int i = 0; i < n; ++i) {
                    if (!cards[i].flipped &&
                        mouseX >= cards[i].rect.x && mouseX < cards[i].rect.x + CARD_WIDTH &&
                        mouseY >= cards[i].rect.y && mouseY < cards[i].rect.y + CARD_HEIGHT) {
                        // 点击卡片的动画效果：示例改变卡片颜色
                        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 翻转后的卡片颜色为黄色
                        SDL_RenderFillRect(renderer, &cards[i].rect);
                        SDL_RenderPresent(renderer);

                        // 模拟动画效果的延迟（可以根据需要调整）
                        SDL_Delay(500); // 延迟500毫秒

                        cards[i].flipped = true;
                        break;
                    }
                }
            }
        }

        // 渲染背景色
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // 渲染卡片
        for (int i = 0; i < n; ++i) {
            if (cards[i].flipped) {
                // 渲染翻转后的卡片内容
                SDL_Color textColor = { 0, 0, 0 };
                SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, cards[i].prizeInfo, textColor);
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_Rect textRect = { cards[i].rect.x, cards[i].rect.y, textSurface->w, textSurface->h };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
            else {
                // 渲染未翻转的卡片背面
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 蓝色表示未翻转的卡片背面
                SDL_RenderFillRect(renderer, &cards[i].rect);
            }
        }

        SDL_RenderPresent(renderer);
    }

    // 释放资源
    free(cards);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_CloseAudioDevice(deviceId);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_Quit();

    return 0;
}
