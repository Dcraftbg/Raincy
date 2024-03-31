#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>


#include "temp.h"



//#define DA_ASSERT(x) if(!(x)) __debugbreak()
#include "darray.h"
#define OS_UTILS_IMPLEMENTATION
#include "os.h"
#define SCALAR 70
#define W_RATIO 16
#define H_RATIO 9
#define WIDTH  (W_RATIO * SCALAR)
#define HEIGHT (H_RATIO * SCALAR)
#define FPS 60

#define MAX_WATER_DROP_COUNT 600
#define MAX_HITBOXES 2048
#define MAX_WATER_PARTICLES MAX_WATER_DROP_COUNT*2
#define HITBOX_COLOR_DEBUG_ENABLED GetColor(0xb7ff807f)
#define HITBOX_COLOR_DEBUG_DISABLED GetColor(0xfa58557f)
typedef struct {
	Vector2 p1;
	Vector2 p2;
	Vector2 p3;
} Triangle;
void DrawTriangleTri(Triangle tri, Color color) {
	DrawTriangle(tri.p1, tri.p2, tri.p3, color);
}

typedef struct {
	Triangle tri;
	Color col;
} TriMeshElm;
typedef struct {
	darray_of(TriMeshElm) body;
} TriMesh;
void DrawTriMesh(TriMesh mesh) {
	for (size_t i = 0; i < mesh.body.len; ++i) {
		TraceLog(LOG_INFO, "(%zu) Color={.r=%d, .g=%d, .b=%d, .a=%d}", i, mesh.body.data[i].col.r, mesh.body.data[i].col.g, mesh.body.data[i].col.b, mesh.body.data[i].col.a);
		DrawTriangleTri(mesh.body.data[i].tri, mesh.body.data[i].col);
	}
}
void AddTriMesh(TriMesh* mesh, TriMeshElm elm) {
	darray_push(&mesh->body, elm);
}
void DropTriMesh(TriMesh* mesh) {
	darray_drop(&mesh->body);
}
typedef struct {
	Rectangle box;
	bool enabled;
} Hitbox;
Hitbox hitbox_new(Rectangle box) {
	return CLITERAL(Hitbox) { .box = box, .enabled=true};
}
darray_of(Hitbox) hitboxes;
size_t createHitbox(Hitbox box) {
	size_t id = hitboxes.len;
	if (id >= MAX_HITBOXES) {
		TraceLog(LOG_FATAL, "Reached limit of hitboxes! Shutting down\n");
		exit(1);
	}
	darray_push(&hitboxes, box);

	return id;
}
void updateBoxPos(size_t hitID, Vector2 pos) {
	assert(hitID < hitboxes.len);
	hitboxes.data[hitID].box.x = pos.x;
	hitboxes.data[hitID].box.y = pos.y;
}

typedef struct {
	Rectangle box;
	size_t hitId;
	Color color;
} Platform;
Platform createPlatform(Rectangle box, Color color) {
	return CLITERAL(Platform) { .box = box, .color = color, .hitId = createHitbox(hitbox_new(box)) };
}
void drawPlatform(Platform* p) {
	DrawRectangleRec(p->box, p->color);
}
void updatePlatformBox(Platform* p, Rectangle box) {
	p->box = box;
	hitboxes.data[p->hitId].box = box;
}


//bool CheckCollisionPointTriMesh(Vector2 p, TriMesh mesh) {
//	for (size_t i = 0; i < mesh.body.len; ++i) {
//		if (CheckCollisionPointTriangle(p, mesh.body.data[i].p1, mesh.body.data[i].p2, mesh.body.data[i].p3)) return true;
//	}
//	return false;
//}
//
//bool CheckCollisionRecTriMesh(Rectangle rec, TriMesh mesh) {
//	for (size_t i = 0; i < mesh.body.len; ++i) {
//		//CheckCollisionTrin
//		//if (CheckCollisionPointTriangle(p, mesh.body.data[i].p1, mesh.body.data[i].p2, mesh.body.data[i].p3)) return true;
//	}
//	return false;
//}
//


#define MAX_ANIMATION_WATER_DROP_FRAMES_COUNT 10
typedef struct {
	Rectangle box;
	Vector2 origin;
	Color color;
	size_t frames;
	bool rDirection;
	Hitbox* bTouch;
} WaterDropParticle;

