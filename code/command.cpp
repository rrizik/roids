#ifndef COMMAND_C
#define COMMAND_C

static void
add_command(String8 name, u32 min, u32 max, Proc* proc){
    CommandInfo command = {0};
    command.name = name;
    command.min_args = min;
    command.max_args = max;
    command.proc = proc;
    array_add(&commands, command);
}

static void
command_help(String8* args){
    for(u32 i=0; i < commands.count; ++i){
        CommandInfo command = commands.array[i];
        array_add(&console.output_history, command.name);
    }
}

static void
command_exit(String8* args){
    array_add(&console.output_history, str8_literal("Existing!"));
    should_quit = true;
}

//static void
//command_load(String8* args){
//    deserialize_data(pm, *args);
//    console_store_output(str8_formatted(global_arena, "loading from file: %s", args->str));
//}
//
//static void
//command_save(String8* args){
//    serialize_data(pm, *args);
//    console_store_output(str8_formatted(global_arena, "saving to file: %s", args->str));
//}

static void
command_add(String8* args){
    s32 left = atoi((char const*)(args->str));
    s32 right = atoi((char const*)(args + 1)->str);
    s32 value = left + right;
    String8 result = str8_formatted(global_arena, "Result: %i", value);
    array_add(&console.output_history, result);
}

static void
command_saves(String8* args){
    ScratchArena scratch = begin_scratch(0);
    //defer(end_scratch(scratch));

    String8Node files = {0};
    files.next = &files;
    files.prev = &files;
    // TODO: I DONT THINK I SHOULD BE PASSING A SCRATCH HERE?
    os_dir_files(scratch.arena, &files, str8_literal("saves"));
    dll_pop_front(&files);
    dll_pop_front(&files);

    for(String8Node* file = files.next; file != &files; file = file->next){
        array_add(&console.output_history, file->str);
    }
    end_scratch(scratch);
}

static void
init_commands(){
    //add_command(str8_literal("load"), 1, 1, command_load);
    //add_command(str8_literal("save"), 1, 1, command_save);
    add_command(str8_literal("add"), 2, 2, command_add);
    add_command(str8_literal("saves"), 0, 0, command_saves);
    add_command(str8_literal("exit"), 0, 0, command_exit);
    add_command(str8_literal("quit"), 0, 0, command_exit);
    add_command(str8_literal("help"), 0, 0, command_help);
}

static u64
parse_line_args(String8 line){
    u64 args_count = 0;
    String8 remaining = line;
    while(remaining.size){
        remaining = str8_eat_whitespace(remaining);
        if(remaining.size < 1){ break; }

        u64 idx = str8_char_idx_from_left(remaining, ' ');
        String8 left_arg = str8_split_left(remaining, idx);
        String8 arg = push_string(global_arena, left_arg); // todo: This doesn't look correct
        array_add(&command_args, arg);

        remaining = str8_advance(remaining, idx);
        args_count++;
    }
    return(args_count);
}

static void
run_command(String8 line){
    // separate command from arguments
    String8 command_name = command_args.array[0];
    String8* arguments = command_args.array + 1;
    command_args.count -= 1;

    // run command proc
    bool found = false;
    for(u32 i=0; i < commands.count; ++i){
        CommandInfo command = commands.array[i];
        if(str8_cmp(command_name, command.name)){
            found = true;
            if(command.min_args > command_args.count){
                array_add(&console.output_history, str8_formatted(global_arena, "Argument count less than min - Expected %i - Got: %i", command.min_args, command_args.count));
                break;
            }
            if(command.max_args < command_args.count){
                array_add(&console.output_history, str8_formatted(global_arena, "Argument count greater than max - Expected %i - Got: %i", command.max_args, command_args.count));
                break;
            }

            command.proc(arguments);
            break;
        }
    }

    // output unkown command
    if(!found){
        array_add(&console.output_history, str8_formatted(global_arena, "Unkown command: %s", command_name.str));
    }
    command_args.count = 0;
}

#endif
