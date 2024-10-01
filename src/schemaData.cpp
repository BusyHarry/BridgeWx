// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "schemaData.h"

/*
A positive setnr: this set is on this table, a negative setnr: get games from that table,
A positive table for a pair means: play line = North/South, negative table means: play line = East/West.
The number of sets is equal to the number of rounds
*/
static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*(5+8)];
    } _5tin08 =
        {5,8,5,"5tin08"
        // R1  R2  R3  R4 R5 etc
        ,  1,  1,  1,  1, -5    /*  sets for table 1*/
        ,  2,  2,  2,  2, -5    /*  sets for table 2*/
        ,  3,  3, -5,  3,  3    /*  sets for table 3*/
        ,  4, -5,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/

        ,  1,  2,  4,  3,  1    /*tables for pair  1*/
        , -1, -3,  2, -4,  2    /*tables for pair  2*/
        ,  2, -1,  3,  4, -3    /*tables for pair  3*/
        , -2, -4,  1, -3, -4    /*tables for pair  4*/
        ,  3,  1, -4, -2, -2    /*tables for pair  5*/
        , -3, -2, -3,  1,  4    /*tables for pair  6*/
        ,  4,  3, -1,  2, -1    /*tables for pair  7*/
        , -4,  4, -2, -1,  3    /*tables for pair  8*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*12];
    } _6multi08 =
        {6,8,4,"6multi08"
        ,  1,  1,  5,  5,  5,  5    /*  sets for table 1*/
        ,  2,  2,  6,  6,  6,  6    /*  sets for table 2*/
        ,  3,  3,  3,  3,  1,  1    /*  sets for table 3*/
        ,  4,  4,  4,  4,  2,  2    /*  sets for table 4*/

        ,  1,  4, -1, -3,  4, -2    /*tables for pair  1*/
        , -1, -3,  4, -2,  1, -4    /*tables for pair  2*/
        ,  2,  3, -2,  4, -3,  1    /*tables for pair  3*/
        , -2, -4,  3, -1, -2,  3    /*tables for pair  4*/
        ,  3, -2, -4,  1,  3,  2    /*tables for pair  5*/
        , -3,  1,  1, -4,  2,  4    /*tables for pair  6*/
        ,  4, -1, -3,  2, -4, -1    /*tables for pair  7*/
        , -4,  2,  2,  3, -1, -3    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*16];
    } _6multi10 =
        {6,10,6,"6multi10"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1, -2, -3,  4,  5,  6    /*tables for pair  1*/
        , -1,  3,  5,  6, -2, -4    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  1    /*tables for pair  3*/
        , -2,  4, -5, -1, -3, -6    /*tables for pair  4*/
        ,  3,  5,  1, -4, -6, -2    /*tables for pair  5*/
        , -3, -4,  6, -2, -5, -1    /*tables for pair  6*/
        ,  4, -5, -6,  1,  2,  3    /*tables for pair  7*/
        , -4,  6,  3, -5, -1,  2    /*tables for pair  8*/
        ,  5, -6, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  2,  4, -6,  1, -3    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6multi12 =
        {6,12,6,"6multi12"
        ,  1,  1,  1,  1, -4,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4, -1    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1, -3, -5,  4,  2,  6    /*tables for pair  1*/
        , -1,  6,  4, -5, -3, -2    /*tables for pair  2*/
        ,  2, -6, -1,  3,  4,  5    /*tables for pair  3*/
        , -2,  4,  3, -1, -5, -6    /*tables for pair  4*/
        ,  3, -5, -6,  1,  1,  2    /*tables for pair  5*/
        , -3,  1,  2, -4, -6, -5    /*tables for pair  6*/
        ,  4, -2, -3,  5,  6,  4    /*tables for pair  7*/
        , -4,  5,  1, -6, -2, -3    /*tables for pair  8*/
        ,  5, -4, -2,  6,  3,  1    /*tables for pair  9*/
        , -5,  3,  6, -2, -4, -4    /*tables for pair  10*/
        ,  6, -1, -4,  2,  5,  3    /*tables for pair  11*/
        , -6,  2,  5, -3, -1, -1    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*21];
    } _6multi14 =
        {6,14,7,"6multi14"
        ,  1,  1,  1,  1, -4,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4, -1    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -4, -5, -2, -6, -3    /*  sets for table 7*/

        ,  1, -3, -7,  4,  2,  6    /*tables for pair  1*/
        , -1,  6,  4, -5, -3, -2    /*tables for pair  2*/
        ,  2, -6, -1,  3,  4,  5    /*tables for pair  3*/
        , -2,  4,  3, -1, -5, -6    /*tables for pair  4*/
        ,  3, -5, -6,  1,  1,  2    /*tables for pair  5*/
        , -3,  1,  2, -4, -6, -5    /*tables for pair  6*/
        ,  4, -2, -3,  5,  7,  4    /*tables for pair  7*/
        , -4,  5,  1, -6, -2, -3    /*tables for pair  8*/
        ,  5, -7, -2,  6,  3,  1    /*tables for pair  9*/
        , -5,  3,  6,  7, -4, -4    /*tables for pair  10*/
        ,  6, -1, -4,  2,  5,  7    /*tables for pair  11*/
        , -6,  2, -5, -3, -1, -1    /*tables for pair  12*/
        ,  7, -4,  5, -2,  6,  3    /*tables for pair  13*/
        , -7,  7,  7, -7, -7, -7    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*24];
    } _6multi16 =
        {6,16,8,"6multi16"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/

        ,  1, -2, -3,  4,  5,  6    /*tables for pair  1*/
        , -1,  6,  4, -3, -7, -2    /*tables for pair  2*/
        ,  2, -6, -5,  8,  4,  3    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -4    /*tables for pair  4*/
        ,  3, -8, -2,  1,  6,  4    /*tables for pair  5*/
        , -3,  2,  1, -5, -4, -7    /*tables for pair  6*/
        ,  4, -3, -1,  6,  8,  5    /*tables for pair  7*/
        , -4,  5,  6, -1, -2, -3    /*tables for pair  8*/
        ,  5, -4, -8,  3,  2,  1    /*tables for pair  9*/
        , -5,  3,  2, -7, -1, -6    /*tables for pair  10*/
        ,  6, -5, -4,  2,  1,  8    /*tables for pair  11*/
        , -6,  7,  5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7, -7,  7,  7,  7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6, -5    /*tables for pair  14*/
        ,  8, -1, -6,  5,  3,  2    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6multi18 =
        {6,18,9,"6multi18"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/

        ,  1, -2, -3,  4,  5,  6    /*tables for pair  1*/
        , -1,  6,  4, -3, -7, -2    /*tables for pair  2*/
        ,  2, -6, -5,  8,  4,  3    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -4    /*tables for pair  4*/
        ,  3, -8, -2,  1,  9,  4    /*tables for pair  5*/
        , -3,  2,  1, -5, -4, -7    /*tables for pair  6*/
        ,  4, -9, -1,  6,  8,  5    /*tables for pair  7*/
        , -4,  5,  6, -1, -2, -3    /*tables for pair  8*/
        ,  5, -4, -8,  3,  2,  9    /*tables for pair  9*/
        , -5,  3,  2, -7, -1, -6    /*tables for pair  10*/
        ,  6, -5, -9,  2,  1,  8    /*tables for pair  11*/
        , -6,  7,  5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7, -7,  7,  7,  7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6, -5    /*tables for pair  14*/
        ,  8, -1, -6,  9,  3,  2    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*30];
    } _6multi20 =
        {6,20,10,"6multi20"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 10*/

        ,  1, -2, -3,  4,  5,  6    /*tables for pair  1*/
        , -1,  6,  4, -3, -7, -2    /*tables for pair  2*/
        ,  2, -6,-10,  8,  4,  3    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -4    /*tables for pair  4*/
        ,  3, -8, -2,  1,  9,  4    /*tables for pair  5*/
        , -3,  2,  1, -5, -4, -7    /*tables for pair  6*/
        ,  4, -9, -1, 10,  8,  5    /*tables for pair  7*/
        , -4,  5,  6, -1, -2, -3    /*tables for pair  8*/
        ,  5,-10, -8,  3,  2,  9    /*tables for pair  9*/
        , -5,  3,  2, -7, -1, -6    /*tables for pair  10*/
        ,  6, -5, -9,  2, 10,  8    /*tables for pair  11*/
        , -6,  7,  5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7, -7,  7,  7,  7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6, -5    /*tables for pair  14*/
        ,  8, -1, -6,  9,  3, 10    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*33];
    } _6multi22 =
        {6,22,11,"6multi22"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 10*/
        , -5, -6, -1, -2, -3, -4    /*  sets for table 11*/

        ,  1, -2, -3,  4,  5,  6    /*tables for pair  1*/
        , -1,  6,  4, -3, -7, -2    /*tables for pair  2*/
        ,  2,-11,-10,  8,  4,  3    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -4    /*tables for pair  4*/
        ,  3, -8, -2,  1,  9, 11    /*tables for pair  5*/
        , -3,  2,  1, -5, -4, -7    /*tables for pair  6*/
        ,  4, -9,-11, 10,  8,  5    /*tables for pair  7*/
        , -4,  5,  6, -1, -2, -3    /*tables for pair  8*/
        ,  5,-10, -8,  3,  2,  9    /*tables for pair  9*/
        , -5,  3,  2, -7, -1, -6    /*tables for pair  10*/
        ,  6, -5, -9, 11, 10,  8    /*tables for pair  11*/
        , -6,  7,  5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7, -7,  7,  7,  7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6, -5    /*tables for pair  14*/
        ,  8, -1, -6,  9, 11, 10    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10    /*tables for pair  20*/
        , 11, -6, -1,  2,  3,  4    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*36];
    } _6multi24 =
        {6,24,12,"6multi24"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 10*/
        , -5, -6, -1, -2, -3, -4    /*  sets for table 11*/
        , -6, -1, -2, -3, -4, -5    /*  sets for table 12*/

        ,  1, -2, -3,  4,  5,  6    /*tables for pair  1*/
        , -1,  6,  4, -3, -7, -2    /*tables for pair  2*/
        ,  2,-11,-10,  8, 12,  3    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -4    /*tables for pair  4*/
        ,  3, -8,-12,  1,  9, 11    /*tables for pair  5*/
        , -3,  2,  1, -5, -4, -7    /*tables for pair  6*/
        ,  4, -9,-11, 10,  8, 12    /*tables for pair  7*/
        , -4,  5,  6, -1, -2, -3    /*tables for pair  8*/
        ,  5,-10, -8, 12,  2,  9    /*tables for pair  9*/
        , -5,  3,  2, -7, -1, -6    /*tables for pair  10*/
        ,  6, -5, -9, 11, 10,  8    /*tables for pair  11*/
        , -6,  7,  5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7, -7,  7,  7,  7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6, -5    /*tables for pair  14*/
        ,  8,-12, -6,  9, 11, 10    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10    /*tables for pair  20*/
        , 11, -6, -1,  2,  3,  4    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11    /*tables for pair  22*/
        , 12, -1, -2,  3,  4,  5    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*12];
    } _7multi08 =
        {7,8,4,"7multi08"
        ,  1,  7,  2,  3,  4,  5,  6    /*  sets for table 1*/
        ,  2,  3,  4,  5,  6,  7,  7    /*  sets for table 2*/
        ,  3,  4,  5,  6,  1,  1,  2    /*  sets for table 3*/
        ,  4,  5,  6,  1,  7,  2,  3    /*  sets for table 4*/

        ,  1,  4, -2, -1,  2, -2,  3    /*tables for pair  1*/
        , -1,  2, -3,  3,  1,  4, -2    /*tables for pair  2*/
        ,  2, -3,  3,  1,  4, -3, -1    /*tables for pair  3*/
        , -2, -1,  2, -3,  3,  1,  4    /*tables for pair  4*/
        ,  3,  1,  4, -2, -1,  3, -3    /*tables for pair  5*/
        , -3,  3,  1,  4, -2, -1,  2    /*tables for pair  6*/
        ,  4, -2, -1,  2, -3,  2,  1    /*tables for pair  7*/
        , -4, -4, -4, -4, -4, -4, -4    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*15];
    } _7multi10 =
        {7,10,5,"7multi10"
        ,  1,  2,  3,  4,  5,  6,  7    /*  sets for table 1*/
        ,  2,  3,  4,  5,  6,  7,  1    /*  sets for table 2*/
        ,  3,  4,  5,  6,  7,  1,  2    /*  sets for table 3*/
        ,  4,  5,  6,  7,  1,  2,  3    /*  sets for table 4*/
        ,  5,  6,  7,  1,  2,  3,  4    /*  sets for table 5*/

        ,  1, -2, -3, -4, -2,  4,  5    /*tables for pair  1*/
        , -1,  1,  1, -1, -1, -1, -1    /*tables for pair  2*/
        ,  2, -3,  4, -2,  4,  5,  1    /*tables for pair  3*/
        , -2, -4, -5,  1,  2,  3, -4    /*tables for pair  4*/
        ,  3,  4,  2,  4,  5,  1,  2    /*tables for pair  5*/
        , -3,  3,  3, -3, -3, -3, -3    /*tables for pair  6*/
        ,  4, -5, -1,  2,  3, -4, -2    /*tables for pair  7*/
        , -4,  2, -4,  5,  1,  2,  3    /*tables for pair  8*/
        ,  5, -1, -2,  3, -4, -2,  4    /*tables for pair  9*/
        , -5,  5,  5, -5, -5, -5, -5    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*18];
    } _7multi12 =
        {7,12,6,"7multi12"
        ,  1,  2,  3,  4,  5,  6,  7    /*  sets for table 1*/
        ,  2,  3,  4,  5,  6,  7,  1    /*  sets for table 2*/
        ,  3,  4,  5,  6,  7,  1,  2    /*  sets for table 3*/
        ,  4,  5,  6,  7,  1,  2,  3    /*  sets for table 4*/
        ,  5,  6,  7,  1,  2,  3,  4    /*  sets for table 5*/
        , -3, -3, -3, -3, -3, -3, -3    /*  sets for table 6*/

        ,  1, -3,  3,  6,  5,  2,  4    /*tables for pair  1*/
        , -1,  1,  1, -1, -1, -1, -1    /*tables for pair  2*/
        ,  2, -4, -1,  3, -3,  6,  5    /*tables for pair  3*/
        , -2,  2,  2, -2, -2, -2, -2    /*tables for pair  4*/
        ,  3,  3, -6,  5,  2,  4,  1    /*tables for pair  5*/
        , -3, -6, -5,  2,  4,  1,  3    /*tables for pair  6*/
        ,  4, -1, -3, -3,  6,  5,  2    /*tables for pair  7*/
        , -4,  4,  4, -4, -4, -4, -4    /*tables for pair  8*/
        ,  5, -2, -4,  1,  3, -3,  6    /*tables for pair  9*/
        , -5,  5,  5, -5, -5, -5, -5    /*tables for pair  10*/
        ,  6, -5, -2,  4,  1,  3, -3    /*tables for pair  11*/
        , -6,  6,  6, -6, -6, -6, -6    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*21];
    } _7multi14 =
        {7,14,7,"7multi14"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1, -3, -5,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2, -4, -6,  1,  3,  5,  7    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  6,  1    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  5,  7,  2    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  4,  6,  1,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -3,  5,  7,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -2, -4,  6,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7multi16 =
        {7,16,8,"7multi16"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/

        ,  1, -3, -5,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2, -4, -6,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  8,  1    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  7,  2    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  6,  1,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  5,  7,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -4,  6,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*27];
    } _7multi18 =
        {7,18,9,"7multi18"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/

        ,  1, -9, -5,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2, -4, -6,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  9,  2    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9,  1,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9,  7,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9,  6,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*30];
    } _7multi20 =
        {7,20,10,"7multi20"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/

        ,  1, -9,-10,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10, -6,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9, 10,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*33];
    } _7multi22 =
        {7,22,11,"7multi22"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/

        ,  1, -9,-10, 11,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11, -7,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10, 11,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*36];
    } _7multi24 =
        {7,24,12,"7multi24"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/
        , -5, -6, -7, -1, -2, -3, -4    /*  sets for table 12*/

        ,  1, -9,-10, 11, 12,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11, 12,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11,-12,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4,-12, -1,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10, 11, 12    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11, 12,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        , 12, -6, -7,  1,  2,  3,  4    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12,-12    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*39];
    } _7multi26 =
        {7,26,13,"7multi26"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/
        , -5, -6, -7, -1, -2, -3, -4    /*  sets for table 12*/
        , -6, -7, -1, -2, -3, -4, -5    /*  sets for table 13*/

        ,  1, -9,-10, 11, 12, 13,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11, 12, 13,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11,-12, 13,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4,-12,-13,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5,-13, -2,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10, 11, 12    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11, 12, 13    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        , 12, -6, -7,  1,  2,  3,  4    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12,-12    /*tables for pair  24*/
        , 13, -7, -1,  2,  3,  4,  5    /*tables for pair  25*/
        ,-13, 13, 13,-13,-13,-13,-13    /*tables for pair  26*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*42];
    } _7multi28 =
        {7,28,14,"7multi28"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/
        , -5, -6, -7, -1, -2, -3, -4    /*  sets for table 12*/
        , -6, -7, -1, -2, -3, -4, -5    /*  sets for table 13*/
        , -7, -1, -2, -3, -4, -5, -6    /*  sets for table 14*/

        ,  1, -9,-10, 11, 12, 13, 14    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11, 12, 13, 14,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11,-12, 13, 14,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4,-12,-13, 14,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5,-13,-14,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6,-14, -8,  9, 10, 11, 12    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11, 12, 13    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        , 12, -6, -7,  1,  2,  3,  4    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12,-12    /*tables for pair  24*/
        , 13, -7, -1,  2,  3,  4,  5    /*tables for pair  25*/
        ,-13, 13, 13,-13,-13,-13,-13    /*tables for pair  26*/
        , 14, -1, -2,  3,  4,  5,  6    /*tables for pair  27*/
        ,-14, 14, 14,-14,-14,-14,-14    /*tables for pair  28*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*12];
    } _4basis8 =
        {4,8,4,"4basis8"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/

        ,  1, -2,  3,  4    /*tables for pair  1*/
        , -1,  3, -4, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -2,  4, -3, -1    /*tables for pair  4*/
        ,  3, -4,  1,  2    /*tables for pair  5*/
        , -3,  1, -2, -4    /*tables for pair  6*/
        ,  4, -3,  2,  1    /*tables for pair  7*/
        , -4,  2, -1, -3    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*15];
    } _5basis10 =
        {5,10,5,"5basis10"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -1,  5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  1, -5, -4, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -1, -5, -4    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  3, -2, -1, -5    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  4, -3, -2, -1    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6basis12_1_5 =
        {6,12,6,"6basis12.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1,  3, -5,  4,  2,  6    /*tables for pair  1*/
        , -1, -6,  4,  5,  3, -2    /*tables for pair  2*/
        ,  2,  1, -6, -5, -4,  3    /*tables for pair  3*/
        , -2, -3,  1,  6, -5,  4    /*tables for pair  4*/
        ,  3, -2, -4,  1,  5, -6    /*tables for pair  5*/
        , -3, -4,  6,  2, -1,  5    /*tables for pair  6*/
        ,  4,  6, -1,  3, -2, -5    /*tables for pair  7*/
        , -4, -5,  2, -1, -6, -3    /*tables for pair  8*/
        ,  5,  4, -2, -6, -3,  1    /*tables for pair  9*/
        , -5, -1, -3, -2,  6, -4    /*tables for pair  10*/
        ,  6,  5,  3, -4,  1,  2    /*tables for pair  11*/
        , -6,  2,  5, -3,  4, -1    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6basis12_2_5 =
        {6,12,6,"6basis12.2-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1,  6,  3, -4,  2,  5    /*tables for pair  1*/
        , -1,  5,  6, -3,  4, -2    /*tables for pair  2*/
        ,  2,  3, -6,  4,  5,  1    /*tables for pair  3*/
        , -2, -1,  4,  6,  3, -5    /*tables for pair  4*/
        ,  3, -5, -4,  2,  6, -1    /*tables for pair  5*/
        , -3, -4,  1,  5, -2,  6    /*tables for pair  6*/
        ,  4,  1, -2,  3, -5, -6    /*tables for pair  7*/
        , -4, -6,  5, -2, -1, -3    /*tables for pair  8*/
        ,  5,  2, -1, -6, -4,  3    /*tables for pair  9*/
        , -5, -3,  2,  1, -6,  4    /*tables for pair  10*/
        ,  6,  4, -5, -1, -3,  2    /*tables for pair  11*/
        , -6, -2, -3, -5,  1, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6basis12_3_5 =
        {6,12,6,"6basis12.3-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  4,  6, -1,  3, -2, -5    /*tables for pair  1*/
        , -5, -1, -3, -2,  6, -4    /*tables for pair  2*/
        ,  6,  5,  3, -4,  1,  2    /*tables for pair  3*/
        , -1, -6,  4,  5,  3, -2    /*tables for pair  4*/
        ,  3, -2, -4,  1,  5, -6    /*tables for pair  5*/
        , -3, -4,  6,  2, -1,  5    /*tables for pair  6*/
        ,  2,  1, -6, -5, -4,  3    /*tables for pair  7*/
        , -6,  2,  5, -3,  4, -1    /*tables for pair  8*/
        ,  1,  3, -5,  4,  2,  6    /*tables for pair  9*/
        , -4, -5,  2, -1, -6, -3    /*tables for pair  10*/
        ,  5,  4, -2, -6, -3,  1    /*tables for pair  11*/
        , -2, -3,  1,  6, -5,  4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6basis12_4_5 =
        {6,12,6,"6basis12.4-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  5,  4, -2, -6, -3,  1    /*tables for pair  1*/
        , -4, -5,  2, -1, -6, -3    /*tables for pair  2*/
        ,  2,  1, -6, -5, -4,  3    /*tables for pair  3*/
        , -2, -3,  1,  6, -5,  4    /*tables for pair  4*/
        ,  3, -2, -4,  1,  5, -6    /*tables for pair  5*/
        , -3, -4,  6,  2, -1,  5    /*tables for pair  6*/
        ,  1,  3, -5,  4,  2,  6    /*tables for pair  7*/
        , -5, -1, -3, -2,  6, -4    /*tables for pair  8*/
        ,  4,  6, -1,  3, -2, -5    /*tables for pair  9*/
        , -1, -6,  4,  5,  3, -2    /*tables for pair  10*/
        ,  6,  5,  3, -4,  1,  2    /*tables for pair  11*/
        , -6,  2,  5, -3,  4, -1    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6basis12_5_5 = 
        {6,12,6,"6basis12.5-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  2,  1, -6, -5, -4,  3    /*tables for pair  1*/
        , -4, -5,  2, -1, -6, -3    /*tables for pair  2*/
        ,  6,  5,  3, -4,  1,  2    /*tables for pair  3*/
        , -5, -1, -3, -2,  6, -4    /*tables for pair  4*/
        ,  3, -2, -4,  1,  5, -6    /*tables for pair  5*/
        , -3, -4,  6,  2, -1,  5    /*tables for pair  6*/
        ,  5,  4, -2, -6, -3,  1    /*tables for pair  7*/
        , -1, -6,  4,  5,  3, -2    /*tables for pair  8*/
        ,  4,  6, -1,  3, -2, -5    /*tables for pair  9*/
        , -6,  2,  5, -3,  4, -1    /*tables for pair  10*/
        ,  1,  3, -5,  4,  2,  6    /*tables for pair  11*/
        , -2, -3,  1,  6, -5,  4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*21];
    } _7basis14 =
        {7,14,7,"7basis14"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -1,  7,  6, -5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -4, -3    /*tables for pair  4*/
        ,  3, -4, -5,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2,  1, -7, -6, -5, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  3,  2, -1, -7, -6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  4,  3, -2, -1, -7, -6    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6,  5,  4, -3, -2, -1, -7    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -3, -2, -1    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*12];
    } _3drive08 =
        {3,8,4,"3drive08"
        ,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3    /*  sets for table 3*/
        ,  1,  2,  3    /*  sets for table 4*/

        ,  1,  4,  3    /*tables for pair  1*/
        , -1, -3, -2    /*tables for pair  2*/
        ,  2,  3, -1    /*tables for pair  3*/
        , -2, -1,  4    /*tables for pair  4*/
        ,  3,  1,  2    /*tables for pair  5*/
        , -3, -2,  1    /*tables for pair  6*/
        ,  4,  2, -3    /*tables for pair  7*/
        , -4, -4, -4    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*9];
    } _5howel06_1_1 =
        {5,6,3,"5howel06.1-1"
        ,  1,  1,  1,  3,  5    /*  sets for table 1*/
        ,  2,  4,  4,  4, -1    /*  sets for table 2*/
        ,  3,  3,  2,  2, -1    /*  sets for table 3*/

        ,  1,  3,  2,  3,  1    /*tables for pair  1*/
        , -1, -2,  3, -1,  2    /*tables for pair  2*/
        ,  2, -3,  1, -2, -2    /*tables for pair  3*/
        , -2,  1, -2,  1,  3    /*tables for pair  4*/
        ,  3,  2, -1, -3, -3    /*tables for pair  5*/
        , -3, -1, -3,  2, -1    /*tables for pair  6*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*12];
    } _7howel08_1_1 =
        {7,8,4,"7howel08.1-1"
        ,  1,  6,  7,  2,  5,  3,  4    /*  sets for table 1*/
        ,  2,  5,  3,  4,  1,  6,  7    /*  sets for table 2*/
        ,  3,  4,  1,  6,  7,  2,  5    /*  sets for table 3*/
        ,  4,  1,  6,  7,  2,  5,  3    /*  sets for table 4*/

        ,  1,  1,  1,  1,  1,  1,  1    /*tables for pair  1*/
        , -1,  3,  2, -4,  4, -2, -3    /*tables for pair  2*/
        ,  2, -4,  4, -2, -3, -1,  3    /*tables for pair  3*/
        , -2, -3, -1,  3,  2, -4,  4    /*tables for pair  4*/
        ,  3,  2, -4,  4, -2, -3, -1    /*tables for pair  5*/
        , -3, -1,  3,  2, -4,  4, -2    /*tables for pair  6*/
        ,  4, -2, -3, -1,  3,  2, -4    /*tables for pair  7*/
        , -4,  4, -2, -3, -1,  3,  2    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*15];
    } _3howel10_1_3 =
        {3,10,5,"3howel10.1-3"
        ,  1,  1,  1    /*  sets for table 1*/
        , -1, -1, -3    /*  sets for table 2*/
        ,  2,  2,  2    /*  sets for table 3*/
        , -3, -5, -5    /*  sets for table 4*/
        ,  3,  3,  3    /*  sets for table 5*/

        ,  1,  4,  2    /*tables for pair  1*/
        , -1, -3, -4    /*tables for pair  2*/
        ,  2,  3,  5    /*tables for pair  3*/
        , -2,  5,  3    /*tables for pair  4*/
        ,  3, -2, -5    /*tables for pair  5*/
        , -3, -5,  1    /*tables for pair  6*/
        ,  4, -4, -1    /*tables for pair  7*/
        , -4, -1,  4    /*tables for pair  8*/
        ,  5,  2, -3    /*tables for pair  9*/
        , -5,  1, -2    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*15];
    } _3howel10_2_3 =
        {3,10,5,"3howel10.2-3"
        ,  1,  1,  1    /*  sets for table 1*/
        , -1, -1, -3    /*  sets for table 2*/
        ,  2,  2,  2    /*  sets for table 3*/
        , -3, -5, -5    /*  sets for table 4*/
        ,  3,  3,  3    /*  sets for table 5*/

        ,  1,  3,  4    /*tables for pair  1*/
        ,  4, -5,  1    /*tables for pair  2*/
        , -5,  2, -2    /*tables for pair  3*/
        ,  3, -1, -5    /*tables for pair  4*/
        ,  2, -3,  5    /*tables for pair  5*/
        , -1,  5,  3    /*tables for pair  6*/
        , -2, -4,  2    /*tables for pair  7*/
        ,  5,  1, -3    /*tables for pair  8*/
        , -4, -2, -4    /*tables for pair  9*/
        , -3,  4, -1    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*15];
    } _3howel10_3_3 =
        {3,10,5,"3howel10.3-3"
        ,  1,  1,  1    /*  sets for table 1*/
        , -1, -1, -3    /*  sets for table 2*/
        ,  2,  2,  2    /*  sets for table 3*/
        , -3, -5, -5    /*  sets for table 4*/
        ,  3,  3,  3    /*  sets for table 5*/

        ,  5,  1,  2    /*tables for pair  1*/
        ,  3, -2,  4    /*tables for pair  2*/
        , -1, -4, -2    /*tables for pair  3*/
        , -3, -1,  5    /*tables for pair  4*/
        ,  2, -3, -4    /*tables for pair  5*/
        ,  1, -5, -3    /*tables for pair  6*/
        ,  4,  2, -5    /*tables for pair  7*/
        , -5,  3, -1    /*tables for pair  8*/
        , -4,  5,  1    /*tables for pair  9*/
        , -2,  4,  3    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*15];
    } _4howel10_2_2 =
        {4,10,5,"4howel10.2-2"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -2, -3, -4    /*  sets for table 5*/

        , -4,  2, -1, -3    /*tables for pair  1*/
        ,  3, -5, -4, -1    /*tables for pair  2*/
        , -2,  3,  1,  4    /*tables for pair  3*/
        , -3, -1, -2, -5    /*tables for pair  4*/
        ,  5,  5,  5,  5    /*tables for pair  5*/
        ,  1, -2,  3, -4    /*tables for pair  6*/
        , -1, -3,  4, -2    /*tables for pair  7*/
        , -5,  4,  2,  3    /*tables for pair  8*/
        ,  4,  1, -3,  2    /*tables for pair  9*/
        ,  2, -4, -5,  1    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*15];
    } _5howel10_1_2 =
        {5,10,5,"5howel10.1-2"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/

        ,  1,  5,  2,  3,  4    /*tables for pair  1*/
        , -1, -2,  5, -4,  3    /*tables for pair  2*/
        ,  2,  3,  1,  4, -5    /*tables for pair  3*/
        , -2,  1, -3, -5, -4    /*tables for pair  4*/
        ,  3, -4, -2,  1,  5    /*tables for pair  5*/
        , -3,  2,  4,  5, -1    /*tables for pair  6*/
        ,  4, -5,  3, -1, -2    /*tables for pair  7*/
        , -4, -3, -5,  2,  1    /*tables for pair  8*/
        ,  5,  4, -1, -2, -3    /*tables for pair  9*/
        , -5, -1, -4, -3,  2    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[9*15];
    } _9howel10_1_1 =
        {9,10,5,"9howel10.1-1"
        ,  1,  2,  5,  3,  4,  6,  7,  8,  9    /*  sets for table 1*/
        ,  2,  5,  3,  4,  6,  7,  8,  9,  1    /*  sets for table 2*/
        ,  3,  4,  6,  7,  8,  9,  1,  2,  5    /*  sets for table 3*/
        ,  4,  6,  7,  8,  9,  1,  2,  5,  3    /*  sets for table 4*/
        ,  5,  3,  4,  6,  7,  8,  9,  1,  2    /*  sets for table 5*/

        ,  1, -4,  2,  4, -1, -2,  5,  3, -3    /*tables for pair  1*/
        , -1, -2,  5,  3, -3,  1, -4,  2,  4    /*tables for pair  2*/
        ,  2,  4, -1, -2,  5,  3, -3,  1, -4    /*tables for pair  3*/
        , -2,  5,  3, -3,  1, -4,  2,  4, -1    /*tables for pair  4*/
        ,  3, -3,  1, -4,  2,  4, -1, -2,  5    /*tables for pair  5*/
        , -3,  1, -4,  2,  4, -1, -2,  5,  3    /*tables for pair  6*/
        ,  4, -1, -2,  5,  3, -3,  1, -4,  2    /*tables for pair  7*/
        , -4,  2,  4, -1, -2,  5,  3, -3,  1    /*tables for pair  8*/
        ,  5,  3, -3,  1, -4,  2,  4, -1, -2    /*tables for pair  9*/
        , -5, -5, -5, -5, -5, -5, -5, -5, -5    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[9*15];
    } _9howel10_2_1 =
        {9,10,5,"9howel10.2-1"
        ,  1,  1,  1,  1,  1,  6,  6,  6,  6    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  7,  7,  7,  7    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  8,  8,  8,  8    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  9,  9,  9,  9    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5, -1, -2, -3, -4    /*  sets for table 5*/

        ,  1,  5,  2,  3,  4, -4,  2, -1, -3    /*tables for pair  1*/
        , -1, -2,  5, -4,  3,  3, -5, -4, -1    /*tables for pair  2*/
        ,  2,  3,  1,  4, -5, -2,  3,  1,  4    /*tables for pair  3*/
        , -2,  1, -3, -5, -4, -3, -1, -2, -5    /*tables for pair  4*/
        ,  3, -4, -2,  1,  5,  5,  5,  5,  5    /*tables for pair  5*/
        , -3,  2,  4,  5, -1,  1, -2,  3, -4    /*tables for pair  6*/
        ,  4, -5,  3, -1, -2, -1, -3,  4, -2    /*tables for pair  7*/
        , -4, -3, -5,  2,  1, -5,  4,  2,  3    /*tables for pair  8*/
        ,  5,  4, -1, -2, -3,  4,  1, -3,  2    /*tables for pair  9*/
        , -5, -1, -4, -3,  2,  2, -4, -5,  1    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*18];
    } _3howel12_3_3 =
        {3,12,6,"3howel12.3-3"
        ,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3    /*  sets for table 3*/
        , -1, -1, -1    /*  sets for table 4*/
        , -2, -2, -2    /*  sets for table 5*/
        , -3, -3, -3    /*  sets for table 6*/

        ,  1,  2,  3    /*tables for pair  1*/
        ,  4, -5, -6    /*tables for pair  2*/
        ,  5,  1,  6    /*tables for pair  3*/
        , -2,  4, -3    /*tables for pair  4*/
        , -5,  6,  1    /*tables for pair  5*/
        , -1,  3, -2    /*tables for pair  6*/
        , -4, -3,  5    /*tables for pair  7*/
        ,  2, -6,  4    /*tables for pair  8*/
        , -3,  5, -1    /*tables for pair  9*/
        , -6, -2, -4    /*tables for pair  10*/
        ,  6, -1, -5    /*tables for pair  11*/
        ,  3, -4,  2    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*18];
    } _3howel12_3_3nz =
        {3,12,6,"3howel12nz.3-3"
        ,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3    /*  sets for table 3*/
        , -1, -1, -1    /*  sets for table 4*/
        , -2, -2, -2    /*  sets for table 5*/
        , -3, -3, -3    /*  sets for table 6*/

        ,  1,  2,  3    /*tables for pair  1*/
        ,  4,  5, -6    /*tables for pair  2*/
        ,  5,  1,  6    /*tables for pair  3*/
        , -2, -4, -3    /*tables for pair  4*/
        , -5, -6,  1    /*tables for pair  5*/
        , -1,  3, -2    /*tables for pair  6*/
        , -4, -3, -5    /*tables for pair  7*/
        ,  2,  6,  4    /*tables for pair  8*/
        , -3, -5, -1    /*tables for pair  9*/
        , -6, -2, -4    /*tables for pair  10*/
        ,  6, -1,  5    /*tables for pair  11*/
        ,  3,  4,  2    /*tables for pair  12*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[3*18];
    } _3harrie_12nzow =
        {3,12,6,"3harrie_nzow"
        ,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3    /*  sets for table 3*/
        , -1, -1, -1    /*  sets for table 4*/
        , -2, -2, -2    /*  sets for table 5*/
        , -3, -3, -3    /*  sets for table 6*/

        ,  1,  2,  3    /*tables for pair  1*/
        , -1, -3, -2    /*tables for pair  2*/
        ,  2,  3,  1    /*tables for pair  3*/
        , -2, -1, -3    /*tables for pair  4*/
        ,  3,  1,  2    /*tables for pair  5*/
        , -3, -2, -1    /*tables for pair  6*/
        ,  4,  5,  6    /*tables for pair  7*/
        , -4, -6, -5    /*tables for pair  8*/
        ,  5,  6,  4    /*tables for pair  9*/
        , -5, -4, -6    /*tables for pair  10*/
        ,  6,  4,  5    /*tables for pair  11*/
        , -6, -5, -4    /*tables for pair  12*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*18];
    } _4howel12_1_3 =
        {4,12,6,"4howel12.1-3"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -1, -3, -3    /*  sets for table 5*/
        , -2, -2, -4, -4    /*  sets for table 6*/

        ,  1, -3,  2,  6    /*tables for pair  1*/
        , -1, -4, -3,  2    /*tables for pair  2*/
        ,  2,  3,  4, -1    /*tables for pair  3*/
        , -2,  4,  1, -5    /*tables for pair  4*/
        ,  3,  5, -6, -2    /*tables for pair  5*/
        , -3, -2, -1,  4    /*tables for pair  6*/
        ,  4, -6,  5,  1    /*tables for pair  7*/
        , -4, -1, -2,  3    /*tables for pair  8*/
        ,  5,  2, -4, -3    /*tables for pair  9*/
        , -5,  6,  6,  5    /*tables for pair  10*/
        ,  6,  1,  3, -4    /*tables for pair  11*/
        , -6, -5, -5, -6    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*18];
    } _4howel12_2_3 =
        {4,12,6,"4howel12.2-3"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -3, -3, -1    /*  sets for table 5*/
        , -2, -2, -4, -4    /*  sets for table 6*/

        ,  2,  3,  1,  4    /*tables for pair  1*/
        ,  5,  6, -6, -3    /*tables for pair  2*/
        , -6, -1, -5, -6    /*tables for pair  3*/
        , -3, -2,  4, -1    /*tables for pair  4*/
        , -1,  2,  3, -4    /*tables for pair  5*/
        , -4, -6,  5,  5    /*tables for pair  6*/
        ,  1, -3, -4,  2    /*tables for pair  7*/
        ,  4,  1, -2,  3    /*tables for pair  8*/
        ,  3, -4, -1, -2    /*tables for pair  9*/
        ,  6,  5,  6, -5    /*tables for pair  10*/
        , -2,  4, -3,  1    /*tables for pair  11*/
        , -5, -5,  2,  6    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*18];
    } _5howel12_2_2 =
        {5,12,6,"5howel12.2-2"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -2, -3, -4, -5    /*  sets for table 6*/

        ,  1, -3, -5, -6,  2    /*tables for pair  1*/
        , -2,  4,  1, -3,  5    /*tables for pair  2*/
        ,  5,  1, -6,  4, -2    /*tables for pair  3*/
        , -4, -6,  3,  5, -1    /*tables for pair  4*/
        ,  6,  6,  5,  3, -4    /*tables for pair  5*/
        , -3, -4, -2,  1, -6    /*tables for pair  6*/
        ,  3, -1, -4, -2, -5    /*tables for pair  7*/
        , -1,  5,  2, -4, -3    /*tables for pair  8*/
        ,  2, -5,  6,  6,  1    /*tables for pair  9*/
        , -5,  2, -3, -1,  4    /*tables for pair  10*/
        ,  4,  3, -1,  2,  6    /*tables for pair  11*/
        , -6, -2,  4, -5,  3    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6howel12_1_2 =
        {6,12,6,"6howel12.1-2"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1, -4,  3, -2, -6, -5    /*tables for pair  1*/
        , -1, -6,  4,  3,  5, -2    /*tables for pair  2*/
        ,  2,  6, -5,  1, -4, -3    /*tables for pair  3*/
        , -2,  1, -3, -6, -5,  4    /*tables for pair  4*/
        ,  3,  2, -1, -5,  4,  6    /*tables for pair  5*/
        , -3,  5, -2, -1,  6, -4    /*tables for pair  6*/
        ,  4,  3,  1,  6,  2,  5    /*tables for pair  7*/
        , -4, -1, -6,  5, -3,  2    /*tables for pair  8*/
        ,  5, -3,  2,  4,  1, -6    /*tables for pair  9*/
        , -5,  4,  6, -3, -2, -1    /*tables for pair  10*/
        ,  6, -2,  5, -4,  3,  1    /*tables for pair  11*/
        , -6, -5, -4,  2, -1,  3    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[11*18];
    } _11howe12_2_1 =
        {11,12,6,"11howe12.2-1"
        ,  1,  1,  1,  1,  1,  1,  7,  7,  7,  7,  7    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  8,  8,  8,  8,  8    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  9,  9,  9,  9,  9    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4, 10, 10, 10, 10, 10    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5, 11, 11, 11, 11, 11    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6, -1, -2, -3, -4, -5    /*  sets for table 6*/

        ,  1, -4,  3, -2, -6, -5,  1, -3, -5, -6,  2    /*tables for pair  1*/
        , -1, -6,  4,  3,  5, -2, -2,  4,  1, -3,  5    /*tables for pair  2*/
        ,  2,  6, -5,  1, -4, -3,  5,  1, -6,  4, -2    /*tables for pair  3*/
        , -2,  1, -3, -6, -5,  4, -4, -6,  3,  5, -1    /*tables for pair  4*/
        ,  3,  2, -1, -5,  4,  6,  6,  6,  5,  3, -4    /*tables for pair  5*/
        , -3,  5, -2, -1,  6, -4, -3, -4, -2,  1, -6    /*tables for pair  6*/
        ,  4,  3,  1,  6,  2,  5,  3, -1, -4, -2, -5    /*tables for pair  7*/
        , -4, -1, -6,  5, -3,  2, -1,  5,  2, -4, -3    /*tables for pair  8*/
        ,  5, -3,  2,  4,  1, -6,  2, -5,  6,  6,  1    /*tables for pair  9*/
        , -5,  4,  6, -3, -2, -1, -5,  2, -3, -1,  4    /*tables for pair  10*/
        ,  6, -2,  5, -4,  3,  1,  4,  3, -1,  2,  6    /*tables for pair  11*/
        , -6, -5, -4,  2, -1,  3, -6, -2,  4, -5,  3    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[11*18];
    } _11howe12_3_1 =
        {11,12,6,"11howe12.3-1"
        ,  1,  1,  1,  1,  5,  5,  5,  5,  9,  9,  9    /*  sets for table 1*/
        ,  2,  2,  2,  2,  6,  6,  6,  6, 10, 10, 10    /*  sets for table 2*/
        ,  3,  3,  3,  3,  7,  7,  7,  7, 11, 11, 11    /*  sets for table 3*/
        ,  4,  4,  4,  4,  8,  8,  8,  8, -1, -1, -1    /*  sets for table 4*/
        , -1, -1, -3, -3, -1, -3, -3, -1, -2, -2, -2    /*  sets for table 5*/
        , -2, -2, -4, -4, -2, -2, -4, -4, -3, -3, -3    /*  sets for table 6*/

        ,  1, -3,  2,  6,  2,  3,  1,  4,  1,  2,  3    /*tables for pair  1*/
        , -1, -4, -3,  2,  5,  6, -6, -3,  4, -5, -6    /*tables for pair  2*/
        ,  2,  3,  4, -1, -6, -1, -5, -6,  5,  1,  6    /*tables for pair  3*/
        , -2,  4,  1, -5, -3, -2,  4, -1, -2,  4, -3    /*tables for pair  4*/
        ,  3,  5, -6, -2, -1,  2,  3, -4, -5,  6,  1    /*tables for pair  5*/
        , -3, -2, -1,  4, -4, -6,  5,  5, -1,  3, -2    /*tables for pair  6*/
        ,  4, -6,  5,  1,  1, -3, -4,  2, -4, -3,  5    /*tables for pair  7*/
        , -4, -1, -2,  3,  4,  1, -2,  3,  2, -6,  4    /*tables for pair  8*/
        ,  5,  2, -4, -3,  3, -4, -1, -2, -3,  5, -1    /*tables for pair  9*/
        , -5,  6,  6,  5,  6,  5,  6, -5, -6, -2, -4    /*tables for pair  10*/
        ,  6,  1,  3, -4, -2,  4, -3,  1,  6, -1, -5    /*tables for pair  11*/
        , -6, -5, -5, -6, -5, -5,  2,  6,  3, -4,  2    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*21];
    } _4howel14_1_3 =
        {4,14,7,"4howel14.1-3"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -1, -1, -2    /*  sets for table 5*/
        , -2, -3, -2, -3    /*  sets for table 6*/
        , -3, -4, -4, -4    /*  sets for table 7*/

        ,  1,  2,  3,  4    /*tables for pair  1*/
        , -1,  6, -4,  5    /*tables for pair  2*/
        ,  2,  7, -3, -1    /*tables for pair  3*/
        , -2, -1,  4,  6    /*tables for pair  4*/
        ,  3,  1,  6, -4    /*tables for pair  5*/
        , -3, -4,  1, -5    /*tables for pair  6*/
        ,  4, -3, -2,  1    /*tables for pair  7*/
        , -4, -2, -1, -3    /*tables for pair  8*/
        ,  5, -6, -7,  2    /*tables for pair  9*/
        , -5,  4, -6,  3    /*tables for pair  10*/
        ,  6, -5,  7, -6    /*tables for pair  11*/
        , -6,  3,  5, -7    /*tables for pair  12*/
        ,  7, -7, -5, -2    /*tables for pair  13*/
        , -7,  5,  2,  7    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*21];
    } _4howel14_2_3 =
        {4,14,7,"4howel14.2-3"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -1, -2, -1    /*  sets for table 5*/
        , -2, -2, -3, -3    /*  sets for table 6*/
        , -3, -4, -4, -4    /*  sets for table 7*/

        ,  1,  2,  6,  7    /*tables for pair  1*/
        ,  7,  7, -5,  5    /*tables for pair  2*/
        , -6, -4, -1, -3    /*tables for pair  3*/
        , -1,  6,  4, -6    /*tables for pair  4*/
        , -4, -5,  3, -2    /*tables for pair  5*/
        ,  5, -2,  7,  6    /*tables for pair  6*/
        , -2,  1, -3, -4    /*tables for pair  7*/
        ,  4,  3, -2, -5    /*tables for pair  8*/
        ,  2,  4, -6, -1    /*tables for pair  9*/
        , -7, -6,  1, -7    /*tables for pair  10*/
        , -3,  5,  5,  4    /*tables for pair  11*/
        ,  6, -3, -7,  1    /*tables for pair  12*/
        ,  3, -1, -4,  2    /*tables for pair  13*/
        , -5, -7,  2,  3    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5howel14_3_3 =
        {5,14,7,"5howel14.3-3"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -3, -3, -1, -2, -1    /*  sets for table 6*/
        , -4, -4, -5, -5, -2    /*  sets for table 7*/

        ,  1,  2,  3,  4,  5    /*tables for pair  1*/
        , -4, -5, -1,  6,  3    /*tables for pair  2*/
        ,  6,  4,  1,  5, -7    /*tables for pair  3*/
        ,  2, -3, -4, -7,  1    /*tables for pair  4*/
        , -6,  7, -7, -6,  6    /*tables for pair  5*/
        ,  3, -4, -5,  1, -2    /*tables for pair  6*/
        , -1,  5,  4, -3,  2    /*tables for pair  7*/
        ,  7, -6,  6,  7,  7    /*tables for pair  8*/
        , -7,  3,  5, -2, -6    /*tables for pair  9*/
        ,  5, -1, -2,  3, -4    /*tables for pair  10*/
        , -3, -2, -6, -5,  4    /*tables for pair  11*/
        , -2,  1,  7, -4, -3    /*tables for pair  12*/
        ,  4,  6,  2, -1, -5    /*tables for pair  13*/
        , -5, -7, -3,  2, -1    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*21];
    } _6howel14_2_2 =
        {6,14,7,"6howel14.2-2"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/

        , -1,  6, -2, -4, -7, -3    /*tables for pair  1*/
        ,  6,  2,  4, -3,  5, -1    /*tables for pair  2*/
        , -7, -4,  5,  3,  6,  2    /*tables for pair  3*/
        , -4, -2, -7, -6, -1,  5    /*tables for pair  4*/
        ,  2, -1,  3, -7, -5,  6    /*tables for pair  5*/
        , -5, -3, -6, -2, -4,  1    /*tables for pair  6*/
        ,  1,  5, -3,  6,  4, -2    /*tables for pair  7*/
        , -2,  3, -1,  4, -6, -5    /*tables for pair  8*/
        ,  3, -5, -4,  1,  2, -7    /*tables for pair  9*/
        , -6, -7,  1,  5,  3, -4    /*tables for pair  10*/
        ,  7,  7,  7,  7,  7,  7    /*tables for pair  11*/
        , -3, -6, -5,  2,  1,  4    /*tables for pair  12*/
        ,  5,  4,  2, -1, -3, -6    /*tables for pair  13*/
        ,  4,  1,  6, -5, -2,  3    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*21];
    } _7howel14_1_2 =
        {7,14,7,"7howel14.1-2"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  4,  7,  5,  6,  3,  2    /*tables for pair  1*/
        , -1, -7,  6,  3, -4, -2,  5    /*tables for pair  2*/
        ,  2, -3, -7,  6, -5,  1, -4    /*tables for pair  3*/
        , -2, -6,  5, -4,  7, -3, -1    /*tables for pair  4*/
        ,  3, -2, -5, -1, -6, -7,  4    /*tables for pair  5*/
        , -3,  6, -4, -5,  2, -1, -7    /*tables for pair  6*/
        ,  4,  1, -2,  7, -3, -6, -5    /*tables for pair  7*/
        , -4,  7, -3, -2,  1, -5,  6    /*tables for pair  8*/
        ,  5, -4,  3, -6, -2,  7,  1    /*tables for pair  9*/
        , -5,  3,  4,  1, -7,  6, -2    /*tables for pair  10*/
        ,  6,  5,  2, -3, -1,  4,  7    /*tables for pair  11*/
        , -6,  2,  1, -7,  4,  5, -3    /*tables for pair  12*/
        ,  7, -5, -1,  4,  3,  2, -6    /*tables for pair  13*/
        , -7, -1, -6,  2,  5, -4,  3    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*24];
    } _5howel16_1_3 =
        {5,16,8,"5howel16.1-3"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -1, -5    /*  sets for table 6*/
        , -2, -2, -4, -2, -4    /*  sets for table 7*/
        , -3, -5, -5, -3, -3    /*  sets for table 8*/

        ,  1,  5, -3,  2,  4    /*tables for pair  1*/
        , -1,  2,  5, -3, -7    /*tables for pair  2*/
        ,  2, -1,  3,  5,  7    /*tables for pair  3*/
        , -2, -5,  4,  1,  3    /*tables for pair  4*/
        ,  3, -2,  1, -5, -4    /*tables for pair  5*/
        , -3,  4, -5, -1, -2    /*tables for pair  6*/
        ,  4, -7,  6, -8,  5    /*tables for pair  7*/
        , -4, -3,  2, -6, -6    /*tables for pair  8*/
        ,  5,  1, -4,  3,  2    /*tables for pair  9*/
        , -5, -4, -1, -2, -3    /*tables for pair  10*/
        ,  6,  3, -7, -7, -5    /*tables for pair  11*/
        , -6, -8, -2, -4, -8    /*tables for pair  12*/
        ,  7,  6, -8,  6,  8    /*tables for pair  13*/
        , -7,  8,  7,  8, -1    /*tables for pair  14*/
        ,  8, -6, -6,  7,  6    /*tables for pair  15*/
        , -8,  7,  8,  4,  1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*24];
    } _5howel16_2_3 =
        {5,16,8,"5howel16.2-3"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -4, -1    /*  sets for table 6*/
        , -3, -3, -3, -5, -4    /*  sets for table 7*/
        , -5, -5, -2, -2, -2    /*  sets for table 8*/

        ,  1,  2,  3,  4, -5    /*tables for pair  1*/
        ,  3,  8, -1,  6, -8    /*tables for pair  2*/
        ,  4,  3, -5,  2, -6    /*tables for pair  3*/
        , -6, -8,  2,  3, -7    /*tables for pair  4*/
        , -2, -7,  6, -7,  7    /*tables for pair  5*/
        , -5, -3, -8, -4,  1    /*tables for pair  6*/
        ,  8,  7,  4,  8, -1    /*tables for pair  7*/
        ,  7,  6,  8,  7,  6    /*tables for pair  8*/
        ,  2,  4,  7,  1,  5    /*tables for pair  9*/
        , -3,  1,  5, -8, -4    /*tables for pair  10*/
        , -4, -1, -2, -5,  3    /*tables for pair  11*/
        , -8, -4, -6, -3,  2    /*tables for pair  12*/
        ,  5, -2, -4, -1, -3    /*tables for pair  13*/
        , -7,  5,  1, -2,  4    /*tables for pair  14*/
        ,  6, -5, -3, -6, -2    /*tables for pair  15*/
        , -1, -6, -7,  5,  8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*24];
    } _5howel16_3_3 =
        {5,16,8,"5howel16.3-3"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -1, -5, -5, -5    /*  sets for table 6*/
        , -2, -2, -4, -4, -4    /*  sets for table 7*/
        , -3, -3, -3, -2, -1    /*  sets for table 8*/

        , -1,  2,  4,  5,  3    /*tables for pair  1*/
        ,  1, -5, -3, -4, -2    /*tables for pair  2*/
        , -6,  4,  5,  3,  2    /*tables for pair  3*/
        ,  6, -2, -8, -7, -5    /*tables for pair  4*/
        , -2,  1,  3,  7,  6    /*tables for pair  5*/
        ,  3, -7, -5, -1, -4    /*tables for pair  6*/
        , -4,  3,  2,  1,  5    /*tables for pair  7*/
        ,  5, -1, -7, -2, -3    /*tables for pair  8*/
        , -8,  5,  1,  8,  4    /*tables for pair  9*/
        ,  2, -3, -1, -6, -7    /*tables for pair  10*/
        , -5,  7,  8,  4,  1    /*tables for pair  11*/
        ,  4, -8, -6, -8, -1    /*tables for pair  12*/
        , -3,  6,  6,  2,  7    /*tables for pair  13*/
        ,  7, -6, -4, -3, -6    /*tables for pair  14*/
        , -7,  8,  7,  6,  8    /*tables for pair  15*/
        ,  8, -4, -2, -5, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7howel16 =
        {7,16,8,"7howel16"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/

        ,  1,  2,  3, -4,  5,  6, -7    /*tables for pair  1*/
        , -1, -4, -7,  3, -6, -2,  5    /*tables for pair  2*/
        ,  2,  4,  6, -1,  3,  5,  8    /*tables for pair  3*/
        , -2, -5, -1,  4, -7, -3,  6    /*tables for pair  4*/
        ,  3,  5,  7, -2,  4, -8, -1    /*tables for pair  5*/
        , -3, -6, -2,  5, -1, -4,  7    /*tables for pair  6*/
        ,  4,  6,  1, -3,  8,  7, -2    /*tables for pair  7*/
        , -4, -7, -3,  6, -2, -5,  1    /*tables for pair  8*/
        ,  5,  7,  2,  8,  6,  1, -3    /*tables for pair  9*/
        , -5, -1, -4,  7, -3, -6,  2    /*tables for pair  10*/
        ,  6,  1, -8, -5,  7,  2, -4    /*tables for pair  11*/
        , -6, -2, -5,  1, -4, -7,  3    /*tables for pair  12*/
        ,  7,  8,  4, -6,  1,  3, -5    /*tables for pair  13*/
        , -7, -3, -6,  2, -5, -1,  4    /*tables for pair  14*/
        ,  8,  3,  5, -7,  2,  4, -6    /*tables for pair  15*/
        , -8, -8,  8, -8, -8,  8, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*27];
    } _5howel18_3_3 =
        {5,18,9,"5howel18.3-3"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -1, -1, -1, -2    /*  sets for table 6*/
        , -2, -2, -2, -3, -3    /*  sets for table 7*/
        , -3, -3, -4, -4, -4    /*  sets for table 8*/
        , -4, -5, -5, -5, -5    /*  sets for table 9*/

        ,  1,  5, -4,  3, -2    /*tables for pair  1*/
        ,  6,  7, -3,  5, -4    /*tables for pair  2*/
        ,  3,  4, -1,  9, -6    /*tables for pair  3*/
        ,  5,  1, -2,  8, -3    /*tables for pair  4*/
        , -6,  9,  2,  4, -7    /*tables for pair  5*/
        , -1,  8, -9,  2,  4    /*tables for pair  6*/
        ,  2, -9,  3,  6, -8    /*tables for pair  7*/
        ,  9,  6, -5,  7,  2    /*tables for pair  8*/
        , -9, -1, -7, -3, -5    /*tables for pair  9*/
        ,  4, -7,  1, -7,  9    /*tables for pair  10*/
        ,  8, -6, -8, -5,  6    /*tables for pair  11*/
        , -2,  3, -6, -8,  5    /*tables for pair  12*/
        , -5, -8,  7, -4, -1    /*tables for pair  13*/
        ,  7, -4,  9,  1,  7    /*tables for pair  14*/
        , -7, -3,  4, -6, -9    /*tables for pair  15*/
        , -8,  2,  5, -1,  8    /*tables for pair  16*/
        , -3, -5,  8, -2,  1    /*tables for pair  17*/
        , -4, -2,  6, -9,  3    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6howel18_1_3 =
        {6,18,9,"6howel18.1-3"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -4, -4, -4, -3, -3    /*  sets for table 7*/
        , -2, -2, -5, -5, -5, -1    /*  sets for table 8*/
        , -3, -6, -6, -1, -2, -6    /*  sets for table 9*/

        ,  1,  9, -3,  4,  8, -2    /*tables for pair  1*/
        , -1, -3,  6, -5,  9,  4    /*tables for pair  2*/
        ,  2,  6, -4,  5,  3, -8    /*tables for pair  3*/
        , -2, -5,  3, -1, -4,  6    /*tables for pair  4*/
        ,  3,  4, -2,  8,  1, -9    /*tables for pair  5*/
        , -3, -2,  4, -6, -5,  1    /*tables for pair  6*/
        ,  4,  5, -1,  6, -2, -7    /*tables for pair  7*/
        , -4, -6,  5, -2, -1,  3    /*tables for pair  8*/
        ,  5,  2, -9,  9,  7, -4    /*tables for pair  9*/
        , -5, -4,  1, -3, -6,  2    /*tables for pair  10*/
        ,  6,  8, -8,  3,  4, -1    /*tables for pair  11*/
        , -6, -1,  2, -4, -3,  5    /*tables for pair  12*/
        ,  7,  3, -7,  2,  6, -5    /*tables for pair  13*/
        , -7, -8,  9, -7, -8,  7    /*tables for pair  14*/
        ,  8,  7, -6,  1,  5, -3    /*tables for pair  15*/
        , -8, -9,  7, -8, -7,  8    /*tables for pair  16*/
        ,  9,  1, -5,  7,  2, -6    /*tables for pair  17*/
        , -9, -7,  8, -9, -9,  9    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6howel18_2_3 =
        {6,18,9,"6howel18.2-3"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -2, -2, -1, -3    /*  sets for table 7*/
        , -2, -4, -4, -4, -5, -5    /*  sets for table 8*/
        , -3, -3, -5, -6, -6, -6    /*  sets for table 9*/

        ,  1, -6,  5,  2, -3,  4    /*tables for pair  1*/
        ,  6, -1,  2,  5,  4,  7    /*tables for pair  2*/
        , -1, -9,  7,  4,  8,  9    /*tables for pair  3*/
        ,  9,  1,  8,  7,  9,  5    /*tables for pair  4*/
        ,  8, -5,  4,  1,  3, -9    /*tables for pair  5*/
        ,  5, -2,  1,  8, -9,  3    /*tables for pair  6*/
        ,  2, -7,  6,  3, -8, -4    /*tables for pair  7*/
        ,  7,  2,  3,  9, -4, -5    /*tables for pair  8*/
        ,  3,  5, -6, -4,  7,  2    /*tables for pair  9*/
        , -5, -3, -8,  6, -2,  1    /*tables for pair  10*/
        , -3,  7, -4, -2,  6,  8    /*tables for pair  11*/
        , -7,  3, -2, -8,  5,  6    /*tables for pair  12*/
        ,  4,  6, -7, -3, -1, -8    /*tables for pair  13*/
        , -6, -8, -3, -7, -5, -1    /*tables for pair  14*/
        , -4,  9,  9, -1, -6, -2    /*tables for pair  15*/
        , -9, -4, -1, -5,  2, -6    /*tables for pair  16*/
        , -8,  4, -9, -6, -7, -7    /*tables for pair  17*/
        , -2,  8, -5, -9,  1, -3    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*15];
    } _4stayr10 =
        {4,10,5,"4stayr10"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -4, -2, -3    /*  sets for table 5*/

        ,  1, -2,  3,  4    /*tables for pair  1*/
        , -5,  3, -4, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -2,  4, -3, -1    /*tables for pair  4*/
        ,  3, -5,  1,  2    /*tables for pair  5*/
        , -3,  1, -5, -4    /*tables for pair  6*/
        ,  4, -3,  2,  1    /*tables for pair  7*/
        , -4,  2, -1, -5    /*tables for pair  8*/
        ,  5, -4,  5,  5    /*tables for pair  9*/
        , -1,  5, -2, -3    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*15];
    } _4stayr10_1_x =
        {4,10,5,"4stayr10.1-x"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -4, -2, -3    /*  sets for table 5*/

        ,  1, -2,  3,  4    /*tables for pair  1*/
        , -1,  3, -4, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -2,  4, -3, -1    /*tables for pair  4*/
        ,  3, -5,  1,  2    /*tables for pair  5*/
        , -3,  1, -5, -4    /*tables for pair  6*/
        ,  4, -3,  2,  1    /*tables for pair  7*/
        , -4,  2, -1, -5    /*tables for pair  8*/
        ,  5, -4,  5,  5    /*tables for pair  9*/
        , -5,  5, -2, -3    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*18];
    } _4stayr12 =
        {4,12,6,"4stayr12"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -3, -1, -3    /*  sets for table 5*/
        , -2, -4, -2, -4    /*  sets for table 6*/

        ,  1, -2,  3,  4    /*tables for pair  1*/
        , -5,  3, -4, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -6,  4, -3, -1    /*tables for pair  4*/
        ,  3, -6,  1,  2    /*tables for pair  5*/
        , -3,  1, -6, -6    /*tables for pair  6*/
        ,  4, -5,  2,  1    /*tables for pair  7*/
        , -4,  2, -5, -5    /*tables for pair  8*/
        ,  5, -4,  6,  5    /*tables for pair  9*/
        , -1,  6, -2, -3    /*tables for pair  10*/
        ,  6, -3,  5,  6    /*tables for pair  11*/
        , -2,  5, -1, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*18];
    } _4stayr12_1_x =
        {4,12,6,"4stayr12.1-x"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -3, -1, -3    /*  sets for table 5*/
        , -2, -4, -2, -4    /*  sets for table 6*/

        ,  1, -2,  3,  4    /*tables for pair  1*/
        , -1,  3, -4, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -2,  4, -3, -1    /*tables for pair  4*/
        ,  3, -6,  1,  2    /*tables for pair  5*/
        , -3,  1, -6, -6    /*tables for pair  6*/
        ,  4, -5,  2,  1    /*tables for pair  7*/
        , -4,  2, -5, -5    /*tables for pair  8*/
        ,  5, -4,  6,  5    /*tables for pair  9*/
        , -5,  6, -2, -3    /*tables for pair  10*/
        ,  6, -3,  5,  6    /*tables for pair  11*/
        , -6,  5, -1, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*18];
    } _5stayr12 =
        {5,12,6,"5stayr12"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -3, -5, -2, -4    /*  sets for table 6*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -6,  5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  1, -6, -4, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -1, -5, -6    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  6, -2, -1, -5    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  4, -3, -6, -1    /*tables for pair  10*/
        ,  6, -6,  6,  6,  6    /*tables for pair  11*/
        , -1,  3, -5, -2, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*18];
    } _5stayr12_1_x =
        {5,12,6,"5stayr12.1-x"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -3, -5, -2, -4    /*  sets for table 6*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -1,  5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  1, -6, -4, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -1, -5, -6    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  6, -2, -1, -5    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  4, -3, -6, -1    /*tables for pair  10*/
        ,  6, -6,  6,  6,  6    /*tables for pair  11*/
        , -6,  3, -5, -2, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*19];
    } _6stayr13 =
        {6,13,6,"6stayr13"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1, -6,  2, -4,  0,  3    /*tables for pair  1*/
        , -1,  5, -3,  6, -4, -2    /*tables for pair  2*/
        ,  2, -1,  3,  0,  5,  6    /*tables for pair  3*/
        , -2,  3, -1,  4, -6, -5    /*tables for pair  4*/
        ,  3, -5,  4, -1,  2,  0    /*tables for pair  5*/
        , -3,  6, -5,  2, -1, -4    /*tables for pair  6*/
        ,  4, -2,  0, -6,  1,  5    /*tables for pair  7*/
        , -4,  1, -6,  5, -2, -3    /*tables for pair  8*/
        ,  0, -4,  5, -3,  6,  2    /*tables for pair  9*/
        , -5,  4, -2,  1, -3, -6    /*tables for pair  10*/
        ,  6,  0,  1, -5,  3,  4    /*tables for pair  11*/
        , -6,  2, -4,  3, -5,  1    /*tables for pair  12*/
        ,  5, -3,  6, -2,  4, -1    /*tables for pair  13*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*19];
    } _6stayr13_1_5 =
        {6,13,6,"6stayr13.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1, -6,  2, -4,  0,  3    /*tables for pair  1*/
        , -1,  5, -3,  6, -4, -2    /*tables for pair  2*/
        ,  2, -1,  3,  0,  5,  6    /*tables for pair  3*/
        , -2,  3, -1,  4, -6, -5    /*tables for pair  4*/
        ,  3, -5,  4, -1,  2,  0    /*tables for pair  5*/
        , -3,  6, -5,  2, -1, -4    /*tables for pair  6*/
        ,  4, -2,  0, -6,  1,  5    /*tables for pair  7*/
        , -4,  1, -6,  5, -2, -3    /*tables for pair  8*/
        ,  5, -3,  6, -2,  4, -1    /*tables for pair  9*/
        , -5,  4, -2,  1, -3, -6    /*tables for pair  10*/
        ,  6,  0,  1, -5,  3,  4    /*tables for pair  11*/
        , -6,  2, -4,  3, -5,  1    /*tables for pair  12*/
        ,  0, -4,  5, -3,  6,  2    /*tables for pair  13*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*21];
    } _4stayr14 =
        {4,14,7,"4stayr14"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -2, -1, -1    /*  sets for table 5*/
        , -2, -3, -2, -3    /*  sets for table 6*/
        , -3, -4, -4, -4    /*  sets for table 7*/

        ,  1, -5,  3,  4    /*tables for pair  1*/
        , -5,  3, -7, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -6,  4, -3, -5    /*tables for pair  4*/
        ,  3, -7,  1,  2    /*tables for pair  5*/
        , -7,  1, -6, -7    /*tables for pair  6*/
        ,  4, -6,  2,  1    /*tables for pair  7*/
        , -4,  2, -5, -6    /*tables for pair  8*/
        ,  5, -4,  6,  6    /*tables for pair  9*/
        , -1,  7, -2, -3    /*tables for pair  10*/
        ,  6, -3,  5,  7    /*tables for pair  11*/
        , -2,  6, -1, -4    /*tables for pair  12*/
        ,  7, -2,  7,  5    /*tables for pair  13*/
        , -3,  5, -4, -1    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*21];
    } _4stayr14special =
        {4,14,7,"4stayr14_sp"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2, -4,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -7, -1, -4    /*  sets for table 5*/
        , -2, -3, -2, -3    /*  sets for table 6*/
        , -3,  2, -4, -1    /*  sets for table 7*/

        ,  1, -5,  3,  4    /*tables for pair  1*/
        , -5,  3, -7, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -6,  4, -3, -7    /*tables for pair  4*/
        ,  3, -2,  1,  2    /*tables for pair  5*/
        , -7,  1, -6, -5    /*tables for pair  6*/
        ,  4, -6,  2,  1    /*tables for pair  7*/
        , -4,  7, -5, -6    /*tables for pair  8*/
        ,  5, -4,  6,  6    /*tables for pair  9*/
        , -1,  2, -2, -3    /*tables for pair  10*/
        ,  6, -3,  5,  5    /*tables for pair  11*/
        , -2,  6, -1, -4    /*tables for pair  12*/
        ,  7, -7,  7,  7    /*tables for pair  13*/
        , -3,  5, -4, -1    /*tables for pair  14*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*21];
    } _4stayr14_1_x =
        {4,14,7,"4stayr14.1-x"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -2, -1, -1    /*  sets for table 5*/
        , -2, -3, -2, -3    /*  sets for table 6*/
        , -3, -4, -4, -4    /*  sets for table 7*/

        ,  1, -5,  3,  4    /*tables for pair  1*/
        , -1,  3, -7, -2    /*tables for pair  2*/
        ,  2, -1,  4,  3    /*tables for pair  3*/
        , -2,  4, -3, -5    /*tables for pair  4*/
        ,  3, -7,  1,  2    /*tables for pair  5*/
        , -3,  1, -6, -7    /*tables for pair  6*/
        ,  4, -6,  2,  1    /*tables for pair  7*/
        , -4,  2, -5, -6    /*tables for pair  8*/
        ,  5, -4,  6,  6    /*tables for pair  9*/
        , -5,  7, -2, -3    /*tables for pair  10*/
        ,  6, -3,  5,  7    /*tables for pair  11*/
        , -6,  6, -1, -4    /*tables for pair  12*/
        ,  7, -2,  7,  5    /*tables for pair  13*/
        , -7,  5, -4, -1    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14 =
        {5,14,7,"5stayr14"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -6,  5, -4, -6, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -7,  1, -7, -4, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -6, -5, -6    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  7, -2, -1, -7    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  6, -3, -7, -1    /*tables for pair  10*/
        ,  6, -7,  7,  7,  6    /*tables for pair  11*/
        , -1,  3, -5, -2, -4    /*tables for pair  12*/
        ,  7, -6,  6,  6,  7    /*tables for pair  13*/
        , -2,  4, -1, -3, -5    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x =
        {5,14,7,"5stayr14.1-x"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -1,  5, -4, -6, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  1, -7, -4, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -6, -5, -6    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  7, -2, -1, -7    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  6, -3, -7, -1    /*tables for pair  10*/
        ,  6, -7,  7,  7,  6    /*tables for pair  11*/
        , -6,  3, -5, -2, -4    /*tables for pair  12*/
        ,  7, -6,  6,  6,  7    /*tables for pair  13*/
        , -7,  4, -1, -3, -5    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*21];
    } _6stayr14 =
        {6,14,7,"6stayr14"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/

        ,  1, -6,  2, -4,  7,  3    /*tables for pair  1*/
        , -1,  5, -3,  6, -4, -2    /*tables for pair  2*/
        ,  2, -1,  3, -7,  5,  6    /*tables for pair  3*/
        , -2,  3, -1,  4, -6, -5    /*tables for pair  4*/
        ,  3, -5,  4, -1,  2,  7    /*tables for pair  5*/
        , -3,  6, -5,  2, -1, -4    /*tables for pair  6*/
        ,  4, -2,  7, -6,  1,  5    /*tables for pair  7*/
        , -4,  1, -6,  5, -2, -3    /*tables for pair  8*/
        ,  7, -4,  5, -3,  6,  2    /*tables for pair  9*/
        , -5,  4, -2,  1, -3, -6    /*tables for pair  10*/
        ,  6, -7,  1, -5,  3,  4    /*tables for pair  11*/
        , -6,  2, -4,  3, -5, -1    /*tables for pair  12*/
        ,  5, -3,  6, -2,  4,  1    /*tables for pair  13*/
        , -7,  7, -7,  7, -7, -7    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*21];
    } _6stayr14_1_5 =
        {6,14,7,"6stayr14.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/

        ,  1, -6,  2, -4,  7,  3    /*tables for pair  1*/
        , -1,  5, -3,  6, -4, -2    /*tables for pair  2*/
        ,  2, -1,  3, -7,  5,  6    /*tables for pair  3*/
        , -2,  3, -1,  4, -6, -5    /*tables for pair  4*/
        ,  3, -5,  4, -1,  2,  7    /*tables for pair  5*/
        , -3,  6, -5,  2, -1, -4    /*tables for pair  6*/
        ,  4, -2,  7, -6,  1,  5    /*tables for pair  7*/
        , -4,  1, -6,  5, -2, -3    /*tables for pair  8*/
        ,  5, -3,  6, -2,  4,  1    /*tables for pair  9*/
        , -5,  4, -2,  1, -3, -6    /*tables for pair  10*/
        ,  6, -7,  1, -5,  3,  4    /*tables for pair  11*/
        , -6,  2, -4,  3, -5, -1    /*tables for pair  12*/
        ,  7, -4,  5, -3,  6,  2    /*tables for pair  13*/
        , -7,  7, -7,  7, -7, -7    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*24];
    } _5stayr16 =
        {5,16,8,"5stayr16"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -4, -4    /*  sets for table 6*/
        , -2, -5, -5, -2, -5    /*  sets for table 7*/
        , -3, -3, -2, -3, -1    /*  sets for table 8*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -6,  7, -4, -8, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -7,  1, -7, -6, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -8,  2, -6, -5, -6    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  8, -8, -1, -7    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  6, -3, -7, -8    /*tables for pair  10*/
        ,  6, -8,  7,  7,  6    /*tables for pair  11*/
        , -1,  3, -5, -2, -4    /*tables for pair  12*/
        ,  7, -6,  6,  8,  7    /*tables for pair  13*/
        , -2,  4, -1, -3, -5    /*tables for pair  14*/
        ,  8, -7,  8,  6,  8    /*tables for pair  15*/
        , -3,  5, -2, -4, -1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*24];
    } _5stayr16_1_x =
        {5,16,8,"5stayr16.1-x"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -4, -4    /*  sets for table 6*/
        , -2, -5, -5, -2, -5    /*  sets for table 7*/
        , -3, -3, -2, -3, -1    /*  sets for table 8*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -1,  7, -4, -8, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  1, -7, -6, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -6, -5, -6    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  8, -8, -1, -7    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  6, -3, -7, -8    /*tables for pair  10*/
        ,  6, -8,  7,  7,  6    /*tables for pair  11*/
        , -6,  3, -5, -2, -4    /*tables for pair  12*/
        ,  7, -6,  6,  8,  7    /*tables for pair  13*/
        , -7,  4, -1, -3, -5    /*tables for pair  14*/
        ,  8, -7,  8,  6,  8    /*tables for pair  15*/
        , -8,  5, -2, -4, -1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*24];
    } _6stayr16 =
        {6,16,8,"6stayr16"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 8*/

        ,  7, -6,  3, -8,  4,  2    /*tables for pair  1*/
        , -1,  2, -3,  6, -5, -4    /*tables for pair  2*/
        ,  8, -1,  6, -7,  5,  3    /*tables for pair  3*/
        , -2,  1, -5,  3, -4, -6    /*tables for pair  4*/
        ,  3, -7,  1, -5,  8,  4    /*tables for pair  5*/
        , -3,  5, -4,  1, -6, -2    /*tables for pair  6*/
        ,  4, -8,  5, -1,  2,  7    /*tables for pair  7*/
        , -4,  6, -1,  2, -3, -5    /*tables for pair  8*/
        ,  5, -4,  7, -2,  6,  8    /*tables for pair  9*/
        , -5,  3, -6,  4, -2, -1    /*tables for pair  10*/
        ,  6, -2,  8, -3,  7,  1    /*tables for pair  11*/
        , -6,  4, -2,  5, -1, -3    /*tables for pair  12*/
        ,  1, -5,  2, -4,  3,  6    /*tables for pair  13*/
        , -7,  7, -7,  7, -7, -7    /*tables for pair  14*/
        ,  2, -3,  4, -6,  1,  5    /*tables for pair  15*/
        , -8,  8, -8,  8, -8, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*24];
    } _6stayr16_1_5 =
        {6,16,8,"6stayr16.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 8*/

        ,  1, -5,  2, -4,  3,  6    /*tables for pair  1*/
        , -1,  2, -3,  6, -5, -4    /*tables for pair  2*/
        ,  2, -3,  4, -6,  1,  5    /*tables for pair  3*/
        , -2,  1, -5,  3, -4, -6    /*tables for pair  4*/
        ,  3, -7,  1, -5,  8,  4    /*tables for pair  5*/
        , -3,  5, -4,  1, -6, -2    /*tables for pair  6*/
        ,  4, -8,  5, -1,  2,  7    /*tables for pair  7*/
        , -4,  6, -1,  2, -3, -5    /*tables for pair  8*/
        ,  5, -4,  7, -2,  6,  8    /*tables for pair  9*/
        , -5,  3, -6,  4, -2, -1    /*tables for pair  10*/
        ,  6, -2,  8, -3,  7,  1    /*tables for pair  11*/
        , -6,  4, -2,  5, -1, -3    /*tables for pair  12*/
        ,  7, -6,  3, -8,  4,  2    /*tables for pair  13*/
        , -7,  7, -7,  7, -7, -7    /*tables for pair  14*/
        ,  8, -1,  6, -7,  5,  3    /*tables for pair  15*/
        , -8,  8, -8,  8, -8, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7stayr16 =
        {7,16,8,"7stayr16"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -8,  7,  6, -5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2,  1, -8, -6, -5, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -1, -7, -6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  4,  3, -2, -1, -7, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6,  5,  4, -3, -8, -1, -7    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -3, -2, -1    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -1,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7stayr16_1_x =
        {7,16,8,"7stayr16.1-x"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -1,  7,  6, -5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2,  1, -8, -6, -5, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -1, -7, -6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  4,  3, -2, -1, -7, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6,  5,  4, -3, -8, -1, -7    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -3, -2, -1    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -8,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*28];
    } _5stayr18 =
        {5,18,10,"5stayr18"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -6, 10, -4, -8, -7    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -7,  6,-10, -9, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -8,  2, -6,-10, -9    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -9,  8, -7, -1,-10    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9, -8, -7, -6    /*tables for pair  10*/
        ,  6, -8, 10,  7,  9    /*tables for pair  11*/
        , -1,  3, -5, -2, -4    /*tables for pair  12*/
        ,  7, -9,  6,  8, 10    /*tables for pair  13*/
        , -2,  4, -1, -3, -5    /*tables for pair  14*/
        ,  8,-10,  7,  9,  6    /*tables for pair  15*/
        , -3,  5, -2, -4, -1    /*tables for pair  16*/
        ,  9, -6,  8, 10,  7    /*tables for pair  17*/
        , -4,  1, -3, -5, -2    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*28];
    } _5stayr18_1_x =
        {5,18,10,"5stayr18.1-x"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -1, -1, -1, -1    /*  sets for table 6*/
        , -2, -2, -2, -2, -2    /*  sets for table 7*/
        , -3, -3, -3, -3, -3    /*  sets for table 8*/
        , -4, -4, -4, -4, -4    /*  sets for table 9*/
        , -5, -5, -5, -5, -5    /*  sets for table 10*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -1,  3, -5, -2, -4    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  4, -1, -3, -5    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  5, -2, -4, -1    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  1, -3, -5, -2    /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9, -8, -7, -6    /*tables for pair  10*/
        ,  6, -8, 10,  7,  9    /*tables for pair  11*/
        , -6, 10, -4, -8, -7    /*tables for pair  12*/
        ,  7, -9,  6,  8, 10    /*tables for pair  13*/
        , -7,  6,-10, -9, -3    /*tables for pair  14*/
        ,  8,-10,  7,  9,  6    /*tables for pair  15*/
        , -8,  2, -6,-10, -9    /*tables for pair  16*/
        ,  9, -6,  8, 10,  7    /*tables for pair  17*/
        , -9,  8, -7, -1,-10    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6stayr18 =
        {6,18,9,"6stayr18"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -1, -4, -4, -4    /*  sets for table 7*/
        , -2, -5, -5, -5, -6, -6    /*  sets for table 8*/
        , -3, -3, -6, -2, -2, -3    /*  sets for table 9*/

        ,  7, -4,  5, -6,  9,  9    /*tables for pair  1*/
        , -1,  6, -5,  3, -2, -7    /*tables for pair  2*/
        ,  2, -3,  9, -7,  1,  5    /*tables for pair  3*/
        , -2,  7, -8,  4, -8, -9    /*tables for pair  4*/
        ,  3, -8,  1, -9,  4,  6    /*tables for pair  5*/
        , -3,  4, -2,  8, -6, -1    /*tables for pair  6*/
        ,  9, -5,  6, -2,  7,  1    /*tables for pair  7*/
        , -4,  2, -6,  5, -1, -3    /*tables for pair  8*/
        ,  5, -9,  4, -1,  2,  8    /*tables for pair  9*/
        , -5,  1, -9,  9, -3, -4    /*tables for pair  10*/
        ,  8, -6,  7, -4,  5,  3    /*tables for pair  11*/
        , -6,  5, -3,  1, -4, -2    /*tables for pair  12*/
        ,  1, -2,  3, -8,  8,  4    /*tables for pair  13*/
        , -7,  3, -4,  2, -5, -6    /*tables for pair  14*/
        ,  6, -7,  2, -5,  3,  7    /*tables for pair  15*/
        , -8,  9, -1,  6, -7, -5    /*tables for pair  16*/
        ,  4, -1,  8, -3,  6,  2    /*tables for pair  17*/
        , -9,  8, -7,  7, -9, -8    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6stayr18_1_5 =
        {6,18,9,"6stayr18.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -1, -4, -4, -4    /*  sets for table 7*/
        , -2, -5, -5, -5, -6, -6    /*  sets for table 8*/
        , -3, -3, -6, -2, -2, -3    /*  sets for table 9*/

        ,  1, -2,  3, -8,  8,  4    /*tables for pair  1*/
        , -1,  6, -5,  3, -2, -7    /*tables for pair  2*/
        ,  2, -3,  9, -7,  1,  5    /*tables for pair  3*/
        , -2,  7, -8,  4, -8, -9    /*tables for pair  4*/
        ,  3, -8,  1, -9,  4,  6    /*tables for pair  5*/
        , -3,  4, -2,  8, -6, -1    /*tables for pair  6*/
        ,  4, -1,  8, -3,  6,  2    /*tables for pair  7*/
        , -4,  2, -6,  5, -1, -3    /*tables for pair  8*/
        ,  5, -9,  4, -1,  2,  8    /*tables for pair  9*/
        , -5,  1, -9,  9, -3, -4    /*tables for pair  10*/
        ,  6, -7,  2, -5,  3,  7    /*tables for pair  11*/
        , -6,  5, -3,  1, -4, -2    /*tables for pair  12*/
        ,  7, -4,  5, -6,  9,  9    /*tables for pair  13*/
        , -7,  3, -4,  2, -5, -6    /*tables for pair  14*/
        ,  8, -6,  7, -4,  5,  3    /*tables for pair  15*/
        , -8,  9, -1,  6, -7, -5    /*tables for pair  16*/
        ,  9, -5,  6, -2,  7,  1    /*tables for pair  17*/
        , -9,  8, -7,  7, -9, -8    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*27];
    } _7stayr18 =
        {7,18,9,"7stayr18"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -8,  7,  9, -5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -9,  1,  7, -6, -5, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2,  1, -8, -6, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -9, -7, -6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3, -2, -1, -7, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6,  5,  4, -3, -8, -1, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -9, -2, -1    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -1,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -2,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*27];
    } _7stayr18_1_x =
        {7,18,9,"7stayr18.1-x"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -1,  7,  9, -5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1,  7, -6, -5, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2,  1, -8, -6, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -9, -7, -6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3, -2, -1, -7, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6,  5,  4, -3, -8, -1, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -9, -2, -1    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -8,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -9,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*30];
    } _6stayr20 = 
        {6,20,10,"6stayr20"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -1, -1, -4, -4    /*  sets for table 7*/
        , -2, -2, -2, -3, -3, -2    /*  sets for table 8*/
        , -3, -3, -5, -5, -5, -5    /*  sets for table 9*/
        , -4, -4, -6, -6, -6, -6    /*  sets for table 10*/

        ,  1, -9,  6,  9,  2, -7    /*tables for pair  1*/
        , -1,  2, -9, -4, -3,  6    /*tables for pair  2*/
        ,  7, -3,  8,  6,  9, -4    /*tables for pair  3*/
        , -2,  1, -5, -6, -4,  3    /*tables for pair  4*/
        ,  3, -5,  4,  1, 10, -8    /*tables for pair  5*/
        , -3, 10, -1, -2, -6,  9    /*tables for pair  6*/
        ,  9, -8,  9,  7,  7,-10    /*tables for pair  7*/
        , -4,  7, -2, -3,-10,  5    /*tables for pair  8*/
        , 10, -6,  5,  3,  1, -2    /*tables for pair  9*/
        , -5,  8, -7,-10, -8,  4    /*tables for pair  10*/
        ,  8,-10, 10,  8,  5, -1    /*tables for pair  11*/
        , -6,  4, -3, -5, -2,  1    /*tables for pair  12*/
        ,  2, -4,  1, 10,  3, -5    /*tables for pair  13*/
        , -7,  5, -6, -8, -7,  2    /*tables for pair  14*/
        ,  6, -1,  2,  4,  8, -9    /*tables for pair  15*/
        , -8,  3, -4, -9, -1, 10    /*tables for pair  16*/
        ,  4, -2,  7,  5,  6, -3    /*tables for pair  17*/
        , -9,  6, -8, -1, -5,  7    /*tables for pair  18*/
        ,  5, -7,  3,  2,  4, -6    /*tables for pair  19*/
        ,-10,  9,-10, -7, -9,  8    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*30];
    } _6stayr20_1_5 = 
        {6,20,10,"6stayr20.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -1, -1, -4, -4    /*  sets for table 7*/
        , -2, -2, -2, -3, -3, -2    /*  sets for table 8*/
        , -3, -3, -5, -5, -5, -5    /*  sets for table 9*/
        , -4, -4, -6, -6, -6, -6    /*  sets for table 10*/

        ,  1, -9,  6,  9,  2, -7    /*tables for pair  1*/
        , -1,  2, -9, -4, -3,  6    /*tables for pair  2*/
        ,  2, -4,  1, 10,  3, -5    /*tables for pair  3*/
        , -2,  1, -5, -6, -4,  3    /*tables for pair  4*/
        ,  3, -5,  4,  1, 10, -8    /*tables for pair  5*/
        , -3, 10, -1, -2, -6,  9    /*tables for pair  6*/
        ,  4, -2,  7,  5,  6, -3    /*tables for pair  7*/
        , -4,  7, -2, -3,-10,  5    /*tables for pair  8*/
        ,  5, -7,  3,  2,  4, -6    /*tables for pair  9*/
        , -5,  8, -7,-10, -8,  4    /*tables for pair  10*/
        ,  6, -1,  2,  4,  8, -9    /*tables for pair  11*/
        , -6,  4, -3, -5, -2,  1    /*tables for pair  12*/
        ,  7, -3,  8,  6,  9, -4    /*tables for pair  13*/
        , -7,  5, -6, -8, -7,  2    /*tables for pair  14*/
        ,  8,-10, 10,  8,  5, -1    /*tables for pair  15*/
        , -8,  3, -4, -9, -1, 10    /*tables for pair  16*/
        ,  9, -8,  9,  7,  7,-10    /*tables for pair  17*/
        , -9,  6, -8, -1, -5,  7    /*tables for pair  18*/
        , 10, -6,  5,  3,  1, -2    /*tables for pair  19*/
        ,-10,  9,-10, -7, -9,  8    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*30];
    } _7stayr20 = 
        {7,20,10,"7stayr20"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/
        , -3, -5, -7, -2, -4, -6, -1    /*  sets for table 10*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -8,  7,  9, -5,-10, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -9,  1, 10, -6, -5, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        ,-10,  2,  1, -8, -6, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -9, -7,-10, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3,-10, -1, -7, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6, 10,  4, -3, -8, -1, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -9, -2,-10    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -1,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -2,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        , 10,-10,-10, 10, 10, 10, 10    /*tables for pair  19*/
        , -3,  5,  7, -2, -4, -6, -1    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*30];
    } _7stayr20_1_x = 
        {7,20,10,"7stayr20.1-x"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/
        , -3, -5, -7, -2, -4, -6, -1    /*  sets for table 10*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -1,  7,  9, -5,-10, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1, 10, -6, -5, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2,  1, -8, -6, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -9, -7,-10, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3,-10, -1, -7, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6, 10,  4, -3, -8, -1, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6,  5, -4, -9, -2,-10    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -8,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -9,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        , 10,-10,-10, 10, 10, 10, 10    /*tables for pair  19*/
        ,-10,  5,  7, -2, -4, -6, -1    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*34];
    } _6stayr22 = 
        {6,22,12,"6stayr22"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -1, -1, -1, -1    /*  sets for table 7*/
        , -2, -2, -2, -2, -2, -2    /*  sets for table 8*/
        , -3, -3, -3, -3, -3, -3    /*  sets for table 9*/
        , -4, -4, -2, -4, -4, -4    /*  sets for table 10*/
        , -5, -5, -5, -5, -5, -5    /*  sets for table 11*/
        , -6, -6, -6, -6, -6, -6    /*  sets for table 12*/

        , 11, -2,  3,-10, 12,  1    /*tables for pair  1*/
        , -1,  9, -4, 11, -6, -2    /*tables for pair  2*/
        ,  7, -8,  4,-12, 11,  9    /*tables for pair  3*/
        , -2,  5, -7,  6, -3,-10    /*tables for pair  4*/
        ,  8, -1,  6, -3, 10, 11    /*tables for pair  5*/
        , -3,  6, -5,  2, -4, -1    /*tables for pair  6*/
        ,  9, -7, 11, -4,  2, 12    /*tables for pair  7*/
        , -4, 12, -1,  5, -2, -3    /*tables for pair  8*/
        , 10, -5, 12, -7,  9,  8    /*tables for pair  9*/
        , -5,  4, -2,  3, -1, -6    /*tables for pair  10*/
        ,  6,-10,  9, -8,  7,  5    /*tables for pair  11*/
        , -6,  3,-10,  1, -5, -4    /*tables for pair  12*/
        ,  2, -4,  5, -1,  6,  3    /*tables for pair  13*/
        , -7,  2, -6,  4, -9, -5    /*tables for pair  14*/
        ,  3,-12, 10,-11,  1, 10    /*tables for pair  15*/
        , -8, 10,-11,  7,-12, -9    /*tables for pair  16*/
        ,  4, -9,  7, -2,  5,  6    /*tables for pair  17*/
        , -9,  8,-12, 10, -7,-11    /*tables for pair  18*/
        ,  5, -3,  1, -6,  4,  2    /*tables for pair  19*/
        ,-10,  1, -3,  8,-11,-12    /*tables for pair  20*/
        ,  1, -6,  2, -5,  3,  4    /*tables for pair  21*/
        ,-11,  7, -9, 12,-10, -8    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*34];
    } _6stayr22_1_5 = 
        {6,22,12,"6stayr22.1-5"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -1, -1, -1, -1, -1    /*  sets for table 7*/
        , -2, -2, -2, -2, -2, -2    /*  sets for table 8*/
        , -3, -3, -3, -3, -3, -3    /*  sets for table 9*/
        , -4, -4, -2, -4, -4, -4    /*  sets for table 10*/
        , -5, -5, -5, -5, -5, -5    /*  sets for table 11*/
        , -6, -6, -6, -6, -6, -6    /*  sets for table 12*/

        ,  1, -6,  2, -5,  3,  4    /*tables for pair  1*/
        , -1,  9, -4, 11, -6, -2    /*tables for pair  2*/
        ,  2, -4,  5, -1,  6,  3    /*tables for pair  3*/
        , -2,  5, -7,  6, -3,-10    /*tables for pair  4*/
        ,  3,-12, 10,-11,  1, 10    /*tables for pair  5*/
        , -3,  6, -5,  2, -4, -1    /*tables for pair  6*/
        ,  4, -9,  7, -2,  5,  6    /*tables for pair  7*/
        , -4, 12, -1,  5, -2, -3    /*tables for pair  8*/
        ,  5, -3,  1, -6,  4,  2    /*tables for pair  9*/
        , -5,  4, -2,  3, -1, -6    /*tables for pair  10*/
        ,  6,-10,  9, -8,  7,  5    /*tables for pair  11*/
        , -6,  3,-10,  1, -5, -4    /*tables for pair  12*/
        ,  7, -8,  4,-12, 11,  9    /*tables for pair  13*/
        , -7,  2, -6,  4, -9, -5    /*tables for pair  14*/
        ,  8, -1,  6, -3, 10, 11    /*tables for pair  15*/
        , -8, 10,-11,  7,-12, -9    /*tables for pair  16*/
        ,  9, -7, 11, -4,  2, 12    /*tables for pair  17*/
        , -9,  8,-12, 10, -7,-11    /*tables for pair  18*/
        , 10, -5, 12, -7,  9,  8    /*tables for pair  19*/
        ,-10,  1, -3,  8,-11,-12    /*tables for pair  20*/
        , 11, -2,  3,-10, 12,  1    /*tables for pair  21*/
        ,-11,  7, -9, 12,-10, -8    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*33];
    } _7stayr22 = 
        {7,22,11,"7stayr22"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/
        , -3, -5, -7, -2, -4, -6, -1    /*  sets for table 10*/
        , -4, -6, -1, -3, -5, -7, -2    /*  sets for table 11*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -8,  7,  9, -5,-10, -3,-11    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -9,  1, 10, -6,-11, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        ,-10,  2, 11, -8, -6, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        ,-11,  8,  2, -9, -7,-10, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3,-10, -1,-11, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6, 10,  4,-11, -8, -1, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7, 11,  5, -4, -9, -2,-10    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -1,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -2,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        , 10,-10,-10, 10, 10, 10, 10    /*tables for pair  19*/
        , -3,  5,  7, -2, -4, -6, -1    /*tables for pair  20*/
        , 11,-11,-11, 11, 11, 11, 11    /*tables for pair  21*/
        , -4,  6,  1, -3, -5, -7, -2    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*33];
    } _7stayr22_1_x = 
        {7,22,11,"7stayr22.1-x"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/
        , -3, -5, -7, -2, -4, -6, -1    /*  sets for table 10*/
        , -4, -6, -1, -3, -5, -7, -2    /*  sets for table 11*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -1,  7,  9, -5,-10, -3,-11    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1, 10, -6,-11, -8, -3    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2, 11, -8, -6, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8,  2, -9, -7,-10, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3,-10, -1,-11, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6, 10,  4,-11, -8, -1, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7, 11,  5, -4, -9, -2,-10    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -8,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -9,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        , 10,-10,-10, 10, 10, 10, 10    /*tables for pair  19*/
        ,-10,  5,  7, -2, -4, -6, -1    /*tables for pair  20*/
        , 11,-11,-11, 11, 11, 11, 11    /*tables for pair  21*/
        ,-11,  6,  1, -3, -5, -7, -2    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*36];
    } _7stayr24 = 
        {7,24,12,"7stayr24"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/
        , -3, -5, -7, -2, -4, -6, -1    /*  sets for table 10*/
        , -4, -6, -1, -3, -5, -7, -2    /*  sets for table 11*/
        , -5, -7, -2, -4, -6, -1, -3    /*  sets for table 12*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -8, 12,  9, -5,-10, -3,-11    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -9,  1, 10, -6,-11, -8,-12    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        ,-10,  2, 11, -8,-12, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        ,-11,  8, 12, -9, -7,-10, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        ,-12,  9,  3,-10, -1,-11, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6, 10,  4,-11, -8,-12, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7, 11,  5,-12, -9, -2,-10    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -1,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -2,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        , 10,-10,-10, 10, 10, 10, 10    /*tables for pair  19*/
        , -3,  5,  7, -2, -4, -6, -1    /*tables for pair  20*/
        , 11,-11,-11, 11, 11, 11, 11    /*tables for pair  21*/
        , -4,  6,  1, -3, -5, -7, -2    /*tables for pair  22*/
        , 12,-12,-12, 12, 12, 12, 12    /*tables for pair  23*/
        , -5,  7,  2, -4, -6, -1, -3    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*36];
    } _7stayr24_1_x = 
        {7,24,12,"7stayr24.1-x"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -3, -5, -7, -2, -4, -6    /*  sets for table 8*/
        , -2, -4, -6, -1, -3, -5, -7    /*  sets for table 9*/
        , -3, -5, -7, -2, -4, -6, -1    /*  sets for table 10*/
        , -4, -6, -1, -3, -5, -7, -2    /*  sets for table 11*/
        , -5, -7, -2, -4, -6, -1, -3    /*  sets for table 12*/

        ,  1, -2, -3,  4,  5,  6,  7    /*tables for pair  1*/
        , -1, 12,  9, -5,-10, -3,-11    /*tables for pair  2*/
        ,  2, -3, -4,  5,  6,  7,  1    /*tables for pair  3*/
        , -2,  1, 10, -6,-11, -8,-12    /*tables for pair  4*/
        ,  3, -4, -8,  6,  7,  1,  2    /*tables for pair  5*/
        , -3,  2, 11, -8,-12, -9, -4    /*tables for pair  6*/
        ,  4, -5, -6,  7,  1,  2,  3    /*tables for pair  7*/
        , -4,  8, 12, -9, -7,-10, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3,-10, -1,-11, -8    /*tables for pair  10*/
        ,  6, -7, -1,  2,  3,  4,  5    /*tables for pair  11*/
        , -6, 10,  4,-11, -8,-12, -9    /*tables for pair  12*/
        ,  7, -1, -2,  3,  4,  5,  6    /*tables for pair  13*/
        , -7, 11,  5,-12, -9, -2,-10    /*tables for pair  14*/
        ,  8, -8, -5,  8,  8,  8,  8    /*tables for pair  15*/
        , -8,  3,  8, -7, -2, -4, -6    /*tables for pair  16*/
        ,  9, -9, -9,  9,  9,  9,  9    /*tables for pair  17*/
        , -9,  4,  6, -1, -3, -5, -7    /*tables for pair  18*/
        , 10,-10,-10, 10, 10, 10, 10    /*tables for pair  19*/
        ,-10,  5,  7, -2, -4, -6, -1    /*tables for pair  20*/
        , 11,-11,-11, 11, 11, 11, 11    /*tables for pair  21*/
        ,-11,  6,  1, -3, -5, -7, -2    /*tables for pair  22*/
        , 12,-12,-12, 12, 12, 12, 12    /*tables for pair  23*/
        ,-12,  7,  2, -4, -6, -1, -3    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*40];
    } _7stayr26 = 
        {7,26,14,"7stayr26"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 8*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 9*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 10*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 11*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 12*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 13*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 14*/

        ,  8, -2, -3, 11, 12, 13, 14    /*tables for pair  1*/
        , -1, 14, 13, -5, -4, -3, -2    /*tables for pair  2*/
        ,  9, -3, -4, 12, 13, 14,  8    /*tables for pair  3*/
        , -2,  8, 14, -6, -5, -4, -3    /*tables for pair  4*/
        , 10, -4, -5,  6, 14,  8,  9    /*tables for pair  5*/
        , -3,  2,  8, -7, -6, -5, -4    /*tables for pair  6*/
        , 11, -5, -6, 14,  1,  9, 10    /*tables for pair  7*/
        , -4, 10,  9, -1, -7, -6, -5    /*tables for pair  8*/
        , 12, -6, -7,  8,  9,  3, 11    /*tables for pair  9*/
        , -5, 11, 10, -2, -1, -7, -6    /*tables for pair  10*/
        , 13, -7, -1,  9, 10, 11,  5    /*tables for pair  11*/
        , -6, 12,  4, -3, -2, -1, -7    /*tables for pair  12*/
        ,  7, -1, -2, 10, 11, 12, 13    /*tables for pair  13*/
        , -7, 13, 12, -4, -3, -2, -1    /*tables for pair  14*/
        ,  1,-10,-12,  7,  2,  4,  6    /*tables for pair  15*/
        , -8,  3,  5,-14, -9,-11,-13    /*tables for pair  16*/
        ,  2,-11,-13,  1,  3,  5,  7    /*tables for pair  17*/
        , -9,  4,  6, -8,-10,-12,-14    /*tables for pair  18*/
        ,  3,-12,-14,  2,  4,  6,  1    /*tables for pair  19*/
        ,-10,  5,  7, -9,-11,-13, -8    /*tables for pair  20*/
        ,  4,-13, -8,  3,  5,  7,  2    /*tables for pair  21*/
        ,-11,  6,  1,-10,-12,-14, -9    /*tables for pair  22*/
        ,  5,-14, -9,  4,  6,  1,  3    /*tables for pair  23*/
        ,-12,  7,  2,-11,-13, -8,-10    /*tables for pair  24*/
        ,  6, -8,-10,  5,  7,  2,  4    /*tables for pair  25*/
        ,-13,  1,  3,-12,-14, -9,-11    /*tables for pair  26*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*40];
    } _7stayr26_1_x = 
        {7,26,14,"7stayr26.1-x"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 8*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 9*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 10*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 11*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 12*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 13*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 14*/

        ,  1, -2, -3, 11, 12, 13, 14    /*tables for pair  1*/
        , -1, 14, 13, -5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4, 12, 13, 14,  8    /*tables for pair  3*/
        , -2,  8, 14, -6, -5, -4, -3    /*tables for pair  4*/
        ,  3, -4, -5,  6, 14,  8,  9    /*tables for pair  5*/
        , -3,  2,  8, -7, -6, -5, -4    /*tables for pair  6*/
        ,  4, -5, -6, 14,  1,  9, 10    /*tables for pair  7*/
        , -4, 10,  9, -1, -7, -6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  8,  9,  3, 11    /*tables for pair  9*/
        , -5, 11, 10, -2, -1, -7, -6    /*tables for pair  10*/
        ,  6, -7, -1,  9, 10, 11,  5    /*tables for pair  11*/
        , -6, 12,  4, -3, -2, -1, -7    /*tables for pair  12*/
        ,  7, -1, -2, 10, 11, 12, 13    /*tables for pair  13*/
        , -7, 13, 12, -4, -3, -2, -1    /*tables for pair  14*/
        ,  8,-10,-12,  7,  2,  4,  6    /*tables for pair  15*/
        , -8,  3,  5,-14, -9,-11,-13    /*tables for pair  16*/
        ,  9,-11,-13,  1,  3,  5,  7    /*tables for pair  17*/
        , -9,  4,  6, -8,-10,-12,-14    /*tables for pair  18*/
        , 10,-12,-14,  2,  4,  6,  1    /*tables for pair  19*/
        ,-10,  5,  7, -9,-11,-13, -8    /*tables for pair  20*/
        , 11,-13, -8,  3,  5,  7,  2    /*tables for pair  21*/
        ,-11,  6,  1,-10,-12,-14, -9    /*tables for pair  22*/
        , 12,-14, -9,  4,  6,  1,  3    /*tables for pair  23*/
        ,-12,  7,  2,-11,-13, -8,-10    /*tables for pair  24*/
        , 13, -8,-10,  5,  7,  2,  4    /*tables for pair  25*/
        ,-13,  1,  3,-12,-14, -9,-11    /*tables for pair  26*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*15];
    } _7txx8 = 
        {7,8,7,"7txx8"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  6,  7,  2,  5,  3,  4    /*tables for pair  1*/
        , -1,  4,  3, -7,  2, -6, -5    /*tables for pair  2*/
        ,  2, -1,  6, -4, -7, -3,  5    /*tables for pair  3*/
        , -2, -4, -7,  6,  1, -5,  3    /*tables for pair  4*/
        ,  3,  5, -6,  7, -1, -2, -4    /*tables for pair  5*/
        , -3, -6,  1,  4, -2,  5, -7    /*tables for pair  6*/
        ,  4, -5, -1, -2,  7,  6, -3    /*tables for pair  7*/
        , -4,  1, -3, -6, -5,  2,  7    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*17];
    } _7txx10 = 
        {7,10,7,"7txx10"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  5, -2, -3,  4, -6,  7    /*tables for pair  1*/
        , -1, -4,  3, -2, -7, -5,  6    /*tables for pair  2*/
        ,  2, -7,  5,  4, -1, -3, -6    /*tables for pair  3*/
        , -2,  1, -3, -6,  5, -7, -4    /*tables for pair  4*/
        ,  3,  7,  6, -1, -4,  5, -2    /*tables for pair  5*/
        , -3,  4, -5,  6, -2,  1, -7    /*tables for pair  6*/
        ,  4, -6, -7,  3, -5, -1,  2    /*tables for pair  7*/
        , -4, -5, -6,  2,  1,  7,  3    /*tables for pair  8*/
        ,  5,  6,  2,  1,  7,  3,  4    /*tables for pair  9*/
        , -5, -1,  7, -4,  2,  6, -3    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*19];
    } _7txx12 = 
        {7,12,7,"7txx12"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  3,  2, -5, -7, -6,  4    /*tables for pair  1*/
        , -1,  5, -6,  7,  4, -3, -2    /*tables for pair  2*/
        ,  2,  1, -3, -6, -4, -7, -5    /*tables for pair  3*/
        , -2, -4, -1,  5, -6,  3,  7    /*tables for pair  4*/
        ,  3, -2, -7,  1,  6,  4,  5    /*tables for pair  5*/
        , -3, -5,  4,  2,  7, -1,  6    /*tables for pair  6*/
        ,  4, -3,  6, -2, -1,  5, -7    /*tables for pair  7*/
        , -4,  2,  1, -3,  5,  7, -6    /*tables for pair  8*/
        ,  5,  7, -4,  3,  1,  6,  2    /*tables for pair  9*/
        , -5,  4,  7,  6,  2,  1,  3    /*tables for pair  10*/
        ,  6, -1, -2, -7, -5, -4, -3    /*tables for pair  11*/
        , -6, -7,  3, -1, -2, -5, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*21];
    } _7txx14 = 
        {7,14,7,"7txx14"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  2,  3,  4, -5, -6, -7    /*tables for pair  1*/
        , -1,  7,  6,  5, -4, -3, -2    /*tables for pair  2*/
        ,  2, -3, -4, -5,  6,  7, -1    /*tables for pair  3*/
        , -2,  1,  7,  6,  5, -4, -3    /*tables for pair  4*/
        ,  3,  4,  5, -6, -7,  1,  2    /*tables for pair  5*/
        , -3, -2,  1, -7, -6, -5,  4    /*tables for pair  6*/
        ,  4, -5, -6,  7, -1, -2,  3    /*tables for pair  7*/
        , -4,  3,  2, -1,  7,  6, -5    /*tables for pair  8*/
        ,  5, -6, -7,  1,  2,  3, -4    /*tables for pair  9*/
        , -5, -4, -3,  2,  1, -7, -6    /*tables for pair  10*/
        ,  6, -7, -1, -2,  3,  4,  5    /*tables for pair  11*/
        , -6,  5,  4,  3, -2, -1,  7    /*tables for pair  12*/
        ,  7, -1, -2, -3,  4,  5,  6    /*tables for pair  13*/
        , -7,  6, -5, -4, -3,  2,  1    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7txx16 = 
        {7,16,8,"7txx16"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/

        ,  1,  2,  3,  4, -5, -6, -7    /*tables for pair  1*/
        , -1, -4, -7,  3,  6,  2,  5    /*tables for pair  2*/
        ,  2,  4, -6, -1, -3, -5, -8    /*tables for pair  3*/
        , -2, -5,  1, -4,  7, -3, -6    /*tables for pair  4*/
        ,  3,  5,  7, -2,  4, -8, -1    /*tables for pair  5*/
        , -3, -6,  2, -5, -1,  4,  7    /*tables for pair  6*/
        ,  4,  6, -1, -3, -8,  7,  2    /*tables for pair  7*/
        , -4, -7, -3,  6, -2,  5,  1    /*tables for pair  8*/
        ,  5,  7, -2,  8, -6, -1, -3    /*tables for pair  9*/
        , -5, -1,  4,  7,  3,  6, -2    /*tables for pair  10*/
        ,  6,  1,  8,  5, -7, -2, -4    /*tables for pair  11*/
        , -6, -2,  5,  1, -4, -7,  3    /*tables for pair  12*/
        ,  7,  8, -4, -6,  1,  3, -5    /*tables for pair  13*/
        , -7, -3,  6,  2,  5,  1,  4    /*tables for pair  14*/
        ,  8,  3, -5, -7,  2, -4,  6    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8,  8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*27];
    } _7txx18 = 
        {7,18,9,"7txx18"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/

        ,  1,  2,  3,  4, -5, -6, -7    /*tables for pair  1*/
        , -1, -4, -7,  3, -6,  2,  5    /*tables for pair  2*/
        ,  2,  3, -4,  5,  6, -7, -1    /*tables for pair  3*/
        , -2, -5, -1, -4, -7,  3,  6    /*tables for pair  4*/
        ,  3,  5,  7, -2,  4, -8, -9    /*tables for pair  5*/
        , -3, -6,  2, -5, -1,  4,  7    /*tables for pair  6*/
        ,  4,  6,  1, -3, -8, -9,  2    /*tables for pair  7*/
        , -4, -7, -3,  6, -2, -5,  1    /*tables for pair  8*/
        ,  5,  7, -2,  8, -9, -1, -3    /*tables for pair  9*/
        , -5, -1,  4,  7, -3,  6, -2    /*tables for pair  10*/
        ,  6,  1,  8, -9,  7, -2, -4    /*tables for pair  11*/
        , -6, -2, -5,  1, -4,  7,  3    /*tables for pair  12*/
        ,  7,  8,  9, -6,  1, -3, -5    /*tables for pair  13*/
        , -7, -3,  6,  2,  5,  1,  4    /*tables for pair  14*/
        ,  8,  9,  5, -7,  2, -4, -6    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8,  8    /*tables for pair  16*/
        ,  9,  4, -6, -1,  3,  5, -8    /*tables for pair  17*/
        , -9, -9, -9,  9,  9,  9,  9    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*12];
    } _7t8 = 
        {7,8,4,"7t8"
        ,  1,  6,  7,  2,  5,  3,  4    /*  sets for table 1*/
        ,  2,  5,  3,  4,  1,  6,  7    /*  sets for table 2*/
        ,  3,  4,  1,  6,  7,  2,  5    /*  sets for table 3*/
        ,  4,  1,  6,  7,  2,  5,  3    /*  sets for table 4*/

        ,  1,  1,  1,  1,  1,  1,  1    /*tables for pair  1*/
        , -1,  3,  2, -4,  4, -2, -3    /*tables for pair  2*/
        ,  2, -4,  4, -2, -3, -1,  3    /*tables for pair  3*/
        , -2, -3, -1,  3,  2, -4,  4    /*tables for pair  4*/
        ,  3,  2, -4,  4, -2, -3, -1    /*tables for pair  5*/
        , -3, -1,  3,  2, -4,  4, -2    /*tables for pair  6*/
        ,  4, -2, -3, -1,  3,  2, -4    /*tables for pair  7*/
        , -4,  4, -2, -3, -1,  3,  2    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*15];
    } _7t10 = 
        {7,10,5,"7t10"
        ,  1,  2,  5,  3,  6,  7,  4    /*  sets for table 1*/
        ,  2,  1,  6,  7,  5,  3,  7    /*  sets for table 2*/
        ,  3,  6,  7,  5,  3,  4,  1    /*  sets for table 3*/
        ,  4,  5,  1,  6,  4,  2,  2    /*  sets for table 4*/
        ,  5,  4,  2,  1,  7,  6,  3    /*  sets for table 5*/

        ,  1,  4, -5, -4, -4, -2,  2    /*tables for pair  1*/
        , -1,  1, -1,  1, -1, -1, -1    /*tables for pair  2*/
        ,  2,  3,  1, -5,  3, -3, -2    /*tables for pair  3*/
        , -2,  2,  2, -2,  2,  2,  1    /*tables for pair  4*/
        ,  3, -1, -2,  3,  5,  3,  3    /*tables for pair  5*/
        , -3, -5,  4,  4, -2,  1,  4    /*tables for pair  6*/
        ,  4, -4, -4,  2,  1, -4,  5    /*tables for pair  7*/
        , -4, -2,  3, -3, -3,  5, -4    /*tables for pair  8*/
        ,  5, -3, -3, -1,  4,  4, -3    /*tables for pair  9*/
        , -5,  5,  5,  5, -5, -5, -5    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*21];
    } _7t14 = 
        {7,14,7,"7t14"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  6, -7, -2, -3, -4, -5    /*tables for pair  1*/
        , -1, -4,  6,  5, -2, -7, -3    /*tables for pair  2*/
        ,  2,  4,  5, -3, -1, -6,  7    /*tables for pair  3*/
        , -2,  3,  1, -6,  5,  7, -4    /*tables for pair  4*/
        ,  3,  2, -5, -1, -7,  4,  6    /*tables for pair  5*/
        , -3, -5, -6, -7, -4, -2, -1    /*tables for pair  6*/
        ,  4, -1,  2,  6,  7,  5,  3    /*tables for pair  7*/
        , -4, -7, -1,  3,  6,  2,  5    /*tables for pair  8*/
        ,  5,  1, -4,  7,  3,  6,  2    /*tables for pair  9*/
        , -5, -2,  7, -4, -6, -3,  1    /*tables for pair  10*/
        ,  6,  7, -3,  1,  2, -5,  4    /*tables for pair  11*/
        , -6, -3, -2, -5,  4,  1, -7    /*tables for pair  12*/
        ,  7,  5,  4,  2,  1,  3, -6    /*tables for pair  13*/
        , -7, -6,  3,  4, -5, -1, -2    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7t16 = 
        {7,16,8,"7t16"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/

        ,  1, -6,  7, -5, -4, -3, -2    /*tables for pair  1*/
        , -1, -7, -8, -8, -6, -2, -5    /*tables for pair  2*/
        ,  2,  6, -5,  7,  3,  4,  1    /*tables for pair  3*/
        , -2,  3,  4, -1,  6,  5, -7    /*tables for pair  4*/
        ,  3,  7,  1, -2,  8, -6,  4    /*tables for pair  5*/
        , -3,  8, -4,  5, -1,  7,  6    /*tables for pair  6*/
        ,  4, -3, -7,  2,  5,  8, -1    /*tables for pair  7*/
        , -4,  1, -2, -6, -3, -7,  5    /*tables for pair  8*/
        ,  5, -1,  3,  4,  2,  6,  7    /*tables for pair  9*/
        , -5, -8, -6,  8,  7, -1,  3    /*tables for pair  10*/
        ,  6, -2,  8, -4, -8,  1,  8    /*tables for pair  11*/
        , -6, -5, -3,  1, -7,  2, -4    /*tables for pair  12*/
        ,  7,  2, -1,  6,  4, -5, -3    /*tables for pair  13*/
        , -7, -4,  5,  3,  1, -8,  2    /*tables for pair  14*/
        ,  8,  4,  2, -7, -5,  3, -6    /*tables for pair  15*/
        , -8,  5,  6, -3, -2, -4, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*27];
    } _7t18 = 
        {7,18,9,"7t18"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/

        ,  1, -7, -2,  4,  5,  6,  3    /*tables for pair  1*/
        , -1,  9,  9,  2, -9, -5, -7    /*tables for pair  2*/
        ,  2,  4, -5, -3,  9,  1,  8    /*tables for pair  3*/
        , -2,  3, -1, -5,  6,  9, -4    /*tables for pair  4*/
        ,  3,  5,  2, -6,  1,  4, -8    /*tables for pair  5*/
        , -3,  8, -9, -7, -8, -8,  9    /*tables for pair  6*/
        ,  4,  6, -7,  1,  8,  2, -3    /*tables for pair  7*/
        , -4,  1, -6,  7, -5,  3,  2    /*tables for pair  8*/
        ,  5, -6,  8, -2, -7, -4,  1    /*tables for pair  9*/
        , -5, -2, -4,  3, -6, -7, -9    /*tables for pair  10*/
        ,  6, -3,  4,  9,  2, -1,  7    /*tables for pair  11*/
        , -6, -8,  5,  8, -3, -9, -1    /*tables for pair  12*/
        ,  7, -5, -3, -1,  4, -6, -2    /*tables for pair  13*/
        , -7, -9,  6, -8, -1, -2, -5    /*tables for pair  14*/
        ,  8,  2,  7,  6,  3,  5,  4    /*tables for pair  15*/
        , -8, -4, -8,  5, -2,  7,  6    /*tables for pair  16*/
        ,  9, -1,  3, -4,  7,  8,  5    /*tables for pair  17*/
        , -9,  7,  1, -9, -4, -3, -6    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*30];
    } _7t20 = 
        {7,20,10,"7t20"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/

        ,  1, -7, -8, 10,  2,  5,  4    /*tables for pair  1*/
        , -1, -9, -5, -4, 10,  2,  6    /*tables for pair  2*/
        ,  2,  6, -1,  7,  4, -5,  3    /*tables for pair  3*/
        , -2, -3, 10,  4,  6,  7, -9    /*tables for pair  4*/
        ,  3, -4, -2,  6,  5,  1,  7    /*tables for pair  5*/
        , -3, -1, -9,  2,-10,  6,  5    /*tables for pair  6*/
        ,  4,  9,  1, -5, -6,  9,  2    /*tables for pair  7*/
        , -4,  7, -3,  9,  1, -6,-10    /*tables for pair  8*/
        ,  5, 10,  3, -7,  9, -2,  9    /*tables for pair  9*/
        , -5,  3,  4,-10,  7,-10, -2    /*tables for pair  10*/
        ,  6,-10, -7,  1, -2,  3, -5    /*tables for pair  11*/
        , -6,  5,  2,  3, -1,  4, -8    /*tables for pair  12*/
        ,  7, -5,  6, -2,  3, -1, -4    /*tables for pair  13*/
        , -7,  8,  9, -3, -5, -8,  1    /*tables for pair  14*/
        ,  8, -6,  5, -8, -7, -3, 10    /*tables for pair  15*/
        , -8, -2, -4,  5, -9, -7, -3    /*tables for pair  16*/
        ,  9,  1,  8,  8,  8,  8,  8    /*tables for pair  17*/
        , -9,  4,  7, -9, -3, 10, -6    /*tables for pair  18*/
        , 10, -8, -6, -1, -8, -4, -7    /*tables for pair  19*/
        ,-10,  2,-10, -6, -4, -9, -1    /*tables for pair  20*/
        };



