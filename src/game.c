#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"

LifeGame g_game;
Config g_cfg;

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void delaySeconds(float seconds) {
    if (seconds <= 0.0f) return;
    clock_t start = clock();
    clock_t duration = (clock_t)(seconds * CLOCKS_PER_SEC);
    while ((clock() - start) < duration);
}

int isWithinBounds(int row, int col) {
    return (row >= 0 && row < g_cfg.rows && col >= 0 && col < g_cfg.cols);
}

int countAliveCells(void) {
    int total = 0;
    for (int r = 0; r < g_cfg.rows; r++) {
        for (int c = 0; c < g_cfg.cols; c++) {
            if (g_game.grid[r][c] == 1) {
                total++;
            }
        }
    }
    return total;
}

void updatePopulationStats(void) {
    int alive = countAliveCells();
    if (alive > g_game.maxAlive) {
        g_game.maxAlive = alive;
        g_game.genMaxAlive = g_game.generation;
    }
    if (alive < g_game.minAlive) {
        g_game.minAlive = alive;
        g_game.genMinAlive = g_game.generation;
    }
}

void initGame(void) {
    g_cfg.rows = MAX_SIZE;
    g_cfg.cols = MAX_SIZE;
    g_cfg.secsPerGen = 0.5f;
    g_cfg.showGridOnAdvance = 1;
    g_cfg.showStatsOnAdvance = 0;

    clearGrid();
}

void clearGrid(void) {
    for (int r = 0; r < MAX_SIZE; r++) {
        for (int c = 0; c < MAX_SIZE; c++) {
            g_game.grid[r][c] = 0;
            g_game.ages[r][c] = 0;
        }
    }
    g_game.generation = 0;
    g_game.maxAlive = 0;
    g_game.genMaxAlive = 0;
    g_game.minAlive = 0;
    g_game.genMinAlive = 0;
}

void printGrid(void) {
    system("cls");

    printf("   ");
    for (int c = 0; c < g_cfg.cols; c++) {
        printf("%2d ", c);
    }
    printf("\n");

    for (int r = 0; r < g_cfg.rows; r++) {
        printf("%2d ", r);
        for (int c = 0; c < g_cfg.cols; c++) {
            if (g_game.grid[r][c] == 1) {
                printf(" * ");
            } else {
                printf(" . ");
            }
        }
        printf("\n");
    }
}

void insertSingleCell(void) {
    int row, col;
    int valid = 0;

    do {
        printf("\nEnter cell coordinates (row col) [0-%d 0-%d]: ",
               g_cfg.rows - 1, g_cfg.cols - 1);
        if (scanf("%d %d", &row, &col) != 2) {
            clearInputBuffer();
            printf("Invalid input! Please enter two numbers separated by space (e.g. 4 5).\n");
            continue;
        }
        clearInputBuffer();

        if (isWithinBounds(row, col)) {
            valid = 1;
        } else {
            printf("Out of bounds! Please enter valid coordinates within grid limits.\n");
        }
    } while (!valid);

    g_game.grid[row][col] = 1;
    g_game.ages[row][col] = 1;
    updatePopulationStats();
    printf("Cell successfully inserted at (%d, %d)!\n", row, col);
    delaySeconds(1.0f);
}

void removeSingleCell(void) {
    int row, col;
    int valid = 0;

    do {
        printf("\nEnter cell coordinates to remove (row col) [0-%d 0-%d]: ",
               g_cfg.rows - 1, g_cfg.cols - 1);
        if (scanf("%d %d", &row, &col) != 2) {
            clearInputBuffer();
            printf("Invalid input! Please enter two numbers separated by space (e.g. 4 5).\n");
            continue;
        }
        clearInputBuffer();

        if (!isWithinBounds(row, col)) {
            printf("Coordinates out of bounds!\n");
        } else if (g_game.grid[row][col] == 0) {
            printf("Notice: This cell position is already empty/dead.\n");
            valid = 1;
        } else {
            valid = 1;
        }
    } while (!valid);

    g_game.grid[row][col] = 0;
    g_game.ages[row][col] = 0;
    updatePopulationStats();
    printf("Position (%d, %d) cleared!\n", row, col);
    delaySeconds(1.0f);
}

