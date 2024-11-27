/*
 *
 */

#include "../bunker.h"

/*
 * RETURN (int status)
 * - 0 | Success
 * - 1 | String argument not allocated
 * - 2 | Failed to split string
 * - 3 | String doesn't include deliminator
 */
int address_and_port_split(char** address, int* port, const char* string)
{
  if(!string) return 1;

  char* string_copy = strdup(string);

  char* token;

  // 1. Try to get address part of string
  if(!(token = strtok(string_copy, ":")))
  {
    free(string_copy);

    return 2;
  }

  if(address) *address = strdup(token);

  // 2. Try to get port part of string
  if(!(token = strtok(NULL, ":")))
  {
    free(string_copy);

    free(*address);

    return 3;
  }

  if(port) *port = atoi(token);

  free(string_copy);

  return 0;
}

/*
 *
 */
static int string_split(char*** strings, size_t* count, const char* string, const char* delim)
{
  if(!strings || !count || !string || !delim) return 1;

  char* string_copy = strdup(string);

  char* token = strtok(string_copy, delim);

  *strings = NULL;
  *count = 0;

  while(token)
  {
    *strings = realloc(*strings, sizeof(char*) * (*count + 1));

    if(!(*strings)) break;

    
    (*strings)[*count] = strdup(token);

    (*count)++;


    token = strtok(NULL, delim);
  }

  free(string_copy);

  if(!(*strings)) return 2;

  return 0;
}

/*
 *
 */
static int line_room_get(room_t* room, char* line)
{
  char* token;

  if(!(token = strtok(line, ",")))
  {
    return 1;
  }

  room->name = strdup(token);

  if(!(token = strtok(NULL, ",")))
  {
    free(room->name);

    return 2;
  }

  if(address_and_port_split(&room->address, &room->port, token) != 0)
  {
    free(room->name);

    return 3;
  }

  return 0;
}

/*
 * Get a list of registered rooms
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Fail
 */
int rooms_load(room_t** rooms, size_t* count)
{
  if(!rooms || !count) return 1;

  // 1. Read file with registered rooms
  size_t file_size = dir_file_size_get("../assets", "rooms.csv");

  char* buffer = malloc(sizeof(char) * (file_size + 1));

  if(dir_file_read(buffer, file_size, "../assets", "rooms.csv") == 0)
  {
    free(buffer);

    printf("Failed to read rooms file\n");

    return 1;
  }

  buffer[file_size] = '\0';


  // 2. Split file into seperate lines
  char** lines;

  if(string_split(&lines, count, buffer, "\n") != 0)
  {
    free(buffer);

    printf("Failed to split lines\n");

    return 2;
  }


  // 3. Allocate memory and populate rooms array
  *rooms = malloc(sizeof(room_t) * *count);

  for(size_t index = 0; index < *count; index++)
  {
    line_room_get(*rooms + index, lines[index]);
  }


  // 4. Free all the temporary memory
  for(size_t index = 0; index < *count; index++)
  {
    free(lines[index]);
  }

  free(lines);

  free(buffer);

  return 0;
}

/*
 *
 */
static int room_line_create(char** line, room_t room)
{
  if(!line) return 1;

  if(!room.name || !room.address) return 1;

  char buffer[256];

  if(sprintf(buffer, "%s,%s:%d", room.name, room.address, room.port) < 0)
  {
    return 2;
  }

  *line = strdup(buffer);

  return 0;
}

/*
 *
 */
static int room_lines_create(char*** lines, room_t* rooms, size_t count)
{
  *lines = malloc(sizeof(char*) * count);

  if(!(*lines))
  {
    return 1;
  }

  for(size_t index = 0; index < count; index++)
  {
    if(room_line_create(*lines + index, rooms[index]) != 0)
    {
      return 2;
    }
  }

  return 0;
}

/*
 *
 */
static void room_lines_free(char*** lines, size_t count)
{
  if(!lines) return;

  for(size_t index = 0; index < count; index++)
  {
    free((*lines)[index]);
  }

  free(*lines);
}

/*
 * 
 */
int rooms_save(room_t* rooms, size_t count)
{
  char** lines;

  if(room_lines_create(&lines, rooms, count) != 0)
  {
    return 2;
  }

  char* buffer = NULL;
  size_t buffer_length = 0;

  for(size_t index = 0; index < count; index++)
  {
    char* line = lines[index];

    size_t line_length = strlen(line);

    buffer = realloc(buffer, sizeof(char) * (buffer_length + line_length + 2));

    if(!buffer)
    {
      fprintf(stderr, "Failed to realloc buffer\n");

      break;
    }

    if(sprintf(buffer + buffer_length, "%s\n", line) < 0)
    {
      fprintf(stderr, "Failed to sprintf buffer\n");

      break;
    }

    buffer_length += (line_length + 1);

  }

  if(dir_file_write(buffer, buffer_length, "../assets", "rooms.csv") == 0)
  {
    if(buffer) free(buffer);

    room_lines_free(&lines, count);

    return 3;
  }

  if(buffer) free(buffer);

  room_lines_free(&lines, count);

  return 0;
}

