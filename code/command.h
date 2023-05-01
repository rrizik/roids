#ifndef COMMAND_H

typedef void Proc(String8* args);

typedef struct CommandInfo{
    String8 name;
    u32 min_args;
    u32 max_args;
    Proc* proc;
} CommandInfo;

#define COMMANDS_MAX 20
static CommandInfo commands[COMMANDS_MAX];
static u32 command_count = 0;

static CommandInfo*
add_command(String8 name, u32 min, u32 max, Proc* proc){
    CommandInfo* command = commands + command_count++;
    command->name     = name;
    command->min_args = min;
    command->max_args = max;
    command->proc = proc;
    return(command);
}

#define COMMAND_ARGS_MAX 50
static String8 command_args[COMMAND_ARGS_MAX] = {0};
static u32 command_args_count = 0;


static void
add_argument(String8 arg){
    if(command_args_count < COMMAND_ARGS_MAX){
        command_args[command_args_count++] = arg;
    }
}

// CONSIDER: I don't even think we need output_history.
static void console_store_output(String8 str);
static void console_store_command(String8 str);

static void
command_default(String8* args){
    console_store_output(str8_literal("Default command proc!"));
}

static void
command_help(String8* args){
    for(u32 i=0; i < command_count; ++i){
        CommandInfo command = commands[i];
        console_store_output(command.name);
    }
}

static void
command_exit(String8* args){
    console_store_output(str8_literal("Exiting!"));
    should_quit = true;

}

static void
command_load(String8* args){
    deserialize_data(pm, *args);
    console_store_output(str8_format(global_arena, "loading from file: %s", args->str));
}

static void
command_save(String8* args){
    serialize_data(pm, *args);
    console_store_output(str8_format(global_arena, "saving to file: %s", args->str));
}

static void
command_add(String8* args){
    s32 left = atoi((char const*)(args->str));
    s32 right = atoi((char const*)(args + 1)->str);
    u32 value = left + right;
    String8 result = str8_format(global_arena, "Result: %i", value);
    console_store_output(result);
}

static void
init_commands(){
    add_command(str8_literal("load"), 1, 1, command_load);
    add_command(str8_literal("level"), 1, 1, command_load);
    add_command(str8_literal("save"), 1, 1, command_save);
    add_command(str8_literal("add"), 2, 2, command_add);
    add_command(str8_literal("exit"), 0, 0, command_exit);
    add_command(str8_literal("help"), 0, 0, command_help);
}

static void
parse_line(String8 line){
    String8 remaining = line;
    while(remaining.size){
        remaining = str8_eat_spaces(remaining);
        if(remaining.size < 1){ break; }

        u64 idx = str8_char_from_left(remaining, ' ');
        String8 left_arg = str8_split_left(remaining, idx);
        String8 arg = push_string(global_arena, left_arg);
		add_argument(arg);

        remaining = str8_advance(remaining, idx);
    }
}

static void
run_command(String8 line){
    // collect arguments
    //String8 remaining = line;
    //while(remaining.size){
    //    remaining = str8_eat_spaces(remaining);
    //    if(remaining.size < 1){ break; }

    //    u64 idx = str8_char_from_left(remaining, ' ');
    //    String8 left_arg = str8_split_left(remaining, idx);
    //    String8 arg = push_string(global_arena, left_arg);
	//	add_argument(arg);

    //    remaining = str8_advance(remaining, idx);
    //}

    //if(!command_args_count){ return; }

    // separate command from arguments
    String8 command_name = command_args[0];
    String8* arguments = command_args + 1;
    command_args_count -= 1;

    // run command proc
    bool found = false;
    for(u32 i=0; i<command_count; ++i){
        CommandInfo command = commands[i];
        if(str8_cmp(command_name, command.name)){
            found = true;
            if(command.min_args > command_args_count){
                console_store_output(str8_format(global_arena, "Argument count less than min - Expected %i - Got: %i", command.min_args, command_args_count));
                break;
            }
            if(command.max_args < command_args_count){
                console_store_output(str8_format(global_arena, "Argument count greater than max - Expected %i - Got: %i", command.max_args, command_args_count));
                break;
            }

            command.proc(arguments);
            break;
        }
    }

    // output unkown command
    if(!found){
        console_store_output(str8_format(global_arena, "Unkown command: %s", command_name.str));
    }
    command_args_count = 0;
}

#define COMMAND_H
#endif
