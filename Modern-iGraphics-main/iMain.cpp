// Player name input
char playerName[32] = "";
int nameCharIndex = 0;

int isFullscreen = 0;
int windowedWidth = 800;
int windowedHeight = 500;
int windowedPosX = 100;
int windowedPosY = 100;

#define WIN32_LEAN_AND_MEAN
#include <glut.h>
#include "iGraphics.h"
#include "iSound.h"
#include <windows.h>
#include <mmsystem.h>
#include <string.h>

//
#define MAX_ENEMIES 10
#define ENEMY_VANISH_DELAY 5

Sprite enemies[MAX_ENEMIES];
bool enemyActive[MAX_ENEMIES];
int enemyVanishTimer[MAX_ENEMIES] = {0};
int enemySpawnTimer = 0;
int score = 0;
int lives = 3;
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

char startScreenBg[] = "start_bg.bmp";
char homemenu[] = "Menu.bmp";
//char bg[] = "BG.bmp";
Image bg;
Image death;
char control[] = "Controls.bmp";
char tile[] = "Tile.bmp";
//char death[] = "death.bmp";
char credits[] = "Credits.bmp";
char difficulty[] = "Difficulty.bmp";

char menuMusic[] = "Main.wav";
char gameMusic[] = "Game.wav";
char creditsMusic[] = "Credits.wav";
bool moveLeft = false;
bool moveRight = false;

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

