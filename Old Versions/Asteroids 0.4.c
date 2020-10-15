#include <os.h>
#include <SDL/SDL.h>
#include <math.h>
#include <time.h>

float PI = 3.14159;
SDL_bool done = SDL_FALSE;
int winningScore = 10;
const int maxBullets = 30;
const int maxParticles = 60;
int maxSpeed = 5;
float turnSpeed = 4;
float accSpeed = 1.5;

struct Player {
	float x, y, velX, velY, dir, moveDir, speed;
	int bulletTimerStart, bulletTimer, maxHealth, health;
	SDL_bool moving, alive, exploded;
};

struct Bullet {
	float x, y, speed, dir;
	int shotBy, life, frame, frameTimerStart, frameTimer, type;
	SDL_bool active, collidable;
};

struct Particle {
	int life, r, g, b, type, size;
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

/*void DrawScore(SDL_Surface *numbers, SDL_Surface *screen, int x, int y, int score) {
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
}*/

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
			if (type == 0) {
				bullets[i].frame = rand() % 5;
				bullets[i].collidable = SDL_TRUE;
				bullets[i].frameTimerStart = 3;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
			}
			else if (type == 1) {
				bullets[i].collidable = SDL_FALSE;
				bullets[i].frame = life;
				bullets[i].frameTimerStart = 1;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
			}
			else if (type == 2) {
				bullets[i].collidable = SDL_FALSE;
				bullets[i].frame = 0;
				bullets[i].frameTimerStart = 1000;
				bullets[i].frameTimer = bullets[i].frameTimerStart;
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
		players[i].maxHealth = 2;
		players[i].health = players[i].maxHealth;
		players[i].exploded = SDL_FALSE;
		players[i].alive = SDL_TRUE;
	}
};

void UpdatePlayers(struct Player players[], struct Bullet bullets[], struct Particle particles[]) {
	int i;
	for (i = 0; i < 2; i++) {
		players[i].x += players[i].velX;
		players[i].y += players[i].velY;
		
		if (players[i].bulletTimer > 0)
			players[i].bulletTimer--;
		
		if (players[i].x < -15)
			players[i].x = SCREEN_WIDTH;
		else if (players[i].x > SCREEN_WIDTH)
			players[i].x = -15;
		
		if (players[i].y < -15)
			players[i].y = SCREEN_HEIGHT;
		else if (players[i].y > SCREEN_HEIGHT)
			players[i].y = -15;
		
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
}

void DrawPlayers(struct Player players[], SDL_Surface *screen, SDL_Surface *ship) {
	int i;
	for (i = 0; i < 2; i++) {
		if (players[i].alive) {
			SDL_Rect dest;
			
			SDL_Rect size;
			size.x = 16 * players[i].moving;
			size.y = 0;
			size.w = 16;
			size.h = 16;
			
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

void DrawHealth(struct Player players[], SDL_Surface *screen) {
	SDL_Rect rect;
	rect.x = 1;
	rect.y = 1;
	rect.w = 50;
	rect.h = 4;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 0));
	rect.w = ((float)players[0].health / (float)players[0].maxHealth) * 50;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 255, 0));
	
	rect.x = SCREEN_WIDTH - 51;
	rect.y = 1;
	rect.w = 50;
	rect.h = 4;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 0, 0));
	rect.x = SCREEN_WIDTH - ((float)players[1].health / (float)players[1].maxHealth) * 51;
	rect.w = ((float)players[1].health / (float)players[1].maxHealth) * 50;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 255, 0));
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
	}
};

