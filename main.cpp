#include <iostream>
#include "raylib.h"
#include <deque>
#include <algorithm>
#include "raymath.h"
#include <random>

#include <vector>

using namespace std;

int bulletCount = 0;
int enemyKilled = 0;
bool proceedToNextLevel = false;
int items_picked = 0;

namespace Config {
    float BASE_WIDTH = 600;
    float BASE_HEIGHT = 400;
    string TITLE = "Swarm Shooter";
    float PLAYER_SCALE = 0.3f;
    float ENEMY_SCALE = 0.3f;
    float BULLET_SCALE = 0.3f;
    float ITEM_SCALE = 0.1f;
} ;

class Player {
public:
    Player(const Texture2D& playerTexture) {
        _texture = playerTexture;
        position = {Config::BASE_WIDTH / 2.0f , Config::BASE_HEIGHT / 2.0f};
    }
    void Draw() {
        if(!isKilled) {
            DrawTextureEx(_texture, position, angle, Config::PLAYER_SCALE, WHITE);
        }
    }
    Vector2 GetCenter() const {
        float width = _texture.width * Config::PLAYER_SCALE;
        float height = _texture.height * Config::PLAYER_SCALE;

        // The local center offset relative to top-left (position)
        Vector2 localCenter = { width / 2.0f, height / 2.0f };

        // Convert player's angle to radians
        float rad = angle * DEG2RAD;
        float cosA = cosf(rad);
        float sinA = sinf(rad);

        // Rotate the center offset around the player's position
        float rotatedX = localCenter.x * cosA - localCenter.y * sinA;
        float rotatedY = localCenter.x * sinA + localCenter.y * cosA;

        return {
            position.x + rotatedX,
            position.y + rotatedY
        };
    }
    float getAngle() {
        return angle;
    }
    void Move() {
       if(!isKilled) {
          Vector2 pos = {0.0f,0.0f};

        if (IsKeyDown(KEY_UP)) {
            pos.y -= 1.0f;
        }
        if (IsKeyDown(KEY_DOWN)) {
            pos.y += 1.0f;
        }
        if (IsKeyDown(KEY_LEFT)) {
            pos.x -= 1.0f;
        }
        if (IsKeyDown(KEY_RIGHT)) {
            pos.x += 1.0f;
        }

        if (Vector2Length(pos) > 0.0f) {
            pos = Vector2Normalize(pos); // normalized positions for smooth movement ensuring magnitude doesnt exceed unity
        }

        float dt = GetFrameTime();
        position.x += pos.x * speed * dt;
        position.y += pos.y * speed * dt;

        Vector2 mousePos = GetMousePosition();
        Vector2 toMouse = Vector2Subtract(mousePos,position);
        angle = atan2f(toMouse.y, toMouse.x) * RAD2DEG;
       }
    }

    void AreYouDead() {
        if(lives <= 0) {
            isKilled = true;
        }
    }
    Texture2D _texture;
    Vector2 position;
    float speed = 200.0f;
    float angle = 0.0f;
    int lives = 70; //hp
    float hitCoolDown = 0.0f; // cooldown timer for player hit
    float IMMUNITY_TIME = 1.0f; // 1 second of immunity after getting hit
    bool isKilled = false;
};

struct BulletInstance {
    Vector2 position;
    Vector2 velocity;
    bool active = false;
    Texture _bullet_tex;
    float angle;
    float hitCoolDown = 0.0f; // cooldown timer for player hit
    float IMMUNITY_TIME = 1.0f; // 1 second of immunity after getting hit
   

    BulletInstance(const Texture2D& bulletTexture, const Vector2& playerPosition) {
        _bullet_tex = bulletTexture;
        position = playerPosition;
    }

};
class Bullet {
public:

