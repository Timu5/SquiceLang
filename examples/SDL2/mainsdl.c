#define SL_IMPLEMENTATION

#include "SDL.h"

#include <stdio.h>
#include <signal.h>

#include "SquiceLang.h"
#include "main.c"


sl_ctx_t *global;

sl_vector(sl_binary_t *) modules = NULL;

static void binding_SDL_Init(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    int b = (int)sl_vector_pop(ctx->stack)->number;
    int ret = SDL_Init(b);
    sl_vector_push(ctx->stack, sl_value_number(ret));
}

static void binding_SDL_Delay(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    int b = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Delay(b);
}

static void binding_SDL_Quit(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Quit();
}

static void binding_SDL_CreateWindow(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    char *title = sl_vector_pop(ctx->stack)->string;
    int x = (int)sl_vector_pop(ctx->stack)->number;
    int y = (int)sl_vector_pop(ctx->stack)->number;
    int w = (int)sl_vector_pop(ctx->stack)->number;
    int h = (int)sl_vector_pop(ctx->stack)->number;
    int flags = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Window *window = SDL_CreateWindow(title, x, y, w, h, flags);
    sl_vector_push(ctx->stack, sl_value_native(window));
}

static void binding_SDL_DestroyWindow(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    sl_value_t *win = sl_vector_pop(ctx->stack);
    SDL_Window *window = (SDL_Window *)(win->ref);
    SDL_DestroyWindow(window);
}

static void binding_SDL_CreateRenderer(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Window *window = (SDL_Window *)sl_vector_pop(ctx->stack)->ref;
    int index = (int)sl_vector_pop(ctx->stack)->number;
    int flags = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Renderer *ren = SDL_CreateRenderer(window, index, flags);
    sl_vector_push(ctx->stack, sl_value_native(ren));
}

static void binding_SDL_DestroyRenderer(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    sl_value_t *ren = sl_vector_pop(ctx->stack);
    SDL_Renderer *renderer = (SDL_Renderer *)(ren->ref);
    SDL_DestroyRenderer(renderer);
}

static void binding_SDL_RenderClear(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    sl_value_t *ren = sl_vector_pop(ctx->stack);
    SDL_Renderer *renderer = (SDL_Renderer *)(ren->ref);
    SDL_RenderClear(renderer);
}

static void binding_SDL_RenderPresent(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    sl_value_t *ren = sl_vector_pop(ctx->stack);
    SDL_Renderer *renderer = (SDL_Renderer *)(ren->ref);
    SDL_RenderPresent(renderer);
}

//SDL_LoadBMP(string)
static void binding_SDL_LoadBMP(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    char *file = sl_vector_pop(ctx->stack)->string;
    SDL_Surface *surface = SDL_LoadBMP(file);
    sl_vector_push(ctx->stack, sl_value_native(surface));
}

//SDL_CreateTextureFromSurface(ren, bmp);
static void binding_SDL_CreateTextureFromSurface(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Renderer *ren = (SDL_Renderer *)sl_vector_pop(ctx->stack)->ref;
    SDL_Surface *sur = (SDL_Surface *)sl_vector_pop(ctx->stack)->ref;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, sur);
    sl_vector_push(ctx->stack, sl_value_native(tex));
}

//SDL_RenderCopy(ren, tex, NULL, NULL);
static void binding_SDL_RenderCopy(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Renderer *ren = (SDL_Renderer *)sl_vector_pop(ctx->stack)->ref;
    SDL_Texture *tex = (SDL_Texture *)sl_vector_pop(ctx->stack)->ref;
    int x = (int)sl_vector_pop(ctx->stack)->number;
    int y = (int)sl_vector_pop(ctx->stack)->number;
    int w = (int)sl_vector_pop(ctx->stack)->number;
    int h = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Rect dst = {x, y, w, h};
    //sl_value_t *src = sl_vector_pop(ctx->stack);
    //sl_value_t *dst = sl_vector_pop(ctx->stack);
    SDL_RenderCopy(ren, tex, NULL, &dst);
}

//SDL_PollEvent(&test_event)
static void binding_SDL_PollEvent(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    SDL_Event event;
    if (!SDL_PollEvent(&event))
    {
        sl_vector_push(ctx->stack, sl_value_null());
        return;
    }

    sl_vector(char *) names = NULL;
    sl_vector(sl_vector_t *) values = NULL;

    sl_vector_push(names, strdup("type"));
    sl_vector_push(values, sl_value_number(event.type));

    switch (event.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        sl_vector_push(names, strdup("key"));
        sl_vector_push(values, sl_value_number(event.key.keysym.scancode));
        break;
    case SDL_QUIT:
        break;
    }

    sl_vector_push(ctx->stack, sl_value_dict(names, values));
}

sl_binary_t *load_module(char *name)
{
    FILE *fd;
    char fullname[255] = {0};
    strcat(fullname, name);
    strcat(fullname, ".sqlang");
    if ((fd = fopen(fullname, "r")) != NULL)
    {
        fclose(fd);
        sl_binary_t *module = sl_compile_file(fullname);
        sl_vector_push(modules, module);
        return module;
    }
    return NULL;
}

int main(void)
{
    char *script = "script.sqlang";

    sl_ctx_t *ctx = sl_ctx_new(NULL);
    sl_builtin_install(ctx);
    global = ctx;

    sl_ctx_addfn(ctx, NULL, strdup("sdl_init"), 1, 0, binding_SDL_Init);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_createwindow"), 1, 0, binding_SDL_CreateWindow);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_destroywindow"), 1, 0, binding_SDL_DestroyWindow);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_quit"), 0, 0, binding_SDL_Quit);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_delay"), 1, 0, binding_SDL_Delay);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_createrenderer"), 1, 0, binding_SDL_CreateRenderer);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_destroyrenderer"), 1, 0, binding_SDL_DestroyRenderer);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_renderclear"), 1, 0, binding_SDL_RenderClear);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_renderpresent"), 1, 0, binding_SDL_RenderPresent);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_loadbmp"), 1, 0, binding_SDL_LoadBMP);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_createtexturefromsurface"), 1, 0, binding_SDL_CreateTextureFromSurface);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_rendercopy"), 1, 0, binding_SDL_RenderCopy);
    sl_ctx_addfn(ctx, NULL, strdup("sdl_pollevent"), 0, 0, binding_SDL_PollEvent);

    sl_ctx_addvar(ctx, strdup("sdl_init_everything"), sl_value_number(SDL_INIT_EVERYTHING));
    sl_ctx_addvar(ctx, strdup("sdl_renderer_accelerated"), sl_value_number(SDL_RENDERER_ACCELERATED));
    sl_ctx_addvar(ctx, strdup("sdl_renderer_presentvsync"), sl_value_number(SDL_RENDERER_PRESENTVSYNC));
    sl_ctx_addvar(ctx, strdup("sdl_keydown"), sl_value_number(SDL_KEYDOWN));
    sl_ctx_addvar(ctx, strdup("sdl_keyup"), sl_value_number(SDL_KEYUP));
    sl_ctx_addvar(ctx, strdup("sdl_quit_"), sl_value_number(SDL_QUIT));

    if (!sl_eval_file(ctx, script, load_module, NULL))

        printf(RED "Error: " RESET "(%s)\n", ex_msg);

    sl_gc_freeall();
    for (int i = 0; i < sl_vector_size(modules); i++)
    {
        sl_binary_free(modules[i]);
    }
    sl_vector_free(modules);

    return 0;
}