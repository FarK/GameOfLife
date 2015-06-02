#ifndef GOL_H_
#define GOL_H_

#include "world.h"

#define RULE_1 (1<<0)
#define RULE_2 (1<<1)
#define RULE_3 (1<<2)
#define RULE_4 (1<<3)
#define RULE_5 (1<<4)
#define RULE_6 (1<<5)
#define RULE_7 (1<<6)
#define RULE_8 (1<<7)

struct GOL;

struct Rule {
	unsigned char birth;
	unsigned char survive;
};

static const struct Rule rule_B3S23 = {
	RULE_3,
	RULE_2 | RULE_3
};

struct GOL *golInit(unsigned int numThreads, const struct Rule *rule,
	struct World *world);
void golEnd(struct GOL *gol);
void iteration(struct GOL *gol);
inline void gol_reviveCell(wsize_t x, wsize_t y, struct GOL *gol);
inline void gol_killCell(wsize_t x, wsize_t y, struct GOL *gol);

#endif
