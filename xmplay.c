/* Enabling lua (and LÖVE) with ability to play .xm files!

    how to find in man page: /texttofind RET
    find next: n

  First, Lua is an interpreted language, largerly written in C and
  having _really_ nice C interface by design. Prom practical point of
  view it means that Lua has an ability to load functions from
  compiled C files, so we can write C functions and register them in
  Lua interpreter, using standard methods.

  We already can init mikmod via C, load and play files. Lets take a look at
  C-Lua interface.  If we read https://www.lua.org/pil/26.2.html, we can
  find some interesting things:

  1. Typically, a C library has one single public (extern) function, which
  is the function that opens the library.

  2. Lua "sees" C functions through this registration process.

  3. So, to define a Lua library in C, one must create an array that looks like
  this:

    static const struct luaL_reg mylib [] = { {"dir", l_dir}, {NULL, NULL}
      };

     This array has elements of type luaL_reg, which is a structure with two
     fields: a string and a function pointer.

     Then, a compiled .so file must have a function with signature

    int luaopen_LIBNAME(lua_State *);

     which has

    luaL_openlib(L, "LIBNAME", LIBNAME, 0);

     function call. It's like main function for regular C programs. When we
     write 'require("mylib")' in Lua, interpreter seeks for mylib.so file in
     search path, and then calls function named 'luaopen_mylib' in it. I
     think it's just a plain dlsym call. Please look in 'man 3 dlsym' if
     you're interested c:

  4. All functions to be registered must have type

    int my_func(lua_State *);

  5. If we want to pass arguments to our function, we must use special
  functions like lua_to<SOMETHING> from Lua-C API:

    double d = lua_tonumber(L, 1); -- to get a number, where L is lua_State
    pointer which is passed to our function

     The full list of such functions available here:
     https://www.lua.org/manual/5.1/manual.html#3.7

  6. If we want to return a value to Lua, we must use functions
  lua_push<SOMETHING>, to push value to stack:

    lua_pushnumber(L, 34.0d); */

#include <mikmod.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* First, 3 mikmod functions.  Their types are int ()(lua_State *), because
   they will be registered in Lua They return nothing, and play_music takes
   one argument -- path to file to play */

static int init_mikmod(lua_State *L)
{
    /* register all the drivers */
    MikMod_RegisterAllDrivers();

    /* register all the module loaders */
    MikMod_RegisterAllLoaders();
    /* initialize the library */
    md_mode |= DMODE_SOFT_MUSIC;

    if (MikMod_Init("")) {
        fprintf(stderr, "Could not initialize sound, reason: %s\n",
                MikMod_strerror(MikMod_errno));
        return 1;
    }

    return 0;
}

void* play_music_thread(void*); /* forward decl to make compiler happy */

static int play_music(lua_State *L)
{
    /* Here! We get a string from fucntion arguments.  A robust
       implementation also should check for validity of this operation:
       i.e., if there are any parameters passed, if the first one is really
       a string, etc.  */
    const char *path = lua_tostring(L, -1);
    /* load module in separate thread.  We don't want to block interpreter
       while we are playing music, so we create another thread to play
       music.

       Threads are just a mean of parallel execution.  Also note that
       there's no way to stop an already playing music, cause
       inter-thread communication is a very separate business from
       just thread creation. So we could create threads, but have no
       means of controlling already running ones (yet!).  As always, I
       remind you that there're many interesting things available in
       'man pthreads' and 'man pthread_create' olden scrolls.  */
    pthread_t new_thread;
    pthread_create(&new_thread, NULL, play_music_thread, (void *) path);
	/*                                   wonder what is it? ^  read the
                                                                  comment!

	   what I love about C is that all pointers are equal, and void * is
       just a polymorhic interface, which helps us pass all we want around
       the program.  Just cast your data to void *, and then cast back to
       original type in reciever function. Useful! Beautiful! */
    return 0;
}

void* play_music_thread(void* args)
{
    const char *path = (char *) args;
    printf("opening file %s...\n", path);
	MODULE *module = Player_Load(path, 64, 0);
    if (module) {
        /* start module */
        Player_Start(module);

        while (Player_Active()) {
            /* we're playing */
            usleep(10000);
            MikMod_Update();
        }

        Player_Stop();
        Player_Free(module);
    } else {
        fprintf(stderr, "Could not load module, reason: %s\n",
                MikMod_strerror(MikMod_errno));
    }
    pthread_exit(0);
}

static int deinit_mikmod(lua_State *L)
{
    MikMod_Exit();
    return 0;
}

/* Lua lacks sleep function, which is a shame */

static int lua_sleep(lua_State *L)
{
    int m = (int)lua_tonumber(L, 1);
    usleep(m * 1000);
    return 0;
}

/* Here we declare our library */

static const struct luaL_Reg xmplay [] = {
    {"play", play_music},
    {"init", init_mikmod},
    {"quit", deinit_mikmod},
    {NULL, NULL}  /* sentinel, i.e. marks end of array */
};

/* Entry point for Lua interpreter */
int luaopen_xmplay(lua_State *L)
{
    printf("Hi there! I'm an xmplay library loader!\n");
    luaL_openlib(L, "xmplay", xmplay, 0);
    printf("Hopefully lib's loaded now\n");
    printf("Reminder: - xmplay.init() to init,\n");
    printf("          - xmplay.play(path) to play,\n");
    printf("          - xmplay.quit() to clean up.\n");

    /* Enable Lua with sleep ability! */
    lua_register(L, "sleep", lua_sleep);
    return 1;
}
