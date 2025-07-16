#include "OpenGL/include/glut.h"

// Fullscreen toggle state
int difficultyLevel = 1; // 1: Easy, 2: Medium, 3: Hard
int rockAmount = 1;
int isFullscreen = 0; // Removed stray 'oop' line
int windowedWidth = 800;
int windowedHeight = 500;
int windowedPosX = 100;
int windowedPosY = 100;

#include "iGraphics.h"
#include "iSound.h"
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
char playerName[32] = "";
int nameCharIndex = 0;
int score = 0;
void autoIncreaseScore();
int lives = 3;

//
#define MAX_ENEMIES 10
#define ENEMY_VANISH_DELAY 5

Sprite enemies[MAX_ENEMIES];
bool enemyActive[MAX_ENEMIES];
int enemyVanishTimer[MAX_ENEMIES] = {0};
int enemySpawnTimer = 0;
// int score = 0;
// int lives = 3;
//

int tileWidth = 64;
int tileHeight = 64;
int MonstervelocityY = 0;
bool jump = false;
int cameraX = 0;
int cameraSpeed = 10;
const int screenWidth = 800;
const int screenHeight = 600;
const int monsterWidth = 95;
Image bg;
int speed = 3;
char startScreenBg[] = "start_bg.bmp";
char homemenu[] = "Menu.bmp";
Image stone;
// char bg[] = "BG.bmp";
char control[] = "Controls.bmp";
char tile[] = "Tile.bmp";
char death[] = "death.bmp";
char credits[] = "Credits.bmp";
char difficulty[] = "Difficulty.bmp";

char menuMusic[] = "Main.wav";
char gameMusic[] = "Game.wav";
char creditsMusic[] = "Credits.wav";
bool moveLeft = false;
bool moveRight = false;

// Stone system variables
int stone_x = 800;
int stone_y = 30; // ground level
int stone_width = 64;
int stone_height = 64;
int stone_active = 1;

#define ROWS 10
#define COLS 15

int tilemap[9][12] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

typedef enum
{
    START_SCREEN,
    NAME_INPUT_STATE,
    MENU_STATE,
    PLAYING_STATE,
    CONTROL_STATE,
    CREDITS_STATE,
    DIFFICULTY_STATE,
    HALLOFFAME_STATE,
    PAUSED_STATE,
    GAME_OVER_STATE,
    EXIT_STATE
} GameState;

typedef enum
{
    IDLE_ANIM,
    WALK_ANIM,
    JUMP_ANIM,
    ATTACK_ANIM
} AnimationState;

GameState currentGameState = START_SCREEN;
int sprite_x = 43, sprite_y = 100;
int jump_peak = 100;
AnimationState animState = IDLE_ANIM;
int direction = 1; // 1 for right, -1 for left

int isMusicOn = 1;
char *currentMusic = NULL;

Image idleMonster[4], walkMonster[6], jumpMonster[8], attackMonster[7]; // , enemy[12];
Sprite monster, demon;                                                  // , enemy sprite commented out

typedef struct
{
    int x1, y1, x2, y2;
    GameState targetState;
} Button;

Button menuButtons[] = {
    {40, 400, 150, 434, PLAYING_STATE},    // Start
    {40, 338, 245, 368, DIFFICULTY_STATE}, // Difficulty
    {40, 270, 235, 305, CONTROL_STATE},    // Controls
    {40, 205, 190, 240, CREDITS_STATE},    // Credits
    {40, 140, 300, 175, HALLOFFAME_STATE}, // Hall of Fame
    {40, 75, 120, 110, EXIT_STATE}         // Exit
};

void loadResources();
void updateMonster();
void drawGameScreen();
void handleMenuClick(int mx, int my);
void handlePlayerMovement(unsigned char key);
void resetGameState();
void playMusic(char *filename, int loop);
// void stopMusic();
void changeGameState(GameState newState);
void updateCamera();

void updateCamera()
{
    cameraX = sprite_x - screenWidth / 2 + monsterWidth / 2;

    int maxCameraX = COLS * tileWidth - screenWidth;
    if (cameraX < 0)
        cameraX = 0;
    if (cameraX > maxCameraX)
        cameraX = maxCameraX;
}