typedef struct {
	Vector2 pos;
	float len;
	float orgLen;
	float mass;
	float acceleration;
	uint8_t alpha;
	bool enabled;
	bool hasHit;
} WaterDrop;



#define WATER_DROP_MASS_PER_PIXEL 0.05f

#define G 10
#define WATER_DROP_PARTICLE_OFFSET 1.0f
#define WATER_DROP_FROM_GROUND 3.0f
#define WATER_DROP_PARTICLE_WIDTH 5.0f
#define WATER_DROP_PARTICLE_ORIGIN_OFFSET 1.0f
#define WATER_DROP_PARTICLE_HEIGHT WATER_DROP_PARTICLE_WIDTH


WaterDrop water_drops[MAX_WATER_DROP_COUNT] = { 0 };
//WaterDropParticle water_drop_particles[MAX_WATER_DROP_COUNT * 2] = { 0 };
darray_of(WaterDropParticle) water_drop_particles = { 0 };




size_t water_drop_count = 0;
int miniScreenW=WIDTH;
int miniScreenH=HEIGHT;
// TODO: Maybe have water heaviness affect the simulation
#define WATER_DROP_COLOR GetColor(0xcbfcf7ff)
#define WATER_DROP_WIDTH 4

#define MIN_WATER_DROP_LEN 10
#define MAX_WATER_DROP_LEN 50
#define WATER_DROP_SPEED 0.05f
#define WATER_DROP_STARTING_ACCELERATION 0.5f
#define BLOCK_WIDTH 100
#define BLOCK_HEIGHT 100

static float rand_float() {
	return (float)rand() / RAND_MAX;
}
static uint8_t rand_u8() {
	return rand() % (UCHAR_MAX+1);
}


static uint8_t rand_u8_r(uint8_t min, uint8_t max) {
	
	return min + rand() % (max - min+1);
}

static Vector2 RandomScreenPos() {
	return CLITERAL(Vector2) { .x = rand_float() * GetScreenWidth(), .y = rand_float() * GetScreenHeight() };
}
static Vector2 RandomPointIn(Rectangle b) {
	return CLITERAL(Vector2) { .x = b.x + rand_float() * b.width, .y = b.y + rand_float() * b.height };
}
static Color RandomColor() {
	return CLITERAL(Color) { rand_u8(), rand_u8(), rand_u8(), 0xff };
}
void regen_water_drops(Rectangle b, size_t count) {
	water_drop_count = count;
	memset(water_drops, 0, sizeof(water_drops));
	for (size_t i = 0; i < count; ++i) {
		water_drops[i].pos = CLITERAL(Vector2) {.x=b.x + rand_float() * b.width, .y = b.y };
		water_drops[i].orgLen = MIN_WATER_DROP_LEN + rand_float() * (MAX_WATER_DROP_LEN-MIN_WATER_DROP_LEN);
		water_drops[i].len = water_drops[i].orgLen;
		water_drops[i].alpha = rand_u8_r(100, 255);
		water_drops[i].enabled = true;
		water_drops[i].mass = water_drops[i].len * WATER_DROP_MASS_PER_PIXEL * ((float)water_drops[i].alpha / 0xff) ;
		water_drops[i].acceleration = WATER_DROP_STARTING_ACCELERATION;
	}
}
void createWaterdropParticle(WaterDropParticle particle) {
	assert(water_drop_particles.len + 1 < MAX_WATER_PARTICLES);
	darray_push(&water_drop_particles, particle);
	//for (size_t i = 0; i < water_drop_particles.len; ++i) {
	//	if (!water_drop_particles.data[i].bTouch) {
	//		water_drop_particles.data[i] = particle;
	//		if(i == water_drop_particles.len) water_drop_particles.len++;
	//		break;
	//	}
	//}
}
void createWaterdropParticles(size_t dropIndex, size_t hitboxIndex) {
	assert(water_drop_particles.len + 2 < MAX_WATER_PARTICLES);
	Color color = WATER_DROP_COLOR;
	color.a = water_drops[dropIndex].alpha;
	WaterDropParticle par1 = {
		.box = {
			.x      = water_drops[dropIndex].pos.x - WATER_DROP_PARTICLE_OFFSET,
			.y      = hitboxes.data[hitboxIndex].box.y - WATER_DROP_FROM_GROUND,
			.width  = WATER_DROP_PARTICLE_WIDTH ,
			.height = WATER_DROP_PARTICLE_HEIGHT
		},
		.origin     = {.x = -WATER_DROP_PARTICLE_ORIGIN_OFFSET},
		.color      = color,
		.bTouch     = &hitboxes.data[hitboxIndex],
		.rDirection = false,
		.frames     = 0
	};
	WaterDropParticle par2 = {
		.box = {
			.x = water_drops[dropIndex].pos.x + WATER_DROP_PARTICLE_OFFSET,
			.y = hitboxes.data[hitboxIndex].box.y - WATER_DROP_FROM_GROUND,
			.width = WATER_DROP_PARTICLE_WIDTH,
			.height = WATER_DROP_PARTICLE_HEIGHT
		},
		.origin = {.x = WATER_DROP_WIDTH + WATER_DROP_PARTICLE_ORIGIN_OFFSET, 0},
		.color = color,
		.bTouch = &hitboxes.data[hitboxIndex],
		.rDirection=true,
		.frames=0
	};
	createWaterdropParticle(par1);
	createWaterdropParticle(par2);
}

