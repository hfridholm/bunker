/*
 * bunker
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-11-25
 */

#include "bunker.h"

static char doc[] = "bunker - a secure chat room";

static char args_doc[] = "[INFO...]";

static struct argp_option options[] =
{
  { "name",  'n', "NAME", 0, "Your nickname in the room" },
  { "room",  'r', "ROOM", 0, "New name of chat room" },
  { "debug", 'd', 0,      0, "Show debug messages" },
  { 0 }
};

struct args
{
  char** args;
  size_t arg_count;
  char*  name;
  char*  room;
  bool   debug;
};

struct args args =
{
  .args      = NULL,
  .arg_count = 0,
  .name      = NULL,
  .room      = NULL,
  .debug     = false
};

/*
 * This is the option parsing function used by argp
 */
static error_t opt_parse(int key, char* arg, struct argp_state* state)
{
  struct args* args = state->input;

  switch(key)
  {
    case 'n':
      args->name = arg;
      break;

    case 'r':
      args->room = arg;
      break;

    case 'd':
      args->debug = true;
      break;

    case ARGP_KEY_ARG:
      args->args = realloc(args->args, sizeof(char*) * (state->arg_num + 1));

      if(!args->args) return ENOMEM;

      args->args[state->arg_num] = arg;

      args->arg_count = state->arg_num + 1;
      break;

    case ARGP_KEY_END:
      if(args->arg_count < 1) argp_usage(state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

/*
 *
 */
static void send_routine()
{

}

/*
 *
 */
static void recv_routine()
{

}

/*
 *
 */
static void room_routine(const char* address, int port, const char* room)
{
  int sockfd = client_socket_create(address, port, args.debug);

  printf("Joining: (%s:%d)\n", address, port);

  socket_close(&sockfd, args.debug);


  if(room) printf("Room: (%s)\n", room);

  // Input nickname
  char* name;

  if(args.name)
  {
    name = strdup(args.name);
  }
  else
  {
    name = getstr("Name: ");
  }

  if(!name)
  {
    fprintf(stderr, "Failed to input name\n");

    name = strdup("Bob");
  }

  printf("Name: %s\n", name);

  free(name);
}

/*
 *
 */
static void join_routine(void)
{
  // Input chat room
  char* string;

  if(args.arg_count >= 2)
  {
    string = strdup(args.args[1]);
  }
  else
  {
    string = getstr("Room: ");
  }


  // Get address and port of string
  char* address;
  int   port;

  int status = address_and_port_get(&address, &port, string);

  if(status == 0)
  {
    free(string);

    printf("bunker: No room was found\n");

    return;
  }


  // If inputted room was a name, store that room name
  char* room = NULL;

  if(status == 1)
  {
    room = strdup(string);
  }

  free(string);


  // Add or rename room with address and port
  if(args.room)
  {
    address_and_port_add(address, port, args.room);

    // Update the room name
    if(room) free(room);
  
    room = strdup(args.room);
  }


  // Enter room
  room_routine(address, port, room);


  free(room);

  free(address);
}

/*
 *
 */
static void list_routine(void)
{
  // 1. Get all registered rooms
  room_t* rooms;
  size_t  count;

  if(rooms_load(&rooms, &count) != 0)
  {
    return;
  }

  for(size_t index = 0; index < count; index++)
  {
    room_t room = rooms[index];

    printf("%s : %s:%d\n", room.name, room.address, room.port);
  }

  rooms_free(&rooms, count);
}

/*
 *
 */
static void add_routine(void)
{
  // 1. Input room name
  char* room;

  if(args.arg_count >= 2)
  {
    room = strdup(args.args[1]);
  }
  else
  {
    room = getstr("Name: ");
  }

  // 2. Input address and port (server)
  char* server;

  if(args.arg_count >= 3)
  {
    server = strdup(args.args[2]);
  }
  else
  {
    server = getstr("Server: ");
  }


  // 3. Split address and port
  char* address;
  int   port;

  if(address_and_port_split(&address, &port, server) != 0)
  {
    free(server);
    free(room);

    fprintf(stderr, "Failed to parse address and port\n");

    return;
  }
  
  free(server);

  address_and_port_add(address, port, room);

  free(address);

  free(room);
}

/*
 *
 */
static void del_routine(void)
{
  // 1. Input room name
  char* room;

  if(args.arg_count >= 2)
  {
    room = strdup(args.args[1]);
  }
  else
  {
    room = getstr("Name: ");
  }


  // 1. Get all registered rooms
  room_t* rooms;
  size_t  count;

  if(rooms_load(&rooms, &count) != 0)
  {
    free(room);

    return;
  }

  if(room_del(&rooms, &count, room) != 0)
  {
    free(room);

    rooms_free(&rooms, count);

    fprintf(stderr, "Failed to del room\n");

    return;
  }

  printf("bunker: Deleted room: %s\n", room);

  free(room);

  rooms_save(rooms, count);
  
  rooms_free(&rooms, count);
}

static struct argp argp = { options, opt_parse, args_doc, doc };

/*
 * This is the main function
 */
int main(int argc, char* argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &args);

  srand(time(NULL));


  char* command = args.args[0];

  if(strcmp(command, "join") == 0)
  {
    join_routine();
  }
  else if(strcmp(command, "list") == 0)
  {
    list_routine();
  }
  else if(strcmp(command, "del") == 0)
  {
    del_routine();
  }
  else if(strcmp(command, "add") == 0)
  {
    add_routine();
  }
  else
  {
    printf("bunker: Unknown command '%s'\n", command);
  }

  free(args.args);

  return 0;
}