// void playMusic(char *filename, int loop)
// {
//     if (!isMusicOn)
//         return;
//     if (currentMusic == NULL || strcmp(currentMusic, filename) != 0)
//     {
//         stopMusic();
//         currentMusic = filename;
//         wchar_t wfilename[256];
//         MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, 256);
//         PlaySound(wfilename, NULL, SND_ASYNC | SND_FILENAME | (loop ? SND_LOOP : 0));
//     }
// }

// void stopMusic()
// {
//     PlaySound(NULL, NULL, 0);
//     currentMusic = NULL;
// }

void drawTiles()
{
    for (int i = 0; i < ROWS - 1; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (tilemap[i][j] == 1)
            {
                iShowImage(j * tileWidth - cameraX, i * tileHeight, tile);
            }
        }
    }
}

void changeGameState(GameState newState)
{
    switch (newState)
    {
    case MENU_STATE:
        // playMusic(menuMusic, 1);
        break;
    case PLAYING_STATE:
    case PAUSED_STATE:
        // playMusic(gameMusic, 1);
        break;
    case CREDITS_STATE:
        // playMusic(creditsMusic, 1);
        break;
    default:
        break;
    }
    currentGameState = newState;
}

void drawStartScreen();

void loadResources()
{
    iLoadFramesFromSheet(idleMonster, "IDLE.png", 1, 10);
    iLoadFramesFromSheet(walkMonster, "RUN.png", 1, 16);
    // iLoadFramesFromSheet(jumpMonster, "assets/images/sprites/1 Pink_Monster/Pink_Monster_Jump_8.png", 1, 8);
    iLoadFramesFromSheet(attackMonster, "ATTACK.png", 1, 7);
    // iLoadFramesFromSheet(enemy, "Demon.png", 1, 12);
    iLoadImage(&bg, "BG.bmp");
    iLoadImage(&stone, "stone.PNG");
    iResizeImage(&stone, stone.width / 2, stone.height / 1.5); // Make stone half size
    iInitSprite(&monster, -1);
    iChangeSpriteFrames(&monster, idleMonster, 10);
    iSetSpritePosition(&monster, 20, 0);
    iScaleSprite(&monster, 3);

    iInitSprite(&demon, -1);
    // iChangeSpriteFrames(&demon, enemy, 12);
    iSetSpritePosition(&demon, 500, 25);
    iScaleSprite(&demon, 1);
}
void updateMonster()
{
    switch (animState)
    {
    case IDLE_ANIM:
        break;
    case WALK_ANIM:
        iAnimateSprite(&monster);
        iAnimateSprite(&monster);
        break;
    case ATTACK_ANIM:
        iAnimateSprite(&monster);
        iAnimateSprite(&monster);
        static int attackFrame = 0;
        attackFrame += 2;
        if (attackFrame >= 7)
        {
            animState = WALK_ANIM;
            iChangeSpriteFrames(&monster, walkMonster, 10);
            attackFrame = 0;
        }
        break;
    case JUMP_ANIM:
        // No special handling needed
        break;
    }
    if (animState == JUMP_ANIM || monster.y > 0)
    {
        // Smoother jump: lower initial velocity, higher gravity
        monster.y += MonstervelocityY;
        MonstervelocityY -= 4; // gravity stronger for less float

        if (monster.y <= 0)
        {
            monster.y = 0;
            MonstervelocityY = 0;
            // After landing, set to WALK_ANIM so character keeps running
            animState = WALK_ANIM;
            iChangeSpriteFrames(&monster, walkMonster, 10);
            jump = false;
        }

        iAnimateSprite(&monster);
    }
}

