#include <os.h>
#include <SDL/SDL.h>
#include <math.h>
#include <time.h>

float PI = 3.14159;
SDL_bool done = SDL_FALSE;
const int maxBullets = 40;
const int maxParticles = 80;
const int maxAsteroids = 20;
const int maxItems = 5;
int maxSpeed = 5;
float turnSpeed = 4;
float accSpeed = 1.5;
SDL_bool PC_Controls = SDL_FALSE;
int gameState = 0;
const int waitTimerStart = 250;
int waitTimer = 250;
int startTimerStart = 150;
int startTimer = 150;
int itemSpawnTimer = 0;
int maxHealth = 5;
SDL_bool over = SDL_FALSE;

Uint32 nextFrame;
int framesSkipped;
int maxFrameSkip = 5;
int skipFramesBehind = 5;
int frameLength = 17;

struct Player {
	float x, y, velX, velY, dir, speed, health;
	int bulletTimerStart, bulletTimer, maxHealth, radius, invincibleTimerStart, invincibleTimer, weapon, wins;
	SDL_bool moving, alive, exploded, won, shield;
	char *weaponName;
};

struct Enemy {
	float x, y, velX, velY, dir, moveDir, shootDir, targetDir, health;
	int shootTimer, shootTimerStart, radius, maxVelocity, turnDir;
	SDL_bool active;
}enemy;

struct Bullet {
	float x, y, velX, velY, speed, dir, damage;
	int shotBy, life, frame, frameTimerStart, frameTimer, type, radius;
	SDL_bool active, collidable;
};

struct Particle {
	int life, r, g, b, type, size;
	float x, y, dir, speed;
	SDL_bool active;
};

struct Asteroid {
	float x, y, velX, velY, moveDir, dir, speed, health;
	int rotDir, rotSpeed, size;
	SDL_bool active, onScreen;
};

struct Item {
	int life, type;
	float x, y, dir, speed;
	SDL_bool active;
};

float GetAngle(float x1, float y1, float x2, float y2) {
	return atan2(y1 - y2, x1 - x2);
}

float GetDistance(float x1, float y1, float x2, float y2) {
	return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

int GetDigit(int x, int n) {
    while (n--) {
        x /= 10;
    }
    return (x % 10);
}

void DrawScore(SDL_Surface *numbers, SDL_Surface *screen, int x, int y, int score) {
	int first = GetDigit(score, 1);
	int second = GetDigit(score, 0);
	
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	SDL_Rect size;
	size.y = 0;
	size.x = 14 * first;
	size.w = 14;
	size.h = 20;
	if (first > 0)
	SDL_BlitSurface(numbers, &size, screen, &dest);
	
	
	dest.x = x + 14;
	dest.y = y;
	size.y = 0;
	size.x = 14 * second;
	size.w = 14;
	size.h = 20;
	SDL_BlitSurface(numbers, &size, screen, &dest);
}

void Init(SDL_Surface **screen) {
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if(screen == NULL) {
        printf("Couldn't initialize display: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    SDL_ShowCursor(SDL_DISABLE);
}

void ResetParticles(struct Particle particles[]) {
	int i;
	for (i = 0; i < maxParticles; i++) {
		particles[i].active = SDL_FALSE;
		particles[i].x = 0;
		particles[i].y = 0;
		particles[i].life = 0;
		particles[i].r = 0;
		particles[i].g = 0;
		particles[i].b = 0;
		particles[i].type = 0;
		particles[i].size = 0;
		particles[i].dir = 0;
		particles[i].speed = 0;
	}
};

void NewBullet(struct Bullet bullets[], int x, int y, int shotBy, float dir, int speed, int life, int type) {
	int i;
	for (i = 0; i < maxBullets; i++) {
		if (!bullets[i].active) {
			bullets[i].active = SDL_TRUE;
			bullets[i].x = x;
			bullets[i].y = y;
			bullets[i].shotBy = shotBy;
			bullets[i].dir = dir;
			bullets[i].life = life;
			bullets[i].speed = speed;
			bullets[i].type = type;
			bullets[i].velX = bullets[i].speed * cos(dir);
			bullets[i].velY = bullets[i].speed * sin(dir);
			if (type == 0) {
				bullets[i].frame = rand() % 5;
				bullets[i].collidable = SDL_TRUE;
				bullets[i].frameTimerStart = 3;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 4;
				bullets[i].damage = 1;
			}
			else if (type == 1) {
				bullets[i].collidable = SDL_FALSE;
				bullets[i].frame = life;
				bullets[i].frameTimerStart = 1;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 0;
			}
			else if (type == 2) {
				bullets[i].collidable = SDL_FALSE;
				bullets[i].frame = 0;
				bullets[i].frameTimerStart = 1000;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 0;
			}
			else if (type == 3) {
				bullets[i].collidable = SDL_FALSE;
				bullets[i].frame = 0;
				bullets[i].frameTimerStart = 2;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 0;
			}
			else if (type == 4) {
				bullets[i].collidable = SDL_TRUE;
				bullets[i].frame = 0;
				bullets[i].frameTimerStart = 10000;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 4;
				bullets[i].damage = .5;
			}
			else if (type == 5) {
				bullets[i].collidable = SDL_TRUE;
				bullets[i].frame = 0;
				bullets[i].frameTimerStart = 10000;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 4;
				bullets[i].damage = .75;
			}
			else if (type == 6) {
				bullets[i].frame = 0;
				bullets[i].collidable = SDL_TRUE;
				bullets[i].frameTimerStart = 3;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 16;
				bullets[i].damage = 0;
			}
			else if (type == 7) {
				bullets[i].frame = 0;
				bullets[i].collidable = SDL_FALSE;
				bullets[i].frameTimerStart = 2;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].radius = 64;
				bullets[i].damage = 0;
			}
			break;
		}
	}
}

void NewParticle(struct Particle particles[], int x, int y, int r, int g, int b, int type, int life, int size, float dir, float speed) {
	int i;
	for (i = 0; i < maxParticles; i++) {
		if (!particles[i].active) {
			particles[i].active = SDL_TRUE;
			particles[i].x = x;
			particles[i].y = y;
			particles[i].life = life;
			particles[i].r = r;
			particles[i].g = g;
			particles[i].b = b;
			particles[i].type = type;
			particles[i].size = size;
			particles[i].dir = dir;
			particles[i].speed = speed;
			
			break;
		}
	}
}

void UpdateParticles(struct Particle particles[]) {
	int i;
	for (i = 0; i < maxParticles; i++) {
		if (particles[i].active) {
			if (particles[i].type != 2)
				particles[i].life--;
			if (particles[i].life < 0)
				particles[i].active = SDL_FALSE;
			
			if (particles[i].type == 0) {
				particles[i].x += rand() % 3 - 1;
				particles[i].y += rand() % 3 - 1;
			}
			else if (particles[i].type == 1) {
				particles[i].x += particles[i].speed * cos(particles[i].dir);
				particles[i].y += particles[i].speed * sin(particles[i].dir);
			}
		}
	}
}

void DrawParticles(struct Particle particles[], SDL_Surface *screen) {
	int i;
	for (i = 0; i < maxParticles; i++) {
		if (particles[i].active) {
			SDL_Rect rect;
			rect.x = particles[i].x;
			rect.y = particles[i].y;
			rect.w = particles[i].size;
			rect.h = particles[i].size;
			
			SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, particles[i].r, particles[i].g, particles[i].b));
		}
	}
};

void InitPlayers(struct Player players[]) {
	int i;
	for (i = 0; i < 2; i++) {
		players[i].x = rand() % (SCREEN_WIDTH - 32) + 16;
		players[i].y = rand() % (SCREEN_HEIGHT - 32) + 16;
		players[i].velY = 0;
		players[i].velX = 0;
		players[i].dir = (rand() % 361 - 180);// * (PI / 180);
		players[i].moving = SDL_FALSE;
		players[i].bulletTimerStart = 30;
		players[i].bulletTimer = players[i].bulletTimerStart;
		players[i].maxHealth = maxHealth;
		players[i].health = players[i].maxHealth;
		players[i].exploded = SDL_FALSE;
		players[i].alive = SDL_TRUE;
		players[i].invincibleTimerStart = 50;
		players[i].invincibleTimer = 0;
		players[i].won = SDL_FALSE;
		players[i].weaponName = "Default";
		players[i].weapon = 0;
		players[i].shield = SDL_FALSE;
	}
};