    void Load(BulletInstance instance) {
        for (int i = 0; i < totalbullet; i++) {
            bullets.emplace_back(BulletInstance(instance._bullet_tex, instance.position));
        }
    }
    void Draw() {

        for (int i = 0; i < bullets.size(); i++) {
            if (bullets[i].active) {
                DrawTextureEx(bullets[i]._bullet_tex, bullets[i].position, angle, Config::BULLET_SCALE,WHITE);
            }
        }
    }
    void Launch(Vector2 playerPosition) {
        if (IsKeyPressed(KEY_SPACE)) {
            for (int i = 0; i < bullets.size(); i++) {
              if(bullets[i].active) continue;

              if (!bullets[i].active && (totalbullet - bulletCount) > 0) {
                  Vector2 mousePos = GetMousePosition();

                  Vector2 direction = Vector2Subtract(mousePos, playerPosition);
                  direction = Vector2Normalize(direction);

                  bullets[i].velocity = Vector2Scale(direction, 800); //800 pixels per sec
                  bullets[i].position = playerPosition;
                  bullets[i].active = true;

                  if(bulletCount < totalbullet) bulletCount++;
                  else bulletCount = totalbullet;

                  Vector2 toMouse = Vector2Subtract(mousePos,bullets[i].position);
                  angle = atan2f(toMouse.y, toMouse.x) * RAD2DEG;
                  break;
              }
            }
        }

        for (int i = 0; i < bullets.size(); i++) {
            if (bullets[i].active) {
                float dt = GetFrameTime();
                bullets[i].position.x += bullets[i].velocity.x * dt;
                bullets[i].position.y += bullets[i].velocity.y * dt;
            }
        }
    }

    void DeactivateBullet(int mapWidth, int mapHeight) {
        for (int i = 0; i < bullets.size(); i++) {
            if(bullets[i].active) {
                if(bullets[i].position.x < 0 || bullets[i].position.x > mapWidth ||
                   bullets[i].position.y < 0 || bullets[i].position.y > mapHeight) {
                    bullets[i].active = false;
                }
            }
        }
    }

    void ReLoad() {
        if(IsKeyPressed(KEY_R)) {
            cout << "Reloading Bullets" << endl;
           if(items_picked > 0) {
                bulletCount = 0;
                items_picked--;
           }
        }
    }
    vector<BulletInstance> bullets;
    int totalbullet = 5;
private:
    float angle = 0.0f;

};

class CollisionMap {
public:
    CollisionMap(const Texture2D& map) {
        _texture_map = map;
    }
    void CheckWallCollision(Vector2& position, int width , int height, float scale) {
         float mapX = _texture_map.width - (width * scale);
         float mapY = _texture_map.height - (height * scale);

         if (position.x <= 0) position.x = 0;
         if (position.x > mapX) position.x = mapX;
         if (position.y <= 0) position.y = 0;
         if (position.y > mapY) position.y = mapY;
    }

    bool CheckCollision(Rectangle point1, Rectangle point2) {
        return CheckCollisionRecs(point1, point2);
    }
    Texture2D _texture_map;
};


struct EnemyInstance {
    Vector2 position = {100.0f, 100.0f};
    bool active = true;
    int currentFrame = 0;
    float angleDeg = 0.0f;
    float speed = 50.0f;
    float frameTimer;
    int totalFrames;
    Texture2D _enemy_texture;
    int total_enemy = 10;
    int live = 40; // hp

    EnemyInstance(const Texture& enemyTex, int total_frames) {
        _enemy_texture = enemyTex;
        totalFrames = total_frames;

        vector<int> xPool;
        vector<int> yPool;

        // Fill pools with multiples of 50
        for (int x = 50; x <= (int)GetScreenWidth() - 50; x += 50) xPool.push_back(x);
        for (int y = 50; y <= (int)GetScreenHeight() - 50; y += 50) yPool.push_back(y);

        // Shuffle both
        random_device rd;
        mt19937 g(rd());
        shuffle(xPool.begin(), xPool.end(), g);
        shuffle(yPool.begin(), yPool.end(), g);

       position = {xPool[0] * 2, yPool[1] * 2};
    }
};

class Enemy {
public:
    void Load(EnemyInstance instance) {
        _texture = instance._enemy_texture;     // keep the texture so we can spawn later
        speed = instance.speed;
        width = instance._enemy_texture.width;
        height = instance._enemy_texture.height;
        totalFrames = instance.totalFrames;
        // no spawning here anymore; ScatterEnemies does it
    }

    bool IsValidSpot(Vector2 p, Vector2 playerPos) {
        if (Vector2Distance(p, playerPos) < SAFE_RADIUS) return false;   // don't spawn on the player
        for (auto& e : enemies) {
            if (Vector2Distance(p, e.position) < MIN_SPACING) return false;
        }
        return true;
    }

