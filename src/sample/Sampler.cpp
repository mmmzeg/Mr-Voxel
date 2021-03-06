/* 
 * File:   Sampler.cpp
 * Author: cgibson
 * 
 * Created on March 24, 2011, 4:59 PM
 */

#include <stdlib.h>

#include "Sampler.h"

namespace sample{

    vector<float> stratefy1D(int samples) {
        //TODO: Implement
    }

    // From http://www.rorydriscoll.com/2009/01/07/better-sampling/
    Vec3 sampleToHCoord(float us, float ts) {

        const float r = sqrt(us);
        const float theta = 2 * PI * ts;

        const float x = r * cos(theta);
        const float y = r * sin(theta);

        return Vec3(x, y, sqrt(1.0f - us));

    }



    // From http://www.rorydriscoll.com/2009/01/07/better-sampling/
    Vec3 sampleToHCoordX(float us, float ts) {

        const float r = sin(us * PI / 2.);
        const float theta = 2 * PI * ts;

        const float x = r * cos(theta);
        const float y = r * sin(theta);

        return Vec3(x, y, sin((PI / 2.) - (us * PI / 2.)));

    }

    HemisphereSampler::HemisphereSampler(Vec3 normal, int us, int ts)
        : _max_us(us), _max_ts(ts), _us(0), _ts(0), _normal(normal) {

        Vec3 up = Vec3(0,1,0);
        Vec3 w = normal;
        Vec3 u;
        Vec3 v;
        w.norm();
        //w = w * -1;

        if(w.y() >= 0.9995 || w.y() <= -0.995) {
        u = Vec3(1,0,0);
        v = Vec3(0,0,1);
        }else{
        up.cross(w, &u);
        u.norm();
        w.cross(u, &v);
        }

        //cout << "Camera Loc: " << endl << m1 << endl;

        MyMat m = MyMat(u.x(), v.x(), w.x(), 0,
                   u.y(), v.y(), w.y(), 0,
                   u.z(), v.z(), w.z(), 0,
                   0, 0, 0, 1);

        _matrix = m;
        //cout << "UVW: " << endl << m2 << endl;

        //_matrix = m.inverse();
    }

    bool
    HemisphereSampler::getSample(Vec3* sample) {
        // End of the line
        if(_ts == _max_ts){ /*printf("DONE\n");*/return false;}

        // Jitter calculation
        const float jitter_us = (drand48() - 0.f) * (1. / _max_us);
        const float jitter_ts = (drand48() - 0.f) * (1. / _max_ts);

        const Vec3 tmpSample = sampleToHCoord((_us / (float)_max_us) + jitter_us, (_ts / (float)_max_ts) + jitter_ts);

        *sample = _matrix * Vec4(tmpSample,0);
        sample->norm();

        //printf("\tsampling %d %d\n", _us, _ts);
        // Incrememt for next sample
        if(++_us == _max_us) {
            _us = 0;
            ++_ts;
        }

        return true;
    }
}