void UpdateBullets(struct Bullet bullets[], struct Particle particles[]) {
	int i, j;
	for (i = 0; i < maxBullets; i++) {
		if (bullets[i].active) {
			if (bullets[i].type == 0 || bullets[i].type == 2) {
				bullets[i].life--;
				if (bullets[i].life < 0) {
					bullets[i].active = SDL_FALSE;
					int amount = rand() % 5 + 10;
					for (j = 0; j < amount; j++) {
						if (bullets[i].type == 0)
							NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, rand() % 255, rand() % 255, rand() % 255, 1, rand() % 10 + 10, 1, (rand() % 361 - 180) * (PI / 180), rand() % 200 * .01);
						else if (bullets[i].type == 2)
							NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, 255, rand() % 255, 0, 1, rand() % 10 + 10, 1, (rand() % 361 - 180) * (PI / 180), rand() % 200 * .01);
					}
				}
			}
			bullets[i].x += bullets[i].speed * cos(bullets[i].dir);
			bullets[i].y += bullets[i].speed * sin(bullets[i].dir);
			
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
			}
			
			if (bullets[i].x < -8)
				bullets[i].x = SCREEN_WIDTH;
			else if (bullets[i].x > SCREEN_WIDTH)
				bullets[i].x = -8;
			
			if (bullets[i].y < -8)
				bullets[i].y = SCREEN_HEIGHT;
			else if (bullets[i].y > SCREEN_HEIGHT)
				bullets[i].y = -8;
			
			if (rand() % 3 == 0 && (bullets[i].type == 0 || bullets[i].type == 2)) {
				if (bullets[i].type == 0)
					NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, 0, 142, 92, 0, 40, 1, 0, 0);
				else if (bullets[i].type == 2)
					NewParticle(particles, (int)bullets[i].x + rand() % 3 - 1, (int)bullets[i].y + rand() % 3 - 1, 255, rand() % 255, 0, 0, 40, 1, 0, 0);
			}
		}
	}
}

void CollideBullets(struct Bullet bullets[], struct Player players[]) {
	int i, j;
	for (i = 0; i < maxBullets; i++) {
		for (j = 0; j < 2; j++) {
			float distance = GetDistance(bullets[i].x + 4, bullets[i].y + 4, players[j].x + 8, players[j].y + 8);
			if (distance <= 144 && bullets[i].shotBy != j && bullets[i].active && bullets[i].collidable) {
				bullets[i].active = SDL_FALSE;
				players[j].health--;
				float angle = GetAngle(players[j].x + 8, players[j].y + 8, bullets[i].x + 4, bullets[i].y + 4);
				players[j].velX += 4 * cos(angle);
				players[j].velY += 4 * sin(angle);
			}
		}
	}
}

void DrawBullets(struct Bullet bullets[], SDL_Surface *bullet, SDL_Surface *explosion, SDL_Surface *flame, SDL_Surface *screen) {
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
				
				SDL_Rect rect;
				rect.x = bullets[i].x;
				rect.y = bullets[i].y;
				rect.w = 8;
				rect.h = 8;
				
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
				
				SDL_Surface *tmp1 = SDL_CreateRGBSurface(0, 64, 64, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
				SDL_BlitSurface(explosion, &size, tmp1, NULL);
				
				SDL_BlitSurface(tmp1, NULL, screen, &dest);
				SDL_FreeSurface(tmp1);
			}
			else if (bullets[i].type == 2) {
				SDL_Rect dest;
				dest.x = bullets[i].x + 4;
				dest.y = bullets[i].y + 4;
				
				SDL_BlitSurface(flame, NULL, screen, &dest);
			}
		}
	}
};