    void ScatterEnemies(Vector2 playerPos, float mapW, float mapH) {
        enemies.clear();

        float frameW = width / totalFrames;     // one animation frame, not the whole sheet
        float frameH = height;

        for (int i = 0; i < total_enemy; i++) {
            for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
                // origin is {w,h} in Draw(), so the sprite extends up/left of position.
                // Keep position at least one sprite size inside the map.
                Vector2 candidate = {
                    (float)GetRandomValue((int)(frameW + EDGE_MARGIN), (int)(mapW - EDGE_MARGIN)),
                    (float)GetRandomValue((int)(frameH + EDGE_MARGIN), (int)(mapH - EDGE_MARGIN))
                };

                if (IsValidSpot(candidate, playerPos)) {
                    EnemyInstance e(_texture, (int)totalFrames);
                    e.position   = candidate;                        // overrides constructor's random spot
                    e.active     = true;
                    e.frameTimer = GetRandomValue(0, 100) / 1000.0f; // desync animations
                    e.currentFrame = GetRandomValue(0, (int)totalFrames - 1);
                    enemies.push_back(e);
                    break;
                }
            }
        }
    }
    void ResurrectEnemy(const Vector2 playerPos, float mapW, float mapH) {
        if(enemyKilled == total_enemy) {
            cout << "All Enemies Killed" << endl;
            float dt = GetFrameTime();
            if(respawn_time <= 0.0f) {
                cout << "Respawning Enemies" << endl;
                enemyKilled = 0;
                total_enemy += 5; // increase total enemy count with each level
                enemies.clear();
                ScatterEnemies(playerPos, mapW, mapH);
                level++;
                for(int i = 0; i < enemies.size(); i++) {
                    enemies[i].active = true;
                    enemies[i].live = 40 + (level * 5); // increase enemy hp with each level
                }
                proceedToNextLevel = false;
                respawn_time = 1.0f; // reset respawn timer
            }
            else {
                proceedToNextLevel = true;
                respawn_time -= dt;
                cout << "Respawn Time: " << respawn_time << endl;
            }
        }
    }
    void Draw() {
        for (int i = 0; i < enemies.size(); i++) {
            if (enemies[i].active) {
                float w = (width / totalFrames) ;
                float h = (height);

                Rectangle source = {
                    w * enemies[i].currentFrame , 0.0f , w, h
                };
                Rectangle dest = {
                    enemies[i].position.x, enemies[i].position.y , w, h
                };

                Vector2 origin = { w, h  };
                DrawTexturePro(enemies[i]._enemy_texture, source, dest, origin, enemies[i].angleDeg, WHITE);
            }
        }
    }
     void Move(const Vector2 playerPosition) {
        for (int i = 0; i < enemies.size(); i++) {
             if (enemies[i].active) {
                 //Vector2 enemy_direction = {0.0f, 0.0f};
                 float dt = GetFrameTime();

                 //Calculate angle so enemy looks at player
                 Vector2 toPlayer = Vector2Subtract(playerPosition, enemies[i].position);
                 float distance = Vector2Length(toPlayer);


                 if (distance > 0.0f) {
                     Vector2 enemy_direction = Vector2Normalize(toPlayer);
                     enemies[i].position.x += enemy_direction.x * speed * dt;
                     enemies[i].position.y += enemy_direction.y * speed * dt;
                 }

                 enemies[i].frameTimer += dt;
                 if (enemies[i].frameTimer >= 0.12f) {
                     enemies[i].frameTimer = 0.0f;
                     enemies[i].currentFrame = (enemies[i].currentFrame + 1) % enemies[i].totalFrames;
                 }



                 float angleRad = atan2f(toPlayer.y, toPlayer.x);
                 enemies[i].angleDeg = angleRad * RAD2DEG;


             }
        }
    }



    // ... Draw(), Move() unchanged ...

    vector<EnemyInstance> enemies;
    int total_enemy = 5;
    int level = 1;
    float respawn_time = 1.0f; // seconds
    float width, height, totalFrames, speed;

private:
    Texture2D _texture;
    const float SAFE_RADIUS  = 250.0f;   // no enemies this close to the player
    const float MIN_SPACING  = 60.0f;
    const int   EDGE_MARGIN  = 40;
    const int   MAX_ATTEMPTS = 30;
};

struct ItemInstance {
    Texture _texture;
    Vector2 position;
    bool active = true;

    ItemInstance(const Texture tex, const Vector2 pos) {
        _texture = tex;
        position = pos;
    }
};