void UpdatePlayers(struct Player players[], struct Bullet bullets[], struct Particle particles[]) {
	int i;
	for (i = 0; i < 2; i++) {
		players[i].x += players[i].velX;
		players[i].y += players[i].velY;
		
		if (players[i].bulletTimer > 0)
			players[i].bulletTimer--;
		
		if (players[i].invincibleTimer > 0)
			players[i].invincibleTimer--;
		
		if (players[i].x < -16)
			players[i].x = SCREEN_WIDTH;
		else if (players[i].x > SCREEN_WIDTH)
			players[i].x = -16;
		
		if (players[i].y < -16)
			players[i].y = SCREEN_HEIGHT;
		else if (players[i].y > SCREEN_HEIGHT)
			players[i].y = -16;
		
		if (!players[i].moving) {
			if (players[i].velX > 0) {
				players[i].velX -= .025;
				if (players[i].velX < .2)
					players[i].velX = 0;
			}
			else if (players[i].velX < 0) {
				players[i].velX += .025;
				if (players[i].velX > -.2)
					players[i].velX = 0;
			}
			
			if (players[i].velY > 0) {
				players[i].velY -= .025;
				if (players[i].velY < .2)
					players[i].velY = 0;
			}
			else if (players[i].velY < 0) {
				players[i].velY += .025;
				if (players[i].velY > -.2)
					players[i].velY = 0;
			}
		}
		if (players[i].velX > maxSpeed)
			players[i].velX = maxSpeed;
		else if (players[i].velX < - maxSpeed)
			players[i].velX = -maxSpeed;
		if (players[i].velY > maxSpeed)
			players[i].velY = maxSpeed;
		else if (players[i].velY < -maxSpeed)
			players[i].velY = -maxSpeed;
		
		if (players[i].health <= 0)
			players[i].alive = SDL_FALSE;
		
		if (!players[i].alive && !players[i].exploded) {
			players[i].exploded = SDL_TRUE;
			int j;
			for (j = 0; j < 10; j++) 
				NewBullet(bullets, players[i].x + 8 + rand() % 33 - 16, players[i].y + 8 + rand() % 33 - 16, -1, 0, 0, rand() % 5 - 3, 1);
			for (j = 0; j < 20; j++)
				NewBullet(bullets, players[i].x + 8, players[i].y + 8, -1, (rand() % 361 - 180) * (PI / 180), 3, rand() % 20 + 10, 2);
		}
		
		if (players[i].moving && players[i].alive)
			if (rand() % 3 == 0)
				NewParticle(particles, players[i].x + 8 + 8 * cos((players[i].dir * (PI / 180)) + PI), players[i].y + 8 + 8 * sin((players[i].dir * (PI / 180)) + PI), 255, rand() % 255, 0, 1, rand() % 10 + 10, rand() % 2 + 1, players[i].dir * (PI / 180) + (rand() % 61 - 30) * (PI / 180) + PI, rand() % 100 * .01);
	}
	if (!over) {
		if (!players[0].alive) {
			players[1].won = SDL_TRUE;
			players[1].invincibleTimer = 10000;
			players[1].wins++;
			over = SDL_TRUE;
		}
		if (!players[1].alive) {
			players[0].won = SDL_TRUE;
			players[0].invincibleTimer = 10000;
			players[0].wins++;
			over = SDL_TRUE;
		}
	}
}

void DrawPlayers(struct Player players[], SDL_Surface *screen, SDL_Surface *ship, SDL_Surface *shield) {
	int i;
	for (i = 0; i < 2; i++) {
		if (players[i].alive) {
			SDL_Rect dest;
			
			SDL_Rect size;
			size.x = 16 * players[i].moving;
			size.y = 16 * i;
			size.w = 16;
			size.h = 16;
			
			if (players[i].shield) {
				dest.x = players[i].x;
				dest.y = players[i].y;
				SDL_BlitSurface(shield, NULL, screen, &dest);
			}
			
			SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 16, 16, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
			SDL_BlitSurface(ship, &size, tmp1, NULL);
			
			SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - players[i].dir, 1.0, 0);
			dest.x = players[i].x + 8 - tmp2->w / 2;
			dest.y = players[i].y + 8 - tmp2->h / 2;
			
			SDL_BlitSurface(tmp2, NULL, screen, &dest);
			SDL_FreeSurface(tmp1);
			SDL_FreeSurface(tmp2);
		}
	}
};

void DrawHealth(struct Player players[], nSDL_Font *font, SDL_Surface *screen) {
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = 50;
	rect.h = 4;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 0));
	rect.w = ((float)players[0].health / (float)players[0].maxHealth) * 50;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 255, 0));
	
	rect.x = SCREEN_WIDTH - 51;
	rect.y = 0;
	rect.w = 55;
	rect.h = 4;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 0));
	
	rect.x = SCREEN_WIDTH - ((float)players[1].health / (float)players[1].maxHealth) * 51;
	rect.w = 55;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 255, 0));
	
	nSDL_DrawString(screen, font, 1, 6, players[0].weaponName);
	nSDL_DrawString(screen, font, SCREEN_WIDTH - 50, 6, players[1].weaponName);
	
	if (enemy.active) {
		SDL_Rect rect;
		rect.x = SCREEN_WIDTH / 2 - 25;
		rect.y = 0;
		rect.w = 50;
		rect.h = 4;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 0));
		
		rect.x = SCREEN_WIDTH / 2;
		rect.w = enemy.health / 10.0 * 25;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 255, 0));
		
		rect.x = SCREEN_WIDTH / 2 - enemy.health / 10.0 * 25;
		rect.y = 0;
		rect.w = enemy.health / 10.0 * 25 + 1;
		rect.h = 4;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 255, 0));
	}
}

void ResetBullets(struct Bullet bullets[]) {
	int i;
	for (i = 0; i < maxBullets; i++) {
		bullets[i].active = SDL_FALSE;
		bullets[i].x = 0;
		bullets[i].y = 0;
		bullets[i].speed = 0;
		bullets[i].dir = 0;
		bullets[i].shotBy = -1;
		bullets[i].life = 0;
		bullets[i].frameTimerStart = 3;
		bullets[i].frameTimer = bullets[i].frameTimerStart;
		bullets[i].frame = 0;
		bullets[i].radius = 0;
		bullets[i].damage = 0;
	}
};

void UpdateBullets(struct Bullet bullets[], struct Particle particles[]) {
	int i, j;
	for (i = 0; i < maxBullets; i++) {
		if (bullets[i].active) {
			if (bullets[i].type == 0 || bullets[i].type == 2 || bullets[i].type == 4 || bullets[i].type == 5 || bullets[i].type == 6) {
				bullets[i].life--;
				if (bullets[i].life < 0) {
					bullets[i].active = SDL_FALSE;
					int amount = rand() % 5 + 10;
					for (j = 0; j < amount; j++) {
						if (bullets[i].type == 0)
							NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, rand() % 255, rand() % 255, rand() % 255, 1, rand() % 10 + 10, 1, (rand() % 361 - 180) * (PI / 180), rand() % 200 * .01);
						else if (bullets[i].type == 2 || bullets[i].type == 5)
							NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, 255, rand() % 255, 0, 1, rand() % 10 + 10, 1, (rand() % 361 - 180) * (PI / 180), rand() % 200 * .01);
					}
				}
			}
			bullets[i].x += bullets[i].velX;
			bullets[i].y += bullets[i].velY;
			
			if (bullets[i].type == 2) {
				bullets[i].x += rand() % 5 - 2;
				bullets[i].y += rand() % 5 - 2;
				bullets[i].dir += (rand() % 501 - 250) * .001;
			}
			
			bullets[i].frameTimer--;
			if (bullets[i].frameTimer < 0) {
				bullets[i].frameTimer = bullets[i].frameTimerStart;
				bullets[i].frame++;
				if (bullets[i].type == 0) {
					if (bullets[i].frame > 4)
						bullets[i].frame = 0;
				}
				else if (bullets[i].type == 1) {
					if (bullets[i].frame > 8)
						bullets[i].active = SDL_FALSE;
				}
				else if (bullets[i].type == 3) {
					if (bullets[i].frame > 5)
						bullets[i].active = SDL_FALSE;
				}
				else if (bullets[i].type == 6) {
					if (bullets[i].frame > 4)
						bullets[i].frame = 4;
					if (bullets[i].life < 12)
						bullets[i].frame -= 2;
				}
				else if (bullets[i].type == 7) {
					if (bullets[i].frame > 7)
						bullets[i].active = SDL_FALSE;
				}
			}
			
			if (bullets[i].x < -8)
				bullets[i].x = SCREEN_WIDTH;
			else if (bullets[i].x > SCREEN_WIDTH)
				bullets[i].x = -8;
			
			if (bullets[i].y < -8)
				bullets[i].y = SCREEN_HEIGHT;
			else if (bullets[i].y > SCREEN_HEIGHT)
				bullets[i].y = -8;
			
			if (rand() % 3 == 0 && (bullets[i].type == 0 || bullets[i].type == 2 || bullets[i].type == 4 || bullets[i].type == 5 || bullets[i].type == 6)) {
				if (bullets[i].type == 0)
					NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, 0, 142, 92, 0, 40, 1, 0, 0);
				else if (bullets[i].type == 2 || bullets[i].type == 5) {
					NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, 255, rand() % 255, 0, 0, 40, 1, 0, 0);
					if (rand() % 2 == 0 && bullets[i].type == 2)
						NewBullet(bullets, bullets[i].x, bullets[i].y, -1, 0, 0, rand() % 5 - 3, 1);
				}
			}
		}
	}
}