void insertMultipleCells(void) {
    int choice = 1;
    while (choice != 0) {
        printGrid();
        insertSingleCell();

        printf("\nDo you want to insert another cell? (1 - Yes / 0 - Stop): ");
        if (scanf("%d", &choice) != 1) {
            choice = 0;
        }
        clearInputBuffer();
    }
}

void removeMultipleCells(void) {
    int choice = 1;
    while (choice != 0) {
        printGrid();
        removeSingleCell();

        printf("\nDo you want to remove another cell? (1 - Yes / 0 - Stop): ");
        if (scanf("%d", &choice) != 1) {
            choice = 0;
        }
        clearInputBuffer();
    }
}

// Counts live neighbors using toroidal geometry (wrap-around infinite board)
static int countToroidalNeighbors(int row, int col) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;

            int nRow = (row + dr + g_cfg.rows) % g_cfg.rows;
            int nCol = (col + dc + g_cfg.cols) % g_cfg.cols;

            if (g_game.grid[nRow][nCol] == 1) {
                count++;
            }
        }
    }
    return count;
}

static void evolveStep(void) {
    int nextGrid[MAX_SIZE][MAX_SIZE];
    int nextAges[MAX_SIZE][MAX_SIZE];

    for (int r = 0; r < g_cfg.rows; r++) {
        for (int c = 0; c < g_cfg.cols; c++) {
            int neighbors = countToroidalNeighbors(r, c);
            int alive = g_game.grid[r][c];

            if (alive) {
                // Survival: 2 or 3 neighbors
                if (neighbors == 2 || neighbors == 3) {
                    nextGrid[r][c] = 1;
                    nextAges[r][c] = g_game.ages[r][c] + 1;
                } else {
                    // Death: underpopulation or overpopulation
                    nextGrid[r][c] = 0;
                    nextAges[r][c] = 0;
                }
            } else {
                // Reproduction: exactly 3 neighbors
                if (neighbors == 3) {
                    nextGrid[r][c] = 1;
                    nextAges[r][c] = 1;
                } else {
                    nextGrid[r][c] = 0;
                    nextAges[r][c] = 0;
                }
            }
        }
    }

    for (int r = 0; r < g_cfg.rows; r++) {
        for (int c = 0; c < g_cfg.cols; c++) {
            g_game.grid[r][c] = nextGrid[r][c];
            g_game.ages[r][c] = nextAges[r][c];
        }
    }

    g_game.generation++;
    updatePopulationStats();
}

void advanceGenerations(int totalGens) {
    if (totalGens <= 0) return;

    for (int i = 0; i < totalGens; i++) {
        evolveStep();

        if (g_cfg.showGridOnAdvance) {
            printGrid();
            if (g_cfg.showStatsOnAdvance) {
                printStatistics();
            }
            delaySeconds(g_cfg.secsPerGen);
        }
    }
}

void printStatistics(void) {
    int currentAlive = countAliveCells();
    int oldestAge = 0;
    int youngestAge = 999999;
    int ageDistribution[100];

    for (int i = 0; i < 100; i++) ageDistribution[i] = 0;

    for (int r = 0; r < g_cfg.rows; r++) {
        for (int c = 0; c < g_cfg.cols; c++) {
            if (g_game.grid[r][c] == 1) {
                int age = g_game.ages[r][c];
                if (age > oldestAge) oldestAge = age;
                if (age < youngestAge) youngestAge = age;
                if (age < 100) ageDistribution[age]++;
            }
        }
    }

    printf("\n=================== GAME STATISTICS ===================\n");
    printf(" Current Generation: %d\n", g_game.generation);
    printf(" Currently Alive Cells: %d\n", currentAlive);
    printf(" Maximum Alive Cells: %d (Occurred in Generation %d)\n", g_game.maxAlive, g_game.genMaxAlive);
    printf(" Minimum Alive Cells: %d (Occurred in Generation %d)\n", g_game.minAlive, g_game.genMinAlive);

    if (currentAlive > 0) {
        printf(" Oldest Alive Cell: Age %d (Born in Generation %d)\n",
               oldestAge, g_game.generation - (oldestAge - 1));
        printf(" Youngest Alive Cell: Age %d (Born in Generation %d)\n",
               youngestAge, g_game.generation - (youngestAge - 1));

        printf("\n--- Cell Age Histogram ---\n");
        for (int i = 1; i <= oldestAge && i < 100; i++) {
            if (ageDistribution[i] > 0) {
                printf(" Age %2d (%2d cells): ", i, ageDistribution[i]);
                for (int b = 0; b < ageDistribution[i]; b++) {
                    printf("#");
                }
                printf("\n");
            }
        }
    } else {
        printf(" No live cells currently present on the grid.\n");
    }
    printf("=======================================================\n");
}