class Item {
public:
    Item(const ItemInstance& item_instance) {
        _texture = item_instance._texture;
           
    }
    void Load() {
        vector<int> xPool;
        vector<int> yPool;

        // Fill pools with multiples of 50
        for (int x = 50; x <= (int)GetScreenWidth() - 50; x += 50) xPool.push_back(x);
        for (int y = 50; y <= (int)GetScreenHeight() - 50; y += 50) yPool.push_back(y);

        // Shuffle both
        random_device rd;
        mt19937 g(rd());
        shuffle(xPool.begin(), xPool.end(), g);
        shuffle(yPool.begin(), yPool.end(), g);

        for (int i = 0; i < num_of_items && i < xPool.size() && i < yPool.size(); i++) {
             Vector2 pos = {xPool[i] * 2,yPool[i] * 2};
            items.emplace_back(ItemInstance(_texture, pos));

        }
    }

    void Draw() {
        static Color colors[] = {GREEN , WHITE, RED, ORANGE, BLUE , YELLOW};
        int size = 6;
        int index = GetRandomValue(0, size-1);
        for(int i = 0; i < items.size() && i < size; i++) {
            if(items[i].active) {
                DrawTextureEx(items[i]._texture,items[i].position, 0.0f, Config::ITEM_SCALE, colors[i]);
            }
        }
    }


    vector<ItemInstance> items;
    Texture _texture;
    Color colors[6];
    int num_of_items = 50;
    vector<float> xPool;
};


struct HealthPotionInstance {
    Texture _texture;
    Vector2 position = {100.0f, 100.0f};
    bool active = false;

    HealthPotionInstance(const Texture tex, const Vector2 pos) {
        _texture = tex;
        position = pos;
    }
};

class HealthPotion {
public:
    HealthPotion(const HealthPotionInstance& potion_instance) {
        _texture = potion_instance._texture;
    }

    void Draw() {
        for (auto& p : potions) {
            if (p.active)
                DrawTextureEx(p._texture, p.position, 0.0f, Config::ITEM_SCALE, WHITE);
        }
    }

    bool IsValidSpot(Vector2 p, Vector2 playerPos) {
        if (Vector2Distance(p, playerPos) < 100.0f) return false;
        for (auto& other : potions) {
            if (Vector2Distance(p, other.position) < MIN_SPACING) return false;
        }
        return true;
    }

    // mapW / mapH = size of the world, not the window
    void ScatterPotions(int count, Vector2 playerPos, float mapW, float mapH) {
        potions.clear();

        float potW = _texture.width  * Config::ITEM_SCALE;
        float potH = _texture.height * Config::ITEM_SCALE;

        for (int i = 0; i < count && i < MAX_POTIONS; i++) {
            for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
                Vector2 candidate = {
                    (float)GetRandomValue(EDGE_MARGIN, (int)(mapW - potW) - EDGE_MARGIN),
                    (float)GetRandomValue(EDGE_MARGIN, (int)(mapH - potH) - EDGE_MARGIN)
                };
                if (IsValidSpot(candidate, playerPos)) {
                    HealthPotionInstance p(_texture, candidate);
                    p.active = true;                 // <-- the missing piece
                    potions.push_back(p);
                    break;
                }
            }
        }
    }

    // Returns HP gained this frame
    int CheckPickup(Rectangle playerRect) {
        int healed = 0;
        for (auto& p : potions) {
            if (!p.active) continue;
            Rectangle r = { p.position.x, p.position.y,
                            p._texture.width  * Config::ITEM_SCALE,
                            p._texture.height * Config::ITEM_SCALE };
            if (CheckCollisionRecs(playerRect, r)) {
                p.active = false;
                healed += 20;
            }
        }
        return healed;
    }

private:
    Texture _texture;
    vector<HealthPotionInstance> potions;
    const int   MAX_POTIONS  = 10;
    const float MIN_SPACING  = 60.0f;
    const int   EDGE_MARGIN  = 40;
    const int   MAX_ATTEMPTS = 30;
};