static void apply_gravity_to_waterdrops(Rectangle b) {
	for(size_t i=0; i < water_drop_count; ++i) {
		if (water_drops[i].enabled) {
			float acceleration = water_drops[i].acceleration;
			water_drops[i].acceleration += (water_drops[i].mass * G) * WATER_DROP_SPEED;
			bool collsion = false;
			for (size_t j = 0; j < hitboxes.len; ++j) {
				if (!hitboxes.data[j].enabled) continue;
				if (CheckCollisionRecs(hitboxes.data[j].box, CLITERAL(Rectangle){.x = water_drops[i].pos.x, .y = water_drops[i].pos.y, .height = water_drops[i].len, .width = WATER_DROP_WIDTH})) {
					if (water_drops[i].len <= 0.0f) {
						water_drops[i].len = water_drops[i].orgLen;
						water_drops[i].pos.y = b.y + -water_drops[i].len;
						water_drops[i].acceleration = WATER_DROP_STARTING_ACCELERATION;
						water_drops[i].hasHit = false;
					}
					else {
						water_drops[i].len -= water_drops[i].acceleration;
						if (!water_drops[i].hasHit) {
							createWaterdropParticles(i, j);
							water_drops[i].hasHit = true;
						}
					}
					collsion = true;
					break;
				}
			}
			if (!collsion) {
				if (water_drops[i].pos.y > b.y + b.height) {
					water_drops[i].len = water_drops[i].orgLen;
					water_drops[i].pos.y = b.y + -water_drops[i].len;
					water_drops[i].acceleration = WATER_DROP_STARTING_ACCELERATION;
					water_drops[i].hasHit = false;
				}
				else water_drops[i].pos.y += water_drops[i].acceleration;
			}

		}
	}
}
//typedef struct {
//	TriMesh model;
//	size_t hitId;
//} House;
//
//House createHouse() {
//	House house = { 0 };
//	house.hitId = createHitbox(hitbox_new(CLITERAL(Rectangle) { .x = 0, .y = 0, .width = 100, .height = 150 }));
//
//	AddTriMesh(&house.model, CLITERAL(TriMeshElm) {.tri = { .p1 = {.x = 0, .y = 50 }, .p2 = {.x = 50 , .y = 0  }, .p3 = {.x = 100, .y = 50} }, .col = WHITE});
//	AddTriMesh(&house.model, CLITERAL(TriMeshElm) {.tri = { .p1 = {.x = 0, .y = 150}, .p2 = {.x = 100, .y = 150}, .p3 = {.x = 100, .y = 50} }, .col = BLUE});
//	AddTriMesh(&house.model, CLITERAL(TriMeshElm) {.tri = { .p1 = {.x = 0, .y = 150}, .p2 = {.x = 100, .y = 50 }, .p3 = {.x = 0  , .y = 50} }, .col = RED});
//	return house;
//}]
//void dropHouse(House* house) {
//	DropTriMesh(&house->model);
//}

