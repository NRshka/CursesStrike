#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <alloca.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bcsgameplay.h"
#include "../libbcsmap/bcsmap.h"
#include "../liblinux_util/linux_util.h"

void bcsgameplay_map_overlay_create(BCSSERVER_FULL_STATE *state)
{   
    uint16_t height = state->map.height;
    uint16_t width = state->map.width;
    uint16_t client_x;
    uint16_t client_y;
    uint16_t size = height * width;
    uint8_t* map_overlay_copy = state->map_overlay.map_primitives;  // copy ptr to map_overlay

    memcpy(state->map_overlay.map_primitives, state->map.map_primitives, 
            height * width);

    for (int i = 0; i < BCSSERVER_MAXCLIENTS; ++i) {
        if (state->client[i].public_info.state == BCSCLST_PLAYING) {
            client_x = state->client[i].public_info.position.x;
            client_y = state->client[i].public_info.position.y;
            map_overlay_copy[width*client_y + client_x] = i;  // размещаем номер "врага"
        }
    }    
}

bool bcsgameplay_bullet_step(BCSSERVER_FULL_STATE *state, BCSBULLET *bullet, 
                                                          int steps) 
{
    int line_size = 0;

    uint16_t rifleman = bullet->creator_id;

    uint16_t player_count = state->player_count;  // number of players in server
    uint16_t height = state->map.height;
    uint16_t width = state->map.width;
    uint16_t bullet_x = bullet->x;
    uint16_t bullet_y = bullet->y;
    uint8_t tmp_primitive; 
    uint8_t* map_overlay_copy = state->map_overlay.map_primitives;

    memcpy(map_overlay_copy, state->map_overlay.map_primitives, height * width);

    switch (bullet->direction) {
        case BCSDIR_LEFT:   // y - constant, x can move at negative direction 
            line_size = width * bullet_y;
            for (int i = 0; i < steps; ++i) {
                if (bullet_x == 0) {
                    return false;
                } else {
                    --bullet_x;
                }
                tmp_primitive = map_overlay_copy[line_size + bullet_x];
                switch (tmp_primitive) {
                    case PUNIT_OPEN_SPACE:
                    case PUNIT_WATER:
                        break;
                    case PUNIT_ROCK:
                    case PUNIT_BOX:
                        return false;
                    default:
                        if (tmp_primitive == rifleman) {  // don't kill yourself!!!
                            break;
                        }
                        ++(state->client[tmp_primitive].public_ext_info.deaths);
                        state->client[tmp_primitive].public_info.state 
                                                        = BCSCLST_RESPAWNING;
                        lassert(gettimeofday(&(state->client[tmp_primitive]
                                                 .private_info
                                                 .time_last_fire), NULL));
                        state->client[tmp_primitive]
                              .private_info
                              .time_last_fire
                              .tv_sec += RESPAWN_TIMER;
                        ++(state->client[rifleman].public_ext_info.frags);
                        

                        return false;
                }
            }
            break;
        case BCSDIR_RIGHT:  // y - constant, x can move at positive direction
            line_size = width * bullet_y;
            for (int i = 0; i < steps; ++i) {
                if (bullet_x == width) {
                    return false;
                } else {
                    ++bullet_x;
                }
                if (bullet_x > width) {
                    return false;
                }
                tmp_primitive = map_overlay_copy[line_size + bullet_x];
                switch (tmp_primitive) {
                    case PUNIT_OPEN_SPACE:
                    case PUNIT_WATER:
                        break;
                    case PUNIT_ROCK:
                    case PUNIT_BOX:
                        return false;
                    default:
                        if (tmp_primitive == rifleman) {  // don't kill yourself!!!
                            break;
                        }
                        ++(state->client[tmp_primitive].public_ext_info.deaths);
                        state->client[tmp_primitive].public_info.state 
                                                        = BCSCLST_RESPAWNING;
                        lassert(gettimeofday(&(state->client[tmp_primitive]
                                                 .private_info
                                                 .time_last_fire), NULL));
                        state->client[tmp_primitive]
                              .private_info
                              .time_last_fire
                              .tv_sec += RESPAWN_TIMER;
                        ++(state->client[rifleman].public_ext_info.frags);
                        return false;
                }
            }
            break;
        case BCSDIR_UP:     // x - constant, y can move at negative direction
            for (int i = 0; i < steps; ++i) {
                if (bullet_y == 0) {
                    return false;
                } else {
                    --bullet_y;
                }
                tmp_primitive = map_overlay_copy[width * bullet_y + bullet_x];
                switch (tmp_primitive) {
                    case PUNIT_OPEN_SPACE:
                    case PUNIT_WATER:
                        break;
                    case PUNIT_ROCK:
                    case PUNIT_BOX:
                        return false;
                    default:
                        if (tmp_primitive == rifleman) {  // don't kill yourself!!!
                            break;
                        }
                        ++(state->client[tmp_primitive].public_ext_info.deaths);
                        state->client[tmp_primitive].public_info.state 
                                                        = BCSCLST_RESPAWNING;
                        lassert(gettimeofday(&(state->client[tmp_primitive]
                                                 .private_info
                                                 .time_last_fire), NULL));
                        state->client[tmp_primitive]
                              .private_info
                              .time_last_fire
                              .tv_sec += RESPAWN_TIMER;
                        ++(state->client[rifleman].public_ext_info.frags);
                        return false;
                }
            }
            break;
        case BCSDIR_DOWN:   // x - constant, y can move at positive direction
            for (int i = 0; i < steps; ++i) {
                if (bullet_y == height - 1) {
                    return false;
                } else {
                    ++bullet_y;
                }
                tmp_primitive = map_overlay_copy[width * bullet_y + bullet_x];
                switch (tmp_primitive) {
                    case PUNIT_OPEN_SPACE:
                    case PUNIT_WATER:
                        break;
                    case PUNIT_ROCK:
                    case PUNIT_BOX:
                        return false;
                    default:
                        if (tmp_primitive == rifleman) {  // don't kill yourself!!!
                            break;
                        }

                        ++(state->client[tmp_primitive].public_ext_info.deaths);
                        state->client[tmp_primitive].public_info.state 
                                                        = BCSCLST_RESPAWNING;
                        lassert(gettimeofday(&(state->client[tmp_primitive]
                                                 .private_info
                                                 .time_last_fire), NULL));
                        state->client[tmp_primitive]
                              .private_info
                              .time_last_fire
                              .tv_sec += RESPAWN_TIMER;
                        ++(state->client[rifleman].public_ext_info.frags);
                        return false;
                }
            }
            break;
        default: 
            break;
    }

    // rewrite x and y coordinates bullet
    bullet->x = bullet_x;
    bullet->y = bullet_y;

    return true;

}

