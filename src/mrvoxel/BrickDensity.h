#ifndef BRICKDENSITY_H_
#define BRICKDENSITY_H_

#include "VolumeRegion.h"
#include "Brick.h"

class BrickDensityRegion : public DensityRegion {
public:
	BrickDensityRegion(Vector min, Vector max, Spectrum absorbtion,
			Spectrum scatter, double greenstein, Spectrum emitt);

	double density( Vector pt );

private:
	BrickGrid m_brickData;
};

#endif /* BRICKDENSITY_H_ */
