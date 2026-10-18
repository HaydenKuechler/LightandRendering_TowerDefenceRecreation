#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include <cassert>
#include <array>
#include <vector>
#include <algorithm>

constexpr float SCREEN_SIZE = 800;

constexpr int TILE_COUNT = 20;
constexpr float TILE_SIZE = SCREEN_SIZE / TILE_COUNT;

//Enemy constants - Provided for part 1 of lab 2
constexpr int ENEMY_COUNT = 10;
constexpr float ENEMY_MAX_HEALTH = 100.0f;
constexpr float ENEMY_SPEED = 100.0f;
constexpr float ENEMY_SPAWN_RATE = 1.0f;
constexpr float ENEMY_RADIUS = 20.0f;

// Turret constants - Provided for part 2 of lab 2
constexpr float TURRET_RANGE = 200.0f;
constexpr float TURRET_FIRE_RATE = 0.6f;
constexpr float BULLET_SPEED = 200.0f;
constexpr float BULLET_DAMAGE = 50.0f; //50 damage means 2 bullets to destroy an enemy
constexpr float BULLET_RADIUS = 5.0f;


enum TileType : int
{
    GRASS,      // Marks unoccupied space, can be overwritten 
    DIRT,       // Marks the path, cannot be overwritten
    WAYPOINT,   // Marks where the path turns, cannot be overwritten
    TURRET_TILE,
};

struct Cell
{
    int row;
    int col;
};

constexpr std::array<Cell, 4> DIRECTIONS{ Cell{ -1, 0 }, Cell{ 1, 0 }, Cell{ 0, -1 }, Cell{ 0, 1 } };

inline bool InBounds(Cell cell, int rows = TILE_COUNT, int cols = TILE_COUNT)
{
    return cell.col >= 0 && cell.col < cols && cell.row >= 0 && cell.row < rows;
}

void DrawTile(int row, int col, Color color)
{
    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

void DrawTile(int row, int col, int type)
{
    Color colors[4] =
    {
        GREEN,
        BEIGE,
        SKYBLUE,
        ORANGE
    };
    //assert(type >= 0 && type < COUNT);
    DrawTile(row, col, colors[type]);
}

Vector2 TileCenter(int row, int col)
{
    float x = col * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = row * TILE_SIZE + TILE_SIZE * 0.5f;
    return { x, y };
}

// Returns a collection of adjacent cells that match the search value.
std::vector<Cell> FloodFill(Cell start, int tiles[TILE_COUNT][TILE_COUNT], TileType searchValue)
{
    // "open" = "places we want to search", "closed" = "places we've already searched".
    std::vector<Cell> result;
    std::vector<Cell> open;
    bool closed[TILE_COUNT][TILE_COUNT]{};
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            // We don't want to search zero-tiles, so add them to closed!
            closed[row][col] = tiles[row][col] == 0;
        }
    }

    // Add the starting cell to the exploration queue & search till there's nothing left!
    open.push_back(start);
    while (!open.empty())
    {
        // Remove from queue and prevent revisiting
        Cell cell = open.back();
        open.pop_back();
        closed[cell.row][cell.col] = true;

        // Add to result if explored cell has the desired value
        if (tiles[cell.row][cell.col] == searchValue)
            result.push_back(cell);

        // Search neighbours
        for (Cell dir : DIRECTIONS)
        {
            Cell adj = { cell.row + dir.row, cell.col + dir.col };
            if (InBounds(adj) && !closed[adj.row][adj.col] && tiles[adj.row][adj.col] != 0)
                open.push_back(adj);
        }
    }

    return result;
}

struct Bullet
{
    Vector2 position{};
    Vector2 direction{};
    float speed = BULLET_SPEED;
    float time = 0.0f;
    float radius = BULLET_RADIUS;
    float damage = BULLET_DAMAGE;
    Color color = WHITE;
    bool destroy = false;
};

// Fill in for Lab 2
struct Enemy
{
    Cell cell;
    Vector2 position{};
    int nextWaypoint = 0;
    float health = ENEMY_MAX_HEALTH;
    bool destroy = false;
	int spawnRate = ENEMY_SPAWN_RATE;
};

// Fill in for Lab 2
struct Turret
{
    Cell cell;
    Vector2 position{};
    float fireRate = TURRET_FIRE_RATE;
	Enemy* target = nullptr;
};

