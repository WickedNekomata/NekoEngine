#ifndef __PANEL_TEST_PCG_H__
#define __PANEL_TEST_PCG_H__

#include "Panel.h"
#include <vector>

#include "PCG/pcg_variants.h"

class PanelTestPCG : public Panel
{
public:

	PanelTestPCG(char* name);
	virtual ~PanelTestPCG();

	virtual bool Draw();

private:

	int rng = 0;
	pcg_state_setseq_64 rngBound;
	pcg32_random_t rngSeedFloat;

	std::vector<float> randomIntNumbers;
	std::vector<float> randomFloatNumbers;
	bool plotBoundedHistogram = false;
	bool plotFloatHistogram = false;
};

#endif

