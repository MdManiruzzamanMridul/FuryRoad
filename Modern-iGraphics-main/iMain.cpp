int attackWindow = 0;
#include "OpenGL/include/glut.h"
// --- Save/Load Game State ---
#include <stdio.h>
int hasSaveGame = 0;
char saveFile[] = "saves/savegame.dat";
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
int attack = 3; // Max 3 attacks
void autoIncreaseScore();

typedef struct
{
    char name[32];
    int score;
} HighScoreEntry;

#define MAX_HIGHSCORES 100
HighScoreEntry highScores[MAX_HIGHSCORES];
int highScoreCount = 0;

void loadHighScores()
{
    FILE *f = fopen("saves/names.txt", "r");
    highScoreCount = 0;
    if (!f)
        return;
    while (fscanf(f, "%31s %d", highScores[highScoreCount].name, &highScores[highScoreCount].score) == 2)
    {
        // Check for duplicate name with same score, skip if found
        int found = -1;
        for (int i = 0; i < highScoreCount; i++)
        {
            if (strcmp(highScores[i].name, highScores[highScoreCount].name) == 0 && highScores[i].score == highScores[highScoreCount].score)
            {
                found = i;
                break;
            }
        }
        if (found == -1)
        {
            highScoreCount++;
        }
        if (highScoreCount >= MAX_HIGHSCORES)
            break;
    }
    fclose(f);
    // Sort descending
    for (int i = 0; i < highScoreCount - 1; i++)
    {
        for (int j = i + 1; j < highScoreCount; j++)
        {
            if (highScores[j].score > highScores[i].score)
            {
                HighScoreEntry tmp = highScores[i];
                highScores[i] = highScores[j];
                highScores[j] = tmp;
            }
        }
    }
}
void saveHighScores()
{
    FILE *f = fopen("saves/names.txt", "w");
    if (!f)
        return;
    int n = highScoreCount < 3 ? highScoreCount : 3;
    for (int i = 0; i < n; i++)
    {
        fprintf(f, "%s %d\n", highScores[i].name, highScores[i].score);
    }
    fclose(f);
}

void updateHighScores(const char *name, int newScore)
{
    loadHighScores();
    // Check if player already exists
    int found = -1;
    for (int i = 0; i < highScoreCount; i++)
    {
        if (strcmp(highScores[i].name, name) == 0)
        {
            found = i;
            break;
        }
    }
    if (found != -1)
    {
        // If new score is higher, update
        if (newScore > highScores[found].score)
        {
            highScores[found].score = newScore;
        }
    }
    else
    {
        // Add new entry
        if (highScoreCount < MAX_HIGHSCORES)
        {
            strncpy(highScores[highScoreCount].name, name, 31);
            highScores[highScoreCount].name[31] = '\0';
            highScores[highScoreCount].score = newScore;
            highScoreCount++;
        }
    }
    // Resort descending
    for (int i = 0; i < highScoreCount - 1; i++)
    {
        for (int j = i + 1; j < highScoreCount; j++)
        {
            if (highScores[j].score > highScores[i].score)
            {
                HighScoreEntry tmp = highScores[i];
                highScores[i] = highScores[j];
                highScores[j] = tmp;
            }
        }
    }
    // Only keep top 3
    if (highScoreCount > 3)
        highScoreCount = 3;
    saveHighScores();
}
int lives = 3;

//
#define MAX_ENEMIES 10
#define ENEMY_VANISH_DELAY 10

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
Image bg, bg2, bg3;
Image trapImg;
Image icon;
int speed = 3;
char startScreenBg[] = "start_bg.bmp";
char homemenu[] = "Menu.bmp";
Image stone;
Image rockEasy, rockMedium, rockHard;
// char bg[] = "BG1.bmp";
char control[] = "Controls.bmp";
char tile[] = "Tile.bmp";
char death[] = "death.bmp";
char credits[] = "Credits.bmp";
char difficulty[] = "Difficulty.bmp";

Image slimeImg;
int obstacleType = 0; // 0: rock, 1: slime

char menuMusic[] = "Main.wav";
char gameMusic[] = "Game.wav";
char creditsMusic[] = "Credits.wav";
char jumpSound[] = "jump.wav";
char hurtSound[] = "hurt.wav";
char hitSound[] = "hit.wav";
char swordSound[] = "sword.wav";
bool moveLeft = false;
bool moveRight = false;

// Stone system variables
int stone_x = 800;
int stone_y = 30; // ground level
int stone_width = 64;
int stone_height = 64;
int stone_active = 1;
// Trap system variables
int trap_x = 900;
int trap_y = 0; // ground level
int trap_width = 64;
int trap_height = 64;
int trap_active = 1;

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
    DEATH_MESSAGE_STATE,
    GAME_OVER_STATE,
    EXIT_STATE,
    SAVE_PROMPT_STATE,
    CONTINUE_PROMPT_STATE
} GameState;