void configureDimensions(void) {
    int rows, cols;
    printf("\nWARNING: Maximum supported grid dimensions: %dx%d\n", MAX_SIZE, MAX_SIZE);
    printf("Current dimensions: %dx%d\n", g_cfg.rows, g_cfg.cols);
    printf("Enter new dimensions (rows cols): ");

    if (scanf("%d %d", &rows, &cols) == 2) {
        clearInputBuffer();
        if (rows > 0 && rows <= MAX_SIZE && cols > 0 && cols <= MAX_SIZE) {
            g_cfg.rows = rows;
            g_cfg.cols = cols;
            printf("Dimensions updated successfully!\n");
        } else {
            printf("Invalid dimensions! Must be between 1 and %d.\n", MAX_SIZE);
        }
    } else {
        clearInputBuffer();
        printf("Invalid input!\n");
    }
    delaySeconds(2.0f);
}

void configureSpeed(void) {
    float secs;
    printf("\nCurrent delay between generations: %.2fs\n", g_cfg.secsPerGen);
    printf("Enter new delay in seconds (e.g. 0.3): ");
    if (scanf("%f", &secs) == 1 && secs >= 0.0f) {
        clearInputBuffer();
        g_cfg.secsPerGen = secs;
        printf("Simulation speed updated successfully!\n");
    } else {
        clearInputBuffer();
        printf("Invalid value!\n");
    }
    delaySeconds(2.0f);
}

void toggleShowGrid(void) {
    g_cfg.showGridOnAdvance = !g_cfg.showGridOnAdvance;
    printf("\nDisplay grid on advance: %s\n",
           g_cfg.showGridOnAdvance ? "ENABLED" : "DISABLED");
    delaySeconds(2.0f);
}

void toggleShowStatistics(void) {
    g_cfg.showStatsOnAdvance = !g_cfg.showStatsOnAdvance;
    printf("\nDisplay statistics on advance: %s\n",
           g_cfg.showStatsOnAdvance ? "ENABLED" : "DISABLED");
    delaySeconds(2.0f);
}

void loadGliderPattern(void) {
    clearGrid();
    int r = 1, c = 1;
    if (g_cfg.rows > 4 && g_cfg.cols > 4) {
        g_game.grid[r][c + 1] = 1;
        g_game.grid[r + 1][c + 2] = 1;
        g_game.grid[r + 2][c] = 1;
        g_game.grid[r + 2][c + 1] = 1;
        g_game.grid[r + 2][c + 2] = 1;
        for (int i = 0; i < g_cfg.rows; i++)
            for (int j = 0; j < g_cfg.cols; j++)
                if (g_game.grid[i][j]) g_game.ages[i][j] = 1;
        updatePopulationStats();
    }
}

void loadBlinkerPattern(void) {
    clearGrid();
    int r = 2, c = 2;
    if (g_cfg.rows > 4 && g_cfg.cols > 4) {
        g_game.grid[r][c] = 1;
        g_game.grid[r][c + 1] = 1;
        g_game.grid[r][c + 2] = 1;
        g_game.ages[r][c] = 1;
        g_game.ages[r][c + 1] = 1;
        g_game.ages[r][c + 2] = 1;
        updatePopulationStats();
    }
}

