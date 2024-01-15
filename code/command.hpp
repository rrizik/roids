#ifndef COMMAND_H
#define COMMAND_H

typedef void Proc(String8* args);

typedef struct CommandInfo{
    String8 name;
    u32 min_args;
    u32 max_args;
    Proc* proc;
} CommandInfo;

array_define(CommandInfo, 1024, Commands);
global Commands commands = {0};

array_define(String8, 50, CommandArgs);
global CommandArgs command_args = {0};

static void add_command(String8 name, u32 min, u32 max, Proc* proc);
static void command_help(String8* args);
static void command_exit(String8* args);
static void command_load(String8* args);
static void command_save(String8* args);
static void command_add(String8* args);
static void command_saves(String8* args);

static void init_commands();
static u64 parse_line_args(String8 line);
static void run_command(String8 line);

#endif