void DrawBullets(struct Bullet bullets[], SDL_Surface *bullet, SDL_Surface *smallBullet, SDL_Surface *tripleBullet, SDL_Surface *magnetBullet, SDL_Surface *explosion, SDL_Surface *flame, SDL_Surface *asteroidExplosion, SDL_Surface *smallAsteroidExplosion, SDL_Surface *shieldExplosion, SDL_Surface *screen) {
	int i;
	for (i = 0; i < maxBullets; i++) {
		if (bullets[i].active) {
			if (bullets[i].type == 0) {
				SDL_Rect dest;
			
				SDL_Rect size;
				size.x = 8 * bullets[i].frame;
				size.y = 0;
				size.w = 8;
				size.h = 8;
				
				//SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 255));
				
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 8, 8, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(bullet, &size, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - bullets[i].dir * (180 / PI), 1.0, 0);
				dest.x = bullets[i].x - tmp2->w / 2;
				dest.y = bullets[i].y - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
			else if (bullets[i].type == 1) {
				//SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 255));
				
				SDL_Rect dest;
				dest.x = bullets[i].x - 32;
				dest.y = bullets[i].y - 32;
				
				SDL_Rect size;
				size.x = 64 * bullets[i].frame;
				size.y = 0;
				size.w = 64;
				size.h = 64;
				
				//SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 64, 64, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				//SDL_BlitSurface(explosion, &size, tmp1, NULL);
				
				SDL_BlitSurface(explosion, &size, screen, &dest);
				//SDL_FreeSurface(tmp1);
			}
			else if (bullets[i].type == 2) {
				SDL_Rect dest;
				dest.x = bullets[i].x + 4;
				dest.y = bullets[i].y + 4;
				
				SDL_BlitSurface(flame, NULL, screen, &dest);
			}
			else if (bullets[i].type == 3) {
				if (bullets[i].life == 1) {
					SDL_Rect dest;
					dest.x = bullets[i].x;
					dest.y = bullets[i].y;
					
					SDL_Rect size;
					size.x = 64 * bullets[i].frame;
					size.y = 0;
					size.w = 64;
					size.h = 64;
					
					SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 64, 64, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
					SDL_BlitSurface(asteroidExplosion, &size, tmp1, NULL);
					
					SDL_BlitSurface(tmp1, NULL, screen, &dest);
					SDL_FreeSurface(tmp1);
				}
				else if (bullets[i].life == 0) {
					SDL_Rect dest;
					dest.x = bullets[i].x;
					dest.y = bullets[i].y;
					
					SDL_Rect size;
					size.x = 32 * bullets[i].frame;
					size.y = 0;
					size.w = 32;
					size.h = 32;
					
					SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 32, 32, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
					SDL_BlitSurface(smallAsteroidExplosion, &size, tmp1, NULL);
					
					SDL_BlitSurface(tmp1, NULL, screen, &dest);
					SDL_FreeSurface(tmp1);
				}
			}
			else if (bullets[i].type == 4) {
				SDL_Rect dest;
			
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 8, 8, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(smallBullet, NULL, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - (bullets[i].dir * (180 / PI)), 1.0, 0);
				dest.x = bullets[i].x + 4 - tmp2->w / 2;
				dest.y = bullets[i].y + 4 - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
			else if (bullets[i].type == 5) {
				SDL_Rect dest;
			
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 8, 8, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(tripleBullet, NULL, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - (bullets[i].dir * (180 / PI)), 1.0, 0);
				dest.x = bullets[i].x + 4 - tmp2->w / 2;
				dest.y = bullets[i].y + 4 - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
			else if (bullets[i].type == 6) {
				SDL_Rect dest;
				
				SDL_Rect size;
				size.x = 32 * bullets[i].frame;
				size.y = 0;
				size.w = 32;
				size.h = 32;
				
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 32, 32, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(magnetBullet, &size, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - bullets[i].dir * (180 / PI), 1.0, 0);
				dest.x = bullets[i].x + 16 - tmp2->w / 2;
				dest.y = bullets[i].y + 16 - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
			else if (bullets[i].type == 7) {
				SDL_Rect dest;
				
				SDL_Rect size;
				size.x = 32 * bullets[i].frame;
				size.y = 0;
				size.w = 32;
				size.h = 32;
				
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 32, 32, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(shieldExplosion, &size, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - bullets[i].dir * (180 / PI), 1.0, 0);
				dest.x = bullets[i].x + 16 - tmp2->w / 2;
				dest.y = bullets[i].y + 16 - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
		}
	}
};

void InitStars(struct Particle particles[]) {
	int i;
	for (i = 0; i < rand() % 5 + 8; i++) {
		int color = rand() % 100 + 100;
		NewParticle(particles, rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, color, color, color, 2, 0, 1, 0, 0);
	}
}

void ResetAsteroids(struct Asteroid asteroids[]) {
	int i;
	for (i = 0; i < maxAsteroids; i++) {
		asteroids[i].x = 0;
		asteroids[i].y = 0;
		asteroids[i].moveDir = 0;
		asteroids[i].dir = 0;
		asteroids[i].rotDir = 1;
		asteroids[i].rotSpeed = 0;
		asteroids[i].speed = 0;
		asteroids[i].size = 0;
		asteroids[i].active = SDL_FALSE;
		asteroids[i].onScreen = SDL_FALSE;
	}
}

void NewAsteroid(struct Asteroid asteroids[], int x, int y, int moveDir, int size) {
	int i;
	for (i = 0; i < maxAsteroids; i++) {
		if (!asteroids[i].active) {
			asteroids[i].active = SDL_TRUE;
			asteroids[i].x = x;
			asteroids[i].y = y;
			asteroids[i].size = size;
			asteroids[i].moveDir = moveDir;
			asteroids[i].dir = (rand() % 361 - 180) * (PI / 180);
			asteroids[i].rotDir = 1;
			asteroids[i].onScreen = SDL_FALSE;
			if (rand() % 2 == 0)
				asteroids[i].rotDir = -1;
			if (size == 0) {
				asteroids[i].rotSpeed = rand() % 4 + 1;
				asteroids[i].speed = (rand() % 100 + 50) * .01;
			}
			else if (size == 1) {
				asteroids[i].rotSpeed = rand() % 2 + 1;
				asteroids[i].speed = (rand() % 50 + 50) * .01;;
			}
			
			//asteroids[i].speed = 1;
			asteroids[i].velX = asteroids[i].speed * cos(asteroids[i].moveDir);
			asteroids[i].velY = asteroids[i].speed * sin(asteroids[i].moveDir);
			asteroids[i].health = size + 1;
			
			break;
		}
	}
}

void SpawnAsteroids(struct Asteroid asteroids[]) {
	int amount = 0;
	int i;
	for (i = 0; i < maxAsteroids; i++) {
		if (asteroids[i].active)
			amount++;
	}
	if (amount < 5 && rand () % 30 == 0) {
		int x = 0, y = 0;
		int place = rand() % 4;
		if (place == 0) {
			x = rand() % SCREEN_WIDTH;
			y = -(rand() % 30 + 64);
		}
		else if (place == 1) {
			x = SCREEN_WIDTH + (rand() % 30 + 64);
			y = rand() % SCREEN_HEIGHT;
		}
		else if (place == 2) {
			x = rand() % SCREEN_WIDTH;
			y = SCREEN_HEIGHT + (rand() % 30 + 64);
		}
		else if (place == 3) {
			x = -(rand() % 30 + 64);
			y = rand() % SCREEN_HEIGHT;
		}
		
		NewAsteroid(asteroids, x, y, (rand() % 361 - 180) * (PI / 180), rand() % 2);
	}
}

void UpdateAsteroids(struct Asteroid asteroids[], struct Bullet bullets[]) {
	int i;
	for (i = 0; i < maxAsteroids; i++) {
		if (asteroids[i].active) {
			if (asteroids[i].health <= 0) {
				asteroids[i].active = SDL_FALSE;
				NewBullet(bullets, asteroids[i].x, asteroids[i].y, -1, 0, 0, asteroids[i].size, 3);
				if (asteroids[i].size == 1) {
					float k;
					int angle = rand() % 360;
					for (k = 0; k < 4; k++)
						NewAsteroid(asteroids, asteroids[i].x + 16, asteroids[i].y + 16, angle + 90 * k, 0);
				}
			}
			
			if (asteroids[i].velX < -1.5)
				asteroids[i].velX = -1.5;
			else if (asteroids[i].velX > 1.5)
				asteroids[i].velX = 1.5;
			if (asteroids[i].velY < -1.5)
				asteroids[i].velY = -1.5;
			else if (asteroids[i].velY > 1.5)
				asteroids[i].velY = 1.5;
			
			asteroids[i].x += asteroids[i].velX;
			asteroids[i].y += asteroids[i].velY;
			asteroids[i].dir += asteroids[i].rotDir * asteroids[i].rotSpeed;
			
			if (asteroids[i].x < -32 * (asteroids[i].size + 1))
				asteroids[i].x = SCREEN_WIDTH;
			else if (asteroids[i].x > SCREEN_WIDTH)
				asteroids[i].x = -32 * (asteroids[i].size + 1);
			
			if (asteroids[i].y < -32 * (asteroids[i].size + 1))
				asteroids[i].y = SCREEN_HEIGHT;
			else if (asteroids[i].y > SCREEN_HEIGHT)
				asteroids[i].y = -32 * (asteroids[i].size + 1);
		}
	}
}

void DrawAsteroids(struct Asteroid asteroids[], SDL_Surface *smallAsteroid, SDL_Surface *bigAsteroid, SDL_Surface *screen) {
	int i;
	for (i = 0; i < maxAsteroids; i++) {
		if (asteroids[i].active) {
			if (asteroids[i].size == 0) {
				SDL_Rect dest;
			
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 32, 32, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(smallAsteroid, NULL, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - asteroids[i].dir, 1.0, 0);
				dest.x = asteroids[i].x + 16 - tmp2->w / 2;
				dest.y = asteroids[i].y + 16 - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
			else {
				SDL_Rect dest;
			
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 64, 64, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(bigAsteroid, NULL, tmp1, NULL);
				
				SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - asteroids[i].dir, 1.0, 0);
				dest.x = asteroids[i].x + 32 - tmp2->w / 2;
				dest.y = asteroids[i].y + 32 - tmp2->h / 2;
				
				SDL_BlitSurface(tmp2, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
				SDL_FreeSurface(tmp2);
			}
		}
	}
}

void ResetItems(struct Item items[]) {
	int i;
	for (i = 0; i < maxItems; i++) {
		items[i].active = SDL_FALSE;
		items[i].x = 0;
		items[i].y = 0;
		items[i].dir = 0;
		items[i].speed = 0;
		items[i].type = 0;
		items[i].life = 0;
	}
}

void SpawnItems(struct Item items[]) {
	itemSpawnTimer--;
	if (itemSpawnTimer < 0) {
		itemSpawnTimer = rand() % 300 + 500;
		int i;
		for (i = 0; i < maxItems; i++) {
			if (!items[i].active) {
				int place = rand() % 4;
				if (place == 0) {
					items[i].x = rand() % SCREEN_WIDTH;
					items[i].y = -(rand() % 30 + 64);
				}
				else if (place == 1) {
					items[i].x = SCREEN_WIDTH + (rand() % 30 + 64);
					items[i].y = rand() % SCREEN_HEIGHT;
				}
				else if (place == 2) {
					items[i].x = rand() % SCREEN_WIDTH;
					items[i].y = SCREEN_HEIGHT + (rand() % 30 + 64);
				}
				else if (place == 3) {
					items[i].x = -(rand() % 30 + 64);
					items[i].y = rand() % SCREEN_HEIGHT;
				}
				items[i].dir = (rand() % 361) * (PI / 180);
				items[i].active = SDL_TRUE;
				items[i].speed = rand() % 2 + 1;
				items[i].life = 700;
				items[i].type = rand() % 6;
				//items[i].type = 5;
				
				break;
			}
		}
	}
}

void UpdateItems(struct Item items[]) {
	int i;
	for (i = 0; i < maxItems; i++) {
		if (items[i].active) {
			items[i].x += items[i].speed * cos(items[i].dir);
			items[i].y += items[i].speed * sin(items[i].dir);
			
			items[i].life--;
			if (items[i].x < -16 || items[i].x > SCREEN_WIDTH || items[i].y < -16 || items[i].y > SCREEN_HEIGHT)
				if (items[i].life < 0)
					items[i].active = SDL_FALSE;
			
			if (items[i].x < -16)
				items[i].x = SCREEN_WIDTH;
			else if (items[i].x > SCREEN_WIDTH)
				items[i].x = -16;
			if (items[i].y < -16)
				items[i].y = SCREEN_HEIGHT;
			else if (items[i].y > SCREEN_HEIGHT)
				items[i].y = -16;
		}
	}
}

void DrawItems(struct Item items[], SDL_Surface *itemImage, SDL_Surface *screen) {
	int i;
	for (i = 0; i < maxItems; i++) {
		if (items[i].active) {
			SDL_Rect dest;
			SDL_Rect size;
			
			dest.x = items[i].x;
			dest.y = items[i].y;
			
			size.x = items[i].type * 16;
			size.y = 0;
			size.w = 16;
			size.h = 16;
			
			SDL_BlitSurface(itemImage, &size, screen, &dest);
		}
	}
}

void InitEnemy() {
	enemy.active = SDL_FALSE;
	enemy.shootTimerStart = 10;
	enemy.shootTimer = enemy.shootTimerStart;
	enemy.velX = 0;
	enemy.velY = 0;
	enemy.maxVelocity = 1;
	enemy.turnDir = 0;
	enemy.dir = 0;
	enemy.moveDir = 0;
	enemy.shootDir = 0;
	enemy.targetDir = 0;
	enemy.health = 5;
	enemy.radius = 8;
}

void ActivateEnemy() {
	enemy.active = SDL_TRUE;
	enemy.turnDir = 0;
	enemy.health = 10;
	int x = 0, y = 0;
	int place = rand() % 4;
	if (place == 0) {
		x = rand() % SCREEN_WIDTH;
		y = -(rand() % 30 + 50);
	}
	else if (place == 1) {
		x = SCREEN_WIDTH + (rand() % 30 + 50);
		y = rand() % SCREEN_HEIGHT;
	}
	else if (place == 2) {
		x = rand() % SCREEN_WIDTH;
		y = SCREEN_HEIGHT + (rand() % 30 + 50);
	}
	else if (place == 3) {
		x = -(rand() % 30 + 50);
		y = rand() % SCREEN_HEIGHT;
	}
	enemy.x = x;
	enemy.y = y;
	enemy.dir = rand() % 361;
	enemy.shootDir = rand() % 361;
}

void UpdateEnemy(struct Bullet bullets[], struct Particle particles[], struct Player players[], struct Asteroid asteroids[], struct Item items[]) {
	if (enemy.active) {
		int i;
		enemy.x += enemy.velX;
		enemy.y += enemy.velY;
			
		enemy.velX += 1 * cos((enemy.dir * (PI / 180))) / 30;
		enemy.velY += 1 * sin((enemy.dir * (PI / 180))) / 30;
		
		if (enemy.turnDir == 0 && rand() % 30 == 0) {
			if (rand() % 2 == 0)
				enemy.turnDir = -1;
			else
				enemy.turnDir = 1;
		}
		if ((enemy.turnDir == -1 || enemy.turnDir == 1) && rand() % 20 == 0)
			enemy.turnDir = 0;
		
		enemy.dir += (rand() % 200) * .01 * enemy.turnDir;
		
		if (enemy.shootTimer > 0)
			enemy.shootTimer--;
			
		if (enemy.x < -16)
			enemy.x = SCREEN_WIDTH;
		else if (enemy.x > SCREEN_WIDTH)
			enemy.x = -16;
		
		if (enemy.y < -16)
			enemy.y = SCREEN_HEIGHT;
		else if (enemy.y > SCREEN_HEIGHT)
			enemy.y = -16;
			
		if (enemy.velX > enemy.maxVelocity)
			enemy.velX = enemy.maxVelocity;
		else if (enemy.velX < -enemy.maxVelocity)
			enemy.velX = -enemy.maxVelocity;
		if (enemy.velY > enemy.maxVelocity)
			enemy.velY = enemy.maxVelocity;
		else if (enemy.velY < -enemy.maxVelocity)
			enemy.velY = -enemy.maxVelocity;

		if (rand() % 3 == 0)
			NewParticle(particles, enemy.x + 8 + 8 * cos((enemy.dir * (PI / 180)) + PI), enemy.y + 8 + 8 * sin((enemy.dir * (PI / 180)) + PI), 255, rand() % 255, 0, 1, rand() % 10 + 10, rand() % 2 + 1, enemy.dir * (PI / 180) + (rand() % 61 - 30) * (PI / 180) + PI, rand() % 100 * .01);
		
		float distance = 100000;
		for (i = 0; i < 2; i++) {
			float distance2 = GetDistance(enemy.x + 8, enemy.y + 8, players[i].x + 8, players[i].y + 8);
			if (distance2 < distance) {
				distance = distance2;
				enemy.targetDir = GetAngle(players[i].x + 8, players[i].y + 8, enemy.x + 8, enemy.y + 8) * (180 / PI);
			}
		}
		for (i = 0; i < maxAsteroids; i++) {
			if (asteroids[i].active) {
				float distance2 = GetDistance(enemy.x + 8, enemy.y + 8, asteroids[i].x + 16 * (asteroids[i].size + 1), asteroids[i].y + 16 * (asteroids[i].size + 1));
				if (distance2 < distance) {
					distance = distance2;
					enemy.targetDir = GetAngle(asteroids[i].x + 16 * (asteroids[i].size + 1), asteroids[i].y + 16 * (asteroids[i].size + 1), enemy.x + 8, enemy.y + 8) * (180 / PI);
				}
			}
		}
		if (GetDistance(enemy.x + 5 * cos((enemy.shootDir + 3) * (PI / 180)), enemy.y + 5 * sin((enemy.shootDir + 3) * (PI / 180)), enemy.x + 5 * cos(enemy.targetDir * (PI / 180)), enemy.y + 5 * sin(enemy.targetDir * (PI / 180))) < GetDistance(enemy.x + 5 * cos(enemy.shootDir * (PI / 180)), enemy.y + 5 * sin(enemy.shootDir * (PI / 180)), enemy.x + 5 * cos(enemy.targetDir * (PI / 180)), enemy.y + 5 * sin(enemy.targetDir * (PI / 180))))
			enemy.shootDir += 3;
		else
			enemy.shootDir -= 3;
		
		if (enemy.shootDir > 360)
			enemy.shootDir = 0;
		else if (enemy.shootDir < 0)
			enemy.shootDir = 360;
		
		if (distance < 5000 && GetDistance(enemy.x + 5 * cos(enemy.shootDir * (PI / 180)), enemy.y + 5 * sin(enemy.shootDir * (PI / 180)), enemy.x + 5 * cos(enemy.targetDir * (PI / 180)), enemy.y + 5 * sin(enemy.targetDir * (PI / 180))) < 5) {
			enemy.shootTimer--;
			if (enemy.shootTimer < 0) {
				enemy.shootTimer = enemy.shootTimerStart;
				float j;
				for (j = -.25; j <= .25; j += .25) {
					int amount = 5;
					if (j == 0)
						amount = 8;
					int x = enemy.x + 8 + amount * cos(enemy.shootDir * (PI / 180));
					int y = enemy.y + 8 + amount * sin(enemy.shootDir * (PI / 180));
					NewBullet(bullets, x, y, 3, enemy.shootDir * (PI / 180) + j, 3, 20, 5);	
				}
			}
		}
		else
			enemy.shootTimer = enemy.shootTimerStart;
		
		if (enemy.health <= 0) {
			enemy.active = SDL_FALSE;
			int i, j;
			for (j = 0; j < 10; j++) 
				NewBullet(bullets, enemy.x + 8 + rand() % 33 - 16, enemy.y + 8 + rand() % 33 - 16, -1, 0, 0, rand() % 5 - 3, 1);
			for (j = 0; j < 20; j++)
				NewBullet(bullets, enemy.x + 8, enemy.y + 8, -1, (rand() % 361 - 180) * (PI / 180), 3, rand() % 20 + 10, 2);
			
			for (i = 0; i < maxItems; i++) {
				if (!items[i].active) {
					items[i].x = enemy.x;
					items[i].y = enemy.y;
					items[i].dir = (rand() % 361) * (PI / 180);
					items[i].active = SDL_TRUE;
					items[i].speed = rand() % 2 + 1;
					items[i].life = 2000;
					items[i].type = 6;
					break;
				}
			}
		}
	}
}

void DrawEnemy(SDL_Surface *enemyImage, SDL_Surface *screen) {
	if (enemy.active) {
		SDL_Rect dest;
		SDL_Rect size;
		size.x = 0;
		size.y = 0;
		size.w = 16;
		size.h = 16;
		
		SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 16, 16, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		SDL_BlitSurface(enemyImage, &size, tmp1, NULL);
		SDL_Surface *tmp2 = rotozoomSurface(tmp1, 360 - enemy.dir, 1.0, 0);
		dest.x = enemy.x + 8 - tmp2->w / 2;
		dest.y = enemy.y + 8 - tmp2->h / 2;
		SDL_BlitSurface(tmp2, NULL, screen, &dest);
		
		size.x = 16;
		size.y = 0;
		size.w = 16;
		size.h = 16;
		
		SDL_Surface *tmp3 = SDL_CreateRGBSurface(0, 16, 16, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		SDL_BlitSurface(enemyImage, &size, tmp3, NULL);
		
		SDL_Surface *tmp4 = rotozoomSurface(tmp3, 360 - enemy.shootDir, 1.0, 0);
		dest.x = enemy.x + 8 - tmp4->w / 2;
		dest.y = enemy.y + 8 - tmp4->h / 2;
		
		SDL_BlitSurface(tmp4, NULL, screen, &dest);
		
		SDL_FreeSurface(tmp1);
		SDL_FreeSurface(tmp2);
		SDL_FreeSurface(tmp3);
		SDL_FreeSurface(tmp4);
	}
}

void Collide(struct Bullet bullets[], struct Player players[], struct Asteroid asteroids[], struct Item items[]) {
	int i, j;
	float distance = GetDistance(players[0].x + 8, players[0].y + 8, players[1].x + 8, players[1].y + 8);
	if (distance < 256) {
		float angle1 = GetAngle(players[0].x, players[0].y, players[1].x, players[1].y);
		float angle2 = GetAngle(players[1].x, players[1].y, players[0].x, players[0].y);
		players[0].velX = 1.5 * cos(angle1);
		players[0].velY = 1.5 * sin(angle1);
		players[1].velX = 1.5 * cos(angle2);
		players[1].velY = 1.5 * sin(angle2);
	}
	
	for (i = 0; i < maxBullets; i++) {
		for (j = 0; j < 2; j++) {
			float distance = GetDistance(bullets[i].x + bullets[i].radius, bullets[i].y + bullets[i].radius, players[j].x + 8, players[j].y + 8);
			if (distance <= (8 + bullets[i].radius) * (8 + bullets[i].radius) && bullets[i].shotBy != j && bullets[i].active && bullets[i].collidable) {
				if (bullets[i].type != 6)
					bullets[i].active = SDL_FALSE;
				if (!players[j].shield) {
					players[j].health -= bullets[i].damage;
					//if (players[j].invincibleTimer <= 0) {players[j].invincibleTimer = players[j].invincibleTimerStart;}
					//float angle = GetAngle(players[j].x + 8, players[j].y + 8, bullets[i].x + 4, bullets[i].y + 4);
					if (bullets[i].type != 6) {
						players[j].velX += 3 * cos(bullets[i].dir);
						players[j].velY += 3 * sin(bullets[i].dir);
					}
					else if (bullets[i].type == 6) {
						players[j].velX += .15 * cos(bullets[i].dir);
						players[j].velY += .15 * sin(bullets[i].dir);
					}
				}
				else {
					players[j].invincibleTimer = players[j].invincibleTimerStart;
					players[j].shield = SDL_FALSE;
					NewBullet(bullets, players[j].x - 8, players[j].y - 8, -1, 0, 0, 1000, 7);
				}
			}
		}
		if (GetDistance(bullets[i].x + bullets[i].radius, bullets[i].y + bullets[i].radius, enemy.x + 8, enemy.y + 8) <= (8 + bullets[i].radius) * (8 + bullets[i].radius) && bullets[i].shotBy != 3 && bullets[i].active && bullets[i].collidable && enemy.active) {
			if (bullets[i].type != 6)
				bullets[i].active = SDL_FALSE;
			enemy.health -= bullets[i].damage;
			if (bullets[i].type != 6) {
				enemy.velX += 3 * cos(bullets[i].dir);
				enemy.velY += 3 * sin(bullets[i].dir);
			}
			else if (bullets[i].type == 6) {
				enemy.velX += .15 * cos(bullets[i].dir);
				enemy.velY += .15 * sin(bullets[i].dir);
			}
		}
		for (j = 0; j < maxAsteroids; j++) {
			if (bullets[i].active && bullets[i].collidable && asteroids[j].active) {
				float distance = GetDistance(bullets[i].x + bullets[i].radius, bullets[i].y + bullets[i].radius, asteroids[j].x + 16 * (asteroids[j].size + 1), asteroids[j].y + 16 * (asteroids[j].size + 1));
				if (distance <= (bullets[i].radius + 16 * (asteroids[j].size + 1)) * (bullets[i].radius + 16 * (asteroids[j].size + 1))) {
					asteroids[j].health -= bullets[i].damage;
					if (bullets[i].type != 6)
						bullets[i].life = 0;
					if (bullets[i].type != 6) {
						asteroids[j].velX += 1 * cos(bullets[i].dir);
						asteroids[j].velY += 1 * sin(bullets[i].dir);
					}
					else if (bullets[i].type == 6) {
						asteroids[j].velX += .05 * cos(bullets[i].dir);
						asteroids[j].velY += .05 * sin(bullets[i].dir);
					}
					break;
				}
			}
		}
		if (bullets[i].type == 7 && bullets[i].active && bullets[i].frame == 0) {
			for (j = 0; j < maxAsteroids; j++) {
				if (asteroids[j].active) {
					float distance = GetDistance(bullets[i].x + 16, bullets[i].y + 16, asteroids[j].x + 16 * (asteroids[j].size + 1), asteroids[j].y + 16 * (asteroids[j].size + 1));
					if (distance <= (bullets[i].radius + 16 * (asteroids[j].size + 1)) * (bullets[i].radius + 16 * (asteroids[j].size + 1))) {
						float angle = GetAngle(asteroids[j].x + 16 * (asteroids[j].size + 1), asteroids[j].y + 16 * (asteroids[j].size + 1), bullets[i].x + 16, bullets[i].y + 16);
						asteroids[j].velX += 1 * cos(angle);
						asteroids[j].velY += 1 * sin(angle);
					}
				}
			}
		}
		if (bullets[i].type == 6) {
			for (j = 0; j < maxBullets; j++) {
				if (bullets[j].type != 6) {
					float distance = GetDistance(bullets[i].x + bullets[i].radius, bullets[i].y + bullets[i].radius, bullets[j].x + bullets[j].radius, bullets[j].y + bullets[j].radius);
					if (distance < (bullets[i].radius + bullets[j].radius) * (bullets[i].radius + bullets[j].radius)) {
						bullets[j].velX += .1 * cos(bullets[i].dir);
						bullets[j].velY += .1 * sin(bullets[i].dir);
					}
				}
			}
		}
	}
	
	for (i = 0; i < maxItems; i++) {
		if (items[i].active) {
			for (j = 0; j < maxAsteroids; j++) {
				if (asteroids[j].active) {
					float distance = GetDistance(items[i].x + 8, items[i].y + 8, asteroids[j].x + 16 * (asteroids[j].size + 1), asteroids[j].y + 16 * (asteroids[j].size + 1));
					if (distance <= (8 + 16 * (asteroids[j].size + 1)) * (8 + 16 * (asteroids[j].size + 1))) {
						items[i].dir = GetAngle(items[i].x, items[i].y, asteroids[j].x, asteroids[j].y);
					}
				}
			}
			for (j = 0; j < 2; j++) {
				if (players[j].alive) {
					float distance = GetDistance(items[i].x + 8, items[i].y + 8, players[j].x + 8, players[j].y + 8);
					if (distance <= 256) {
						items[i].active = SDL_FALSE;
						if (items[i].type == 0) {
							players[j].health += 2;
							if (players[j].health > maxHealth)
								players[j].health = maxHealth;
						}
						else if (items[i].type == 1) {
							players[j].shield = SDL_TRUE;
						}
						else if (items[i].type == 2) {
							players[j].weaponName = "Rapid";
							players[j].weapon = 1;
							players[j].bulletTimerStart = 10;
						}
						else if (items[i].type == 3) {
							players[j].weaponName = "Triple";
							players[j].weapon = 2;
							players[j].bulletTimerStart = 50;
						}
						else if (items[i].type == 4) {
							players[j].weaponName = "Magnet";
							players[j].weapon = 3;
							players[j].bulletTimerStart = 15;
						}
						else if (items[i].type == 5) {
							players[j].weaponName = "Default";
							players[j].weapon = 0;
							players[j].bulletTimerStart = 30;
						}
						else if (items[i].type == 6) {
							players[j].weaponName = "R Triple";
							players[j].weapon = 2;
							players[j].bulletTimerStart = 10;
						}
					}
				}
			}
			for (j = 0; j < maxBullets; j++) {
				if (bullets[i].active && bullets[i].collidable) {
					float distance = GetDistance(items[i].x + 8, items[i].y + 8, bullets[j].x + bullets[j].radius, bullets[j].y + bullets[j].radius);
					if (distance <= (8 + bullets[j].radius) * (8 * bullets[j].radius)) {
						if (bullets[j].type != 6)
							bullets[j].life = 0;
						items[i].dir = bullets[j].dir;
					}
				}
			}
		}
	}
	
	for (i = 0; i < maxAsteroids; i++) {
		if (asteroids[i].active) {
			for (j = 0; j < 2; j++) {
				if (players[j].alive) {
					float distance = GetDistance(players[j].x + 8, players[j].y + 8, asteroids[i].x + 16 * (asteroids[i].size + 1), asteroids[i].y + 16 * (asteroids[i].size + 1));
					if (distance <= (8 + 14 * (asteroids[i].size + 1)) * (8 + 14 * (asteroids[i].size + 1))) {
						asteroids[i].active = SDL_FALSE;
						if (!players[j].shield) {
							if (players[j].invincibleTimer <= 0) {
								players[j].health--;
								players[j].invincibleTimer = players[j].invincibleTimerStart;
							}
							float playerAngle = GetAngle(players[j].x + 8, players[j].y + 8, asteroids[i].x + 16 * (asteroids[i].size + 1), asteroids[i].y + 16 * (asteroids[i].size + 1));
							players[j].velX = 1.5 * (asteroids[i].size + 1) * cos(playerAngle);
							players[j].velY = 1.5 * (asteroids[i].size + 1) * sin(playerAngle);
						}
						else {
							players[j].invincibleTimer = players[j].invincibleTimerStart;
							players[j].shield = SDL_FALSE;
							NewBullet(bullets, players[j].x - 8, players[j].y - 8, -1, 0, 0, 1000, 7);
						}
						NewBullet(bullets, asteroids[i].x, asteroids[i].y, -1, 0, 0, asteroids[i].size, 3);
						if (asteroids[i].size == 1) {
							int k;
							int angle = rand() % 360;
							for (k = 0; k < 4; k++)
								NewAsteroid(asteroids, asteroids[i].x, asteroids[i].y, angle + 90 * k, 0);
						}
					}
				}
			}
			if (enemy.active) {
				if (GetDistance(enemy.x + 8, enemy.y + 8, asteroids[i].x + 16 * (asteroids[i].size + 1), asteroids[i].y + 16 * (asteroids[i].size + 1)) <= (8 + 14 * (asteroids[i].size + 1)) * (8 + 14 * (asteroids[i].size + 1))) {
					asteroids[i].active = SDL_FALSE;
					enemy.health--;
					float enemyAngle = GetAngle(enemy.x + 8, enemy.y + 8, asteroids[i].x + 16 * (asteroids[i].size + 1), asteroids[i].y + 16 * (asteroids[i].size + 1));
					enemy.velX = 1.5 * (asteroids[i].size + 1) * cos(enemyAngle);
					enemy.velY = 1.5 * (asteroids[i].size + 1) * sin(enemyAngle);
					
					NewBullet(bullets, asteroids[i].x, asteroids[i].y, -1, 0, 0, asteroids[i].size, 3);
					if (asteroids[i].size == 1) {
						int k;
						int angle = rand() % 360;
						for (k = 0; k < 4; k++)
							NewAsteroid(asteroids, asteroids[i].x, asteroids[i].y, angle + 90 * k, 0);
					}
				}
			}
		}
	}
}

int Controls(struct Player players[], struct Bullet bullets[], struct Asteroid asteroids[]) {
	if (isKeyPressed(KEY_NSPIRE_CTRL) && gameState == 0) {
		show_1numeric_input("Set Starting Health", "", "", &maxHealth, 1, 100);
		InitPlayers(players);
	}
	if (isKeyPressed(KEY_NSPIRE_SHIFT) && gameState == 1)
		ActivateEnemy();
	if (isKeyPressed(KEY_NSPIRE_Z) && gameState == 1)
		SpawnAsteroids(asteroids);
	
	if (isKeyPressed(KEY_NSPIRE_1))
		players[0].health = 0;
	if ((isKeyPressed(KEY_NSPIRE_A) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_eEXP) && !PC_Controls)) {
		players[0].dir -= turnSpeed;
	}
	if ((isKeyPressed(KEY_NSPIRE_D) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_TENX) && !PC_Controls)) {
		players[0].dir += turnSpeed;
	}
	if ((isKeyPressed(KEY_NSPIRE_W) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_TAB) && !PC_Controls)) {
		players[0].velX += accSpeed * cos((players[0].dir * (PI / 180))) / 30;
		players[0].velY += accSpeed * sin((players[0].dir * (PI / 180))) / 30;
		players[0].moving = SDL_TRUE;
	}
	else
		players[0].moving = SDL_FALSE;
	if (((isKeyPressed(KEY_NSPIRE_SPACE) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_SCRATCHPAD) && !PC_Controls)) && players[0].alive) {
		if (players[0].bulletTimer <= 0) {
			players[0].bulletTimer = players[0].bulletTimerStart;
			float dir = players[0].dir * (PI / 180);
			if (players[0].weapon == 0)
				NewBullet(bullets, players[0].x + 8 + 5 * cos(dir), players[0].y + 8 + 5 * sin(dir), 0, dir, 3, 40, 0);
			else if (players[0].weapon == 1)
				NewBullet(bullets, players[0].x + 8 + 5 * cos(dir) - 4, players[0].y + 8 + 5 * sin(dir) - 4, 0, dir, 3, 30, 4);
			else if (players[0].weapon == 2) {
				float j;
				for (j = -.25; j <= .25; j += .25) {
					int amount = 5;
					if (j == 0)
						amount = 8;
					int x = players[0].x + 8 + amount * cos(dir);
					int y = players[0].y + 8 + amount * sin(dir);
					NewBullet(bullets, x, y, 0, dir + j, 3, 25, 5);	
				}
			}
			else if (players[0].weapon == 3)
				NewBullet(bullets, players[0].x + 8 + 5 * cos(dir) - 16, players[0].y + 8 + 5 * sin(dir) - 16, 0, dir, 3, 40, 6);
		}
	}
	
	if (isKeyPressed(KEY_NSPIRE_3))
		players[1].health = 0;
	if ((isKeyPressed(KEY_NSPIRE_J) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_PLUS) && !PC_Controls)) {
		players[1].dir -= turnSpeed;
	}
	if ((isKeyPressed(KEY_NSPIRE_L) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_MINUS) && !PC_Controls)) {
		players[1].dir += turnSpeed;
	}
	if ((isKeyPressed(KEY_NSPIRE_I) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_MENU) && !PC_Controls)) {
		players[1].velX += accSpeed * cos((players[1].dir * (PI / 180))) / 30;
		players[1].velY += accSpeed * sin((players[1].dir * (PI / 180))) / 30;
		players[1].moving = SDL_TRUE;
	}
	else
		players[1].moving = SDL_FALSE;
	if (((isKeyPressed(KEY_NSPIRE_PERIOD) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_DOC) && !PC_Controls)) && players[1].alive) {
		if (players[1].bulletTimer <= 0) {
			players[1].bulletTimer = players[1].bulletTimerStart;
			float dir = players[1].dir * (PI / 180);
			if (players[1].weapon == 0)
				NewBullet(bullets, players[1].x + 8 + 5 * cos(dir), players[1].y + 8 + 5 * sin(dir), 1, dir, 3, 40, 0);
			else if (players[1].weapon == 1)
				NewBullet(bullets, players[1].x + 8 + 5 * cos(dir) - 4, players[1].y + 8 + 5 * sin(dir) - 4, 1, dir, 3, 30, 4);
			else if (players[1].weapon == 2) {
				float j;
				for (j = -.25; j <= .25; j += .25) {
					int amount = 5;
					if (j == 0)
						amount = 8;
					int x = players[1].x + 8 + amount * cos(dir);
					int y = players[1].y + 8 + amount * sin(dir);
					NewBullet(bullets, x, y, 1, dir + j, 3, 25, 5);	
				}
			}
			else if (players[1].weapon == 3)
				NewBullet(bullets, players[1].x + 8 + 5 * cos(dir) - 16, players[1].y + 8 + 5 * sin(dir) - 16, 1, dir, 3, 40, 6);
		}
	}
	
	return 0;
}

