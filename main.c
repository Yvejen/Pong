#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TARGET_FPS 60

SDL_Window *g_win;
SDL_Renderer *g_rend;
struct {
  Uint64 now;
  Uint64 last;
  float delta_t;
} time;

const unsigned screen_width = 640, screen_height = 400;

struct Rect {
  float posX, posY;
  unsigned sizeX, sizeY;
};

struct Paddle {
  struct Rect r;
  float velY;
};

struct Ball {
  struct Rect r;
  float velX, velY;
};

enum Player { PLAYER_ONE, PLAYER_TWO };

struct GameState {
  struct Paddle paddles[2];
  int score[2];
  struct Ball ball;
  int run;
} g_state;

struct sdl_initinfo {
  int width, height;
};

int init_SDL(struct sdl_initinfo *info) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to start SDL!\n");
    return -1;
  }
  g_win =
      SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       info->width, info->height, SDL_WINDOW_SHOWN);
  if (!g_win) {
    fprintf(stderr, "Window creation failed!\n");
    return -1;
  }
  g_rend = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
  if (!g_rend) {
    fprintf(stderr, "Failed to get renderer: %s!\n", SDL_GetError());
    return -1;
  }
  return 0;
}

int close_SDL() {
  SDL_DestroyRenderer(g_rend);
  g_rend = NULL;
  SDL_DestroyWindow(g_win);
  g_win = NULL;
  SDL_Quit();
  return 0;
}

void draw_rect(struct Rect *r) {
  SDL_Rect fillRect = {(int)round(r->posX) - r->sizeX / 2,
                       (int)round(r->posY) - r->sizeY / 2, r->sizeX, r->sizeY};
  SDL_SetRenderDrawColor(g_rend, 255, 255, 255, 255);
  SDL_RenderFillRect(g_rend, &fillRect);
}

void draw_scene(struct GameState *s) {
  SDL_SetRenderDrawColor(g_rend, 0, 0, 0, 255);
  SDL_RenderClear(g_rend);
  draw_rect(&s->ball.r);
  draw_rect(&s->paddles[0].r);
  draw_rect(&s->paddles[1].r);
  SDL_RenderPresent(g_rend);
}

void calc_delta_time() {
  time.last = time.now;
  time.now = SDL_GetPerformanceCounter();
  /*Delta t in ms*/
  time.delta_t = (float)(time.now - time.last) * 1000 /
                 (float)SDL_GetPerformanceFrequency();
  /*Reject large times due to pauses*/
  if (time.delta_t > 20)
    time.delta_t = 0;
}

struct Ball new_ball() {
  static int side = 1;
  side *= -1; /*Switch sides everytime a ball is spawned*/
  float ur = rand() / (float)RAND_MAX * 2 * M_PI - M_PI;
  float rad1 = atan(ur) * M_PI / 4;
  const float init_speed = 0.2;
  return (struct Ball){
      {(float)screen_width / 2, (float)screen_height / 2, 15, 15},
      init_speed * cos(rad1) * side,
      init_speed * sin(rad1) * side};
}

int check_rect_rect_col(struct Rect *r1, struct Rect *r2) {
  float d1x = fabs(r1->posX - r2->posX);
  float d1y = fabs(r1->posY - r2->posY);
  return (d1x < (r1->sizeX + r2->sizeX) / 2.0f &&
          d1y < (r1->sizeY + r2->sizeY) / 2.0f);
}

void update_ball(struct Ball *p, struct GameState *s) {
  p->r.posX += time.delta_t * p->velX;
  p->r.posY += time.delta_t * p->velY;
  /*Check collision*/
  if (p->r.posX < 0) {
    s->score[PLAYER_TWO] += 1;
    printf("Player 2 scored: %d\n", s->score[PLAYER_TWO]);
    s->ball = new_ball();
  }
  if (p->r.posX > screen_width) {
    s->score[PLAYER_ONE] += 1;
    printf("Player 1 scored: %d\n", s->score[PLAYER_ONE]);
    s->ball = new_ball();
  }
  if (p->r.posY < 0 || p->r.posY > screen_height) {
    p->velY *= -1;
  }
  if (check_rect_rect_col(&p->r, &s->paddles[0].r) ||
      check_rect_rect_col(&p->r, &s->paddles[1].r)) {
    p->velX *= -1;
  }
}

void update_paddle(struct Paddle *p) {
  p->r.posY -= time.delta_t * p->velY;
  if (p->r.posY > screen_height)
    p->r.posY = screen_height;
  if (p->r.posY < 0)
    p->r.posY = 0;
}

void update_scene(struct GameState *s) {
  calc_delta_time();
  update_ball(&s->ball, s);
  update_paddle(&s->paddles[0]);
  update_paddle(&s->paddles[1]);
}

void handle_events(struct GameState *s) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      s->run = 0;
  }
  const Uint8 *keyState = SDL_GetKeyboardState(NULL);
  s->paddles[PLAYER_ONE].velY = s->paddles[PLAYER_TWO].velY = 0;
  if (keyState[SDL_SCANCODE_UP])
    s->paddles[PLAYER_ONE].velY += 1;
  if (keyState[SDL_SCANCODE_DOWN])
    s->paddles[PLAYER_ONE].velY -= 1;
  if (keyState[SDL_SCANCODE_I])
    s->paddles[PLAYER_TWO].velY += 1;
  if (keyState[SDL_SCANCODE_K])
    s->paddles[PLAYER_TWO].velY -= 1;
}

int event_loop(struct GameState *s) {

  while (s->run) {
    handle_events(s);
    update_scene(s);
    draw_scene(s);
    SDL_Delay(16);
  }
  return 0;
}

void new_game(struct GameState *s) {
  s->paddles[0] = (struct Paddle){
      {screen_width * 0.02, (float)screen_height / 2, 20, 50}, 0};
  s->paddles[1] = (struct Paddle){
      {screen_width * 0.98, (float)screen_height / 2, 20, 50}, 0};
  s->ball = new_ball();
  s->score[0] = s->score[1] = 0;
  s->run = 1;
}

int main(int argc, char *argv[]) {
  if (init_SDL(&(struct sdl_initinfo){screen_width, screen_height}) < 0)
    return -1;
  new_game(&g_state);
  event_loop(&g_state);
  close_SDL();
  return 0;
}