// Function prototypes
void drawSavePrompt();
void drawContinuePrompt();
void checkSaveGame();
int loadGame();

// Draw the continue/resume prompt (move out of iMouse)
void drawContinuePrompt()
{
    if (!hasSaveGame)
        return; // Only draw if a save exists
    iClear();
    iSetColor(255, 255, 255);
    iFilledRectangle(250, 180, 320, 140);
    iSetColor(0, 0, 0);
    iText(270, 280, "Resume previous game?", GLUT_BITMAP_TIMES_ROMAN_24);
    iText(325, 230, "Yes", GLUT_BITMAP_HELVETICA_18);
    iText(330, 200, "No", GLUT_BITMAP_HELVETICA_18);
}
typedef enum
{
    IDLE_ANIM,
    WALK_ANIM,
    JUMP_ANIM,
    ATTACK_ANIM,
    HURT_ANIM
} AnimationState;

GameState currentGameState = START_SCREEN;
int sprite_x = 43, sprite_y = 100;
int jump_peak = 100;
AnimationState animState = IDLE_ANIM;
int direction = 1; // 1 for right, -1 for left

int isMusicOn = 1;
char *currentMusic = NULL;

Image idleMonster[4], walkMonster[6], jumpMonster[8], attackMonster[7], hurtMonster[4];
Image demonFrames[14];
Image dragonFrames[3];
Image startScreenBgImg;
Sprite monster, demon, dragon;
int dragonActive = 1;
int dragonVanishTimer = 0;
int dragonY = 200; // Centered vertically (adjusted for sprite height)
int dragonSpeed = 10;

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
void stopMusic();
void changeGameState(GameState newState);
void updateCamera();
void saveGame();
void deleteSaveGame();

void updateCamera()
{
    cameraX = sprite_x - screenWidth / 2 + monsterWidth / 2;

    int maxCameraX = COLS * tileWidth - screenWidth;
    if (cameraX < 0)
        cameraX = 0;
    if (cameraX > maxCameraX)
        cameraX = maxCameraX;
}

void playMusic(char *filename, int loop)
{
    if (!isMusicOn)
        return;
    if (currentMusic == NULL || strcmp(currentMusic, filename) != 0)
    {
        stopMusic();
        currentMusic = filename;
        PlaySoundA(filename, NULL, SND_ASYNC | SND_FILENAME | (loop ? SND_LOOP : 0));
    }
}

void stopMusic()
{
    PlaySound(NULL, NULL, 0);
    currentMusic = NULL;
}

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
        playMusic(menuMusic, 1);
        break;
    case PLAYING_STATE:
    case PAUSED_STATE:
        playMusic(gameMusic, 1);
        break;
    case CREDITS_STATE:
        playMusic(creditsMusic, 1);
        break;
    default:
        break;
    }
    currentGameState = newState;
}

void drawStartScreen();

void loadResources()

{
    iLoadImage(&icon, "char.png");
    iResizeImage(&icon, 180, 110);
    iLoadImage(&trapImg, "trap.png");
    iResizeImage(&trapImg, trapImg.width * 2, trapImg.height * 2); // Make trap larger
    trap_width = trapImg.width;
    trap_height = trapImg.height;
    iLoadFramesFromSheet(idleMonster, "IDLE.png", 1, 10);

    iLoadFramesFromSheet(walkMonster, "RUN.png", 1, 16);

    // iLoadFramesFromSheet(jumpMonster, "JUMP.png", 1, 8); // Make sure JUMP.png exists

    iLoadFramesFromSheet(attackMonster, "ATTACK.png", 1, 7);

    iLoadFramesFromSheet(demonFrames, "demon.png", 1, 12);
    iLoadFramesFromSheet(hurtMonster, "HURT.png", 1, 4); // Load hurt frames
    // Load dragon (flying enemy) frames
    iLoadFramesFromSheet(dragonFrames, "dragon.png", 1, 3);
    iLoadImage(&bg, "BG.bmp");
    // printf("Loaded BG.bmp: %dx%d\n", bg.width, bg.height);
    iLoadImage(&bg2, "BG1.bmp");
    // printf("Loaded BG1.bmp: %dx%d\n", bg2.width, bg2.height);
    // iResizeImage(&bg2, 800, 500);
    iLoadImage(&bg3, "BG3.bmp");
    // printf("Loaded BG3.bmp: %dx%d\n", bg3.width, bg3.height);

    iLoadImage(&rockEasy, "rock0.png");

    iLoadImage(&rockMedium, "rock1.png");

    iLoadImage(&rockHard, "rock2.png");

    // Default stone image for compatibility
    // iLoadImage(&stone, "stone.PNG");
    iResizeImage(&rockEasy, rockEasy.width / 2, rockEasy.height / 1.5);

    iResizeImage(&rockMedium, rockMedium.width / 2, rockMedium.height / 1.5);

    iResizeImage(&rockHard, rockHard.width / 2, rockHard.height * 0.75);

    iLoadImage(&startScreenBgImg, startScreenBg);

    iLoadImage(&slimeImg, "Slime.png");

    iResizeImage(&slimeImg, slimeImg.width * 4, slimeImg.height * 4);
    iInitSprite(&monster, -1);
    iChangeSpriteFrames(&monster, idleMonster, 10);
    iSetSpritePosition(&monster, 20, 0);

    iScaleSprite(&monster, 3);

    iInitSprite(&demon, -1);
    iChangeSpriteFrames(&demon, demonFrames, 12);
    iSetSpritePosition(&demon, 900, 45); // Start off-screen right
    iScaleSprite(&demon, 2);

    // Initialize dragon sprite
    iInitSprite(&dragon, -1);
    iChangeSpriteFrames(&dragon, dragonFrames, 3);
    iSetSpritePosition(&dragon, 900, dragonY);
    iScaleSprite(&dragon, 2);
}
// Demon state variables must be global for linker
int demonActive = 1;
int demonVanishTimer = 0;

