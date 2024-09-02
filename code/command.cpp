#ifndef COMMAND_C
#define COMMAND_C

static void
init_console_commands(void){
    //add_command(str8_literal("saves"), 1, 1, command_save);
    add_command(str8_literal("add"), 2, 2, command_add);
    add_command(str8_literal("save"), 1, 1, command_save);
    add_command(str8_literal("load"), 1, 1, command_load);
    add_command(str8_literal("exit"), 0, 0, command_exit);
    add_command(str8_literal("quit"), 0, 0, command_exit);
    add_command(str8_literal("help"), 0, 0, command_help);
}

static void
add_command(String8 name, u32 min, u32 max, Proc* proc){
    CommandInfo command = {0};
    command.name = name;
    command.min_args = min;
    command.max_args = max;
    command.proc = proc;
    commands[commands_count++] = command;
}

static void
command_help(String8* args){
    for(s32 i=0; i < commands_count; ++i){
        CommandInfo command = commands[i];
        console.output_history[console.output_history_count++] = command.name;
    }
}

static void
command_exit(String8* args){
    console.output_history[console.output_history_count++] = str8_literal("Existing!");
    Event event = {0};
    event.type = QUIT;
    events_add(&events, event);
}

static void
command_load(String8* args){
    console.output_history[console.output_history_count++] = str8_formatted(console.arena, "loading from file: %s", args->str);
    deserialize_data(*args);
}

static void
command_save(String8* args){
    console.output_history[console.output_history_count++] = str8_formatted(console.arena, "saving to file: %s", args->str);
    serialize_data(*args);
    //console_store_output(str8_formatted(console.arena, "saving to file: %s", args->str));
}

static void
command_add(String8* args){
    s32 left = atoi((char const*)(args->str));
    s32 right = atoi((char const*)(args + 1)->str);
    s32 value = left + right;
    String8 result = str8_formatted(console.arena, "Result: %i", value);
    console.output_history[console.output_history_count++] = result;
}

// todo: redo this entire function and reconsider how to are passing scratch arena
static void
command_saves(String8* args){
    ScratchArena scratch = begin_scratch();

    String8Node files = {0};
    files.next = &files;
    files.prev = &files;
    os_dir_files(scratch.arena, &files, str8_literal("saves"));
    dll_pop_front(&files);
    dll_pop_front(&files);

    for(String8Node* file = files.next; file != &files; file = file->next){
        console.output_history[console.output_history_count++] = file->str;
    }
    end_scratch(scratch);
}

static String8
push_str8(Arena* arena, String8 value){
    u8* str = push_array(arena, u8, value.count + 1);
    memcpy(str, value.data, value.count);
    String8 result = {str, value.count};
    return(result);
}

static u64
command_parse_args(String8 line){
    u64 args_count = 0;
    String8 remaining = line;
    while(remaining.size){
        remaining = str8_eat_spaces(remaining);
        if(remaining.size < 1){ break; }

        s64 index = byte_index_from_left(remaining, ' ');
        if(index != -1){
            String8 left_arg = str8_split_left(remaining, (u64)index);
            String8 arg = push_str8(console.arena, left_arg);
            command_args[command_args_count++] = arg;
            remaining = str8_advance(remaining, (u64)index);
        }
        else{
            String8 arg = push_str8(console.arena, remaining);
            command_args[command_args_count++] = arg;
            remaining = str8_advance(remaining, remaining.count);
        }

        args_count++;
    }
    return(args_count);
}

static void
run_command(String8 line){
    // separate command from arguments
    String8 command_name = command_args[0];
    String8* arguments = command_args + 1;
    command_args_count -= 1;

    // run command proc
    bool found = false;
    for(s32 i=0; i < commands_count; ++i){
        CommandInfo command = commands[i];
        if(str8_compare(command_name, command.name)){
            found = true;
            if(command.min_args > command_args_count){
                console.output_history[console.output_history_count++] = str8_formatted(console.arena, "Argument count less than min - Expected %i - Got: %i", command.min_args, command_args_count);
                break;
            }
            if(command.max_args < command_args_count){
                console.output_history[console.output_history_count++] = str8_formatted(console.arena, "Argument count greater than max - Expected %i - Got: %i", command.max_args, command_args_count);
                break;
            }

            command.proc(arguments);
            break;
        }
    }

    // output unkown command
    if(!found){
        console.output_history[console.output_history_count++] = str8_formatted(console.arena, "Unkown command: %s", command_name.str);
    }
    command_args_count = 0;
}

#endif
