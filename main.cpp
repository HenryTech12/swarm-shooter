#include <iostream>
#include <raylib.h>
#include <deque>
#include <algorithm>
#include <raymath.h>

#include <vector>

using namespace std;

int bulletCount = 0;

namespace Config {
    float BASE_WIDTH = 600;
    float BASE_HEIGHT = 400;
    string TITLE = "Swarm Shooter";
    float PLAYER_SCALE = 0.3f;
    float ENEMY_SCALE = 0.3f;
    float BULLET_SCALE = 0.3f;
} ;

class Player {
public:
    Player(const Texture2D& playerTexture) {
        _texture = playerTexture;
        position = {Config::BASE_WIDTH / 2.0f , Config::BASE_HEIGHT / 2.0f};
    }
    void Draw() {
        DrawTextureEx(_texture,position, angle, Config::PLAYER_SCALE,WHITE);
    }
    void Move() {
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
    Texture2D _texture;
    Vector2 position;
    float speed = 200.0f;
    float angle = 0.0f;
};

struct BulletInstance {
    Vector2 position;
    Vector2 velocity;
    bool active = false;
    Texture _bullet_tex;
    float angle;

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
              if (!bullets[i].active) {
                  Vector2 mousePos = GetMousePosition();

                  Vector2 direction = Vector2Subtract(mousePos, playerPosition);
                  direction = Vector2Normalize(direction);

                  bullets[i].velocity = Vector2Scale(direction, 800); //800 pixels per sec
                  bullets[i].position = playerPosition;
                  bullets[i].active = true;

                  bulletCount++; //bullets used

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

    void Update(const Vector2 playerPosition) {
        for (int i = 0; i < bullets.size(); i++) {
            bullets[i].position = playerPosition;
        }
    }

private:
    int totalbullet = 100;
    vector<BulletInstance> bullets;
    float angle = 0.0f;
};

class CollisionMap {
public:
    CollisionMap(const Texture2D& map) {
        _texture_map = map;
    }
    void CheckWallCollision(Vector2& position, int width , int height, int scale) {
         float mapX = _texture_map.width - (width * scale);
         float mapY = _texture_map.height - (height * scale);

         if (position.x <= 0) position.x = 0;
         if (position.x > mapX) position.x = mapX;
         if (position.y <= 0) position.y = 0;
         if (position.y > mapY) position.y = mapY;
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
    int total_enemy = 1;

    EnemyInstance(const Texture& enemyTex, int total_frames) {
        _enemy_texture = enemyTex;
        totalFrames = total_frames;
    }
};

class Enemy {
public:
    void Load(EnemyInstance instance) {
        for (int i = 0; i < instance.total_enemy; i++) {
            enemies.emplace_back(EnemyInstance(instance._enemy_texture,instance.totalFrames));
        }
        speed = instance.speed;
        width = instance._enemy_texture.width;
        height = instance._enemy_texture.height;
        totalFrames = instance.totalFrames;
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


                 /*
                 if (playerPosition.x < enemies[i].position.x) {
                      enemy_direction.x -= 1.0f;
                 }
                 else {
                     enemy_direction.x += 1.0f;
                 }

                 if (playerPosition.y < enemies[i].position.y) {
                     enemy_direction.y -= 1.0f;
                 }
                 else {
                     enemy_direction.y += 1.0f;
                 }*/

                 //enemy_direction = Vector2Normalize(enemy_direction);

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
    vector<EnemyInstance> enemies;
    float width;
    float height;
    float totalFrames;
    float speed;
};
int main() {

    //Definition
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(Config::BASE_WIDTH, Config::BASE_HEIGHT, Config::TITLE.c_str());
    SetTargetFPS(120);

    RenderTexture canvas = LoadRenderTexture(Config::BASE_WIDTH, Config::BASE_HEIGHT);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    Texture wallTexture = LoadTexture("C:\\Users\\HomePC\\CLionProjects\\swarm-shooter\\assets\\images\\Walls.png");
    Texture floorTexture = LoadTexture("C:\\Users\\HomePC\\CLionProjects\\swarm-shooter\\assets\\images\\Floor.png");
    Texture shooter = LoadTexture("C:\\Users\\HomePC\\CLionProjects\\swarm-shooter\\assets\\images\\survivor-idle_shotgun_0.png");
    Texture collisionMap = LoadTexture("C:\\Users\\HomePC\\CLionProjects\\swarm-shooter\\assets\\images\\gameBgCollision.png");
    Texture bulletTexture = LoadTexture("C:\\Users\\HomePC\\CLionProjects\\swarm-shooter\\assets\\images\\bullet.png");
    Texture enemyTexture = LoadTexture("C:\\Users\\HomePC\\CLionProjects\\swarm-shooter\\assets\\images\\cockroach-move.png");

    Player player(shooter);
    CollisionMap map(collisionMap);

    Camera2D camera = {0};
    camera.target = player.position;
    camera.offset = {player.position.x, player.position.y};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Vector2 playerCenter = {
        player.position.x + ((shooter.width * Config::PLAYER_SCALE ) / 2.0f) + 20,
        player.position.y + ((shooter.height * Config::PLAYER_SCALE) / 2.0f) + 10
    };
    BulletInstance bullet_instance(bulletTexture,player.position);
    Bullet playerBullet;
    playerBullet.Load(bullet_instance);

    EnemyInstance enemy_instance(enemyTexture,8);
    Enemy enemy;
    enemy.Load(enemy_instance);


    //Game Loop
    while (!WindowShouldClose()) {
        //1 . Event Handling
        player.Move();

        //2. Updating Position
        camera.target = player.position;
        map.CheckWallCollision(player.position, shooter.width,shooter.height,Config::PLAYER_SCALE); // check player collision of wall and restrict movement
        playerCenter = {
            player.position.x - ((shooter.width * Config::PLAYER_SCALE ) / 2.0f),
            player.position.y - ((shooter.height * Config::PLAYER_SCALE) / 2.0f)
        };

        playerBullet.Launch(playerCenter);

        enemy.Move(player.position);

        //3. Drawing Object
        BeginTextureMode(canvas);
        ClearBackground(BLACK);
            BeginMode2D(camera);
                DrawTextureEx(floorTexture,{0.0f,0.0f},0.0f,1.0f,WHITE);
                DrawTextureEx(wallTexture,{0.0f,0.0f},0.0f,1.0f,WHITE);
                player.Draw();
                playerBullet.Draw();
                enemy.Draw();
            EndMode2D();
            DrawRectangle(0,GetScreenHeight() - 20, 40, 30, BLUE);
            DrawText(TextFormat("%i",bulletCount), 10,GetScreenHeight() - 15, 12, WHITE);
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
    UnloadRenderTexture(canvas);

    CloseWindow();

    return 0;
}