/*
 *
 */
static void room_free(room_t room)
{
  if(room.name)    free(room.name);

  if(room.address) free(room.address);
}

/*
 *
 */
static room_t room_create(const char* name, const char* address, int port)
{
  room_t room;

  room.name = strdup(name);

  room.address = strdup(address);

  room.port = port;

  return room;
}

/*
 *
 */
void rooms_free(room_t** rooms, size_t count)
{
  if(!(*rooms)) return;

  for(size_t index = 0; index < count; index++)
  {
    room_free((*rooms)[index]);
  }

  free(*rooms);

  *rooms = NULL;
}

/*
 *
 */
static room_t* room_get(room_t* rooms, size_t count, const char* name)
{
  for(size_t index = 0; index < count; index++)
  {
    room_t* room = &rooms[index];

    if(strcmp(room->name, name) == 0) return room;
  }

  return NULL;
}

/*
 *
 */
static room_t room_dup(const room_t room)
{
  room_t dup = { 0 };

  if(room.name)    dup.name = strdup(room.name);

  if(room.address) dup.address = strdup(room.address);

  dup.port = room.port;

  return dup;
}

/*
 *
 */
static size_t room_index_get(room_t* rooms, size_t count, const char* name)
{
  size_t index;

  for(index = 0; index < count; index++)
  {
    if(strcmp(rooms[index].name, name) == 0) break;
  }

  return index;
}

/*
 *
 */
static int room_add(room_t** rooms, size_t* count, room_t room)
{
  if(!rooms || !count) return 1;

  if(!(*rooms) && *count > 0) return 2;

  // 3. 
  size_t index = room_index_get(*rooms, *count, room.name);

  if(index < *count)
  {
    // Free the previous room
    room_free((*rooms)[index]);

    // Allocate the new room
    (*rooms)[index] = room_dup(room);
  }
  else
  {
    // Allocate new memory for the new room
    *rooms = realloc(*rooms, sizeof(room_t) * (*count + 1));

    if(!(*rooms))
    {
      printf("Failed to realloc rooms\n");

      return 2;
    }

    (*rooms)[*count] = room_dup(room);

    (*count)++;
  }

  return 0;
}

/*
 *
 */
int room_del(room_t** rooms, size_t* count, const char* name)
{
  if(!rooms || !count || !name) return 1;

  if(!(*rooms) || *count == 0) return 2;

  size_t room_index = room_index_get(*rooms, *count, name);

  if(room_index == *count)
  {
    return 2;
  }

  room_free((*rooms)[room_index]);


  for(size_t index = room_index; index < (*count - 1); index++)
  {
    (*rooms)[index] = (*rooms)[index + 1];
  }

  *rooms = realloc(*rooms, sizeof(room_t) * (*count - 1));

  (*count)--;

  return 0;
}

/*
 *
 */
int address_and_port_add(char* address, int port, char* name)
{
  // 1. Get all registered rooms
  room_t* rooms;
  size_t  count;

  if(rooms_load(&rooms, &count) != 0)
  {
    return 1;
  }

  // 2. Create room to store
  room_t room = room_create(name, address, port);


  room_add(&rooms, &count, room);


  room_free(room);

  rooms_save(rooms, count);
  
  rooms_free(&rooms, count);

  return 0;
}

/*
 * Lookup string in lookup table to get address and port
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Fail
 */
static int address_and_port_lookup(char** address, int* port, const char* string)
{
  // 1. Get all registered rooms
  room_t* rooms;
  size_t  count;

  if(rooms_load(&rooms, &count) != 0)
  {
    return 1;
  }

  // Try to get room with matching name
  room_t* room = room_get(rooms, count, string);

  if(!room)
  {
    rooms_free(&rooms, count);

    return 1;
  }

  // Allocate memory and assign port and address
  if(address && room->address)
  {
    *address = strdup(room->address);
  }

  if(port) *port = room->port;

  rooms_free(&rooms, count);

  return 0;
}

/*
 * Get address and port from string
 *
 * RETURN (int status)
 * - 0 | Failed to get address and port
 * - 1 | Looked up address and port
 * - 2 | Parsed address and port
 */
int address_and_port_get(char** address, int* port, const char* string)
{
  if(address_and_port_lookup(address, port, string) == 0)
  {
    // printf("Looked up address and port\n");
    return 1;
  }
  else if(address_and_port_split(address, port, string) == 0)
  {
    // printf("Parsed address and port\n");
    return 2;
  }
  else return 0;
}