void updateMonster()
{
    // Dragon (flying enemy) logic: flies left, resets if off screen or hit
    if (dragonActive)
    {
        iAnimateSprite(&dragon);
        dragon.x -= dragonSpeed;
        if (dragon.x < -100)
        {
            dragon.x = 900 + rand() % 200;
            dragonActive = 1;
            dragonVanishTimer = 0;
            iSetSpritePosition(&dragon, dragon.x, dragonY);
        }
        // Collision with player (flying, so check y overlap as well)
        int dragon_left = dragon.x;
        int dragon_right = dragon.x + 80;
        int player_left = sprite_x;
        int player_right = sprite_x + monsterWidth;
        int dragon_top = dragon.y + 60;
        int dragon_bottom = dragon.y;
        int player_top = monster.y + monsterWidth;
        int player_bottom = monster.y;
        bool horizontal_overlap = dragon_right > player_left && dragon_left < player_right;
        bool vertical_overlap = dragon_top > player_bottom && dragon_bottom < player_top;
        bool dragon_collision = horizontal_overlap && vertical_overlap;
        if (dragon_collision && dragonVanishTimer == 0)
        {
            if (attackWindow > 0 && attack > 0)
            {
                dragonVanishTimer = 20; // Dragon vanishes for 20 frames
                dragonActive = 0;
                dragon.x = 900 + rand() % 200;
                iSetSpritePosition(&dragon, dragon.x, dragonY);
                score += 40;
                attack--;
                attackWindow = 0;
            }
            else
            {
                lives--;
                iPlaySound(hitSound, 0);
                dragonVanishTimer = 20;
                dragonActive = 0;
                dragon.x = 900 + rand() % 200;
                iSetSpritePosition(&dragon, dragon.x, dragonY);
                sprite_x = 43;
                monster.y = 0;
                iSetSpritePosition(&monster, sprite_x, monster.y);
                animState = HURT_ANIM;
                iChangeSpriteFrames(&monster, hurtMonster, 4);
                if (lives <= 0)
                {
                    currentGameState = DEATH_MESSAGE_STATE;
                    return;
                }
            }
        }
    }
    else if (dragonVanishTimer > 0)
    {
        dragonVanishTimer--;
        if (dragonVanishTimer == 0)
        {
            dragonActive = 1;
            dragon.x = 900 + rand() % 200;
            iSetSpritePosition(&dragon, dragon.x, dragonY);
        }
    }

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
    case HURT_ANIM:
        iAnimateSprite(&monster);
        static int hurtFrame = 0;
        hurtFrame++;
        if (hurtFrame >= 8) // Show hurt for 8 frames
        {
            animState = WALK_ANIM;
            iChangeSpriteFrames(&monster, walkMonster, 10);
            hurtFrame = 0;
        }
        break;
    case JUMP_ANIM:
        // No special handling needed
        break;
    }
    // Decrement attack window if active
    if (attackWindow > 0)
        attackWindow--;
    if (animState == JUMP_ANIM || monster.y > 0)
    {
        // Floatier jump: increased initial velocity, gentler gravity
        monster.y += MonstervelocityY;
        MonstervelocityY -= 4; // gentler gravity for floatier jump
        if (monster.y <= 0)
        {
            monster.y = 0;
            MonstervelocityY = 0;
            animState = WALK_ANIM;
            iChangeSpriteFrames(&monster, walkMonster, 10);
            jump = false;
        }
        iAnimateSprite(&monster);
    }

    // Demon logic: walks left, resets if off screen or killed
    if (demonActive)
    {
        iAnimateSprite(&demon);
        demon.x -= 8; // Demon speed increased
        if (demon.x < -100)
        {
            demon.x = 900 + rand() % 200;
            demonActive = 1;
            demonVanishTimer = 0;
            iSetSpritePosition(&demon, demon.x, demon.y);
        }
        // Collision with player
        int demon_left = demon.x;
        int demon_right = demon.x + 80;
        int player_left = sprite_x;
        int player_right = sprite_x + monsterWidth;
        bool is_on_ground = (monster.y == 0);
        bool demon_collision = is_on_ground && demon_right > player_left && demon_left < player_right && abs(demon.y - monster.y) < 60;
        if (demon_collision && demonVanishTimer == 0)
        {
            if (attackWindow > 0 && attack > 0)
            {
                demonVanishTimer = 20; // Demon vanishes for 20 frames
                demonActive = 0;
                demon.x = 900 + rand() % 200;
                iSetSpritePosition(&demon, demon.x, demon.y);
                score += 30;
                attack--;
                attackWindow = 0;
            }
            else
            {
                lives--;
                iPlaySound(hitSound, 0);
                demonVanishTimer = 20;
                demonActive = 0;
                demon.x = 900 + rand() % 200;
                iSetSpritePosition(&demon, demon.x, demon.y);
                sprite_x = 43;
                monster.y = 0;
                iSetSpritePosition(&monster, sprite_x, monster.y);
                animState = HURT_ANIM;
                iChangeSpriteFrames(&monster, hurtMonster, 4);
                if (lives <= 0)
                {
                    currentGameState = DEATH_MESSAGE_STATE;
                    return;
                }
            }
        }
    }
    else if (demonVanishTimer > 0)
    {
        demonVanishTimer--;
        if (demonVanishTimer == 0)
        {
            demonActive = 1;
            demon.x = 900 + rand() % 200;
            iSetSpritePosition(&demon, demon.x, demon.y);
        }
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
    // Set difficulty defaults
    if (difficultyLevel == 1)
    {
        lives = 5; // Easy
        speed = 3;
        stone_y = 30;
        rockAmount = 1;
        jump_peak = 40; // Much higher jump for easy mode
    }
    else if (difficultyLevel == 2)
    {
        lives = 4; // Medium
        speed = 5;
        stone_y = 30;
        rockAmount = 1;
        jump_peak = 30; // Much higher jump for medium mode
    }
    else if (difficultyLevel == 3)
    {
        lives = 3; // Hard
        speed = 7;
        stone_y = 30;
        rockAmount = 1;
        jump_peak = 25; // Much higher jump for hard mode
    }
    attack = 3; // Reset attack to max
    // Randomize first obstacle type
    obstacleType = rand() % 2;
}