void doFrame(SDL_bool draw, struct Player players[], struct Bullet bullets[], struct Asteroid asteroids[], struct Particle particles[],
				struct Item items[], SDL_Surface *ship, SDL_Surface *bullet, SDL_Surface *smallBullet, SDL_Surface *tripleBullet, SDL_Surface *magnetBullet,
				SDL_Surface *explosion,	SDL_Surface *flame, SDL_Surface *smallAsteroid, SDL_Surface *bigAsteroid, SDL_Surface *asteroidExplosion,
				SDL_Surface *smallAsteroidExplosion, SDL_Surface *numbers, SDL_Surface *itemImage, SDL_Surface *shield, SDL_Surface *shieldExplosion,
				SDL_Surface *enemyImage, nSDL_Font *font, SDL_Surface *screen) {

	Controls(players, bullets, asteroids);
	if ((isKeyPressed(KEY_NSPIRE_ESC) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_SHIFT) && !PC_Controls))
		done = SDL_TRUE;
	if (isKeyPressed(KEY_NSPIRE_DEL)) {
		waitTimer = waitTimerStart;
		over = SDL_FALSE;
		gameState = 0;
		startTimer = startTimerStart;
		InitPlayers(players);
		InitEnemy(enemy);
		ResetBullets(bullets);
		ResetParticles(particles);
		InitStars(particles);
		ResetAsteroids(asteroids);
		ResetItems(items);
		InitEnemy();
	}
	
	if (startTimer < startTimerStart) {
		startTimer--;
		if (startTimer <= 0) {
			gameState = 1;
			startTimer = startTimerStart;
		}
	}
	
	UpdatePlayers(players, bullets, particles);
	UpdateEnemy(bullets, particles, players, asteroids, items);
	UpdateBullets(bullets, particles);
	UpdateParticles(particles);
	UpdateAsteroids(asteroids, bullets);
	UpdateItems(items);
	
	if (gameState == 1) {
		if (!over) {
			Collide(bullets, players, asteroids, items);
			SpawnAsteroids(asteroids);
			SpawnItems(items);
			if (rand() % 20000 == 0 && !enemy.active)
				ActivateEnemy();
		}
		else {
			waitTimer--;
			if (waitTimer < 0) {
				waitTimer = waitTimerStart;
				over = SDL_FALSE;
				gameState = 0;
				startTimer = startTimerStart;
				InitPlayers(players);
				InitEnemy(enemy);
				ResetBullets(bullets);
				ResetParticles(particles);
				InitStars(particles);
				ResetAsteroids(asteroids);
				ResetItems(items);
				InitEnemy();
			}
		}
	}
	else if (((isKeyPressed(KEY_NSPIRE_ENTER) && PC_Controls) || (isKeyPressed(KEY_NSPIRE_VAR) && !PC_Controls)) && startTimer == startTimerStart) {
		startTimer--;
	}

	if(draw) {
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		DrawParticles(particles, screen);
		DrawAsteroids(asteroids, smallAsteroid, bigAsteroid, screen);
		DrawItems(items, itemImage, screen);
		DrawPlayers(players, screen, ship, shield);
		DrawEnemy(enemyImage, screen);
		DrawBullets(bullets, bullet, smallBullet, tripleBullet, magnetBullet, explosion, flame, asteroidExplosion, smallAsteroidExplosion, shieldExplosion, screen);
		DrawHealth(players, font, screen);

		if (gameState == 0) {
			if (startTimer == startTimerStart) {
				if (isKeyPressed(KEY_NSPIRE_ENTER)) {
					nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "Wins") / 2, SCREEN_HEIGHT / 2 - 25, "Wins");
					nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "Blue: 0") / 2, SCREEN_HEIGHT / 2 - 5, "Blue: %i", players[0].wins);
					nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "Red: 0") / 2, SCREEN_HEIGHT / 2 + 15, "Red: %i", players[1].wins);
				}
				else {
					if (PC_Controls)
						nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "Press Enter To Start") / 2, SCREEN_HEIGHT / 2 - 5, "Press Enter To Start");
					else
						nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "Press Var To Start") / 2, SCREEN_HEIGHT / 2 - 5, "Press Var To Start");
				}
			}
			if (startTimer < startTimerStart)
				DrawScore(numbers, screen, SCREEN_WIDTH / 2 - 14, SCREEN_HEIGHT / 2 - 10, (startTimer + 50) / 50);
		}

		if (over) {
			if (players[0].won)
				nSDL_DrawString(screen, font, players[0].x + 8 - nSDL_GetStringWidth(font, "Win") / 2, players[0].y - 8, "Win");
			else if (players[1].won)
				nSDL_DrawString(screen, font, players[1].x + 8 - nSDL_GetStringWidth(font, "Win") / 2, players[1].y - 8, "Win");
		}

		SDL_Flip(screen);
	}
}

