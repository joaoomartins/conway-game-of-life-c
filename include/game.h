#ifndef GAME_H
#define GAME_H

#define MAX_SIZE 25

// Simulator configurations structure
typedef struct {
    int rows;
    int cols;
    float secsPerGen;
    int showGridOnAdvance;
    int showStatsOnAdvance;
} Config;

// Game state and statistics structure
typedef struct {
    int grid[MAX_SIZE][MAX_SIZE];
    int ages[MAX_SIZE][MAX_SIZE];
    int generation;
    int maxAlive;
    int genMaxAlive;
    int minAlive;
    int genMinAlive;
} LifeGame;

// Core setup and board operations
void initGame(void);
void clearGrid(void);

// UI and Menus
void printGrid(void);
void printStatistics(void);
int showMainMenu(void);
int showConfigMenu(void);

// Cell management
int isWithinBounds(int row, int col);
void insertSingleCell(void);
void removeSingleCell(void);
void insertMultipleCells(void);
void removeMultipleCells(void);

// Simulation
int countAliveCells(void);
void advanceGenerations(int totalGens);

// Configuration handlers
void configureDimensions(void);
void configureSpeed(void);
void toggleShowGrid(void);
void toggleShowStatistics(void);

// Preset test patterns
void loadGliderPattern(void);
void loadBlinkerPattern(void);
void loadBlockPattern(void);
void loadBoatPattern(void);

// Utilities
void delaySeconds(float seconds);
void clearInputBuffer(void);

#endif // GAME_H