int main() {

    //Definition
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(Config::BASE_WIDTH, Config::BASE_HEIGHT, Config::TITLE.c_str());
    SetTargetFPS(120);

    RenderTexture canvas = LoadRenderTexture(Config::BASE_WIDTH, Config::BASE_HEIGHT);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    Texture wallTexture = LoadTexture("assets/images/Walls.png");
    Texture floorTexture = LoadTexture("assets/images/Floor.png");
    Texture shooter = LoadTexture("assets/images/survivor-idle_shotgun_0.png");
    Texture collisionMap = LoadTexture("assets/images/gameBgCollision.png");
    Texture bulletTexture = LoadTexture("assets/images/bullet.png");
    Texture enemyTexture = LoadTexture("assets/images/cockroach-move.png");
    Texture itemTexture = LoadTexture("assets/images/book1.png");
    Texture healthPotionTexture = LoadTexture("assets/images/healthPotion.png");

    Player player(shooter);
    CollisionMap map(collisionMap);

    Camera2D camera = {0};
    camera.target = player.position;
    camera.offset = {player.position.x, player.position.y};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Vector2 playerCenter = player.GetCenter();
    BulletInstance bullet_instance(bulletTexture,playerCenter);
    Bullet playerBullet;
    playerBullet.Load(bullet_instance);

    EnemyInstance enemy_instance(enemyTexture,8);
    Enemy enemy;
    enemy.Load(enemy_instance);
    enemy.ScatterEnemies( player.position,
                     (float)collisionMap.width, (float)collisionMap.height);


    ItemInstance item_instance(itemTexture, {0.0f, 0.0f});
    Item item(item_instance);
    item.Load();

    HealthPotionInstance potion_instance(healthPotionTexture, {0.0f, 0.0f});
    HealthPotion healthPotion(potion_instance);
    healthPotion.ScatterPotions(10, player.position,
                            (float)collisionMap.width, (float)collisionMap.height);

    //Game Loop
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        player.AreYouDead();

        //1 . Event Handling
        player.Move();
        playerBullet.ReLoad();


         for(auto &item : item.items) {
             if(map.CheckCollision(Rectangle{player.position.x, player.position.y, shooter.width * Config::PLAYER_SCALE ,shooter.height * Config::PLAYER_SCALE},
             Rectangle{item.position.x, item.position.y, itemTexture.width * Config::ITEM_SCALE, itemTexture.height * Config::ITEM_SCALE})) {
                 cout << "Item Picked Up" << endl;
                 item.active = false;
                 items_picked++;
                 break; // Exit the loop after picking up one item to avoid multiple pickups in a single frame
             }
         }

        //2. Updating Position
        camera.target = player.position;
        map.CheckWallCollision(player.position, shooter.width,shooter.height,Config::PLAYER_SCALE); // check player collision of wall and restrict movement
        for(auto& item : item.items) map.CheckWallCollision(item.position, itemTexture.width, itemTexture.height, Config::ITEM_SCALE); // check item restriction
        playerCenter = player.GetCenter();
        playerBullet.Launch(playerCenter);
        playerBullet.DeactivateBullet(collisionMap.width, collisionMap.height);

        enemy.Move(player.position);
        enemy.ResurrectEnemy(player.position,
                     (float)collisionMap.width, (float)collisionMap.height);


    

        

        for(auto &b : playerBullet.bullets) { //Check Bullet and Enemy Collision
                if(b.hitCoolDown > 0.0f) {
                    b.hitCoolDown -= dt;
                }

                if(!b.active) continue;
                for(auto &e: enemy.enemies) {
                    if(!e.active) continue;


                    float frameW = e._enemy_texture.width / (float)e.totalFrames;
                    float frameH = e._enemy_texture.height;
                    // Draw() uses origin {w,h}, so the sprite sits up/left of position
                    Rectangle enemyRect = { e.position.x - frameW, e.position.y - frameH, frameW, frameH };


                    if(b.hitCoolDown <= 0.0f && map.CheckCollision(Rectangle{b.position.x, b.position.y,bulletTexture.width * Config::BULLET_SCALE, 
                    bulletTexture.height * Config::BULLET_SCALE}, enemyRect)) {
                            cout << "Enemy Live Left" << e.live << endl;
                            b.active = false;

                            if(e.live > 0) e.live -= 5; // remoeve 5hp from enemy live for every bullet attack

                            if(e.live <= 0) {
                                enemyKilled++;
                                e.active = false;
                            }

                            b.hitCoolDown = b.IMMUNITY_TIME; // reset cooldown timer
                            
                            cout << "Enemy Killed" << endl;
                            
                            break;
                    }
                }
        }

        //Check Enemy tp Player Collision
        if(player.hitCoolDown > 0.0f) {
            player.hitCoolDown -= dt;
        }

        for(auto&e : enemy.enemies) {
            if(!e.active) continue;

            float frameW = e._enemy_texture.width / (float)e.totalFrames;
            float frameH = e._enemy_texture.height;
            // Draw() uses origin {w,h}, so the sprite sits up/left of position
            Rectangle enemyRect = { e.position.x - frameW, e.position.y - frameH, frameW, frameH };



            if(player.hitCoolDown <= 0.0f && map.CheckCollision(Rectangle{player.position.x, player.position.y, shooter.width * Config::PLAYER_SCALE ,shooter.height * Config::PLAYER_SCALE}, enemyRect)) {
                if(player.lives > 0) player.lives -= 7;
                player.hitCoolDown = player.IMMUNITY_TIME; // reset cooldown timer
                if(player.lives <= 0) {
                    cout << "Player Killed" << endl;
                }
                break;
            }
        }

       if(player.lives < 100) {
         player.lives += healthPotion.CheckPickup(Rectangle{
            player.position.x, player.position.y,
                shooter.width * Config::PLAYER_SCALE, shooter.height * Config::PLAYER_SCALE });
       }
        
        //3. Drawing Object
        BeginTextureMode(canvas);
        ClearBackground(BLACK);
            BeginMode2D(camera);
                DrawTextureEx(floorTexture,{0.0f,0.0f},0.0f,1.0f,WHITE);
                DrawTextureEx(wallTexture,{0.0f,0.0f},0.0f,1.0f,WHITE);
                player.Draw();
                playerBullet.Draw();
                enemy.Draw();
                healthPotion.Draw();
                item.Draw();
            EndMode2D();
            DrawRectangle(0,GetScreenHeight() - 20, 130, 30, BLUE);
            DrawText(TextFormat("BULLET COUNT: %i",bulletCount), 10,GetScreenHeight() - 15, 12, WHITE);
            DrawRectangle(140,GetScreenHeight() - 20, 80, 30, BLUE);
            int aliveCount = count_if(enemy.enemies.begin(), enemy.enemies.end(),
                          [](const EnemyInstance& e) { return e.active; });
             DrawText(TextFormat("Enemies %i",aliveCount), 150,GetScreenHeight() - 15, 12, WHITE);
             DrawRectangle(230,GetScreenHeight() - 20, 50, 30, BLUE);
            DrawText(TextFormat("HP : %i",player.lives), 230,GetScreenHeight() - 15, 12, WHITE); 
           

            if(playerBullet.totalbullet - bulletCount <= 20 && playerBullet.totalbullet - bulletCount > 0) {
                DrawText("LOW AMMO", GetScreenWidth() - 150, GetScreenHeight() - 15, 12, RED);
            }
            else if(playerBullet.totalbullet - bulletCount <= 0) {
                DrawText("OUT OF AMMO", GetScreenWidth() - 150, GetScreenHeight() - 15, 12, RED);
            }

            if(player.isKilled) {
                DrawText("GAME OVER", GetScreenWidth() / 2 - 50, GetScreenHeight() / 2, 20, RED);
            }
            if(proceedToNextLevel) {
                DrawText(TextFormat("Next Level: %i",enemy.level), GetScreenWidth() / 2 - 50, GetScreenHeight() / 2, 20, RED);
            }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
            float scale = std::min(
                GetScreenWidth() / Config::BASE_WIDTH, GetScreenHeight() / Config::BASE_HEIGHT
            );
            Rectangle source = {
                0.0f,0.0f, Config::BASE_WIDTH, -Config::BASE_HEIGHT
            };
            Rectangle dest = {
                ((float) GetScreenWidth() - (Config::BASE_WIDTH * scale)) * 0.5f,((float) GetScreenHeight() - (Config::BASE_HEIGHT * scale)) * 0.5f, Config::BASE_WIDTH * scale, Config::BASE_HEIGHT * scale
            };

            Vector2 origin = {
                0.0f,0.0f
            };
            DrawTexturePro(canvas.texture,source,dest,origin,0.0f,WHITE);
        EndDrawing();

    }

    
    UnloadTexture(floorTexture);
    UnloadTexture(wallTexture);
    UnloadTexture(shooter);
    UnloadTexture(collisionMap);
    UnloadTexture(bulletTexture);
    UnloadTexture(enemyTexture);
    UnloadTexture(itemTexture);
    UnloadRenderTexture(canvas);

    CloseWindow();

    return 0;
}