void drawStartScreen()
{
    iShowLoadedImage(0, 0, &startScreenBgImg);
    iSetColor(255, 255, 255); // White text
    iText(250, 50, "Press SPACE to Play Game", GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawGameScreen()
{

    cameraX = 0;
    // Smooth speed increase, capped at a max value
    float baseSpeed, speedFactor, maxSpeed;
    if (difficultyLevel == 1)
    {
        baseSpeed = 3.0f;
        speedFactor = 0.01f;
        maxSpeed = 7.0f;
    }
    else if (difficultyLevel == 2)
    {
        baseSpeed = 5.0f;
        speedFactor = 0.015f;
        maxSpeed = 9.0f;
    }
    else
    {
        baseSpeed = 7.0f;
        speedFactor = 0.02f;
        maxSpeed = 12.0f;
    }
    speed = baseSpeed + speedFactor * score;
    if (speed > maxSpeed)
        speed = maxSpeed;
    // drawTiles();
    iSetColor(0, 0, 0);
    // Select background based on difficulty
    if (difficultyLevel == 1)
        iShowLoadedImage(0, -20, &bg);
    else if (difficultyLevel == 2)
        iShowLoadedImage(0, -20, &bg2);
    else if (difficultyLevel == 3)
        iShowLoadedImage(0, -20, &bg3);
    iFilledRectangle(-cameraX, 0, 800, 50);
    // Draw trap.png randomly like stone
    if (trap_active == 1)
    {
        iShowLoadedImage(trap_x - cameraX, 45, &trapImg);
        trap_x -= speed;
        // Trap collision logic
        int monster_left = sprite_x;
        int monster_right = sprite_x + monsterWidth;
        int trap_left = trap_x;
        int trap_right = trap_x + trap_width;
        bool is_on_ground = (monster.y == 0);
        bool trap_collision = is_on_ground && (monster_right > trap_left && monster_left < trap_right) && (monster.y <= trap_y + trap_height);
        if (trap_collision)
        {
            lives--;
            iPlaySound(hitSound, 0);
            trap_active = 0;
            trap_x = 900 + rand() % 200;
            // Hurt animation
            sprite_x = 43;
            monster.y = 0;
            iSetSpritePosition(&monster, sprite_x, monster.y);
            animState = HURT_ANIM;
            iChangeSpriteFrames(&monster, hurtMonster, 4);
            if (lives <= 0)
            {
                currentGameState = DEATH_MESSAGE_STATE;
                return;
            }
        }
        if (trap_x < -trap_width)
        {
            trap_x = 900 + rand() % 200;
            trap_active = 1;
        }
    }
    else
    {
        trap_active = 1;
    }
    iSetSpritePosition(&monster, sprite_x, monster.y);
    iShowSprite(&monster);
    // Draw demon if active
    extern int demonActive;
    if (demonActive)
    {
        iSetSpritePosition(&demon, demon.x, demon.y);
        iShowSprite(&demon);
    }
    // Draw dragon (flying enemy) if active
    if (dragonActive)
    {
        iSetSpritePosition(&dragon, dragon.x, dragonY);
        iShowSprite(&dragon);
    }
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemyActive[i])
        {
            iSetSpritePosition(&enemies[i], enemies[i].x, enemies[i].y);
            iShowSprite(&enemies[i]);
        }
    }
    // (Remove this block, logic will be handled in collision below)
    // Draw stones based on rockAmount
    for (int r = 0; r < rockAmount; r++)
    {
        int rx = stone_x - cameraX - r * 200; // space rocks apart
        if (obstacleType == 0)
        {
            // Rock
            if (difficultyLevel == 1)
                iShowLoadedImage(rx, 50, &rockEasy);
            else if (difficultyLevel == 2)
                iShowLoadedImage(rx, 50, &rockMedium);
            else if (difficultyLevel == 3)
                iShowLoadedImage(rx, 50, &rockHard);
        }
        else
        {
            // Slime
            iShowLoadedImage(rx, 50, &slimeImg);
        }
        // Move obstacle to the left (simulate world movement)
        if (stone_active == 1)
        {
            stone_x -= speed;
            // Restore previous collision logic: rocks use central 25%, slimes use full bounding box
            int monster_left = sprite_x;
            int monster_right = sprite_x + monsterWidth;
            int monster_top = monster.y + monsterWidth;
            int monster_bottom = monster.y;
            int obs_left, obs_right, obs_top, obs_bottom;
            int ry = 50; // obstacle y position (fixed)
            if (obstacleType == 0)
            {
                // Rock: central part of stone (middle 25%)
                obs_left = rx + stone_width * 3 / 8;
                obs_right = rx + stone_width * 5 / 8;
                obs_top = ry + stone_height;
                obs_bottom = ry;
            }
            else
            {
                // Slime: use full bounding box
                obs_left = rx;
                obs_right = rx + slimeImg.width;
                obs_top = ry + slimeImg.height;
                obs_bottom = ry;
            }
            // Only check collision if monster is on the ground
            bool is_on_ground = (monster.y == 0);
            bool horizontal_overlap = monster_right > obs_left && monster_left < obs_right;
            bool collision = is_on_ground && horizontal_overlap;
            if (collision)
            {
                bool lost_life = false;
                if (obstacleType == 0) // Rock/stone: not attackable
                {
                    // Always lose life on collision with rock
                    lives--;
                    lost_life = true;
                }
                else // Slime: attackable
                {
                    // If attacking, remove slime and prevent collision
                    if (attackWindow > 0 && attack > 0)
                    {
                        stone_active = 0;
                        stone_x = 700 + rand() % 200; // Respawn obstacle
                        obstacleType = rand() % 2;    // Randomize next obstacle
                        score += 15;
                        attack--;
                        attackWindow = 0;
                        // Do not lose life, do not trigger hurt animation
                        return;
                    }
                    else
                    {
                        // Not attacking: lose life
                        lives--;
                        lost_life = true;
                    }
                }
                if (lost_life)
                {
                    stone_active = 0;
                    stone_x = 700 + rand() % 200; // Respawn obstacle
                    obstacleType = rand() % 2;    // Randomize next obstacle
                    // Reset monster position to initial area
                    sprite_x = 43;
                    monster.y = 0;
                    iSetSpritePosition(&monster, sprite_x, monster.y);
                    // Switch to hurt animation
                    animState = HURT_ANIM;
                    iChangeSpriteFrames(&monster, hurtMonster, 4);
                    if (lives <= 0)
                    {
                        currentGameState = DEATH_MESSAGE_STATE;
                        return;
                    }
                }
            }
            // If obstacle goes off screen, respawn
            if (stone_x < -stone_width)
            {
                stone_x = 700 + rand() % 200; // 700 to 900
                stone_active = 1;
                obstacleType = rand() % 2; // Randomize next obstacle
            }
        }
        else
        {
            stone_active = 1;
        }
    }

    iShowLoadedImage(620, 390, &icon);
    iSetColor(255, 255, 255);
    char scoreText[32];
    sprintf(scoreText, "%d", score);
    iText(650, 416, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
    char livesText[32];
    sprintf(livesText, "%d", lives);
    iText(680, 454, livesText, GLUT_BITMAP_TIMES_ROMAN_24);

    float bgSpeed = speed;
    if (difficultyLevel == 1)
    {
        iWrapImage(&bg, -bgSpeed);
    }
    else if (difficultyLevel == 2)
    {
        iWrapImage(&bg2, -bgSpeed);
    }
    else if (difficultyLevel == 3)
    {
        iWrapImage(&bg3, -bgSpeed);
    }
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
        iShowImage(0, 0, "name.bmp");
        iText(350, 230, playerName, GLUT_BITMAP_HELVETICA_18);
        // iText(300, 20, "Press ENTER to continue", GLUT_BITMAP_9_BY_15);
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
        iShowImage(0, 0, "difficulty.bmp");
        // iSetColor(0, 0, 0);
        // iFilledRectangle(0, 0, windowedWidth, windowedHeight);
        // iSetColor(255, 255, 255);
        // iText(300, 350, "Select Difficulty", GLUT_BITMAP_TIMES_ROMAN_24);
        // iSetColor(0, 255, 0);
        // iText(300, 280, "Easy", GLUT_BITMAP_HELVETICA_18);
        // iSetColor(255, 255, 0);
        // iText(300, 240, "Medium", GLUT_BITMAP_HELVETICA_18);
        // iSetColor(255, 0, 0);
        // iText(300, 200, "Hard", GLUT_BITMAP_HELVETICA_18);
        break;
    case CREDITS_STATE:
        iShowImage(0, 0, credits);
        break;
    case HALLOFFAME_STATE:
        iShowImage(0, 0, "BG4.bmp");
        loadHighScores();
        iSetColor(0, 0, 0);
        iText(300, 400, "Hall of Fame - Top 3", GLUT_BITMAP_TIMES_ROMAN_24);
        iSetColor(0, 0, 0);
        for (int i = 0; i < 3 && i < highScoreCount; i++)
        {
            char entry[64];
            sprintf(entry, "%d. %s - %d", i + 1, highScores[i].name, highScores[i].score);
            iText(300, 360 - i * 40, entry, GLUT_BITMAP_HELVETICA_18);
        }
        if (highScoreCount == 0)
        {
            iText(320, 320, "No scores yet!", GLUT_BITMAP_HELVETICA_18);
        }
        break;
    case DEATH_MESSAGE_STATE:
        iClear();
        iShowImage(0, 0, "deathm.bmp");
        break;
    case GAME_OVER_STATE:
        iClear();
        iShowImage(0, 0, death); // Show 800x500 death.bmp image
        // Show player name and score in front of death.bmp
        iSetColor(255, 255, 255); // Black text for visibility
        char nameText[64];
        sprintf(nameText, "%s", playerName);
        iText(420, 160, nameText, GLUT_BITMAP_TIMES_ROMAN_24);
        char scoreText[64];
        sprintf(scoreText, "%d", score);
        iText(400, 115, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
        updateHighScores(playerName, score);
        break;
    case PAUSED_STATE:
        iClear();
        iShowImage(0, 0, "pos.bmp"); // Show only the 800x500 pos.bmp image
        break;
    case EXIT_STATE:
        exit(0);
        break;
    case SAVE_PROMPT_STATE:
        drawSavePrompt();
        break;
    case CONTINUE_PROMPT_STATE:
        drawContinuePrompt();
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
                checkSaveGame();
                if (hasSaveGame)
                {
                    currentGameState = CONTINUE_PROMPT_STATE;
                }
                else
                {
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    currentGameState = NAME_INPUT_STATE;
                }
            }
            else
            {
                currentGameState = btn.targetState;
            }
            return;
        }
    }
    // Difficulty selection
    if (currentGameState == DIFFICULTY_STATE)
    {
        if (mx >= 280 && mx <= 520 && my >= 235 && my <= 290)
        {
            difficultyLevel = 1;
            nameCharIndex = 0;
            // Do NOT reset playerName here, keep last entered name
            resetGameState();
            currentGameState = NAME_INPUT_STATE;
        }
        else if (mx >= 280 && mx <= 520 && my >= 165 && my <= 225)
        {
            difficultyLevel = 2;
            nameCharIndex = 0;
            playerName[0] = '\0';
            resetGameState();
            currentGameState = NAME_INPUT_STATE;
        }
        else if (mx >= 280 && mx <= 520 && my >= 100 && my <= 155)
        {
            difficultyLevel = 3;
            nameCharIndex = 0;
            playerName[0] = '\0';
            resetGameState();
            currentGameState = NAME_INPUT_STATE;
        }
    }
    // Retry from GAME_OVER_STATE
    if (currentGameState == GAME_OVER_STATE)
    {
        if (mx >= 75 && mx <= 250 && my >= 10 && my <= 50)
        {
            nameCharIndex = 0;
            playerName[0] = '\0';
            resetGameState();
            currentGameState = NAME_INPUT_STATE;
        }
    }
}

