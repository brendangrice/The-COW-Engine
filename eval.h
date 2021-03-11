#include "types.h"
#include "main.h"

#ifndef EVAL_H
#define EVAL_H
int numberOfBits(Board b); // count number of set bits on a given board
int calculatePositionAdvantage(); // value based off of positional advantage
int calculateMaterialAdvantage(); // value based off of materia advantage
float calculateAdvantage(); // final value of total game state advantage
#endif