void resyncFrameTime() {
    nextFrame = SDL_GetTicks();
    framesSkipped = 0;
}

int main(int arc, char **argv) {
	srand(time(0));
	enable_relative_paths(argv);
	SDL_Surface *screen = SDL_SetVideoMode(320, 240, is_cx ? 16 : 8, SDL_SWSURFACE);
	SDL_Surface *numbers = SDL_LoadBMP("Resources\\numbers.bmp.tns");
	//SDL_Surface *win = SDL_LoadBMP("Resources\\win.bmp.tns");
	SDL_Surface *ship = SDL_LoadBMP("Resources\\ship.bmp.tns");
	SDL_Surface *bullet = SDL_LoadBMP("Resources\\bullet.bmp.tns");
	SDL_Surface *smallBullet = SDL_LoadBMP("Resources\\small bullet.bmp.tns");
	SDL_Surface *tripleBullet = SDL_LoadBMP("Resources\\triple bullet.bmp.tns");
	SDL_Surface *magnetBullet = SDL_LoadBMP("Resources\\magnet bullet.bmp.tns");
	SDL_Surface *explosion = SDL_LoadBMP("Resources\\explosion.bmp.tns");
	SDL_Surface *flame= SDL_LoadBMP("Resources\\flame.bmp.tns");
	SDL_Surface *smallAsteroid = SDL_LoadBMP("Resources\\smallAsteroid.bmp.tns");
	SDL_Surface *bigAsteroid = SDL_LoadBMP("Resources\\bigAsteroid.bmp.tns");
	SDL_Surface *asteroidExplosion = SDL_LoadBMP("Resources\\asteroidExplosion.bmp.tns");
	SDL_Surface *smallAsteroidExplosion = SDL_LoadBMP("Resources\\smallAsteroidExplosion.bmp.tns");
	SDL_Surface *itemImage = SDL_LoadBMP("Resources\\Items.bmp.tns");
	SDL_Surface *shield = SDL_LoadBMP("Resources\\shield.bmp.tns");
	SDL_Surface *shieldExplosion = SDL_LoadBMP("Resources\\shield explosion.bmp.tns");
	SDL_Surface *enemyImage = SDL_LoadBMP("Resources\\enemy.bmp.tns");
	SDL_SetColorKey(bullet, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(bullet->format, 255, 0, 255));
	SDL_SetColorKey(smallBullet, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(smallBullet->format, 255, 0, 255));
	SDL_SetColorKey(tripleBullet, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(tripleBullet->format, 255, 0, 255));
	SDL_SetColorKey(magnetBullet, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(magnetBullet->format, 255, 0, 255));
	SDL_SetColorKey(ship, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(ship->format, 255, 0, 255));
	SDL_SetColorKey(explosion, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(explosion->format, 255, 0, 255));
	SDL_SetColorKey(flame, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(flame->format, 255, 0, 255));
	SDL_SetColorKey(smallAsteroid, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(smallAsteroid->format, 255, 0, 255));
	SDL_SetColorKey(bigAsteroid, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(bigAsteroid->format, 255, 0, 255));
	SDL_SetColorKey(asteroidExplosion, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(asteroidExplosion->format, 255, 0, 255));
	SDL_SetColorKey(smallAsteroidExplosion, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(smallAsteroidExplosion->format, 255, 0, 255));
	SDL_SetColorKey(numbers, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(numbers->format, 255, 0, 255));
	SDL_SetColorKey(itemImage, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(itemImage->format, 255, 0, 255));
	SDL_SetColorKey(shield, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(shield->format, 255, 0, 255));
	SDL_SetColorKey(shieldExplosion, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(shieldExplosion->format, 255, 0, 255));
	SDL_SetColorKey(enemyImage, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(enemyImage->format, 255, 0, 255));
	
	nSDL_Font *font = nSDL_LoadFont(NSDL_FONT_TINYTYPE,255, 255, 255);
	
	struct Bullet bullets[maxBullets];
	struct Particle particles[maxParticles];
	struct Player players[2];
	struct Asteroid asteroids[maxAsteroids];
	struct Item items[maxItems];
	
	
	Init(&screen);
	InitPlayers(players);
	InitEnemy();
	ResetBullets(bullets);
	ResetParticles(particles);
	InitStars(particles);
	ResetAsteroids(asteroids);
	ResetItems(items);
	
	itemSpawnTimer = rand() % 200 + 200;
	players[0].wins = 0;
	players[1].wins = 0;
	
    while(!done) {
		Sint32 dif = (Sint32)(SDL_GetTicks() - nextFrame);

		if(dif < 0)
		{
			SDL_Delay(1);
			continue;
		}

		nextFrame += frameLength;
		if(dif >= (frameLength * skipFramesBehind))
		{
			if(framesSkipped >= maxFrameSkip)
				resyncFrameTime();
			else
			{
				++framesSkipped;
				doFrame(SDL_FALSE, players, bullets, asteroids, particles, items, ship, bullet, smallBullet, tripleBullet, magnetBullet, explosion, flame, smallAsteroid, bigAsteroid, asteroidExplosion, smallAsteroidExplosion, numbers, itemImage, shield, shieldExplosion, enemyImage, font, screen);
				continue;
			}
		}

		framesSkipped = 0;
		doFrame(SDL_TRUE, players, bullets, asteroids, particles, items, ship, bullet, smallBullet, tripleBullet, magnetBullet, explosion, flame, smallAsteroid, bigAsteroid, asteroidExplosion, smallAsteroidExplosion, numbers, itemImage, shield, shieldExplosion, enemyImage, font, screen);
    }
	return 1;
}