static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*33];
    } _7t22 = 
        {7,22,11,"7t22"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/

        ,  1,-10,  3,-10,  2, -7, -5    /*tables for pair  1*/
        , -1,  7, -8,  5, -4, -2, -6    /*tables for pair  2*/
        ,  2,  1, -5,  4,  3,  6,  7    /*tables for pair  3*/
        , -2, -3, -9,-11, -6, -5,  1    /*tables for pair  4*/
        ,  3, -7,  4, 10,  8,  1,  2    /*tables for pair  5*/
        , -3, 11,  2, -7, -1,  8,  4    /*tables for pair  6*/
        ,  4, -6,  5, 11, 11, 11, 11    /*tables for pair  7*/
        , -4,-11,  6,  2, -7, -3, -1    /*tables for pair  8*/
        ,  5,  9,  9, -1,  7, -6, 10    /*tables for pair  9*/
        , -5, -8, -1, -8,-10, -8,-11    /*tables for pair  10*/
        ,  6, -5,  8,  7,-11, -4,-10    /*tables for pair  11*/
        , -6,  2, -7,  8, -5, -1, -3    /*tables for pair  12*/
        ,  7, -2,-11, -3,  4, 10,  5    /*tables for pair  13*/
        , -7,  4, -3,  6, -8,  2,  9    /*tables for pair  14*/
        ,  8,  8, 10, -4,  6,  3,  8    /*tables for pair  15*/
        , -8, -9,  7, -9,  9,-11, -4    /*tables for pair  16*/
        ,  9, -4, 11, -5,  1,  7,  3    /*tables for pair  17*/
        , -9,  5, -4,  3, -9, -9, -9    /*tables for pair  18*/
        , 10, 10, -2,  1,  5,  9,  6    /*tables for pair  19*/
        ,-10, -1, -6,  9, 10,  4, -2    /*tables for pair  20*/
        , 11,  6,  1, -2, -3,  5, -8    /*tables for pair  21*/
        ,-11,  3,-10, -6, -2,-10, -7    /*tables for pair  22*/
        };