Image idleMonster[4], walkMonster[6], jumpMonster[8], attackMonster[7], enemy[12];
Sprite monster, demon;

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
            PlaySound(TEXT(filename), NULL, SND_ASYNC | SND_FILENAME | (loop ? SND_LOOP : 0));
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
        iLoadFramesFromSheet(idleMonster, "IDLE.png", 1, 10);
        iLoadFramesFromSheet(walkMonster, "RUN.png", 1, 16);
        // iLoadFramesFromSheet(jumpMonster, "assets/images/sprites/1 Pink_Monster/Pink_Monster_Jump_8.png", 1, 8);
        iLoadFramesFromSheet(attackMonster, "ATTACK.png", 1, 7);
        iLoadFramesFromSheet(enemy, "Demon.png", 1, 12);

        iLoadImage(&bg, "BG.bmp");
        iLoadImage(&death, "death.bmp");
        iResizeImage(&death, 80, 50);
        iResizeImage(&bg, 800, 500);
        iInitSprite(&monster, -1);
        iChangeSpriteFrames(&monster, idleMonster, 10);
        iSetSpritePosition(&monster, 5, 400);
        iScaleSprite(&monster, 4);

        iInitSprite(&demon, -1);
        iChangeSpriteFrames(&demon, enemy, 12);
        iSetSpritePosition(&demon, 500, 25);
        iScaleSprite(&demon, 1);
    }
    void updateMonster()
    {
        switch (animState)
        {
        case IDLE_ANIM:
            // If left/right is held, move camera and player
            if (moveLeft) {
                if (direction == 1) {
                    iMirrorSprite(&monster, HORIZONTAL);
                    direction = -1;
                }
                sprite_x -= 5;
                if (sprite_x < 0) sprite_x = 0;
                cameraX -= 5;
                if (cameraX < 0) cameraX = 0;
                animState = WALK_ANIM;
                iChangeSpriteFrames(&monster, walkMonster, 10);
            } else if (moveRight) {
                if (direction == -1) {
                    iMirrorSprite(&monster, HORIZONTAL);
                    direction = 1;
                }
                sprite_x += 5;
                if (sprite_x > screenWidth - monsterWidth) sprite_x = screenWidth - monsterWidth;
                cameraX += 5;
                int maxCameraX = COLS * tileWidth - screenWidth;
                if (cameraX > maxCameraX) cameraX = maxCameraX;
                animState = WALK_ANIM;
                iChangeSpriteFrames(&monster, walkMonster, 10);
            }
            break;
        case WALK_ANIM:
            // Animate walk faster (call twice per update)
            iAnimateSprite(&monster);
            iAnimateSprite(&monster);
            // If no movement key is held, return to idle
            if (!moveLeft && !moveRight) {
                animState = IDLE_ANIM;
                iChangeSpriteFrames(&monster, idleMonster, 10);
            }
            break;
        case ATTACK_ANIM:
            // Animate attack faster (call twice per update)
            iAnimateSprite(&monster);
            iAnimateSprite(&monster);
            static int attackFrame = 0;
            attackFrame += 2; // Increase by 2 for faster attack
            if (attackFrame >= 7)
            {
                animState = IDLE_ANIM;
                iChangeSpriteFrames(&monster, idleMonster, 10);
                attackFrame = 0;
            }
            break;
        case JUMP_ANIM:
            // No-op for now, handled in physics below
            break;
        }
        if (animState == JUMP_ANIM || monster.y > 0)
        {
            monster.y += MonstervelocityY;
            MonstervelocityY -= 2;

            if (monster.y <= 0)
            {
                monster.y = 0;
                MonstervelocityY = 0;
                animState = IDLE_ANIM;
                iChangeSpriteFrames(&monster, idleMonster, 10);
                jump = false;
            }

            iAnimateSprite(&monster);
        }
    }

    //
    void spawnEnemy()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemyActive[i])
        {
            iInitSprite(&enemies[i], -1);
            iChangeSpriteFrames(&enemies[i], enemy, 12);
            iSetSpritePosition(&enemies[i], 500-i, 25);
            iScaleSprite(&enemies[i], 1.0);
            //enemies[i].x = -1;
            enemyActive[i] = true;
            break;
        }
    }
}
//
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
    }

    void drawStartScreen()
    {
        iShowImage(0, 0, startScreenBg);
        iSetColor(255, 255, 255); // White text
        iText(250, 50, "Press SPACE to Play Game", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    int speed=3;
    void drawGameScreen()
    {
    //iShowImage(0, 0, bg);
    iShowLoadedImage(0, 0, &bg);
    //iWrapImage(&bg, speed);
    if(moveRight)
    iWrapImage(&bg, -speed);
    else if(moveLeft)
    iWrapImage(&bg, speed);
    //cameraX = 0;
    //drawTiles();
    iSetColor(0, 0, 0);
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
    iSetColor(255, 255, 255);
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    iText(650, 450, scoreText, GLUT_BITMAP_TIMES_ROMAN_24);
    char livesText[32];
    sprintf(livesText, "Lives: %d", lives);
    iText(650, 420, livesText, GLUT_BITMAP_HELVETICA_18);
}

void updateEnemy()
{
    enemySpawnTimer++;
    if (enemySpawnTimer >= 30 + rand() % 20)
    {
        spawnEnemy();
        enemySpawnTimer = 0;
    }
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemyActive[i])
        {
            iAnimateSprite(&enemies[i]);
            enemies[i].x -= 2;
            // Fix: compare both in world coordinates
            int playerWorldX = sprite_x + cameraX;
            if (enemyVanishTimer[i] == 0 && abs(enemies[i].x - playerWorldX) < 40 && abs(enemies[i].y - monster.y) < 60) {
                lives--;
                if (lives <= 0) {
                    currentGameState = GAME_OVER_STATE;
                }
                enemyVanishTimer[i] = ENEMY_VANISH_DELAY; 
                enemyActive[i] = false;
            }
            if (enemyVanishTimer[i] > 0) {
                enemyVanishTimer[i]--;
                if (enemyVanishTimer[i] == 0) {
                    enemyActive[i] = false;
                }
            }
            if (enemies[i].x < -100)
            {
                enemyActive[i] = false;
                enemyVanishTimer[i] = 0;
            }
        }
    }
}


    void iDraw()
    {
        iClear();

        switch (currentGameState)
        {
        case START_SCREEN:
            drawStartScreen();
            break;
        case MENU_STATE:
            iShowImage(0, 0, homemenu);
            break;
        case NAME_INPUT_STATE:
            iClear();
            iSetColor(255, 0, 0);
            iFilledRectangle(0, 0, windowedWidth, windowedHeight);
            iSetColor(0, 0, 0);
            iText(250, 350, "Enter your name:", GLUT_BITMAP_TIMES_ROMAN_24);
            iText(450, 350, playerName, GLUT_BITMAP_9_BY_15);
            iText(300, 10, "Press ENTER to continue", GLUT_BITMAP_HELVETICA_18);
            break;
        case PLAYING_STATE:
            drawGameScreen();
            break;
        case CONTROL_STATE:
            iShowImage(0, 0, control);
            break;
        case DIFFICULTY_STATE:
            iShowImage(0, 0, difficulty);
            break;
        case CREDITS_STATE:
            iShowImage(0, 0, credits);
            break;
        case HALLOFFAME_STATE:
            //iShowImage(0, 0, &bg);
            break;
        case GAME_OVER_STATE: {
            iSetColor(0, 0, 0);
            iFilledRectangle(0, 0, 800, 500);

            // Center the death image at the top middle
            int deathImgWidth = 80;
            int deathImgHeight = 50;
            int deathImgX = (800 - deathImgWidth) / 2;
            int deathImgY = 430; // Top margin (adjust as needed)
            iShowLoadedImage(deathImgX, deathImgY, &death);

            // Show player's score and name on death screen
            iSetTransparentColor(255, 255, 255, 0.5);
            char deathScoreText[64];
            sprintf(deathScoreText, "Game Over!\n %s\n SCORE: %d", playerName, score);
            iText(200, 300, deathScoreText, GLUT_BITMAP_TIMES_ROMAN_24);
            iText(250, 250, "Click to return to menu", GLUT_BITMAP_HELVETICA_18);
        } break;
        case PAUSED_STATE:
            drawGameScreen();
            iSetColor(0, 0, 0);
            iFilledRectangle(0, 0, 800, 500);
            iSetColor(255, 255, 255);
            iText(350, 250, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
            break;
        case EXIT_STATE:
            exit(0);
            break;
        }
    }

    void handleMenuClick(int mx, int my)
    {
        for (int i = 0; i < sizeof(menuButtons) / sizeof(menuButtons[0]); i++)
        {
            Button btn = menuButtons[i];
            if (mx >= btn.x1 && mx <= btn.x2 && my >= btn.y1 && my <= btn.y2)
            {
                if (btn.targetState == PLAYING_STATE) {
                    // Show name input before starting game
                    nameCharIndex = 0;
                    playerName[0] = '\0';
                    currentGameState = NAME_INPUT_STATE;
                } else {
                    currentGameState = btn.targetState;
                }
                return;
            }
        }
    }

    void handlePlayerMovement(unsigned char key)
    {
        if (key == GLUT_KEY_LEFT)
        {
            if (direction == 1)
            {
                iMirrorSprite(&monster, HORIZONTAL);
                direction = -1;
            }
            // sprite_x -= 10;
            if (sprite_x > 0)
            {
                sprite_x -= 5;
                if (sprite_x < 0)
                    sprite_x = 0;
            }
            if (animState != WALK_ANIM)
            {
                animState = WALK_ANIM;
                iChangeSpriteFrames(&monster, walkMonster, 10);
            }
        }
        else if (key == GLUT_KEY_RIGHT)
        {
            if (direction == -1)
            {
                iMirrorSprite(&monster, HORIZONTAL);
                direction = 1;
            }
            if (sprite_x < screenWidth - monsterWidth)
            {
                sprite_x += 5;
                if (sprite_x > screenWidth - monsterWidth)
                    sprite_x = screenWidth - monsterWidth;
            }

            if (animState != WALK_ANIM)
            {
                animState = WALK_ANIM;
                iChangeSpriteFrames(&monster, walkMonster, 10);
            }
        }
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
                        if (abs(enemies[i].x - sprite_x) <= hitbox) {
                            enemyVanishTimer[i] = ENEMY_VANISH_DELAY;
                            score += 1;
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
                case DIFFICULTY_STATE:
                case CREDITS_STATE:
                case HALLOFFAME_STATE:
                    changeGameState(MENU_STATE);
                    break;
                case GAME_OVER_STATE:
                    resetGameState();
                    changeGameState(MENU_STATE);
                    break;
                case PAUSED_STATE:
                    changeGameState(PLAYING_STATE);
                    break;
                }
            }
        }
    }

    void iKeyboard(unsigned char key)
    {
        if (currentGameState == NAME_INPUT_STATE) {
            if ((key >= 32 && key <= 126) && nameCharIndex < 30) {
                playerName[nameCharIndex++] = key;
                playerName[nameCharIndex] = '\0';
            } else if (key == 8 && nameCharIndex > 0) { // Backspace
                nameCharIndex--;
                playerName[nameCharIndex] = '\0';
            } else if (key == 13 && nameCharIndex > 0) { // Enter
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
                MonstervelocityY = 18;
                iChangeSpriteFrames(&monster, jumpMonster, 8);
            }
            break;
        case 'f':
        case 'F':
            // Toggle fullscreen
            if (!isFullscreen) {
                windowedWidth = glutGet(GLUT_WINDOW_WIDTH);
                windowedHeight = glutGet(GLUT_WINDOW_HEIGHT);
                windowedPosX = glutGet(GLUT_WINDOW_X);
                windowedPosY = glutGet(GLUT_WINDOW_Y);
                glutFullScreen();
                isFullscreen = 1;
            } else {
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

            handlePlayerMovement(key);
            if (key == GLUT_KEY_LEFT)
            {
                moveLeft = true;
            }
            else moveLeft = false;

            if (key == GLUT_KEY_RIGHT)
            {
                moveRight = true;
            }
            else moveRight = false;
        }
    }
    void iSpecialKeyboardUp(unsigned char key)
    {
        if (currentGameState == PLAYING_STATE)
        {
            if (key == GLUT_KEY_LEFT)
            {
                moveLeft = false;
            }
            else if (key == GLUT_KEY_RIGHT)
            {
                moveRight = false;
            }
        }
    }

    void iMouseMove(int mx, int my) {}
    void iMouseDrag(int mx, int my) {}
    void iMouseWheel(int dir, int mx, int my) {}

    int main(int argc, char *argv[])
    {
    glutInit(&argc, argv);
    loadResources();
    iSetTimer(100, updateMonster);
    iSetTimer(100, updateEnemy);
    playMusic(menuMusic, 1);
    iInitialize(windowedWidth, windowedHeight, "Hellfire");
    return 0;
    }