//
//     void spawnEnemy()
// {
//     for (int i = 0; i < MAX_ENEMIES; i++)
//     {
//         if (!enemyActive[i])
//         {
//             iInitSprite(&enemies[i], -1);
//             iChangeSpriteFrames(&enemies[i], enemy, 12);
//             iSetSpritePosition(&enemies[i], 500-i, 25);
//             iScaleSprite(&enemies[i], 1.0);
//             //enemies[i].x = -1;
//             enemyActive[i] = true;
//             break;
//         }
//     }
// }
// //
void resetGameState()
{
    sprite_x = 43;
    sprite_y = 200;
    animState = IDLE_ANIM;
    direction = 1;
    iChangeSpriteFrames(&monster, idleMonster, 4);
    iSetSpritePosition(&monster, sprite_x, sprite_y);
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemyActive[i] = false;
        enemyVanishTimer[i] = 0;
    }
    score = 0;
    lives = 3;
    // Set difficulty defaults
    if (difficultyLevel == 1)
    {
        speed = 1;
        stone_y = 30;
        rockAmount = 1;
    }
    else if (difficultyLevel == 2)
    {
        speed = 2;
        stone_y = 30;
        rockAmount = 1;
    }
    else if (difficultyLevel == 3)
    {
        speed = 3;
        stone_y = 30;
        rockAmount = 1;
    }
}