static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*38];
    } _7t24 = 
        {7,24,14,"7t24"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1,  0,  0, -1, -1, -1, -1    /*  sets for table 8*/
        , -2, -2,  0,  0, -2, -2, -2    /*  sets for table 9*/
        , -3, -3, -3,  0,  0, -3, -3    /*  sets for table 10*/
        , -4, -4, -4, -4,  0,  0, -4    /*  sets for table 11*/
        , -5, -5, -5, -5, -5,  0,  0    /*  sets for table 12*/
        ,  0, -6, -6, -6, -6, -6,  0    /*  sets for table 13*/
        ,  0,  0, -7, -7, -7, -7, -7    /*  sets for table 14*/

        , -5,  2, -6, -4, -3, -1,  7    /*tables for pair  1*/
        , -4, -5, -7, -2, -6, -8,  3    /*tables for pair  2*/
        ,  2,  4,-13,-12,  3, -7, -8    /*tables for pair  3*/
        ,  4, -1,  3,-13,  7, -2, -5    /*tables for pair  4*/
        , 12, -7, 13,  4, -1, -3,  2    /*tables for pair  5*/
        ,  3,  1,  2,-14,  6, -5,  4    /*tables for pair  6*/
        ,  5,  9, 10, -6,  1,  7, 11    /*tables for pair  7*/
        ,  6, 10,  4, 14,  2,  8,  5    /*tables for pair  8*/
        ,  9,  7,  6, -5,  4,-10,  8    /*tables for pair  9*/
        ,  1, 11,  7, 13,  5, -9, 10    /*tables for pair  10*/
        , 11,  3,  1, 12,  9,-13, -7    /*tables for pair  11*/
        , 10,  5, 11, -7,  8, -6,  9    /*tables for pair  12*/
        ,-10, -2,  5, -1, 14, -4,  6    /*tables for pair  13*/
        ,  8,  6, 12, -3, -9,  4, 14    /*tables for pair  14*/
        ,  7, 13,-11,  3, 12,  9,  1    /*tables for pair  15*/
        , -9, -3, -5,-11, 13,-14, -1    /*tables for pair  16*/
        , -8, 12, 14, 11, -2,  6,-10    /*tables for pair  17*/
        ,-11, -9,-14, -8,-12, 10, -6    /*tables for pair  18*/
        , -3, -6, -4,  8, -5, 14, -9    /*tables for pair  19*/
        , -7,-12,-10,  1, -4, 13, -2    /*tables for pair  20*/
        , -1,-10,-12,  7,-13,  2, -4    /*tables for pair  21*/
        , -2,-13, -1,  5,-14,  3,-11    /*tables for pair  22*/
        , -6,-11, -3,  2, -8,  5,-14    /*tables for pair  23*/
        ,-12, -4, -2,  6, -7,  1, -3    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*12];
    } _6t8 =
        {6,8,4,"6t8"
        ,  1,  5,  4,  5,  1,  3    /*  sets for table 1*/
        ,  2,  6,  5,  4,  3,  4    /*  sets for table 2*/
        ,  3,  1,  6,  2,  6,  2    /*  sets for table 3*/
        ,  4,  3,  2,  6,  5,  1    /*  sets for table 4*/

        ,  1,  4, -1,  1, -3,  3    /*tables for pair  1*/
        , -1, -1,  3,  3,  2, -2    /*tables for pair  2*/
        ,  2,  1,  1,  4, -1, -1    /*tables for pair  3*/
        , -2,  2,  2, -2, -2,  4    /*tables for pair  4*/
        ,  3, -3, -3,  2,  4, -3    /*tables for pair  5*/
        , -3, -2,  4, -1,  1,  2    /*tables for pair  6*/
        ,  4,  3, -2, -3,  3,  1    /*tables for pair  7*/
        , -4, -4, -4, -4, -4, -4    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*16];
    } _6t10 =
        {6,10,6,"6t10"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1,  6, -3, -5,  4, -2    /*tables for pair  1*/
        , -1,  2, -6,  4,  5, -3    /*tables for pair  2*/
        ,  2,  1,  3, -4, -6, -5    /*tables for pair  3*/
        , -2,  4,  5, -6, -1,  3    /*tables for pair  4*/
        ,  3, -1, -5, -2, -4, -6    /*tables for pair  5*/
        , -3, -6,  2,  1, -5,  4    /*tables for pair  6*/
        ,  4, -2,  1,  6,  3,  5    /*tables for pair  7*/
        , -4, -3, -2,  5,  1,  6    /*tables for pair  8*/
        ,  5, -4,  6, -1, -3,  2    /*tables for pair  9*/
        , -5,  3, -1,  2,  6, -4    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6t12 =
        {6,12,6,"6t12"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1,  5, -2, -6,  4,  3    /*tables for pair  1*/
        , -1, -6,  4, -5,  3,  2    /*tables for pair  2*/
        ,  2,  6, -1, -3, -4,  5    /*tables for pair  3*/
        , -2, -3, -4,  6, -5,  1    /*tables for pair  4*/
        ,  3, -5, -6, -4,  2, -1    /*tables for pair  5*/
        , -3,  4,  5,  2, -1,  6    /*tables for pair  6*/
        ,  4, -2, -5,  1,  6, -3    /*tables for pair  7*/
        , -4,  1,  6,  3,  5, -2    /*tables for pair  8*/
        ,  5, -1, -3, -2, -6,  4    /*tables for pair  9*/
        , -5,  2,  1,  4, -3, -6    /*tables for pair  10*/
        ,  6, -4,  3, -1, -2, -5    /*tables for pair  11*/
        , -6,  3,  2,  5,  1, -4    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*21];
    } _6t14 =
        {6,14,7,"6t14"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/

        ,  1, -7,  7,  6,  7,  4    /*tables for pair  1*/
        , -1,  4,  3, -5, -6, -2    /*tables for pair  2*/
        ,  2, -6,  5, -1, -4, -3    /*tables for pair  3*/
        , -2,  3,  1, -7,  6, -5    /*tables for pair  4*/
        ,  3,  5, -1,  2,  4,  7    /*tables for pair  5*/
        , -3,  1, -4,  5, -2,  6    /*tables for pair  6*/
        ,  4, -2, -3, -6,  5, -1    /*tables for pair  7*/
        , -4, -1,  6,  3, -7,  2    /*tables for pair  8*/
        ,  5, -4, -6, -2,  3,  1    /*tables for pair  9*/
        , -5,  2, -7,  7, -1, -6    /*tables for pair  10*/
        ,  6, -5,  2,  4,  1,  3    /*tables for pair  11*/
        , -6,  7,  4,  1, -3,  5    /*tables for pair  12*/
        ,  7, -3, -5, -4,  2, -7    /*tables for pair  13*/
        , -7,  6, -2, -3, -5, -4    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*24];
    } _6t16 =
        {6,16,8,"6t16"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 8*/

        ,  1, -2, -6,  3,  4,  5    /*tables for pair  1*/
        , -1, -8,  2, -8,  6,  4    /*tables for pair  2*/
        ,  2,  5, -8, -3, -1, -6    /*tables for pair  3*/
        , -2,  1, -4, -6,  3, -5    /*tables for pair  4*/
        ,  3, -5,  4,  1, -6,  2    /*tables for pair  5*/
        , -3, -1, -2, -5, -4,  6    /*tables for pair  6*/
        ,  4,  2,  3, -1,  7, -7    /*tables for pair  7*/
        , -4,  8,  5,  6, -2, -8    /*tables for pair  8*/
        ,  5, -6,  1, -7,  2,  3    /*tables for pair  9*/
        , -5,  4, -7, -2, -8, -1    /*tables for pair  10*/
        ,  6,  7,  7,  7,  5,  8    /*tables for pair  11*/
        , -6,  3, -1,  2, -7, -4    /*tables for pair  12*/
        ,  7, -3, -5,  4,  8, -2    /*tables for pair  13*/
        , -7, -7,  8,  8, -3,  7    /*tables for pair  14*/
        ,  8,  6, -3, -4, -5,  1    /*tables for pair  15*/
        , -8, -4,  6,  5,  1, -3    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6t18 =
        {6,18,9,"6t18"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 8*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 9*/

        ,  1,  7,  7,  9,  7,  4    /*tables for pair  1*/
        , -1, -9, -9,  3, -8, -2    /*tables for pair  2*/
        ,  2,  6, -4,  8,  1,  3    /*tables for pair  3*/
        , -2,  1,  9, -7, -3, -6    /*tables for pair  4*/
        ,  3,  5, -8, -9,  2, -8    /*tables for pair  5*/
        , -3,  2, -1,  5, -6, -4    /*tables for pair  6*/
        ,  4, -5,  3,  1,  6,  2    /*tables for pair  7*/
        , -4, -2, -6, -3, -7,  8    /*tables for pair  8*/
        ,  5, -1, -7,  6, -4, -9    /*tables for pair  9*/
        , -5,  3,  2,  4, -1, -7    /*tables for pair  10*/
        ,  6,  4, -2, -8,  3, -1    /*tables for pair  11*/
        , -6, -3,  4,  2, -9,  5    /*tables for pair  12*/
        ,  7, -8,  8, -5,  8,  9    /*tables for pair  13*/
        , -7, -7, -3,  7, -5,  7    /*tables for pair  14*/
        ,  8,  8, -5, -4,  9,  6    /*tables for pair  15*/
        , -8,  9,  1, -6,  5, -3    /*tables for pair  16*/
        ,  9, -6,  5, -2,  4,  1    /*tables for pair  17*/
        , -9, -4,  6, -1, -2, -5    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*12];
    } _6multi08_nieuw =
        {6,8,4,"6multi08_nieuw"
        ,  1,  1,  5,  5,  5,  5    /*  sets for table 1*/
        ,  2,  2,  6,  6,  6,  6    /*  sets for table 2*/
        ,  3,  3,  3,  3,  1,  1    /*  sets for table 3*/
        ,  4,  4,  4,  4,  2,  2    /*  sets for table 4*/

        ,  1,  4, -1, -3, -2,  4    /*tables for pair  1*/
        , -1, -3,  4, -2, -4,  1    /*tables for pair  2*/
        ,  2,  3, -2,  4,  1, -3    /*tables for pair  3*/
        , -2, -4,  3, -1,  3, -2    /*tables for pair  4*/
        ,  3, -2, -4,  1,  2,  3    /*tables for pair  5*/
        , -3,  1,  1, -4,  4,  2    /*tables for pair  6*/
        ,  4, -1, -3,  2, -1, -4    /*tables for pair  7*/
        , -4,  2,  2,  3, -3, -1    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*16];
    } _6multi10_nieuw =
        {6,10,6,"6multi10_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1,  6, -2,  4, -3, -5    /*tables for pair  1*/
        , -1,  4, -3, -5, -6, -2    /*tables for pair  2*/
        ,  2, -6,  3, -1,  5,  4    /*tables for pair  3*/
        , -2,  1, -6,  5,  4, -3    /*tables for pair  4*/
        ,  3, -4,  6, -2,  1,  5    /*tables for pair  5*/
        , -3,  2, -5,  1, -4,  6    /*tables for pair  6*/
        ,  4, -1,  5, -6,  3,  2    /*tables for pair  7*/
        , -4, -3, -1,  2, -5, -6    /*tables for pair  8*/
        ,  5, -2,  1, -4,  6,  3    /*tables for pair  9*/
        , -5,  3,  2,  6, -1, -4    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*18];
    } _6multi12_nieuw =
        {6,12,6,"6multi12_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/

        ,  1,  2,  4,  6,  3, -5    /*tables for pair  1*/
        , -1,  3,  5, -2, -6,  4    /*tables for pair  2*/
        ,  2, -4, -5,  3,  1, -6    /*tables for pair  3*/
        , -2, -5,  6,  4, -3,  1    /*tables for pair  4*/
        ,  3,  5,  1, -6, -2, -4    /*tables for pair  5*/
        , -3,  1,  2,  5, -4,  6    /*tables for pair  6*/
        ,  4, -2,  3, -5,  6, -1    /*tables for pair  7*/
        , -4, -6, -1, -3, -5,  2    /*tables for pair  8*/
        ,  5, -3, -6,  1,  4, -2    /*tables for pair  9*/
        , -5,  6, -2, -4, -1, -3    /*tables for pair  10*/
        ,  6, -1, -4,  2,  5,  3    /*tables for pair  11*/
        , -6,  4, -3, -1,  2,  5    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*21];
    } _6multi14_nieuw =
        {6,14,7,"6multi14_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/

        ,  1,  2,  4,  6,  3, -5    /*tables for pair  1*/
        , -1,  3,  5, -2, -6,  4    /*tables for pair  2*/
        ,  2, -4, -5,  3,  1, -7    /*tables for pair  3*/
        , -2, -5,  6,  4, -3,  1    /*tables for pair  4*/
        ,  3,  5,  1, -6, -2, -4    /*tables for pair  5*/
        , -3,  1,  2,  5, -4,  6    /*tables for pair  6*/
        ,  4, -7,  3, -5,  6, -1    /*tables for pair  7*/
        , -4, -6, -1, -3, -7,  2    /*tables for pair  8*/
        ,  5, -3, -6,  1,  4, -2    /*tables for pair  9*/
        , -5,  6, -2, -7, -1, -3    /*tables for pair  10*/
        ,  6, -1, -4,  2,  5,  3    /*tables for pair  11*/
        , -6,  4, -7, -1,  2,  5    /*tables for pair  12*/
        ,  7,  7,  7,  7,  7,  7    /*tables for pair  13*/
        , -7, -2, -3, -4, -5, -6    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*24];
    } _6multi16_nieuw =
        {6,16,8,"6multi16_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/

        ,  1, -2, -3,  4, -5,  6    /*tables for pair  1*/
        , -1,  6, -4,  3, -7,  2    /*tables for pair  2*/
        ,  2, -6,  5,  8,  4,  3    /*tables for pair  3*/
        , -2, -1, -7,  6,  5,  4    /*tables for pair  4*/
        ,  3,  8, -2, -1,  6, -4    /*tables for pair  5*/
        , -3,  2,  1,  5, -4,  7    /*tables for pair  6*/
        ,  4, -3, -1, -6, -8, -5    /*tables for pair  7*/
        , -4, -5,  6,  1, -2, -3    /*tables for pair  8*/
        ,  5, -4,  8, -3,  2,  1    /*tables for pair  9*/
        , -5,  3,  2,  7, -1, -6    /*tables for pair  10*/
        ,  6,  5,  4,  2,  1, -8    /*tables for pair  11*/
        , -6,  7, -5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7,  7, -7,  7, -7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6,  5    /*tables for pair  14*/
        ,  8,  1, -6, -5,  3, -2    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*27];
    } _6multi18_nieuw =
        {6,18,9,"6multi18_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/

        ,  1, -2, -3,  4, -5,  6    /*tables for pair  1*/
        , -1,  6, -4,  3, -7,  2    /*tables for pair  2*/
        ,  2, -6,  5,  8,  4,  3    /*tables for pair  3*/
        , -2, -1, -7,  6,  5,  4    /*tables for pair  4*/
        ,  3,  8, -2, -1,  9, -4    /*tables for pair  5*/
        , -3,  2,  1,  5, -4,  7    /*tables for pair  6*/
        ,  4, -9, -1, -6, -8, -5    /*tables for pair  7*/
        , -4, -5,  6,  1, -2, -3    /*tables for pair  8*/
        ,  5, -4,  8, -3,  2,  9    /*tables for pair  9*/
        , -5,  3,  2,  7, -1, -6    /*tables for pair  10*/
        ,  6,  5,  9,  2,  1, -8    /*tables for pair  11*/
        , -6,  7, -5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7,  7, -7,  7, -7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6,  5    /*tables for pair  14*/
        ,  8,  1, -6, -9,  3, -2    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8    /*tables for pair  16*/
        ,  9, -3,  4, -5,  6,  1    /*tables for pair  17*/
        , -9,  9, -9,  9, -9, -9    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*30];
    } _6multi20_nieuw =
        {6,20,10,"6multi20_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 10*/

        ,  1, -2, -3,  4, -5,  6    /*tables for pair  1*/
        , -1,  6, -4,  3, -7,  2    /*tables for pair  2*/
        ,  2, -6, 10,  8,  4,  3    /*tables for pair  3*/
        , -2, -1, -7,  6,  5,  4    /*tables for pair  4*/
        ,  3,  8, -2, -1,  9, -4    /*tables for pair  5*/
        , -3,  2,  1,  5, -4,  7    /*tables for pair  6*/
        ,  4, -9, -1,-10, -8, -5    /*tables for pair  7*/
        , -4, -5,  6,  1, -2, -3    /*tables for pair  8*/
        ,  5,-10,  8, -3,  2,  9    /*tables for pair  9*/
        , -5,  3,  2,  7, -1, -6    /*tables for pair  10*/
        ,  6,  5,  9,  2, 10, -8    /*tables for pair  11*/
        , -6,  7, -5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7,  7, -7,  7, -7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6,  5    /*tables for pair  14*/
        ,  8,  1, -6, -9,  3,-10    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8    /*tables for pair  16*/
        ,  9, -3,  4, -5,  6,  1    /*tables for pair  17*/
        , -9,  9, -9,  9, -9, -9    /*tables for pair  18*/
        , 10, -4,  5, -6,  1, -2    /*tables for pair  19*/
        ,-10, 10,-10, 10,-10, 10    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*33];
    } _6multi22_nieuw =
        {6,22,11,"6multi22_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 10*/
        , -5, -6, -1, -2, -3, -4    /*  sets for table 11*/

        ,  1, -2, -3,  4, -5,  6    /*tables for pair  1*/
        , -1,  6, -4,  3, -7,  2    /*tables for pair  2*/
        ,  2,-11, 10,  8,  4,  3    /*tables for pair  3*/
        , -2, -1, -7,  6,  5,  4    /*tables for pair  4*/
        ,  3,  8, -2, -1,  9,-11    /*tables for pair  5*/
        , -3,  2,  1,  5, -4,  7    /*tables for pair  6*/
        ,  4, -9,-11,-10, -8, -5    /*tables for pair  7*/
        , -4, -5,  6,  1, -2, -3    /*tables for pair  8*/
        ,  5,-10,  8, -3,  2,  9    /*tables for pair  9*/
        , -5,  3,  2,  7, -1, -6    /*tables for pair  10*/
        ,  6,  5,  9, 11, 10, -8    /*tables for pair  11*/
        , -6,  7, -5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7,  7, -7,  7, -7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6,  5    /*tables for pair  14*/
        ,  8,  1, -6, -9, 11,-10    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8    /*tables for pair  16*/
        ,  9, -3,  4, -5,  6,  1    /*tables for pair  17*/
        , -9,  9, -9,  9, -9, -9    /*tables for pair  18*/
        , 10, -4,  5, -6,  1, -2    /*tables for pair  19*/
        ,-10, 10,-10, 10,-10, 10    /*tables for pair  20*/
        , 11, -6, -1,  2,  3, -4    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11, 11    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[6*36];
    } _6multi24_nieuw =
        {6,24,12,"6multi24_nieuw"
        ,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        , -1, -2, -3, -4, -5, -6    /*  sets for table 7*/
        , -4, -5, -6, -1, -2, -3    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -1, -2    /*  sets for table 10*/
        , -5, -6, -1, -2, -3, -4    /*  sets for table 11*/
        , -6, -1, -2, -3, -4, -5    /*  sets for table 12*/

        ,  1, -2, -3,  4, -5,  6    /*tables for pair  1*/
        , -1,  6, -4,  3, -7,  2    /*tables for pair  2*/
        ,  2,-11, 10,  8, 12,  3    /*tables for pair  3*/
        , -2, -1, -7,  6,  5,  4    /*tables for pair  4*/
        ,  3,  8,-12, -1,  9,-11    /*tables for pair  5*/
        , -3,  2,  1,  5, -4,  7    /*tables for pair  6*/
        ,  4, -9,-11,-10, -8,-12    /*tables for pair  7*/
        , -4, -5,  6,  1, -2, -3    /*tables for pair  8*/
        ,  5,-10,  8,-12,  2,  9    /*tables for pair  9*/
        , -5,  3,  2,  7, -1, -6    /*tables for pair  10*/
        ,  6,  5,  9, 11, 10, -8    /*tables for pair  11*/
        , -6,  7, -5, -4, -3, -1    /*tables for pair  12*/
        ,  7, -7,  7, -7,  7, -7    /*tables for pair  13*/
        , -7,  4,  3, -2, -6,  5    /*tables for pair  14*/
        ,  8, 12, -6, -9, 11,-10    /*tables for pair  15*/
        , -8, -8, -8, -8,  8,  8    /*tables for pair  16*/
        ,  9, -3,  4, -5,  6,  1    /*tables for pair  17*/
        , -9,  9, -9,  9, -9, -9    /*tables for pair  18*/
        , 10, -4,  5, -6,  1, -2    /*tables for pair  19*/
        ,-10, 10,-10, 10,-10, 10    /*tables for pair  20*/
        , 11, -6, -1,  2,  3, -4    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11, 11    /*tables for pair  22*/
        , 12,  1, -2, -3,  4, -5    /*tables for pair  23*/
        ,-12,-12, 12, 12,-12, 12    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*12];
    } _7multi08_nieuw =
        {7,8,4,"7multi08_nieuw"
        ,  1,  7,  6,  5,  2,  3,  4    /*  sets for table 1*/
        ,  2,  6,  1,  7,  4,  5,  3    /*  sets for table 2*/
        ,  3,  1,  5,  1,  6,  2,  7    /*  sets for table 3*/
        ,  4,  3,  4,  2,  7,  6,  5    /*  sets for table 4*/

        ,  1,  1,  1,  1,  1,  1,  1    /*tables for pair  1*/
        , -1,  4,  4,  4, -4, -4, -4    /*tables for pair  2*/
        ,  2,  2, -2, -2, -2, -1,  4    /*tables for pair  3*/
        , -2, -1, -4,  3,  3, -2,  2    /*tables for pair  4*/
        ,  3, -2,  3, -3,  4, -3, -1    /*tables for pair  5*/
        , -3,  3, -1, -4,  2,  2, -3    /*tables for pair  6*/
        ,  4, -3, -3,  2, -1,  4, -2    /*tables for pair  7*/
        , -4, -4,  2, -1, -3,  3,  3    /*tables for pair  8*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*15];
    } _7multi10_nieuw =
        {7,10,5,"7multi10_nieuw"
        ,  1,  2,  3,  4,  5,  6,  7    /*  sets for table 1*/
        ,  2,  3,  4,  5,  6,  7,  1    /*  sets for table 2*/
        ,  3,  4,  5,  6,  7,  1,  2    /*  sets for table 3*/
        ,  4,  5,  6,  7,  1,  2,  3    /*  sets for table 4*/
        ,  5,  6,  7,  1,  2,  3,  4    /*  sets for table 5*/

        ,  1, -2,  3,  4, -2,  4,  5    /*tables for pair  1*/
        , -1,  1,  1, -1, -1, -1, -1    /*tables for pair  2*/
        ,  2, -3,  4, -2,  4,  5,  1    /*tables for pair  3*/
        , -2, -4, -5,  1,  2,  3, -4    /*tables for pair  4*/
        ,  3,  4, -2, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  3, -3, -3, -3, -3,  3    /*tables for pair  6*/
        ,  4,  5, -1,  2,  3, -4, -2    /*tables for pair  7*/
        , -4,  2, -4,  5,  1,  2, -3    /*tables for pair  8*/
        ,  5, -1,  2,  3, -4, -2,  4    /*tables for pair  9*/
        , -5, -5,  5, -5, -5, -5, -5    /*tables for pair  10*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*19];
    } _7multi12_nieuw =
        {7,12,7,"7multi12_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1,  5,  6,  7,  4,  3,  2    /*tables for pair  1*/
        , -1,  2,  7,  3, -6,  5, -4    /*tables for pair  2*/
        ,  2, -7,  1, -6,  5, -3,  4    /*tables for pair  3*/
        , -2,  3, -7, -1, -4, -6,  5    /*tables for pair  4*/
        ,  3, -1,  4,  6, -7, -5, -2    /*tables for pair  5*/
        , -3,  4, -1,  2,  6, -7, -5    /*tables for pair  6*/
        ,  4, -2, -3,  5,  1,  7, -6    /*tables for pair  7*/
        , -4,  1, -2, -3, -5,  6,  7    /*tables for pair  8*/
        ,  5, -3, -4, -7,  2, -1,  6    /*tables for pair  9*/
        , -5,  7, -6, -2, -1,  4, -3    /*tables for pair  10*/
        ,  6, -5,  3,  1, -2, -4, -7    /*tables for pair  11*/
        , -6, -4,  2, -5,  7,  1,  3    /*tables for pair  12*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*21];
    } _7multi14_nieuw =
        {7,14,7,"7multi14_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/

        ,  1, -3, -5,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2, -4, -6,  1,  3,  5,  7    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  6,  1    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  5,  7,  2    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  4,  6,  1,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -3,  5,  7,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -2, -4,  6,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*24];
    } _7multi16_nieuw =
        {7,16,8,"7multi16_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/

        ,  1, -3, -5,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2, -4, -6,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  8,  1    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  7,  2    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  6,  1,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  5,  7,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -4,  6,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*27];
    } _7multi18_nieuw =
        {7,18,9,"7multi18_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/

        ,  1, -9, -5,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2, -4, -6,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  9,  2    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9,  1,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9,  7,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9,  6,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*30];
    } _7multi20_nieuw =
        {7,20,10,"7multi20_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/

        ,  1, -9,-10,  7,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10, -6,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3, -5, -7,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9, 10,  3    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10,  2,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10,  1,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*33];
    } _7multi22_nieuw =
        {7,22,11,"7multi22_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/

        ,  1, -9,-10, 11,  2,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11,  1,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11, -7,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4, -6, -1,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10, 11,  4    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11,  3,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*36];
    } _7multi24_nieuw =
        {7,24,12,"7multi24_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/
        , -5, -6, -7, -1, -2, -3, -4    /*  sets for table 12*/

        ,  1, -9,-10, 11, 12,  4,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11, 12,  3,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11,-12,  2,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4,-12, -1,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5, -7, -2,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10, 11, 12    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11, 12,  5    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        , 12, -6, -7,  1,  2,  3,  4    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12,-12    /*tables for pair  24*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*39];
    } _7multi26_nieuw =
        {7,26,13,"7multi26_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/
        , -5, -6, -7, -1, -2, -3, -4    /*  sets for table 12*/
        , -6, -7, -1, -2, -3, -4, -5    /*  sets for table 13*/

        ,  1, -9,-10, 11, 12, 13,  6    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11, 12, 13,  5,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11,-12, 13,  4,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4,-12,-13,  3,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5,-13, -2,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6, -1, -8,  9, 10, 11, 12    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11, 12, 13    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        , 12, -6, -7,  1,  2,  3,  4    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12,-12    /*tables for pair  24*/
        , 13, -7, -1,  2,  3,  4,  5    /*tables for pair  25*/
        ,-13, 13, 13,-13,-13,-13,-13    /*tables for pair  26*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[7*42];
    } _7multi28_nieuw =
        {7,28,14,"7multi28_nieuw"
        ,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        , -1, -2, -3, -4, -5, -6, -7    /*  sets for table 8*/
        , -2, -3, -4, -5, -6, -7, -1    /*  sets for table 9*/
        , -3, -4, -5, -6, -7, -1, -2    /*  sets for table 10*/
        , -4, -5, -6, -7, -1, -2, -3    /*  sets for table 11*/
        , -5, -6, -7, -1, -2, -3, -4    /*  sets for table 12*/
        , -6, -7, -1, -2, -3, -4, -5    /*  sets for table 13*/
        , -7, -1, -2, -3, -4, -5, -6    /*  sets for table 14*/

        ,  1, -9,-10, 11, 12, 13, 14    /*tables for pair  1*/
        , -1,  4,  7, -3, -6, -2, -5    /*tables for pair  2*/
        ,  2,-10,-11, 12, 13, 14,  8    /*tables for pair  3*/
        , -2,  5,  1, -4, -7, -3, -6    /*tables for pair  4*/
        ,  3,-11,-12, 13, 14,  8,  9    /*tables for pair  5*/
        , -3,  6,  2, -5, -1, -4, -7    /*tables for pair  6*/
        ,  4,-12,-13, 14,  8,  9, 10    /*tables for pair  7*/
        , -4,  7,  3, -6, -2, -5, -1    /*tables for pair  8*/
        ,  5,-13,-14,  8,  9, 10, 11    /*tables for pair  9*/
        , -5,  1,  4, -7, -3, -6, -2    /*tables for pair  10*/
        ,  6,-14, -8,  9, 10, 11, 12    /*tables for pair  11*/
        , -6,  2,  5, -1, -4, -7, -3    /*tables for pair  12*/
        ,  7, -8, -9, 10, 11, 12, 13    /*tables for pair  13*/
        , -7,  3,  6, -2, -5, -1, -4    /*tables for pair  14*/
        ,  8, -2, -3,  4,  5,  6,  7    /*tables for pair  15*/
        , -8,  8,  8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9, -3, -4,  5,  6,  7,  1    /*tables for pair  17*/
        , -9,  9,  9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10, -4, -5,  6,  7,  1,  2    /*tables for pair  19*/
        ,-10, 10, 10,-10,-10,-10,-10    /*tables for pair  20*/
        , 11, -5, -6,  7,  1,  2,  3    /*tables for pair  21*/
        ,-11, 11, 11,-11,-11,-11,-11    /*tables for pair  22*/
        , 12, -6, -7,  1,  2,  3,  4    /*tables for pair  23*/
        ,-12, 12, 12,-12,-12,-12,-12    /*tables for pair  24*/
        , 13, -7, -1,  2,  3,  4,  5    /*tables for pair  25*/
        ,-13, 13, 13,-13,-13,-13,-13    /*tables for pair  26*/
        , 14, -1, -2,  3,  4,  5,  6    /*tables for pair  27*/
        ,-14, 14, 14,-14,-14,-14,-14    /*tables for pair  28*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[8*24];
    } _8sche_a16 =
        {8,16,8,"8sche_a16"
        ,  1,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        ,  8,  8,  8,  8,  8,  8,  8,  8    /*  sets for table 8*/

        ,  1,  4,  6,  7,  5, -8,  2, -3    /*tables for pair  1*/
        , -1, -8, -4, -5, -2,  7, -3,  6    /*tables for pair  2*/
        ,  5,  8,  2,  3,  1, -4,  6, -7    /*tables for pair  3*/
        , -5, -4, -8, -1, -6,  3, -7,  2    /*tables for pair  4*/
        ,  2,  3,  5,  8,  6, -7,  1, -4    /*tables for pair  5*/
        , -2, -7, -3, -6, -1,  8, -4,  5    /*tables for pair  6*/
        ,  6,  7,  1,  4,  2, -3,  5, -8    /*tables for pair  7*/
        , -6, -3, -7, -2, -5,  4, -8,  1    /*tables for pair  8*/
        ,  3,  2,  8,  5,  7, -6,  4, -1    /*tables for pair  9*/
        , -3, -6, -2, -7, -4,  5, -1,  8    /*tables for pair 10*/
        ,  7,  6,  4,  1,  3, -2,  8, -5    /*tables for pair 11*/
        , -7, -2, -6, -3, -8,  1, -5,  4    /*tables for pair 12*/
        ,  4,  1,  7,  6,  8, -5,  3, -2    /*tables for pair 13*/
        , -4, -5, -1, -8, -3,  6, -2,  7    /*tables for pair 14*/
        ,  8,  5,  3,  2,  4, -1,  7, -6    /*tables for pair 15*/
        , -8, -1, -5, -4, -7,  2, -6,  3    /*tables for pair 16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[8*24];
    } _8sche_bc16 =
        {8,16,8,"8sche_bc16"
        ,  1,  1,  1,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6,  6    /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7,  7    /*  sets for table 7*/
        ,  8,  8,  8,  8,  8,  8,  8,  8    /*  sets for table 8*/

        ,  1,  7,  4,  6,  2, -8,  3, -5    /*tables for pair  1*/
        , -1, -8, -7, -2, -3,  6, -5,  4    /*tables for pair  2*/
        ,  2,  8,  3,  5,  1, -7,  4, -6    /*tables for pair  3*/
        , -2, -7, -8, -1, -4,  5, -6,  3    /*tables for pair  4*/
        ,  3,  5,  2,  8,  4, -6,  1, -7    /*tables for pair  5*/
        , -3, -6, -5, -4, -1,  8, -7,  2    /*tables for pair  6*/
        ,  4,  6,  1,  7,  3, -5,  2, -8    /*tables for pair  7*/
        , -4, -5, -6, -3, -2,  7, -8,  1    /*tables for pair  8*/
        ,  5,  3,  8,  2,  6, -4,  7, -1    /*tables for pair  9*/
        , -5, -4, -3, -6, -7,  2, -1,  8    /*tables for pair 10*/
        ,  6,  4,  7,  1,  5, -3,  8, -2    /*tables for pair 11*/
        , -6, -3, -4, -5, -8,  1, -2,  7    /*tables for pair 12*/
        ,  7,  1,  6,  4,  8, -2,  5, -3    /*tables for pair 13*/
        , -7, -2, -1, -8, -5,  4, -3,  6    /*tables for pair 14*/
        ,  8,  2,  5,  3,  7, -1,  6, -4    /*tables for pair 15*/
        , -8, -1, -2, -7, -6,  3, -4,  5    /*tables for pair 16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*22];
    } _4mitchel14h =
        {4,14,8,"4mitchel14_horne"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -1, -1, -1    /*  sets for table 5*/
        , -2, -2, -2, -2    /*  sets for table 6*/
        , -3, -3, -3, -3    /*  sets for table 7*/
        , -4, -4, -4, -4    /*  sets for table 8*/

        ,  1, -6,  3, -8    /*tables for pair  1*/
        , -1,  3, -4,  2    /*tables for pair  2*/
        ,  2, -1,  8, -7    /*tables for pair  3*/
        , -2,  4, -3,  1    /*tables for pair  4*/
        ,  3, -8,  5, -2    /*tables for pair  5*/
        , -3,  1, -2,  4    /*tables for pair  6*/
        ,  4, -7,  6, -5    /*tables for pair  7*/
        , -4,  2, -1,  3    /*tables for pair  8*/
        ,  5, -4,  2, -3    /*tables for pair  9*/
        , -5,  8, -6,  7    /*tables for pair  10*/
        ,  6, -3,  1, -4    /*tables for pair  11*/
        , -6,  7, -5,  8    /*tables for pair  12*/
        ,  7, -2,  4, -1    /*tables for pair  13*/
        , -7,  6, -8,  5    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[4*24];
    } _4mitchel16h =
        {4,16,8,"4mitchel16_horne"
        ,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4    /*  sets for table 4*/
        , -1, -1, -1, -1    /*  sets for table 5*/
        , -2, -2, -2, -2    /*  sets for table 6*/
        , -3, -3, -3, -3    /*  sets for table 7*/
        , -4, -4, -4, -4    /*  sets for table 8*/

        ,  1, -6,  7, -8    /*tables for pair  1*/
        , -1,  3, -4,  2    /*tables for pair  2*/
        ,  2, -5,  8, -7    /*tables for pair  3*/
        , -2,  4, -3,  1    /*tables for pair  4*/
        ,  3, -8,  5, -6    /*tables for pair  5*/
        , -3,  1, -2,  4    /*tables for pair  6*/
        ,  4, -7,  6, -5    /*tables for pair  7*/
        , -4,  2, -1,  3    /*tables for pair  8*/
        ,  5, -4,  2, -3    /*tables for pair  9*/
        , -5,  8, -6,  7    /*tables for pair  10*/
        ,  6, -3,  1, -4    /*tables for pair  11*/
        , -6,  7, -5,  8    /*tables for pair  12*/
        ,  7, -2,  4, -1    /*tables for pair  13*/
        , -7,  6, -8,  5    /*tables for pair  14*/
        ,  8, -1,  3, -2    /*tables for pair  15*/
        , -8,  5, -7,  6    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x_bl =
        {5,14,7,"5stayr14.1-x_bl"   // R3: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2, -3,  4,  5    /*tables for pair  1*/
        , -1,  5,  4, -6, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  1    /*tables for pair  3*/
        , -2,  1,  7, -4, -3    /*tables for pair  4*/
        ,  3, -4, -5,  1,  2    /*tables for pair  5*/
        , -3,  2,  6, -5, -6    /*tables for pair  6*/
        ,  4, -5, -1,  2,  3    /*tables for pair  7*/
        , -4,  7,  2, -1, -7    /*tables for pair  8*/
        ,  5, -1, -2,  3,  4    /*tables for pair  9*/
        , -5,  6,  3, -7, -1    /*tables for pair  10*/
        ,  6, -7, -7,  7,  6    /*tables for pair  11*/
        , -6,  3,  5, -2, -4    /*tables for pair  12*/
        ,  7, -6, -6,  6,  7    /*tables for pair  13*/
        , -7,  4,  1, -3, -5    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x_br =
        {5,14,7,"5stayr14.1-x_br"   // R4: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2,  3, -4,  5    /*tables for pair  1*/
        , -1,  5, -4,  6, -2    /*tables for pair  2*/
        ,  2, -3,  4, -5,  1    /*tables for pair  3*/
        , -2,  1, -7,  4, -3    /*tables for pair  4*/
        ,  3, -4,  5, -1,  2    /*tables for pair  5*/
        , -3,  2, -6,  5, -6    /*tables for pair  6*/
        ,  4, -5,  1, -2,  3    /*tables for pair  7*/
        , -4,  7, -2,  1, -7    /*tables for pair  8*/
        ,  5, -1,  2, -3,  4    /*tables for pair  9*/
        , -5,  6, -3,  7, -1    /*tables for pair  10*/
        ,  6, -7,  7, -7,  6    /*tables for pair  11*/
        , -6,  3, -5,  2, -4    /*tables for pair  12*/
        ,  7, -6,  6, -6,  7    /*tables for pair  13*/
        , -7,  4, -1,  3, -5    /*tables for pair  14*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x_ge =
        {5,14,7,"5stayr14.1-x_ge"   // R5: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2,  3,  4, -5    /*tables for pair  1*/
        , -1,  5, -4, -6,  2    /*tables for pair  2*/
        ,  2, -3,  4,  5, -1    /*tables for pair  3*/
        , -2,  1, -7, -4,  3    /*tables for pair  4*/
        ,  3, -4,  5,  1, -2    /*tables for pair  5*/
        , -3,  2, -6, -5,  6    /*tables for pair  6*/
        ,  4, -5,  1,  2, -3    /*tables for pair  7*/
        , -4,  7, -2, -1,  7    /*tables for pair  8*/
        ,  5, -1,  2,  3, -4    /*tables for pair  9*/
        , -5,  6, -3, -7,  1    /*tables for pair  10*/
        ,  6, -7,  7,  7, -6    /*tables for pair  11*/
        , -6,  3, -5, -2,  4    /*tables for pair  12*/
        ,  7, -6,  6,  6, -7    /*tables for pair  13*/
        , -7,  4, -1, -3,  5    /*tables for pair  14*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x_gr =
        {5,14,7,"5stayr14.1-x_gr"   // R3+R4: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2, -3, -4,  5    /*tables for pair  1*/
        , -1,  5,  4,  6, -2    /*tables for pair  2*/
        ,  2, -3, -4, -5,  1    /*tables for pair  3*/
        , -2,  1,  7,  4, -3    /*tables for pair  4*/
        ,  3, -4, -5, -1,  2    /*tables for pair  5*/
        , -3,  2,  6,  5, -6    /*tables for pair  6*/
        ,  4, -5, -1, -2,  3    /*tables for pair  7*/
        , -4,  7,  2,  1, -7    /*tables for pair  8*/
        ,  5, -1, -2, -3,  4    /*tables for pair  9*/
        , -5,  6,  3,  7, -1    /*tables for pair  10*/
        ,  6, -7, -7, -7,  6    /*tables for pair  11*/
        , -6,  3,  5,  2, -4    /*tables for pair  12*/
        ,  7, -6, -6, -6,  7    /*tables for pair  13*/
        , -7,  4,  1,  3, -5    /*tables for pair  14*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x_ro =
        {5,14,7,"5stayr14.1-x_ro"   // R3+R5: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2, -3,  4, -5    /*tables for pair  1*/
        , -1,  5,  4, -6,  2    /*tables for pair  2*/
        ,  2, -3, -4,  5, -1    /*tables for pair  3*/
        , -2,  1,  7, -4,  3    /*tables for pair  4*/
        ,  3, -4, -5,  1, -2    /*tables for pair  5*/
        , -3,  2,  6, -5,  6    /*tables for pair  6*/
        ,  4, -5, -1,  2, -3    /*tables for pair  7*/
        , -4,  7,  2, -1,  7    /*tables for pair  8*/
        ,  5, -1, -2,  3, -4    /*tables for pair  9*/
        , -5,  6,  3, -7,  1    /*tables for pair  10*/
        ,  6, -7, -7,  7, -6    /*tables for pair  11*/
        , -6,  3,  5, -2,  4    /*tables for pair  12*/
        ,  7, -6, -6,  6, -7    /*tables for pair  13*/
        , -7,  4,  1, -3,  5    /*tables for pair  14*/
        };


static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*21];
    } _5stayr14_1_x_zw =
        {5,14,7,"5stayr14.1-x_zw"   // R4+R5: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -4, -1, -3, -4    /*  sets for table 6*/
        , -2, -3, -5, -2, -5    /*  sets for table 7*/

        ,  1, -2,  3, -4, -5    /*tables for pair  1*/
        , -1,  5, -4,  6,  2    /*tables for pair  2*/
        ,  2, -3,  4, -5, -1    /*tables for pair  3*/
        , -2,  1, -7,  4,  3    /*tables for pair  4*/
        ,  3, -4,  5, -1, -2    /*tables for pair  5*/
        , -3,  2, -6,  5,  6    /*tables for pair  6*/
        ,  4, -5,  1, -2, -3    /*tables for pair  7*/
        , -4,  7, -2,  1,  7    /*tables for pair  8*/
        ,  5, -1,  2, -3, -4    /*tables for pair  9*/
        , -5,  6, -3,  7,  1    /*tables for pair  10*/
        ,  6, -7,  7, -7, -6    /*tables for pair  11*/
        , -6,  3, -5,  2,  4    /*tables for pair  12*/
        ,  7, -6,  6, -6, -7    /*tables for pair  13*/
        , -7,  4, -1,  3,  5    /*tables for pair  14*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n =
        {5,16,10,"5stayr16.1-x_n"   // spellen blijven liggen
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2,  3,  4,  5    /*tables for pair  1*/
        , -1,  10,-4, -8, -2    /*tables for pair  2*/
        ,  2, -3,  4,  5,  1    /*tables for pair  3*/
        , -2,  1, -10,-9, -3    /*tables for pair  4*/
        ,  3, -4,  5,  1,  2    /*tables for pair  5*/
        , -3,  2, -6, -5, -9    /*tables for pair  6*/
        ,  4, -5,  1,  2,  3    /*tables for pair  7*/
        , -4,  8, -7, -1, -10   /*tables for pair  8*/
        ,  5, -1,  2,  3,  4    /*tables for pair  9*/
        , -5,  9, -3, -7, -6    /*tables for pair  10*/
        ,  6, -8,  10, 7,  9    /*tables for pair  11*/
        , -6,  3, -5, -2, -4    /*tables for pair  12*/
        ,  7, -9,  6,  8,  10   /*tables for pair  13*/
        , -7,  4, -1, -3, -5    /*tables for pair  14*/
        ,  8, -10, 7,  9,  6    /*tables for pair  15*/
        , -8,  5, -2, -4, -1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n_bl =
        {5,16,10,"5stayr16.1-x_n_bl"    // spellen blijven liggen, R3: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2, -3,  4,  5    /*tables for pair  1*/
        , -1,  10, 4, -8, -2    /*tables for pair  2*/
        ,  2, -3, -4,  5,  1    /*tables for pair  3*/
        , -2,  1,  10,-9, -3    /*tables for pair  4*/
        ,  3, -4, -5,  1,  2    /*tables for pair  5*/
        , -3,  2,  6, -5, -9    /*tables for pair  6*/
        ,  4, -5, -1,  2,  3    /*tables for pair  7*/
        , -4,  8,  7, -1, -10   /*tables for pair  8*/
        ,  5, -1, -2,  3,  4    /*tables for pair  9*/
        , -5,  9,  3, -7, -6    /*tables for pair  10*/
        ,  6, -8, -10, 7,  9    /*tables for pair  11*/
        , -6,  3,  5, -2, -4    /*tables for pair  12*/
        ,  7, -9, -6,  8,  10   /*tables for pair  13*/
        , -7,  4,  1, -3, -5    /*tables for pair  14*/
        ,  8, -10,-7,  9,  6    /*tables for pair  15*/
        , -8,  5,  2, -4, -1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n_br =
        {5,16,10,"5stayr16.1-x_n_br"    // spellen blijven liggen, R4: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2,  3, -4,  5    /*tables for pair  1*/
        , -1,  10,-4,  8, -2    /*tables for pair  2*/
        ,  2, -3,  4, -5,  1    /*tables for pair  3*/
        , -2,  1, -10, 9, -3    /*tables for pair  4*/
        ,  3, -4,  5, -1,  2    /*tables for pair  5*/
        , -3,  2, -6,  5, -9    /*tables for pair  6*/
        ,  4, -5,  1, -2,  3    /*tables for pair  7*/
        , -4,  8, -7,  1, -10   /*tables for pair  8*/
        ,  5, -1,  2, -3,  4    /*tables for pair  9*/
        , -5,  9, -3,  7, -6    /*tables for pair  10*/
        ,  6, -8,  10,-7,  9    /*tables for pair  11*/
        , -6,  3, -5,  2, -4    /*tables for pair  12*/
        ,  7, -9,  6, -8,  10   /*tables for pair  13*/
        , -7,  4, -1,  3, -5    /*tables for pair  14*/
        ,  8, -10, 7, -9,  6    /*tables for pair  15*/
        , -8,  5, -2,  4, -1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n_ge =
        {5,16,10,"5stayr16.1-x_n_ge"    // spellen blijven liggen, R5: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2,  3,  4, -5    /*tables for pair  1*/
        , -1,  10,-4, -8,  2    /*tables for pair  2*/
        ,  2, -3,  4,  5, -1    /*tables for pair  3*/
        , -2,  1, -10,-9,  3    /*tables for pair  4*/
        ,  3, -4,  5,  1, -2    /*tables for pair  5*/
        , -3,  2, -6, -5,  9    /*tables for pair  6*/
        ,  4, -5,  1,  2, -3    /*tables for pair  7*/
        , -4,  8, -7, -1,  10   /*tables for pair  8*/
        ,  5, -1,  2,  3, -4    /*tables for pair  9*/
        , -5,  9, -3, -7,  6    /*tables for pair  10*/
        ,  6, -8,  10, 7, -9    /*tables for pair  11*/
        , -6,  3, -5, -2,  4    /*tables for pair  12*/
        ,  7, -9,  6,  8, -10   /*tables for pair  13*/
        , -7,  4, -1, -3,  5    /*tables for pair  14*/
        ,  8, -10, 7,  9, -6    /*tables for pair  15*/
        , -8,  5, -2, -4,  1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n_gr =
        {5,16,10,"5stayr16.1-x_n_gr"    // spellen blijven liggen, R3+R4: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2, -3, -4,  5    /*tables for pair  1*/
        , -1,  10, 4,  8, -2    /*tables for pair  2*/
        ,  2, -3, -4, -5,  1    /*tables for pair  3*/
        , -2,  1,  10, 9, -3    /*tables for pair  4*/
        ,  3, -4, -5, -1,  2    /*tables for pair  5*/
        , -3,  2,  6,  5, -9    /*tables for pair  6*/
        ,  4, -5, -1, -2,  3    /*tables for pair  7*/
        , -4,  8,  7,  1, -10   /*tables for pair  8*/
        ,  5, -1, -2, -3,  4    /*tables for pair  9*/
        , -5,  9,  3,  7, -6    /*tables for pair  10*/
        ,  6, -8, -10,-7,  9    /*tables for pair  11*/
        , -6,  3,  5,  2, -4    /*tables for pair  12*/
        ,  7, -9, -6, -8,  10   /*tables for pair  13*/
        , -7,  4,  1,  3, -5    /*tables for pair  14*/
        ,  8, -10,-7, -9,  6    /*tables for pair  15*/
        , -8,  5,  2,  4, -1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n_ro =
        {5,16,10,"5stayr16.1-x_n_ro"    // spellen blijven liggen, R3+R5: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2, -3,  4, -5    /*tables for pair  1*/
        , -1,  10, 4, -8,  2    /*tables for pair  2*/
        ,  2, -3, -4,  5, -1    /*tables for pair  3*/
        , -2,  1,  10,-9,  3    /*tables for pair  4*/
        ,  3, -4, -5,  1, -2    /*tables for pair  5*/
        , -3,  2,  6, -5,  9    /*tables for pair  6*/
        ,  4, -5, -1,  2, -3    /*tables for pair  7*/
        , -4,  8,  7, -1,  10   /*tables for pair  8*/
        ,  5, -1, -2,  3, -4    /*tables for pair  9*/
        , -5,  9,  3, -7,  6    /*tables for pair  10*/
        ,  6, -8, -10, 7, -9    /*tables for pair  11*/
        , -6,  3,  5, -2,  4    /*tables for pair  12*/
        ,  7, -9, -6,  8, -10   /*tables for pair  13*/
        , -7,  4,  1, -3,  5    /*tables for pair  14*/
        ,  8, -10,-7,  9, -6    /*tables for pair  15*/
        , -8,  5,  2, -4,  1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*26];
    } _5stayr16_1_x_n_zw =
        {5,16,10,"5stayr16.1-x_n_zw"    // spellen blijven liggen, R4+R5: nz<->ow
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        ,  1,  1,  1,  1,  1    /*  sets for table 6*/
        ,  2,  2,  2,  2,  2    /*  sets for table 7*/
        ,  3,  3,  3,  3,  3    /*  sets for table 8*/
        ,  4,  4,  4,  4,  4    /*  sets for table 9*/
        ,  5,  5,  5,  5,  5    /*  sets for table 10*/

        ,  1, -2,  3, -4, -5    /*tables for pair  1*/
        , -1,  10,-4,  8,  2    /*tables for pair  2*/
        ,  2, -3,  4, -5, -1    /*tables for pair  3*/
        , -2,  1, -10, 9,  3    /*tables for pair  4*/
        ,  3, -4,  5, -1, -2    /*tables for pair  5*/
        , -3,  2, -6,  5,  9    /*tables for pair  6*/
        ,  4, -5,  1, -2, -3    /*tables for pair  7*/
        , -4,  8, -7,  1,  10   /*tables for pair  8*/
        ,  5, -1,  2, -3, -4    /*tables for pair  9*/
        , -5,  9, -3,  7,  6    /*tables for pair  10*/
        ,  6, -8,  10,-7, -9    /*tables for pair  11*/
        , -6,  3, -5,  2,  4    /*tables for pair  12*/
        ,  7, -9,  6, -8, -10   /*tables for pair  13*/
        , -7,  4, -1,  3,  5    /*tables for pair  14*/
        ,  8, -10, 7, -9, -6    /*tables for pair  15*/
        , -8,  5, -2,  4,  1    /*tables for pair  16*/
        };

static struct
    {   UINT rounds;     UINT pairs;
        UINT tables;     const char* name;
        signed char data[5*30];
    } _5mitchell10_10 =
        {5,20,10,"5mitchell10_10"
        ,  1,  1,  1,  1,  1    /*  sets for table 1*/
        ,  2,  2,  2,  2,  2    /*  sets for table 2*/
        ,  3,  3,  3,  3,  3    /*  sets for table 3*/
        ,  4,  4,  4,  4,  4    /*  sets for table 4*/
        ,  5,  5,  5,  5,  5    /*  sets for table 5*/
        , -1, -2, -3, -4, -5    /*  sets for table 6*/
        , -2, -3, -4, -5, -1    /*  sets for table 7*/
        , -3, -4, -5, -1, -2    /*  sets for table 8*/
        , -4, -5, -1, -2, -3    /*  sets for table 9*/
        , -5, -1, -2, -3, -4    /*  sets for table 10*/

        ,  1,  2,  3,  4,  5    /*tables for pair  1*/
        , -1, -4, -2, -5, -3    /*tables for pair  2*/
        ,  2,  8,  9, 10,  6    /*tables for pair  3*/
        , -2, -5, -3, -1, -4    /*tables for pair  4*/
        ,  3,  9, 10,  6,  7    /*tables for pair  5*/
        , -3, -1, -4, -2, -5    /*tables for pair  6*/
        ,  4, 10,  6,  7,  8    /*tables for pair  7*/
        , -4, -2, -5, -3, -1    /*tables for pair  8*/
        ,  5,  6,  7,  8,  9    /*tables for pair  9*/
        , -5, -3, -1, -4, -2    /*tables for pair  10*/
        ,  6,  7,  8,  9, 10    /*tables for pair  11*/
        , -6, -6, -6, -6, -6    /*tables for pair  12*/
        ,  7,  3,  4,  5,  1    /*tables for pair  13*/
        , -7, -7, -7, -7, -7    /*tables for pair  14*/
        ,  8,  4,  5,  1,  2    /*tables for pair  15*/
        , -8, -8, -8, -8, -8    /*tables for pair  16*/
        ,  9,  5,  1,  2,  3    /*tables for pair  17*/
        , -9, -9, -9, -9, -9    /*tables for pair  18*/
        , 10,  1,  2,  3,  4    /*tables for pair  19*/
        ,-10,-10,-10,-10,-10    /*tables for pair  20*/
        };

static struct
        {       UINT rounds;             UINT pairs;
                UINT tables;             const char* name;
                signed char data[8*(14+8)];
        } _8special14_08 =
                {8,14,8,"8special14_08"
        ,  1,  1,  1,  1,  1,  1,  1,  1   /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2,  2   /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3,  3   /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4,  4   /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5,  5   /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6,  6   /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7,  7   /*  sets for table 7*/
        ,  8,  8,  8,  8,  8,  8,  8,  8   /*  sets for table 8*/

        ,  1,  5, -4,  2,  6, -8, -3, -7   /*tables for pair  1*/
        , -1,  6, -8, -3,  7, -5, -4, -2   /*tables for pair  2*/
        ,  2, -8,  4,  3, -1,  7,  6,  5   /*tables for pair  3*/
        , -2,  1, -5, -4, -7, -6, -8, -3   /*tables for pair  4*/
        ,  3, -5,  8,  4, -2,  1,  7,  6   /*tables for pair  5*/
        , -3,  2, -7, -8,  1,  6, -5, -4   /*tables for pair  6*/
        ,  4, -6,  5,  8, -3,  2,  1,  7   /*tables for pair  7*/
        , -4,  3, -1, -6,  2, -7,  5, -8   /*tables for pair  8*/
        ,  5, -1,  7,  6, -8,  4,  3,  2   /*tables for pair  9*/
        , -5,  4, -2, -7,  3, -1, -6,  8   /*tables for pair  10*/
        ,  6, -2,  1,  7, -5,  8,  4,  3   /*tables for pair  11*/
        , -6, -4, -3, -1,  8, -2, -7, -5   /*tables for pair  12*/
        ,  7, -3,  2,  1, -6,  5,  8,  4   /*tables for pair  13*/
        , -7,  8,  3, -2,  5, -4, -1, -6   /*tables for pair  14*/
};

static struct
        {       UINT rounds;             UINT pairs;
                UINT tables;             const char* name;
                signed char data[8*(18+9)];
        } _8mitchel18_09 =
                {8,18,9,"8mitchel18_09"
        ,  1,  1,  1,  1,  1,  1,  1,  1   /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2,  2   /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3,  3   /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4,  4   /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5,  5   /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6,  6   /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7,  7   /*  sets for table 7*/
        ,  8,  8,  8,  8,  8,  8,  8,  8   /*  sets for table 8*/
        , -1, -2, -3, -4, -5, -6, -7, -8   /*  sets for table 9*/

        ,  1,  2,  3,  4,  5,  6,  7,  8   /*tables for pair  1*/
        , -1, -6, -8, -3, -7, -4, -2, -5   /*tables for pair  2*/
        ,  2,  7,  8,  1,  3,  9,  5,  4   /*tables for pair  3*/
        , -2, -5, -7, -4, -8, -3, -1, -6   /*tables for pair  4*/
        ,  3,  6,  5,  9,  2,  7,  8,  1   /*tables for pair  5*/
        , -3, -8, -6, -1, -5, -2, -4, -7   /*tables for pair  6*/
        ,  4,  5,  6,  3,  1,  8,  9,  2   /*tables for pair  7*/
        , -4, -7, -5, -2, -6, -1, -3, -8   /*tables for pair  8*/
        ,  5,  4,  9,  6,  8,  1,  2,  7   /*tables for pair  9*/
        , -5, -2, -4, -7, -3, -8, -6, -1   /*tables for pair  10*/
        ,  6,  3,  4,  5,  7,  2,  1,  9   /*tables for pair  11*/
        , -6, -1, -3, -8, -4, -7, -5, -2   /*tables for pair  12*/
        ,  7,  9,  1,  8,  6,  3,  4,  5   /*tables for pair  13*/
        , -7, -4, -2, -5, -1, -6, -8, -3   /*tables for pair  14*/
        ,  8,  1,  2,  7,  9,  4,  3,  6   /*tables for pair  15*/
        , -8, -3, -1, -6, -2, -5, -7, -4   /*tables for pair  16*/
        ,  9,  8,  7,  2,  4,  5,  6,  3   /*tables for pair  17*/
        , -9, -9, -9, -9, -9, -9, -9, -9   /*tables for pair  18*/
};

static struct
        {       UINT rounds;             UINT pairs;
                UINT tables;             const char* name;
                signed char data[8*(18+9)];
        } _8BridgeBoost18_09 =
                {8,18,9,"8BridgeBoost18_09"
        ,  1,  1,  1,  1,  1,  1,  1,  1   /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2,  2   /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3,  3   /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4,  4   /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5,  5   /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6,  6   /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7,  7   /*  sets for table 7*/
        ,  8,  8,  8,  8,  8,  8,  8,  8   /*  sets for table 8*/
        , -8, -7, -6, -5, -4, -3, -2, -1   /*  sets for table 9*/

        ,  1,  2,  8,  7,  3,  4,  5,  6   /*tables for pair  1*/
        , -1, -3, -7, -6, -5, -8, -4, -2   /*tables for pair  2*/
        ,  2, -1,  7,  8,  9,  3,  6, -5   /*tables for pair  3*/
        , -2,  4, -8,  9, -6, -7, -3,  1   /*tables for pair  4*/
        ,  3, -4,  5,  6, -1,  2,  8,  7   /*tables for pair  5*/
        , -3,  1, -6, -7,  8, -5,  9, -4   /*tables for pair  6*/
        ,  4,  3,  9,  5,  2, -1, -7,  8   /*tables for pair  7*/
        , -4, -2, -5, -8, -7,  6,  1, -3   /*tables for pair  8*/
        ,  5,  6,  3,  4, -8,  7, -1,  2   /*tables for pair  9*/
        , -5, -8, -4, -2,  1,  9,  7, -6   /*tables for pair  10*/
        ,  6,  5,  4, -3,  7,  8,  2,  9   /*tables for pair  11*/
        , -6, -7, -3,  1, -2, -4, -8,  5   /*tables for pair  12*/
        ,  7,  8, -2, -1,  6,  5,  4,  3   /*tables for pair  13*/
        , -7, -6,  1,  3, -4, -2, -5, -8   /*tables for pair  14*/
        ,  8, -9, -1,  2,  5, -6,  3,  4   /*tables for pair  15*/
        , -8, -5,  2, -4, -3,  1, -6, -7   /*tables for pair  16*/
        ,  9,  7,  6, -5,  4, -3, -2, -1   /*tables for pair  17*/
        , -9,  9, -9, -9, -9, -9, -9, -9   /*tables for pair  18*/
};

static struct
        {       UINT rounds;             UINT pairs;
                UINT tables;             const char* name;
        signed char data[8*(20+10)];
        } _8mitchel20_10 =
                {8,20,10,"8mitchel20_10"
        ,  1,  1,  1,  1,  1,  1,  1,  1   /*  sets for table 1*/
        ,  2,  2,  2,  2,  2,  2,  2,  2   /*  sets for table 2*/
        ,  3,  3,  3,  3,  3,  3,  3,  3   /*  sets for table 3*/
        ,  4,  4,  4,  4,  4,  4,  4,  4   /*  sets for table 4*/
        ,  5,  5,  5,  5,  5,  5,  5,  5   /*  sets for table 5*/
        ,  6,  6,  6,  6,  6,  6,  6,  6   /*  sets for table 6*/
        ,  7,  7,  7,  7,  7,  7,  7,  7   /*  sets for table 7*/
        ,  8,  8,  8,  8,  8,  8,  8,  8   /*  sets for table 8*/
        , -1, -2, -3, -4, -5, -6, -7, -8   /*  sets for table 9*/
        , -2, -1, -4, -3, -6, -5, -8, -7   /*  sets for table10*/

        ,  1,  2,  3,  4,  5,  6,  7,  8   /*tables for pair  1*/
        , -1, -6, -8, -3, -7, -4, -2, -5   /*tables for pair  2*/
        ,  2,  1,  4,  3,  6,  5,  8,  7   /*tables for pair  3*/
        , -2, -5, -7, -4, -8, -3, -1, -6   /*tables for pair  4*/
        ,  3,  6,  5,  9,  2,  7, 10,  1   /*tables for pair  5*/
        , -3, -8, -6, -1, -5, -2, -4, -7   /*tables for pair  6*/
        ,  4,  5,  6, 10,  1,  8,  9,  2   /*tables for pair  7*/
        , -4, -7, -5, -2, -6, -1, -3, -8   /*tables for pair  8*/
        ,  5,  4,  9,  6,  8,  1,  2, 10   /*tables for pair  9*/
        , -5, -2, -4, -7, -3, -8, -6, -1   /*tables for pair  10*/
        ,  6,  3, 10,  5,  7,  2,  1,  9   /*tables for pair  11*/
        , -6, -1, -3, -8, -4, -7, -5, -2   /*tables for pair  12*/
        ,  7,  9,  1,  8, 10,  3,  4,  5   /*tables for pair  13*/
        , -7, -4, -2, -5, -1, -6, -8, -3   /*tables for pair  14*/
        ,  8, 10,  2,  7,  9,  4,  3,  6   /*tables for pair  15*/
        , -8, -3, -1, -6, -2, -5, -7, -4   /*tables for pair  16*/
        ,  9,  8,  7,  2,  4, 10,  6,  3   /*tables for pair  17*/
        , -9, -9, -9, -9, -9, -9, -9, -9   /*tables for pair  18*/
        , 10,  7,  8,  1,  3,  9,  5,  4   /*tables for pair  19*/
        ,-10,-10,-10,-10,-10,-10,-10,-10   /*tables for pair  20*/
};

const struct SCHEMA_DATA* const schemaTable[]=
{
    (struct SCHEMA_DATA*) &_5tin08,
    (struct SCHEMA_DATA*) &_6multi08, (struct SCHEMA_DATA*) &_6multi10,
    (struct SCHEMA_DATA*) &_6multi12, (struct SCHEMA_DATA*) &_6multi14,
    (struct SCHEMA_DATA*) &_6multi16, (struct SCHEMA_DATA*) &_6multi18,
    (struct SCHEMA_DATA*) &_6multi20, (struct SCHEMA_DATA*) &_6multi22,
    (struct SCHEMA_DATA*) &_6multi24, (struct SCHEMA_DATA*) &_7multi08,
    (struct SCHEMA_DATA*) &_7multi10, (struct SCHEMA_DATA*) &_7multi12,
    (struct SCHEMA_DATA*) &_7multi14, (struct SCHEMA_DATA*) &_7multi16,
    (struct SCHEMA_DATA*) &_7multi18, (struct SCHEMA_DATA*) &_7multi20,
    (struct SCHEMA_DATA*) &_7multi22, (struct SCHEMA_DATA*) &_7multi24,
    (struct SCHEMA_DATA*) &_7multi26, (struct SCHEMA_DATA*) &_7multi28,

    (struct SCHEMA_DATA*) &_6multi08_nieuw,   (struct SCHEMA_DATA*) &_6multi10_nieuw,
    (struct SCHEMA_DATA*) &_6multi12_nieuw,   (struct SCHEMA_DATA*) &_6multi14_nieuw,
    (struct SCHEMA_DATA*) &_6multi16_nieuw,   (struct SCHEMA_DATA*) &_6multi18_nieuw,
    (struct SCHEMA_DATA*) &_6multi20_nieuw,   (struct SCHEMA_DATA*) &_6multi22_nieuw,
    (struct SCHEMA_DATA*) &_6multi24_nieuw,
    (struct SCHEMA_DATA*) &_7multi08_nieuw,   (struct SCHEMA_DATA*) &_7multi10_nieuw,
    (struct SCHEMA_DATA*) &_7multi12_nieuw,   (struct SCHEMA_DATA*) &_7multi14_nieuw,
    (struct SCHEMA_DATA*) &_7multi16_nieuw,   (struct SCHEMA_DATA*) &_7multi18_nieuw,
    (struct SCHEMA_DATA*) &_7multi20_nieuw,   (struct SCHEMA_DATA*) &_7multi22_nieuw,
    (struct SCHEMA_DATA*) &_7multi24_nieuw,   (struct SCHEMA_DATA*) &_7multi26_nieuw,
    (struct SCHEMA_DATA*) &_7multi28_nieuw,

    (struct SCHEMA_DATA*) &_4basis8,      (struct SCHEMA_DATA*) &_5basis10,
    (struct SCHEMA_DATA*) &_6basis12_1_5, (struct SCHEMA_DATA*) &_6basis12_2_5,
    (struct SCHEMA_DATA*) &_6basis12_3_5, (struct SCHEMA_DATA*) &_6basis12_4_5,
    (struct SCHEMA_DATA*) &_6basis12_5_5, (struct SCHEMA_DATA*) &_7basis14,
    (struct SCHEMA_DATA*) &_3drive08,     (struct SCHEMA_DATA*) &_5howel06_1_1,
    (struct SCHEMA_DATA*) &_7howel08_1_1, (struct SCHEMA_DATA*) &_3howel10_1_3,
    (struct SCHEMA_DATA*) &_3howel10_2_3, (struct SCHEMA_DATA*) &_3howel10_3_3,
    (struct SCHEMA_DATA*) &_4howel10_2_2, (struct SCHEMA_DATA*) &_5howel10_1_2,
    (struct SCHEMA_DATA*) &_9howel10_1_1, (struct SCHEMA_DATA*) &_9howel10_2_1,
    (struct SCHEMA_DATA*) &_3howel12_3_3, (struct SCHEMA_DATA*) &_4howel12_1_3,
    (struct SCHEMA_DATA*) &_4howel12_2_3, (struct SCHEMA_DATA*) &_5howel12_2_2,
    (struct SCHEMA_DATA*) &_6howel12_1_2, (struct SCHEMA_DATA*) &_11howe12_2_1,
    (struct SCHEMA_DATA*) &_11howe12_3_1, (struct SCHEMA_DATA*) &_4howel14_1_3,
    (struct SCHEMA_DATA*) &_4howel14_2_3, (struct SCHEMA_DATA*) &_5howel14_3_3,
    (struct SCHEMA_DATA*) &_6howel14_2_2, (struct SCHEMA_DATA*) &_7howel14_1_2,
    (struct SCHEMA_DATA*) &_5howel16_1_3, (struct SCHEMA_DATA*) &_5howel16_2_3,
    (struct SCHEMA_DATA*) &_5howel16_3_3, (struct SCHEMA_DATA*) &_5howel18_3_3,
    (struct SCHEMA_DATA*) &_7howel16,
    (struct SCHEMA_DATA*) &_6howel18_1_3, (struct SCHEMA_DATA*) &_6howel18_2_3,
    (struct SCHEMA_DATA*) &_4stayr10,     (struct SCHEMA_DATA*) &_4stayr10_1_x,
    (struct SCHEMA_DATA*) &_4stayr12,     (struct SCHEMA_DATA*) &_4stayr12_1_x,
    (struct SCHEMA_DATA*) &_5stayr12,     (struct SCHEMA_DATA*) &_5stayr12_1_x,
    (struct SCHEMA_DATA*) &_6stayr13,     (struct SCHEMA_DATA*) &_6stayr13_1_5,
    (struct SCHEMA_DATA*) &_4stayr14,     (struct SCHEMA_DATA*) &_4stayr14_1_x,
    (struct SCHEMA_DATA*) &_5stayr14,     (struct SCHEMA_DATA*) &_5stayr14_1_x,
    (struct SCHEMA_DATA*) &_5stayr14_1_x_bl,(struct SCHEMA_DATA*) &_5stayr14_1_x_br,
    (struct SCHEMA_DATA*) &_5stayr14_1_x_ge,(struct SCHEMA_DATA*) &_5stayr14_1_x_gr,
    (struct SCHEMA_DATA*) &_5stayr14_1_x_ro,(struct SCHEMA_DATA*) &_5stayr14_1_x_zw,
    (struct SCHEMA_DATA*) &_6stayr14,     (struct SCHEMA_DATA*) &_6stayr14_1_5,
    (struct SCHEMA_DATA*) &_5stayr16,     (struct SCHEMA_DATA*) &_5stayr16_1_x,
    (struct SCHEMA_DATA*) &_5stayr16_1_x_n,
    (struct SCHEMA_DATA*) &_5stayr16_1_x_n_bl,(struct SCHEMA_DATA*) &_5stayr16_1_x_n_br,
    (struct SCHEMA_DATA*) &_5stayr16_1_x_n_ge,(struct SCHEMA_DATA*) &_5stayr16_1_x_n_gr,
    (struct SCHEMA_DATA*) &_5stayr16_1_x_n_ro,(struct SCHEMA_DATA*) &_5stayr16_1_x_n_zw,
    (struct SCHEMA_DATA*) &_6stayr16,     (struct SCHEMA_DATA*) &_6stayr16_1_5,
    (struct SCHEMA_DATA*) &_7stayr16,     (struct SCHEMA_DATA*) &_7stayr16_1_x,
    (struct SCHEMA_DATA*) &_5stayr18,     (struct SCHEMA_DATA*) &_5stayr18_1_x,
    (struct SCHEMA_DATA*) &_6stayr18,     (struct SCHEMA_DATA*) &_6stayr18_1_5,
    (struct SCHEMA_DATA*) &_7stayr18,     (struct SCHEMA_DATA*) &_7stayr18_1_x,
    (struct SCHEMA_DATA*) &_6stayr20,     (struct SCHEMA_DATA*) &_6stayr20_1_5,
    (struct SCHEMA_DATA*) &_7stayr20,     (struct SCHEMA_DATA*) &_7stayr20_1_x,
    (struct SCHEMA_DATA*) &_6stayr22,     (struct SCHEMA_DATA*) &_6stayr22_1_5,
    (struct SCHEMA_DATA*) &_7stayr22,     (struct SCHEMA_DATA*) &_7stayr22_1_x,
    (struct SCHEMA_DATA*) &_7stayr24,     (struct SCHEMA_DATA*) &_7stayr24_1_x,
    (struct SCHEMA_DATA*) &_7stayr26,     (struct SCHEMA_DATA*) &_7stayr26_1_x,
    (struct SCHEMA_DATA*) &_7txx8 ,       (struct SCHEMA_DATA*) &_7txx10 ,
    (struct SCHEMA_DATA*) &_7txx12 ,      (struct SCHEMA_DATA*) &_7txx14 ,
    (struct SCHEMA_DATA*) &_7txx16 ,      (struct SCHEMA_DATA*) &_7txx18 ,
    (struct SCHEMA_DATA*) &_7t8 ,         (struct SCHEMA_DATA*) &_7t10 ,
                                        (struct SCHEMA_DATA*) &_7t14 ,
    (struct SCHEMA_DATA*) &_7t16 ,        (struct SCHEMA_DATA*) &_7t18 ,
    (struct SCHEMA_DATA*) &_7t20 ,        (struct SCHEMA_DATA*) &_7t22 ,
    (struct SCHEMA_DATA*) &_7t24 ,
    (struct SCHEMA_DATA*) &_6t8 ,         (struct SCHEMA_DATA*) &_6t10 ,
    (struct SCHEMA_DATA*) &_6t12 ,        (struct SCHEMA_DATA*) &_6t14 ,
    (struct SCHEMA_DATA*) &_6t16 ,        (struct SCHEMA_DATA*) &_6t18 ,
    (struct SCHEMA_DATA*) &_8sche_a16,    (struct SCHEMA_DATA*) &_8sche_bc16, 
    (struct SCHEMA_DATA*) &_4mitchel14h,  (struct SCHEMA_DATA*) &_4mitchel16h,
    (struct SCHEMA_DATA*) &_5mitchell10_10,
    (struct SCHEMA_DATA*) &_4stayr14special,
    (struct SCHEMA_DATA*) &_3howel12_3_3nz,
    (struct SCHEMA_DATA*) &_3harrie_12nzow,
    (struct SCHEMA_DATA*) &_8special14_08,
    (struct SCHEMA_DATA*) &_8mitchel18_09,
    (struct SCHEMA_DATA*) &_8mitchel20_10,
    (struct SCHEMA_DATA*) &_8BridgeBoost18_09,
    0
};

const int SCHEMA_NUM_ENTRIES = sizeof(schemaTable)/sizeof(schemaTable[0]) - 1;