int Controls(struct Player players[], struct Bullet bullets[], int gameState) {
	if (isKeyPressed(KEY_NSPIRE_ESC))
		done = SDL_TRUE;
	
	if (isKeyPressed(KEY_NSPIRE_DEL)) {
		return 1;
	}
	
	if (isKeyPressed(KEY_NSPIRE_CTRL) && gameState == 0) {
		show_1numeric_input("Set Winning Score", "", "", &winningScore, 1, 100);
	}
 
	if (isKeyPressed(KEY_NSPIRE_A)) {
		players[0].dir -= turnSpeed;
	}
	if (isKeyPressed(KEY_NSPIRE_D)) {
		players[0].dir += turnSpeed;
	}
	if (isKeyPressed(KEY_NSPIRE_W)) {
		players[0].velX += accSpeed * cos((players[0].dir * (PI / 180))) / 30;
		players[0].velY += accSpeed * sin((players[0].dir * (PI / 180))) / 30;
		players[0].moving = SDL_TRUE;
	}
	else
		players[0].moving = SDL_FALSE;
	if (isKeyPressed(KEY_NSPIRE_SPACE) && players[0].alive) {
		if (players[0].bulletTimer <= 0) {
			players[0].bulletTimer = players[0].bulletTimerStart;
			float dir = players[0].dir * (PI / 180);
			NewBullet(bullets, players[0].x + 8 + 5 * cos(dir), players[0].y + 8 + 5 * sin(dir), 0, dir, 3, 40, 0);
		}
	}
	
	if (isKeyPressed(KEY_NSPIRE_J)) {
		players[1].dir -= turnSpeed;
	}
	if (isKeyPressed(KEY_NSPIRE_L)) {
		players[1].dir += turnSpeed;
	}
	if (isKeyPressed(KEY_NSPIRE_I)) {
		players[1].velX += cos((players[1].dir * (PI / 180))) / 30;
		players[1].velY += sin((players[1].dir * (PI / 180))) / 30;
		players[1].moving = SDL_TRUE;
	}
	else
		players[1].moving = SDL_FALSE;
	if (isKeyPressed(KEY_NSPIRE_PERIOD) && players[1].alive) {
		if (players[1].bulletTimer <= 0) {
			players[1].bulletTimer = players[1].bulletTimerStart;
			float dir = players[1].dir * (PI / 180);
			NewBullet(bullets, players[1].x + 8 + 5 * cos(dir), players[1].y + 8 + 5 * sin(dir), 1, dir, 3, 40, 0);
		}
	}
	
	return 0;
}