bool bcsgameplay_respawn(BCSSERVER_FULL_STATE *state, size_t id)
{

    if ((state->client[id].public_info.state) != BCSCLST_RESPAWNING) {
        return false;
    }

    srand(time(NULL));

    LIST_VALTYPE *lv;
    LINKED_LIST_ENTRY *lle = NULL;
    size_t spawn_coordinate_y;
    size_t spawn_coordinate_x;
    uint16_t height = state->map.height;
    uint16_t width = state->map.width;

    int start_area;
    int end_area;

    // int offset = (CHECK_AREA_SIZE/2) * CHECK_AREA_SIZE + (CHECK_AREA_SIZE/2);

    int danger_lvl = 0;
    
    size_t map_size = height * width;
    uint8_t* map_overlay_copy = state->map_overlay.map_primitives;

    while (true) {
        spawn_coordinate_y = rand() % height;                                   // выбираем рандомное место спауна
        spawn_coordinate_x = rand() % width;

        start_area = (spawn_coordinate_y - CHECK_AREA_SIZE) * width + 
                     (spawn_coordinate_x - CHECK_AREA_SIZE);                    // считаем начало области проверки
        end_area = (spawn_coordinate_y + CHECK_AREA_SIZE) * width + 
                     (spawn_coordinate_x + CHECK_AREA_SIZE);                    // конец области проверки
        
        if (map_overlay_copy[spawn_coordinate_y * width + spawn_coordinate_x] 
                                                        != PUNIT_OPEN_SPACE) {  // проверяем
            continue;                                                           // можно ли появиться
        }                                                                       // в данной координате

        for (int i = start_area; i < end_area; ++i) {                           // поиск других игроков в области
            if (map_overlay_copy[i] < BCSSERVER_MAXCLIENTS) {                   // потенциальной угрозы
                ++danger_lvl;
            }
        }

        while((lv = linkedlist_next_r(&state->bullets, &lle)) != NULL) {
            BCSBULLET *bullet = (BCSBULLET*)(lv->ptr);

            if (bullet->y == spawn_coordinate_y) {                              // bullet flying on line of player
                if (bullet->x < spawn_coordinate_x && 
                    bullet->direction == BCSDIR_RIGHT) {                        // bullet flying to player from leftside
                    ++danger_lvl;
                } else if (bullet->x > spawn_coordinate_x && 
                           bullet->direction == BCSDIR_LEFT) {                  // bullet flying to player from rightside
                    ++danger_lvl;
                }
            } else if (bullet->x == spawn_coordinate_x) {                       // bullet flying on line of player
                if (bullet->y > spawn_coordinate_y && 
                    bullet->direction == BCSDIR_UP) {                           // bullet flying to player from upper
                    ++danger_lvl;
                } else if (bullet->y < spawn_coordinate_y &&                    // bullet flying to player from down
                           bullet->direction == BCSDIR_DOWN) {
                    ++danger_lvl;
                }
            }
        }

        if (danger_lvl > 1) {            // обнаружен игрок помимо нас самих
            continue;                    // область опасна для возрождения        
        }                                // ищем новую

        break;
    }

    state->client[id].public_info.state = BCSCLST_PLAYING;
    state->client[id].public_info.position.x = spawn_coordinate_x;
    state->client[id].public_info.position.y = spawn_coordinate_y;

    return true;
}

