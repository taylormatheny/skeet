#pragma once

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;
typedef int bool32;
typedef float float32;
typedef double float64;

const float32 TURN_SPEED = 1.0f; //how fast a player turns
const float32 ACCELERATION = 20.0f;
const float32 MAX_SPEED = 50.0f;
const uint32 TICKS_PER_SECOND = 60;
const float32 SECONDS_PER_TICK = 1.0f / float32(TICKS_PER_SECOND);

const uint16 PORT = 9999;
const uint32 SOCKET_BUFFER_SIZE = 1024;