int main(int arc, char **argv) {
	srand(time(0));
	enable_relative_paths(argv);
	SDL_Surface *screen = SDL_SetVideoMode(320, 240, is_cx ? 16 : 8, SDL_SWSURFACE);
	//SDL_Surface *numbers = SDL_LoadBMP("Resources\\numbers.bmp.tns");
	//SDL_Surface *win = SDL_LoadBMP("Resources\\win.bmp.tns");
	SDL_Surface *ship = SDL_LoadBMP("Resources\\ship.bmp.tns");
	SDL_Surface *bullet = SDL_LoadBMP("Resources\\bullet.bmp.tns");
	SDL_Surface *explosion = SDL_LoadBMP("Resources\\explosion.bmp.tns");
	SDL_Surface *flame = SDL_LoadBMP("Resources\\flame.bmp.tns");
	SDL_SetColorKey(bullet, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(bullet->format, 255, 0, 255));
	SDL_SetColorKey(ship, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(ship->format, 255, 0, 255));
	SDL_SetColorKey(explosion, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(explosion->format, 255, 0, 255));
	SDL_SetColorKey(flame, (SDL_SRCCOLORKEY | SDL_RLEACCEL), SDL_MapRGB(flame->format, 255, 0, 255));
	
	nSDL_Font *font = nSDL_LoadFont(NSDL_FONT_TINYTYPE,255, 255, 255);
	
	float frameTime = 0;
	int gameState = 0;
	int prevTime = 0;
	int curTime = 0;
	float deltaTime = 0;
	int lastScored = rand() % 2;
	int waitTimerStart = 50;
	int waitTimer = waitTimerStart;
	SDL_bool scored = SDL_TRUE;
	SDL_bool over = SDL_FALSE;
	
	struct Bullet bullets[maxBullets];
	struct Particle particles[maxParticles];
	struct Player players[2];
	
	Init(&screen);
	InitPlayers(players);
	ResetBullets(bullets);
	ResetParticles(particles);
	
	while (!done) {
		prevTime = curTime;
		curTime = SDL_GetTicks();
		deltaTime = (curTime - prevTime) / 1000.0f;
		frameTime += deltaTime;
		
		if (frameTime >= 1.0/60.0) {
			frameTime = 0;
			
			int control = Controls(players, bullets, gameState);
			
			if (control == 1) {
				scored = SDL_TRUE;
				over = SDL_FALSE;
				gameState = 0;
				lastScored = rand() % 2;
				waitTimer = waitTimerStart;
				InitPlayers(players);
				ResetBullets(bullets);
			}
			
			if (gameState == 1) {
				if (!over) {
					//if (!scored)
						//UpdateBall(&ball, players, &lastScored, &scored);
					UpdatePlayers(players, bullets, particles);
					UpdateBullets(bullets, particles);
					CollideBullets(bullets, players);
					UpdateParticles(particles);
					
					/*if (scored) {
						if (players[0].score == winningScore || players[1].score == winningScore) {
							over = SDL_TRUE;
							waitTimer = 200;
						}
						waitTimer--;
						if (waitTimer < 0) {
							waitTimer = waitTimerStart;
							if (lastScored == 0) {
								ball.dir = 0 + (rand() % 51 - 25) * (M_PI / 180);
								ball.y = players[1].y + 15;
							}
							else {
								ball.dir = PI + (rand() % 51 - 25) * (M_PI / 180);
								ball.y = players[0].y + 15;
							}
							ball.speed = 2;
							scored = SDL_FALSE;
						}
					}*/
				}
				else {
					waitTimer--;
					if (waitTimer < 0) {
						waitTimer = waitTimerStart;
						scored = SDL_TRUE;
						over = SDL_FALSE;
						gameState = 0;
						lastScored = rand() % 2;
						waitTimer = waitTimerStart;
						InitPlayers(players);
						ResetBullets(bullets);
						ResetParticles(particles);
					}
				}
			}
			else if (any_key_pressed()) {
				if (!isKeyPressed(KEY_NSPIRE_DEL) && !isKeyPressed(KEY_NSPIRE_CTRL) && !isKeyPressed(KEY_NSPIRE_ENTER)) {
					gameState = 1;
					/*if (rand() % 2 == 0)
						ball.dir = 0 + (rand() % 51 - 25) * (M_PI / 180);
					else
						ball.dir = PI + (rand() % 51 - 25) * (M_PI / 180);
					ball.speed = 2;*/
				}
			}
			
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
			//DrawScore(numbers, screen, SCREEN_WIDTH / 2 - 25 - (numbers->w / 10 * 2), 25, players[0].score);
			//DrawScore(numbers, screen, SCREEN_WIDTH / 2 + 25, 25, players[1].score);
			DrawParticles(particles, screen);
			DrawPlayers(players, screen, ship);
			DrawBullets(bullets, bullet, explosion, flame, screen);
			DrawHealth(players, screen);
			if (gameState == 0) {
				SDL_Rect menu;
				menu.x = SCREEN_WIDTH / 2 - 50;
				menu.y = SCREEN_HEIGHT / 2 - 10;
				menu.w = 100;
				menu.h = 28;

				nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "Press Any Key To Start") / 2, SCREEN_HEIGHT / 2 - 5, "Press Any Key To Start");
				nSDL_DrawString(screen, font, SCREEN_WIDTH / 2 - nSDL_GetStringWidth(font, "First to 10 wins") / 2, SCREEN_HEIGHT / 2 + 5, "First to %i wins", winningScore);
			}
			/*if (over) {
				SDL_Rect dest;
				if (players[0].score == winningScore)
					dest.x = SCREEN_WIDTH / 2 - SCREEN_WIDTH / 4 - win->w / 2;
				else
					dest.x = SCREEN_WIDTH / 2 + SCREEN_WIDTH / 4 - win->w / 2;
				dest.y = 100;
				SDL_BlitSurface(win, NULL, screen, &dest);
			}*/
			
			SDL_Flip(screen);
		}
	};
	
	return 0;
}