void drawStartScreen()
{
    iShowImage(0, 0, startScreenBg);
    iSetColor(255, 255, 255); // White text
    iText(250, 50, "Press SPACE to Play Game", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawGameScreen()
{

    cameraX = 0;
    // drawTiles();
    iSetColor(0, 0, 0);
    iShowLoadedImage(0, 0, &bg);
    iFilledRectangle(-cameraX, 0, 800, 50);
    iSetSpritePosition(&monster, sprite_x, monster.y);
    iShowSprite(&monster);
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemyActive[i])
        {
            iSetSpritePosition(&enemies[i], enemies[i].x, enemies[i].y);
            iShowSprite(&enemies[i]);
        }
    }
    // Draw stones based on rockAmount
    for (int r = 0; r < rockAmount; r++)
    {
        int rx = stone_x - cameraX - r * 200; // space rocks apart
        int ry = stone_y;
        iShowLoadedImage(rx, 50, &stone);
        // Move stone to the left (simulate world movement)
        if (stone_active == 1)
        {
            stone_x -= speed;
            // Calculate monster and stone bounding boxes
            int monster_left = sprite_x;
            int monster_right = sprite_x + monsterWidth;
            int monster_top = monster.y + monsterWidth;
            int monster_bottom = monster.y;
            // Central part of stone (middle 25%)
            int stone_central_left = rx + stone_width * 3 / 8;
            int stone_central_right = rx + stone_width * 5 / 8;
            int stone_top = ry + stone_height;
            int stone_bottom = ry;
            // Only check collision if monster is on the ground
            bool is_on_ground = (monster.y == 0);
            bool horizontal_overlap = monster_right > stone_central_left && monster_left < stone_central_right;
            bool collision = is_on_ground && horizontal_overlap;
            if (collision)
            {
                lives--;
                stone_active = 0;
                stone_x = 700 + rand() % 200; // Respawn stone
                // Reset monster position to initial area
                sprite_x = 43;
                monster.y = 0;
                iSetSpritePosition(&monster, sprite_x, monster.y);
                if (lives <= 0)
                {
                    currentGameState = GAME_OVER_STATE;
                    return;
                }
            }
            // If stone goes off screen, respawn
            if (stone_x < -stone_width)
            {
                stone_x = 700 + rand() % 200; // 700 to 900
                stone_active = 1;
            }
        }
        else
        {
            stone_active = 1;
        }
    }
    iSetColor(255, 255, 255);
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    iText(650, 450, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
    char livesText[32];
    sprintf(livesText, "Lives: %d", lives);
    iText(650, 420, livesText, GLUT_BITMAP_HELVETICA_18);
    if (moveRight)
    {
        iWrapImage(&bg, -speed);
    }
    // if(moveLeft){
    //     iWrapImage(&bg, +speed);
    // PlaySound(filename, NULL, SND_ASYNC | SND_FILENAME | (loop ? SND_LOOP : 0));
}

// void updateEnemy()
// {
//     enemySpawnTimer++;
//     if (enemySpawnTimer >= 30 + rand() % 20)
//     {
//         spawnEnemy();
//         enemySpawnTimer = 0;
//     }
//     for (int i = 0; i < MAX_ENEMIES; i++)
//     {
//         if (enemyActive[i])
//         {
//             iAnimateSprite(&enemies[i]);
//             enemies[i].x -= 2;
//             // Fix: compare both in world coordinates
//             int playerWorldX = sprite_x + cameraX;
//             if (enemyVanishTimer[i] == 0 && abs(enemies[i].x - playerWorldX) < 40 && abs(enemies[i].y - monster.y) < 60) {
//                 // lives--;
//                 // if (lives <= 0) {
//                 //     currentGameState = GAME_OVER_STATE;
//                 // }
//                 enemyVanishTimer[i] = ENEMY_VANISH_DELAY;
//                 enemyActive[i] = false;
//             }
//             if (enemyVanishTimer[i] > 0) {
//                 enemyVanishTimer[i]--;
//                 if (enemyVanishTimer[i] == 0) {
//                     enemyActive[i] = false;
//                 }
//             }
//             if (enemies[i].x < -100)
//             {
//                 enemyActive[i] = false;
//                 enemyVanishTimer[i] = 0;
//             }
//         }
//     }
// }

void iDraw()
{
    iClear();

    switch (currentGameState)
    {
    case START_SCREEN:
        drawStartScreen();
        break;
    case NAME_INPUT_STATE:
        iClear();
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, windowedWidth, windowedHeight);
        iSetColor(255, 255, 255);
        iText(100, 350, "ENTER YOUR NAME:", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(350, 350, playerName, GLUT_BITMAP_HELVETICA_18);
        iText(300, 20, "Press ENTER to continue", GLUT_BITMAP_9_BY_15);
        break;
    case MENU_STATE:
        iShowImage(0, 0, homemenu);
        break;
    case PLAYING_STATE:
        drawGameScreen();
        break;
    case CONTROL_STATE:
        iShowImage(0, 0, control);
        break;
    case DIFFICULTY_STATE:
        iClear();
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, windowedWidth, windowedHeight);
        iSetColor(255, 255, 255);
        iText(300, 350, "Select Difficulty", GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(0, 255, 0);
        iText(300, 280, "Easy", GLUT_BITMAP_HELVETICA_18);
        iSetColor(255, 255, 0);
        iText(300, 240, "Medium", GLUT_BITMAP_HELVETICA_18);
        iSetColor(255, 0, 0);
        iText(300, 200, "Hard", GLUT_BITMAP_HELVETICA_18);
        break;
    case CREDITS_STATE:
        iShowImage(0, 0, credits);
        break;
    case HALLOFFAME_STATE:
        iShowImage(0, 0, "BG.bmp");
        break;
    case GAME_OVER_STATE:
        iClear();
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, windowedWidth, windowedHeight);
        iSetColor(255, 255, 255);
        iText(320, 350, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        char scoreMsg[64];
        iSetColor(0, 0, 255);
        sprintf(scoreMsg, "Player: %s", playerName);
        iText(320, 320, scoreMsg, GLUT_BITMAP_HELVETICA_18);
        sprintf(scoreMsg, "Your Score: %d", score);
        iText(320, 300, scoreMsg, GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 255, 0);
        iText(320, 220, "Retry", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 255, 0);
        iText(320, 180, "Menu", GLUT_BITMAP_HELVETICA_18);
        break;
    case PAUSED_STATE:
        drawGameScreen();
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, windowedWidth, windowedHeight);
        iSetColor(255, 255, 255);
        iText(350, 350, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(0, 255, 0);
        iText(350, 280, "Resume", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 255, 0);
        iText(350, 240, "Restart", GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 255, 0);
        iText(350, 200, "Main Menu", GLUT_BITMAP_HELVETICA_18);
        iSetColor(255, 255, 0);
        iText(350, 160, "Difficulty", GLUT_BITMAP_HELVETICA_18);
        break;
    case EXIT_STATE:
        exit(0);
        break;
    }
}

void handleMenuClick(int mx, int my)
{
    for (size_t i = 0; i < sizeof(menuButtons) / sizeof(menuButtons[0]); i++)
    {
        Button btn = menuButtons[i];
        if (mx >= btn.x1 && mx <= btn.x2 && my >= btn.y1 && my <= btn.y2)
        {
            if (btn.targetState == PLAYING_STATE)
            {
                nameCharIndex = 0;
                playerName[0] = '\0';
                currentGameState = NAME_INPUT_STATE;
            }
            else
            {
                currentGameState = btn.targetState;
            }
            return;
        }
    }
}

void handlePlayerMovement(unsigned char key)
{
    // Left and right arrow key input disabled
    // Calculate monster and stone bounding boxes
    int monster_left = sprite_x;
    int monster_right = sprite_x + monsterWidth;
    int monster_top = monster.y + monsterWidth; // or monster.y + monsterHeight if you have it
    int monster_bottom = monster.y;

    int stone_left = stone_x;
    int stone_right = stone_x + stone_width;
    int stone_top = stone_y + stone_height;
    int stone_bottom = stone_y;

    // Check collision
    bool collision = monster_right > stone_left &&
                     monster_left < stone_right &&
                     monster_top > stone_bottom &&
                     monster_bottom < stone_top;
}

void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (currentGameState == PLAYING_STATE)
        {

            animState = ATTACK_ANIM;
            iChangeSpriteFrames(&monster, attackMonster, 7);
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemyActive[i])
                {
                    int hitbox = 60;
                    if (abs(enemies[i].x - sprite_x) <= hitbox)
                    {
                        enemyVanishTimer[i] = ENEMY_VANISH_DELAY;
                        // score += 1;
                        break;
                    }
                }
            }
        }
        else
        {
            switch (currentGameState)
            {
            case MENU_STATE:
                handleMenuClick(mx, my);
                break;
            case CONTROL_STATE:
                changeGameState(MENU_STATE);
                break;
            case DIFFICULTY_STATE:
                // Easy: x=300 to 380, y=280 to 300
                if (mx >= 300 && mx <= 380 && my >= 280 && my <= 300)
                {
                    difficultyLevel = 1;
                    resetGameState();
                    currentGameState = PLAYING_STATE;
                }
                // Medium: x=300 to 380, y=240 to 260
                else if (mx >= 300 && mx <= 380 && my >= 240 && my <= 260)
                {
                    difficultyLevel = 2;
                    resetGameState();
                    currentGameState = PLAYING_STATE;
                }
                // Hard: x=300 to 380, y=200 to 220
                else if (mx >= 300 && mx <= 380 && my >= 200 && my <= 220)
                {
                    difficultyLevel = 3;
                    resetGameState();
                    currentGameState = PLAYING_STATE;
                }
                break;
            case CREDITS_STATE:
                changeGameState(MENU_STATE);
                break;
            case HALLOFFAME_STATE:
                changeGameState(MENU_STATE);
                break;
            case GAME_OVER_STATE:
                // Retry button area: x=320-20 to 320+60, y=220-10 to 220+20
                if (mx >= 320 && mx <= 400 && my >= 220 && my <= 240)
                {
                    resetGameState();
                    currentGameState = PLAYING_STATE;
                }
                // Menu button area: x=320-20 to 320+60, y=180-10 to 180+20
                else if (mx >= 320 && mx <= 400 && my >= 180 && my <= 200)
                {
                    resetGameState();
                    currentGameState = MENU_STATE;
                }
                break;
            case PAUSED_STATE:
                // Resume button: x=350-20 to 350+80, y=280-10 to 280+20
                if (mx >= 350 && mx <= 430 && my >= 280 && my <= 300)
                {
                    changeGameState(PLAYING_STATE);
                }
                // Restart button: x=350-20 to 350+80, y=240-10 to 240+20
                else if (mx >= 350 && mx <= 430 && my >= 240 && my <= 260)
                {
                    resetGameState();
                    changeGameState(PLAYING_STATE);
                }
                // Main Menu button: x=350-20 to 350+120, y=200-10 to 200+20
                else if (mx >= 350 && mx <= 470 && my >= 200 && my <= 220)
                {
                    resetGameState();
                    changeGameState(MENU_STATE);
                }
                // Difficulty button: x=350-20 to 350+120, y=160-10 to 160+20
                else if (mx >= 350 && mx <= 470 && my >= 160 && my <= 180)
                {
                    changeGameState(DIFFICULTY_STATE);
                }
                break;
            case START_SCREEN:
                // No action needed
                break;
            case PLAYING_STATE:
                // No action needed (handled above)
                break;
            case EXIT_STATE:
                // No action needed
                break;
            }
        }
    }
}