static inline bool inRange(float f1, float dif) {
	return fabsf(f1) <= dif;
}
#define FMT_BUFFER_SCREENSHOT_CAP 1024
char fmtBufScreenshot[FMT_BUFFER_SCREENSHOT_CAP];



int main(void) {	
	char* screenShotFolder = getenv("SCREENSHOT_FOLDER");
	if (!screenShotFolder) {
		char* home = os_get_home();
		if (!home) {
			TraceLog(LOG_FATAL, "Could not find home variable!");
			return 1;
		}
#ifdef TARGET_PLATFORM_WINDOWS
		snprintf(fmtBufScreenshot, FMT_BUFFER_SCREENSHOT_CAP, "%s\\screenshots", home);
#else
		snprintf(fmtBufScreenshot, FMT_BUFFER_SCREENSHOT_CAP, "%s/screenshots", home);
#endif
		screenShotFolder = fmtBufScreenshot;
		if (!os_folder_exists(screenShotFolder)) {
			TraceLog(LOG_WARNING, "Screenshot folder \"%s\" does not exist. Trying to create it...", screenShotFolder);
			if (!os_folder_create(screenShotFolder)) {
				TraceLog(LOG_FATAL, "Could not create screenshot folder \"%s\"", screenShotFolder);
				return 1;
			}
		}
	}
	TraceLog(LOG_INFO, "screenshot folder: %s", screenShotFolder);

	srand(time(NULL));
	SetTargetFPS(FPS);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WIDTH, HEIGHT, "Rain");	
	darray_of(Platform) blocks = { 0 };

	Texture2D houseTexture = LoadTexture("rsc/House.png");
	size_t houseHitID = createHitbox(CLITERAL(Hitbox) { .box = { .x=0, .y=0, houseTexture.width, houseTexture.height }, .enabled=true });
	//Platform platform = createPlatform(CLITERAL(Rectangle) { .x = 100, .y = 100, .width = 300, .height = 100 }, RAYWHITE);
	Platform umbrellaPlat = createPlatform(CLITERAL(Rectangle) { .x = 0, .y = 0, .width = 200, .height = 50 }, GOLD);
	bool umbrellaPlaced = false;
	bool umbrellaEnabled = true;
	float heaviness = 0.5f;
	float heavinessSpeed = 1.f;
	bool debugInfo = false;
	bool showHitboxes = false;
	

#if 0
	float rotation = 0;
	float rotationSpeed = 600.0f;