void handlePlayerMovement(unsigned char key)
{
    // Left and right arrow key input disabled
}

void iMouse(int button, int state, int mx, int my)
{
    // Any mouse click in death message state goes to game over
    if (currentGameState == DEATH_MESSAGE_STATE && state == GLUT_DOWN)
    {
        currentGameState = GAME_OVER_STATE;
        deleteSaveGame();
        return;
    }

    // Handle continue prompt
    if (currentGameState == CONTINUE_PROMPT_STATE && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        checkSaveGame();
        if (!hasSaveGame)
        {
            // If no save exists, skip prompt and go to name input
            nameCharIndex = 0;
            playerName[0] = '\0';
            currentGameState = NAME_INPUT_STATE;
            return;
        }
        // Yes: Resume previous game
        if (mx >= 325 && mx <= 400 && my >= 230 && my <= 250)
        {
            if (loadGame())
            {
                currentGameState = PLAYING_STATE;
            }
            else
            {
                // If load fails, go to name input
                nameCharIndex = 0;
                playerName[0] = '\0';
                currentGameState = NAME_INPUT_STATE;
            }
        }
        // No: Start new game
        else if (mx >= 330 && mx <= 400 && my >= 200 && my <= 220)
        {
            nameCharIndex = 0;
            playerName[0] = '\0';
            resetGameState();
            currentGameState = NAME_INPUT_STATE;
        }
        return;
    }
    if (currentGameState == SAVE_PROMPT_STATE && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (mx >= 375 && mx <= 400 && my >= 260 && my <= 280)
        {
            // Yes: save and go to menu
            saveGame();
            printf("Game saved successfully!\n");
            currentGameState = MENU_STATE;
        }
        else if (mx >= 375 && mx <= 400 && my >= 230 && my <= 250)
        {
            // No: just go to menu
            deleteSaveGame();
            // printf("Save deleted successfully!\n");
            currentGameState = MENU_STATE;
        }
        return;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (currentGameState == PLAYING_STATE)
        {
            if (attackWindow == 0)
            {
                animState = ATTACK_ANIM;
                iChangeSpriteFrames(&monster, attackMonster, 7);
                attackWindow = 8;          // Attack duration is 8 frames (more forgiving)
                iPlaySound(swordSound, 0); // Play sword attack sound
                // No attack decrement, unlimited attacks
            }
            // Enemy hit logic (does not consume attack)
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemyActive[i])
                {
                    int hitbox = 60;
                    if (abs(enemies[i].x - sprite_x) <= hitbox)
                    {
                        enemyActive[i] = false; // Make enemy disappear immediately
                        enemyVanishTimer[i] = 0;
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
                if (mx >= 280 && mx <= 520 && my >= 235 && my <= 290)
                {
                    difficultyLevel = 1;
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    resetGameState();
                    currentGameState = NAME_INPUT_STATE;
                }
                // Medium: x=300 to 380, y=240 to 260
                else if (mx >= 280 && mx <= 520 && my >= 165 && my <= 225)
                {
                    difficultyLevel = 2;
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    resetGameState();
                    currentGameState = NAME_INPUT_STATE;
                }
                // Hard: x=300 to 380, y=200 to 220
                else if (mx >= 280 && mx <= 520 && my >= 100 && my <= 155)
                {
                    difficultyLevel = 3;
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    resetGameState();
                    currentGameState = NAME_INPUT_STATE;
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
                if (mx >= 75 && mx <= 250 && my >= 10 && my <= 50)
                {
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    resetGameState();
                    currentGameState = PLAYING_STATE;
                }
                // Menu button area: x=320-20 to 320+60, y=180-10 to 180+20
                else if (mx >= 505 && mx <= 730 && my >= 10 && my <= 50)
                {
                    resetGameState();
                    currentGameState = MENU_STATE;
                }
                break;
            case PAUSED_STATE:
                // Resume button: x=350-20 to 350+80, y=280-10 to 280+20
                if (mx >= 0 && mx <= 250 && my >= 310 && my <= 370)
                {
                    changeGameState(PLAYING_STATE);
                }
                // Restart button: x=350-20 to 350+80, y=240-10 to 240+20
                else if (mx >= 0 && mx <= 240 && my >= 230 && my <= 275)
                {
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    resetGameState();
                    currentGameState = NAME_INPUT_STATE;
                }
                // Main Menu button: x=350-20 to 350+120, y=200-10 to 200+20
                else if (mx >= 0 && mx <= 290 && my >= 80 && my <= 130)
                {
                    // Instead of going directly to menu, show save prompt
                    currentGameState = SAVE_PROMPT_STATE;
                }
                // Difficulty button: x=350-20 to 350+120, y=160-10 to 160+20
                else if (mx >= 0 && mx <= 260 && my >= 150 && my <= 210)
                {
                    changeGameState(DIFFICULTY_STATE);
                }
                break;
            case START_SCREEN:
                // No action needed
                break;
            case NAME_INPUT_STATE:
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
            nameCharIndex = 0;
            // Do NOT reset playerName here, only reset game state
            resetGameState();
            currentGameState = PLAYING_STATE;
        }
        return;
    }
    if (currentGameState == DEATH_MESSAGE_STATE)
    {
        currentGameState = GAME_OVER_STATE;
        deleteSaveGame(); // Remove save after game over
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
            // Use jump_peak for jump height
            MonstervelocityY = jump_peak;
            iChangeSpriteFrames(&monster, jumpMonster, 8);
            iPlaySound(jumpSound, 0);
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
        isMusicOn = !isMusicOn;
        if (isMusicOn)
        {
            switch (currentGameState)
            {
            case MENU_STATE:
                playMusic(menuMusic, 1);
                break;
            case PLAYING_STATE:
            case PAUSED_STATE:
                playMusic(gameMusic, 1);
                break;
            case CREDITS_STATE:
                playMusic(creditsMusic, 1);
                break;
            default:
                break;
            }
        }
        else
        {
            stopMusic();
        }
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
            currentGameState = SAVE_PROMPT_STATE;
        }
        else
        {
            changeGameState(EXIT_STATE);
        }
        break;
        // (No case for SAVE_PROMPT_STATE here. drawSavePrompt is a function, not a case label or inline function.)
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

typedef struct
{
    int score, lives, sprite_x, monster_y, MonstervelocityY, attack, difficultyLevel, demon_x, demon_y, demonActive, demonVanishTimer, obstacleType, stone_x, stone_active, trap_x, trap_active;
    char playerName[32];
} SaveGameState;

void saveGame()
{
    FILE *f = fopen(saveFile, "wb");
    if (!f)
        return;
    SaveGameState s;
    s.score = score;
    s.lives = lives;
    s.sprite_x = sprite_x;
    s.monster_y = monster.y;
    s.MonstervelocityY = MonstervelocityY;
    s.attack = attack;
    s.difficultyLevel = difficultyLevel;
    s.demon_x = demon.x;
    s.demon_y = demon.y;
    s.demonActive = demonActive;
    s.demonVanishTimer = demonVanishTimer;
    s.obstacleType = obstacleType;
    s.stone_x = stone_x;
    s.stone_active = stone_active;
    s.trap_x = trap_x;
    s.trap_active = trap_active;
    strncpy(s.playerName, playerName, 32);
    fwrite(&s, sizeof(s), 1, f);
    fclose(f);
}

int loadGame()
{
    FILE *f = fopen(saveFile, "rb");
    if (!f)
        return 0;
    SaveGameState s;
    if (fread(&s, sizeof(s), 1, f) != 1)
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    score = s.score;
    lives = s.lives;
    sprite_x = s.sprite_x;
    monster.y = s.monster_y;
    MonstervelocityY = s.MonstervelocityY;
    attack = s.attack;
    difficultyLevel = s.difficultyLevel;
    demon.x = s.demon_x;
    demon.y = s.demon_y;
    demonActive = s.demonActive;
    demonVanishTimer = s.demonVanishTimer;
    obstacleType = s.obstacleType;
    stone_x = s.stone_x;
    stone_active = s.stone_active;
    trap_x = s.trap_x;
    trap_active = s.trap_active;
    strncpy(playerName, s.playerName, 32);
    return 1;
}

void checkSaveGame()
{
    FILE *f = fopen(saveFile, "rb");
    hasSaveGame = (f != NULL);
    if (f)
        fclose(f);
}

void deleteSaveGame()
{
    remove(saveFile);
    hasSaveGame = 0;
}

void drawSavePrompt()
{
    iClear();
    iSetColor(255, 255, 255);
    iFilledRectangle(250, 200, 300, 150);
    iSetColor(0, 0, 0);
    iText(270, 300, "Save game before exiting?", GLUT_BITMAP_TIMES_ROMAN_24);
    iText(370, 260, "Yes", GLUT_BITMAP_HELVETICA_18);
    iText(375, 230, "No", GLUT_BITMAP_HELVETICA_18);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    iInitializeSound(); // Initialize the sound system before any sound is played
    loadResources();
    deleteSaveGame(); // Ensure no save exists at start
    iSetTimer(100, updateMonster);
    iSetTimer(1500, autoIncreaseScore); // Increase score every 1.5 seconds
    playMusic(menuMusic, 1);
    iInitialize(windowedWidth, windowedHeight, "Hellfire");
    return 0;
}