void loadBlockPattern(void) {
    clearGrid();
    int r = 2, c = 2;
    if (g_cfg.rows > 3 && g_cfg.cols > 3) {
        g_game.grid[r][c] = 1;
        g_game.grid[r][c + 1] = 1;
        g_game.grid[r + 1][c] = 1;
        g_game.grid[r + 1][c + 1] = 1;
        g_game.ages[r][c] = 1;
        g_game.ages[r][c + 1] = 1;
        g_game.ages[r + 1][c] = 1;
        g_game.ages[r + 1][c + 1] = 1;
        updatePopulationStats();
    }
}

void loadBoatPattern(void) {
    clearGrid();
    int r = 2, c = 2;
    if (g_cfg.rows > 4 && g_cfg.cols > 4) {
        g_game.grid[r][c] = 1;
        g_game.grid[r][c + 1] = 1;
        g_game.grid[r + 1][c] = 1;
        g_game.grid[r + 1][c + 2] = 1;
        g_game.grid[r + 2][c + 1] = 1;
        for (int i = 0; i < g_cfg.rows; i++)
            for (int j = 0; j < g_cfg.cols; j++)
                if (g_game.grid[i][j]) g_game.ages[i][j] = 1;
        updatePopulationStats();
    }
}

int showMainMenu(void) {
    int choice;
    printf("*-------------------------------------------------------------------------------------*\n");
    printf("|       _                                 _            __      __  _       _          |\n");
    printf("|      | |                               | |           \\ \\    / / (_)     | |         |\n");
    printf("|      | |   ___     __ _    ___       __| |   __ _     \\ \\  / /   _    __| |   __ _  |\n");
    printf("|  _   | |  / _ \\   / _` |  / _ \\     / _` |  / _` |     \\ \\/ /   | |  / _` |  / _` | |\n");
    printf("| | |__| | | (_) | | (_| | | (_) |   | (_| | | (_| |      \\  /    | | | (_| | | (_| | |\n");
    printf("|  \\____/   \\___/   \\__, |  \\___/     \\__,_|  \\__,_|       \\/     |_|  \\__,_|  \\__,_| |\n");
    printf("|                    __/ |                                                            |\n");
    printf("|                   |___/                                                             |\n");
    printf("|*************************************************************************************|\n");
    printf("|*********************************** MAIN MENU ***************************************|\n");
    printf("|************************************ Options ****************************************|\n");
    printf("|*************************************************************************************|\n");
    printf("|**      {1} - Insert cell              **       {6} - Advance one generation       **|\n");
    printf("|**                                     **                                          **|\n");
    printf("|**      {2} - Remove cell              **       {7} - Advance n generations        **|\n");
    printf("|**                                     **                                          **|\n");
    printf("|**      {3} - Insert multiple cells    **       {8} - Statistics                   **|\n");
    printf("|**                                     **                                          **|\n");
    printf("|**      {4} - Remove multiple cells    **       {9} - Settings                     **|\n");
    printf("|**                                     **                                          **|\n");
    printf("|**      {5} - Clear grid               **       {0} - Exit                         **|\n");
    printf("*-------------------------------------------------------------------------------------*\n");
    printf("Choose an option: ");

    if (scanf("%d", &choice) != 1) {
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();
    return choice;
}

int showConfigMenu(void) {
    int choice;
    system("cls");
    printf("*-------------------------------------------------------------------------------------*\n");
    printf("|********************************* SETTINGS MENU *************************************|\n");
    printf("|************************************ Options ****************************************|\n");
    printf("|*************************************************************************************|\n");
    printf("|**      {1} - Grid dimensions          **       {3} - Display grid on step         **|\n");
    printf("|**                                     **                                          **|\n");
    printf("|**      {2} - Simulation speed         **       {4} - Display stats on step        **|\n");
    printf("|**                                     **                                          **|\n");
    printf("|**                       {0} - Return to main menu                                 **|\n");
    printf("*-------------------------------------------------------------------------------------*\n");
    printf("Choose an option: ");

    if (scanf("%d", &choice) != 1) {
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();
    return choice;
}