#endif
	
	while (!WindowShouldClose()) {

#if 0
		float rad = -20.0f;
		BeginDrawing();
		ClearBackground(GetColor(0x212121ff));
		DrawText(TextFormat("Rotation: %f.o", rotation), 0, 0, 26, RAYWHITE);
		Rectangle rect = { .x = (float)GetScreenWidth() / 2, .y = (float)GetScreenHeight() / 2, .width = 200, .height = 100 };
		//Vector2 orig = { rect.width/2, rect.height/2 };

		Vector2 orig = { .x = rad, .y = rect.height / 2 };
		//Vector2 centre = { 0, 0 };

		//rect.x = rect.x - rect.width  / 2;
		//rect.y = rect.y - rect.height / 2;
		//TraceLog(LOG_INFO, "rect {.x=%f, .y=%f, .width=%f, .height=%f}", rect.x, rect.y, rect.width, rect.height);
		Vector2 dot = CLITERAL(Vector2) { rect.x + orig.x, rect.y + orig.y };
		//DrawRectangleRec(rect, GREEN);
		DrawRectanglePro(rect, orig, 0.0, GREEN);
		DrawRectanglePro(rect, orig, 90.0, GREEN);
		DrawRectanglePro(rect, orig, 180.0, GREEN);
		DrawRectanglePro(rect, orig, 270.0, GREEN);


		DrawCircleV(dot, 5, RED);
		//DrawRectanglePro(rect, orig, rotation, WHITE);
		rotation += rotationSpeed * GetFrameTime();
		if (rotation >= 360.0f) rotation -= 360.0f;
		EndDrawing();
#else
		Rectangle screen = CLITERAL(Rectangle) { .x = 0, .y = 0, .width = GetScreenWidth(), .height = GetScreenHeight() };
		Rectangle b = screen;
		Vector2 cursor = GetMousePosition();
		if (!umbrellaPlaced) {
			updatePlatformBox(&umbrellaPlat,
				CLITERAL(Rectangle){
				.x = cursor.x - umbrellaPlat.box.width / 2,
					.y = cursor.y - umbrellaPlat.box.height / 2,
					.width = umbrellaPlat.box.width,
					.height = umbrellaPlat.box.height
			});
		}
		if (IsKeyPressed(KEY_F4)) {
			char* screenShotPath = NULL;
			for (size_t i = 0; i < 500; ++i) {
#ifdef TARGET_PLATFORM_WINDOWS
				const char* f = TextFormat("%s\\screenshot_%zu.png", screenShotFolder, i);
#else
				const char* f = TextFormat("%s/screenshot_%zu.png", screenShotFolder, i);
#endif
				if (!os_path_exists(f)) {
					screenShotPath = f;
					break;
				}
			}
			if (!screenShotPath) {
				TraceLog(LOG_ERROR, "Could not create screenshot... Reached screenshot limit");
			}
			else {
				Image scr = LoadImageFromScreen();
				ExportImage(scr, screenShotPath);
				TraceLog(LOG_INFO, "Taken screenshot: %s", screenShotPath);
				UnloadImage(scr);
			}
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && umbrellaEnabled) {
			if (!umbrellaPlaced) umbrellaPlaced = true;
			else if (CheckCollisionPointRec(cursor, umbrellaPlat.box)) umbrellaPlaced = false;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
			umbrellaEnabled = !umbrellaEnabled;
			hitboxes.data[umbrellaPlat.hitId].enabled = umbrellaEnabled;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && umbrellaPlaced) {
			Platform plat = createPlatform(CLITERAL(Rectangle) { .x = cursor.x - (BLOCK_WIDTH / 2), .y = cursor.y - (BLOCK_HEIGHT / 2), .width = BLOCK_WIDTH, .height = BLOCK_HEIGHT }, RandomColor());
			darray_push(&blocks, plat);
		}

		if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
			// (-IsKeyPressed(KEY_LEFT) + IsKeyPressed(KEY_RIGHT))
			heaviness += GetMouseWheelMove() * heavinessSpeed * GetFrameTime();
			if (heaviness > 1.0f) heaviness = 1.0f;
			if (heaviness < 0.0f) heaviness = 0.0f;
		}
		//DrawText(TextFormat("Enabled: %s", curEnabled ? "true" : "false"), 0, 0, 25, WHITE);
		if (IsKeyPressed(KEY_F1)) {
			if (IsWindowFullscreen()) {
				ToggleFullscreen();
				SetWindowSize(miniScreenW, miniScreenH);
			}
			else {
				miniScreenW = GetScreenWidth();
				miniScreenH = GetScreenHeight();
				int m = GetCurrentMonitor();
				SetWindowSize(GetMonitorWidth(m), GetMonitorHeight(m));
				ToggleFullscreen();
			}
		}
		if (IsKeyPressed(KEY_F2)) {
			showHitboxes = !showHitboxes;
		}
		if (IsKeyPressed(KEY_F3)) {
			debugInfo = !debugInfo;
		}
		if (IsKeyPressed(KEY_R)) {
			regen_water_drops(b, (size_t)MAX_WATER_DROP_COUNT * heaviness);
		}

		
		
		Vector2 housePos = CLITERAL(Vector2) { .x = 50, .y = GetScreenHeight() / 2 - houseTexture.height / 2 };
		updateBoxPos(houseHitID, housePos);

		apply_gravity_to_waterdrops(b);
		BeginDrawing();
		ClearBackground(GetColor(0x212121ff));


		//Rectangle ground = { .x = 0, .y= housePos.y + houseTexture.height-40, 0, 0};
		//Rectangle ground = { .x = 0, .y = GetScreenHeight() / 3, 0, 0};
		//ground.width = GetScreenWidth() - ground.x;
		//ground.height = GetScreenHeight() - ground.y;
		//
		//DrawRectangleRec(ground, GetColor(0x448017ff));



		for (size_t i = 0; i < water_drop_particles.len; ++i) {
			if (water_drop_particles.data[i].bTouch) {
				if (water_drop_particles.data[i].frames >= MAX_ANIMATION_WATER_DROP_FRAMES_COUNT || !inRange(water_drop_particles.data[i].bTouch->box.y - (water_drop_particles.data[i].box.y + WATER_DROP_FROM_GROUND), 1.5f)) water_drop_particles.data[i].bTouch = NULL;
				else {
					float rotT = water_drop_particles.data[i].rDirection ? 0.0f : 180.0f;
					float progress = (float)water_drop_particles.data[i].frames / MAX_ANIMATION_WATER_DROP_FRAMES_COUNT;
					float rot = (90.0f + rotT) * progress;
					Color color = water_drop_particles.data[i].color;
					color = ColorAlpha(color, 1.0 - progress);
					DrawRectanglePro(water_drop_particles.data[i].box, water_drop_particles.data[i].origin, rot, color);
					water_drop_particles.data[i].frames++;
				}
			}
		}


		for (size_t i = 0; i < water_drop_count; ++i) {
			if (water_drops[i].enabled) {
				Color color = WATER_DROP_COLOR;
				color.a = water_drops[i].alpha;
				DrawRectangleRec(CLITERAL(Rectangle) {
					.x = water_drops[i].pos.x,
						.y = water_drops[i].pos.y,
						.width = WATER_DROP_WIDTH,
						.height = water_drops[i].len
				}, color);
			}
		}
		DrawTextureV(houseTexture, housePos, WHITE);

		//drawPlatform(&platform);
		if(umbrellaEnabled) drawPlatform(&umbrellaPlat);
		for (size_t i = 0; i < blocks.len; ++i) {
			drawPlatform(&blocks.data[i]);
		}
		if (showHitboxes) {
			for (size_t i = 0; i < hitboxes.len; ++i) {
				Rectangle box = hitboxes.data[i].box;
				DrawRectangleRec(box, hitboxes.data[i].enabled ? HITBOX_COLOR_DEBUG_ENABLED : HITBOX_COLOR_DEBUG_DISABLED);
				DrawCircleV(CLITERAL(Vector2) { box.x              , box.y              }, 2.0f, GetColor(0x101010ff));
				DrawCircleV(CLITERAL(Vector2) { box.x + box.width  , box.y              }, 2.0f, GetColor(0x101010ff));
				DrawCircleV(CLITERAL(Vector2) { box.x + box.width  , box.y + box.height }, 2.0f, GetColor(0x101010ff));
				DrawCircleV(CLITERAL(Vector2) { box.x              , box.y + box.height }, 2.0f, GetColor(0x101010ff));
			}
		}
		SetTextLineSpacing(24);
		if (debugInfo) {
			DrawText(TextFormat(
				"FPS: %d, Blocks: %zu, Hitboxes: %zu/%zu\n"
				"Heaviness: %f\n"
				"Particles: %zu\n"
				"Umbrella (%f:%f) [%s]",
				GetFPS(), blocks.len, hitboxes.len, (size_t)MAX_HITBOXES,
				heaviness,
				water_drop_particles.len,
				umbrellaPlat.box.x,
				umbrellaPlat.box.y,
				hitboxes.data[umbrellaPlat.hitId].enabled ? "enabled" : "disabled"
			), 0, 0, 20, RAYWHITE);
		}
		//while (water_drop_particles_count >= 1 && !water_drop_particles.data[water_drop_particles_count-1].bTouch) water_drop_particles_count--;
		{
			size_t i = 0;
			while (i < water_drop_particles.len) {
				if (!water_drop_particles.data[i].bTouch) {
					size_t begin = i;
					size_t cur = i;
					while (cur < water_drop_particles.len && !water_drop_particles.data[i].bTouch) cur++;
					memmove(water_drop_particles.data + begin, water_drop_particles.data + cur, sizeof(WaterDropParticle)* (water_drop_particles.len - cur));
					water_drop_particles.len -= cur - begin;
				}
				else i++;

			}
		}
		EndDrawing();
#endif
	}
	UnloadTexture(houseTexture);
	CloseWindow();
	darray_drop(&hitboxes);
	darray_drop(&blocks);
	darray_drop(&water_drop_particles);
}