int main()
{
    int tiles[TILE_COUNT][TILE_COUNT]
    {
        //col:0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    row:
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 }, // 0
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 1
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 2
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 3
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 4
            { 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 5
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 6
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // 7
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0 }, // 10
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 13
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 14
            { 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 15
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 16
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 17
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0 }, // 18
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // 19
    };
    
    std::vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);
   
    float bullet_time_current = 0.0f;
    float bullet_time_total = 0.25f;
	bool bullet_fired = false;
    std::vector<Bullet> bullets;
   
    std::vector<Turret> turrets; // Use in Assignment 1
    std::vector<Enemy> enemies;  // Use in Lab 2 and Assignment 1
	float enemy_spawn_timer = 0.0f;
	int enemiesSpawned = 0;


    int curr = 0;
    int next = curr + 1;

    Vector2 enemy_position = TileCenter(waypoints[curr].row, waypoints[curr].col);
    float enemy_speed = 250.0f;   // <-- 250 pixels per second
    float min_distance = enemy_speed / 60.0f;
    min_distance *= 1.1f;
    bool at_end = false;
    // Manual turret creation example
   // turrets.resize(3);
   // turrets[0].cell = { 3,  7  };
   // turrets[1].cell = { 10, 7  };
   // turrets[2].cell = { 15, 14 };

    // Search array for desired tiles (ie turrets)
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            if (tiles[row][col] == 3)
            {
                // Construct turret here
                Turret turret{};
                turret.position = TileCenter(row, col);
                turret.fireRate = TURRET_FIRE_RATE;
                turrets.push_back(turret);
            }
        }
    }

    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Tower Defense");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (enemies.empty() && enemiesSpawned < ENEMY_COUNT)
        {
            enemy_spawn_timer += dt;
            if (enemy_spawn_timer > ENEMY_SPAWN_RATE)
            {

                Enemy enemy{};
                enemy.cell = waypoints[0];
                enemy.position = TileCenter(enemy.cell.row, enemy.cell.col);
                enemy.nextWaypoint = 1;

                enemies.push_back(enemy);
                ++enemiesSpawned;

            }
        }
        // Enemy movement (need to loop through enemies vector to get marks on Lab 2 and Assignment 1)
        for (Enemy& enemy : enemies)
        {
			if (enemy.nextWaypoint >= waypoints.size())
			{
				enemy.destroy = true;
				continue;
			}

			Vector2 target = TileCenter(waypoints[enemy.nextWaypoint].row, waypoints[enemy.nextWaypoint].col
            );

			Vector2 offset = target - enemy.position;
			float distance = Vector2Length(offset);
			float step = enemy_speed * dt;

            if(distance <= step)
			{
				enemy.position = target;
				enemy.cell = waypoints[enemy.nextWaypoint];
				++enemy.nextWaypoint;
			}
			
            else
            {
				enemy.position += Vector2Scale(Vector2Normalize(offset), step);
            }
			enemy_spawn_timer += dt;

            // Lift this movement logic to the above loop to make a generalized enemy system
           
            if (!at_end)
            {
               Vector2 from = TileCenter(waypoints[curr].row, waypoints[curr].col);
               Vector2 to = TileCenter(waypoints[next].row, waypoints[next].col);
             Vector2 direction = Vector2Normalize(to - from);
               enemy_position += direction * enemy_speed * dt;

                // Tolorance depends on enemy speed
                if (CheckCollisionPointCircle(enemy_position, to, min_distance))
                {
                    enemy_position = to;

                    curr++;
                    next++;
                    at_end = curr == waypoints.size() - 1;
                }
            }
        }

        // Bullet creation
        bullet_time_current += dt;
        if (bullet_time_current >= bullet_time_total && IsKeyDown(KEY_SPACE))
        {
            bullet_time_current = 0.0f;

            Bullet bullet{};
            bullet.position = GetMousePosition();
            bullet.direction = Vector2Normalize(enemy_position - bullet.position);
            bullet.speed = BULLET_SPEED;
            bullet.radius = BULLET_RADIUS;
            bullet_fired = true;
			bullets.push_back(bullet);
        }

        // Acquire nearest enemy as target and shoot it if the turret is off-cooldown
        for (Turret& turret : turrets)
        {
            Vector2 turret_position = TileCenter(turret.cell.row, turret.cell.col);
            float max = FLT_MAX;
			Enemy* nearest_enemy = nullptr;
            for (Enemy& enemy : enemies)
            {
                if (enemy.health <= 0.0f)
                    continue;

                Vector2 enemy_position = TileCenter(enemy.cell.row, enemy.cell.col);
                float distance = Vector2Distance(turret_position, enemy_position);
                if (distance < max)
                {
                    max = distance;
					nearest_enemy = &enemy;
                    // Target nearest enemy
                }
               
            }
            
        }
    
        for (Bullet& bullet : bullets)
        {
            Vector2 bullet_velocity = bullet.direction * bullet.speed;
            bullet.position += bullet_velocity * dt;
            bullet.time += dt;

            // "Flag the current bullet to be destroyed if its been alive for 1 or more seconds"
            bullet.destroy |= bullet.time >= 1.0f;

            Rectangle screen_rec;
            screen_rec.x = 0.0f;
            screen_rec.y = 0.0f;
            screen_rec.width = GetScreenWidth();
            screen_rec.height = GetScreenHeight();

            bullet.destroy |= !CheckCollisionCircleRec(bullet.position, bullet.radius, screen_rec);
        }
        for (Bullet& bullet : bullets)
        {
            for (Enemy& enemy : enemies)
            {
                // Check if the current bullet is colliding with the current enemy
                // If so, damage the enemy
                // Do any on-bullet-enemy behaviour here such as playing sounds
            }

        }
        std::erase_if(bullets, [](Bullet& bullet)
        {
            return bullet.destroy;
        });

        std::erase_if(enemies, [](Enemy& enemy)
        {
            return enemy.destroy;
        });

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw entire grid
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }

        for (const Bullet& bullet : bullets)
        {
            DrawCircleV(bullet.position, bullet.radius, RED);
        }

        for (const Turret& turret : turrets)
        {
            DrawRectangleV(TileCenter(turret.cell.row, turret.cell.col), { TILE_SIZE, TILE_SIZE }, ORANGE);
        }

		for (const Enemy& enemy : enemies)
		{
			DrawCircleV(enemy.position, ENEMY_RADIUS, GOLD);
		}

       // DrawCircleV(enemy_position, 20.0f, GOLD);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