void iKeyboard(unsigned char key)
{
    if (currentGameState == NAME_INPUT_STATE)
    {
        if ((key >= 32 && key <= 126) && nameCharIndex < 30)
        {
            playerName[nameCharIndex++] = key;
            playerName[nameCharIndex] = '\0';
        }
        else if (key == 8 && nameCharIndex > 0)
        { // Backspace
            nameCharIndex--;
            playerName[nameCharIndex] = '\0';
        }
        else if (key == 13 && nameCharIndex > 0)
        { // Enter
            resetGameState();
            currentGameState = PLAYING_STATE;
        }
        return;
    }
    switch (key)
    {
    case ' ':
        if (currentGameState == START_SCREEN)
        {
            currentGameState = MENU_STATE;
        }
        else if (currentGameState == PLAYING_STATE && monster.y == 0)
        {
            animState = JUMP_ANIM;
            MonstervelocityY = 32; // Increased jump velocity for higher jump
            iChangeSpriteFrames(&monster, jumpMonster, 8);
        }
        break;
    case 'f':
    case 'F':
        // Toggle fullscreen
        if (!isFullscreen)
        {
            windowedWidth = glutGet(GLUT_WINDOW_WIDTH);
            windowedHeight = glutGet(GLUT_WINDOW_HEIGHT);
            windowedPosX = glutGet(GLUT_WINDOW_X);
            windowedPosY = glutGet(GLUT_WINDOW_Y);
            glutFullScreen();
            isFullscreen = 1;
        }
        else
        {
            glutReshapeWindow(windowedWidth, windowedHeight);
            glutPositionWindow(windowedPosX, windowedPosY);
            isFullscreen = 0;
        }
        break;
    case 'm':
    case 'M':
        // isMusicOn = !isMusicOn;
        // if (isMusicOn)
        // {
        //     switch (currentGameState)
        //     {
        //     case MENU_STATE:
        //         playMusic(menuMusic, 1);
        //         break;
        //     case PLAYING_STATE:
        //     case PAUSED_STATE:
        //         playMusic(gameMusic, 1);
        //         break;
        //     case CREDITS_STATE:
        //         playMusic(creditsMusic, 1);
        //         break;
        //     default:
        //         break;
        //     }
        // }
        // else
        // {
        //     stopMusic();
        // }
        break;
    case 'p':
    case 'P':
        if (currentGameState == PLAYING_STATE)
        {
            currentGameState = PAUSED_STATE;
        }
        else if (currentGameState == PAUSED_STATE)
        {
            currentGameState = PLAYING_STATE;
        }
        break;
    case 'x':
    case 27: // ESC key
        if (currentGameState == PLAYING_STATE || currentGameState == PAUSED_STATE)
        {
            changeGameState(MENU_STATE);
        }
        else
        {
            changeGameState(EXIT_STATE);
        }
        break;
    }
}

void iSpecialKeyboard(unsigned char key)
{
    if (key == GLUT_KEY_END)
    {
        currentGameState = EXIT_STATE;
    }

    if (currentGameState == PLAYING_STATE)
    {
        // Left and right arrow key input disabled
    }
}
void iSpecialKeyboardUp(unsigned char key)
{
    if (currentGameState == PLAYING_STATE)
    {
        // Left and right arrow key input disabled
    }
}

void iMouseMove(int mx, int my) {}
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}

void autoIncreaseScore()
{
    if (currentGameState == PLAYING_STATE)
    {
        score++;
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    loadResources();
    iSetTimer(100, updateMonster);
    iSetTimer(2000, autoIncreaseScore); // Increase score every 2 seconds
    // playMusic(menuMusic, 1);
    iInitialize(windowedWidth, windowedHeight, "Hellfire");
    return 0;
}
