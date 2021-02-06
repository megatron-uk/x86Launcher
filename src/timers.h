
#define ARTWORK_FIRE		500		// Artwork display fires after this amount of timeout after the last user input

void timers_Print(clock_t start, clock_t end, char* name, int enabled);
int timers_FireArt(clock_t last);
