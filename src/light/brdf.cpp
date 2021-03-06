/* 
 * File:   brdf.cpp
 * Author: cgibson
 * 
 * Created on March 27, 2011, 11:51 AM
 */

#include "brdf.h"

double max(double a, double b)
{
  return (a > b) ? a : b;
}

namespace light{

    Color shadeIndirect(Surface &surface, bool gather = true, bool ambient = true) {
        Color result = 0.;

        float pdf = 1. / (4. * PI);
        
        if(gather && config::ambience == AMBIENT_FULL) {
            double tt;
            Color amb;

            // Generate a sampler with a basis on the surface normal
            HemisphereSampler sampler = HemisphereSampler(surface.n, config::hemisphere_u, config::hemisphere_t);

            double rndn;
            Vec3 smpl;

            float sample_mul = pdf * (config::hemisphere_u*config::hemisphere_t);

            // Gather samples until the sampler runs out
            while(sampler.getSample(&smpl)) {

                rndn = (smpl * surface.n);
                amb = config::scenePtr->lightCache()->gather(Ray(surface.p + (smpl * 0.0), smpl), &tt) * rndn;

                result = result + amb * surface.finish.ambient * surface.color / sample_mul;
            }

        } else if(config::ambience == AMBIENT_FLAT && ambient){
            result = result + surface.color * surface.finish.ambient;
        } else {
            //nothing.
        }

        return result;
    }

    Color shadeDiffuse(Vec3 &V, Surface &surf, bool specular = true) {

        Color result = 0.;
        
        // Grab light sources
        LightSource** lights = config::scenePtr->getLightSources();

        Ray shadow_ray;

        // Iterate through each light
        for(int i = 0; i < config::scenePtr->getLightSourceCount(); i++) {

            // Grab temporary light pointer
            LightSource *light = lights[i];

            // Light position, relative to surface
            Vec3 L = light->position - surf.p;
            double l_dist = L.norm();

            // Generate shadow ray
            shadow_ray = Ray(surf.p, L);

            Surface surface2;

            // Beginning transmittance is 1
            Color Tr(1.);

            // If an object is in the way
            if(config::scenePtr->intersect(shadow_ray, &surface2) && (surface2.t <= l_dist)) {
                Tr = 0.0;

            // Otherwise, integrate through all volumes
            }else{
                Tr = config::volume_integrator->Transmittance( shadow_ray );
            }

            result = result + light::brdf(V, L, surf, light, Tr, specular);
        }

        return result;
        
    }

    Color brdf(Vec3 &V, Vec3 &L, Surface &surf, LightSource *light, Color &lightTr, bool specular = true) {
        
        Vec3 H = V + L;
        H.norm();

        // invert shininess so it makes sense
        double shininess = 1 / surf.finish.roughness;

        Color result = 0.;
        result = ((surf.color * light->color * lightTr) * surf.finish.diffuse * max(0., surf.n * L));

        if(config::specular && specular && !lightTr.isBlack() && L.dot(surf.n) > 0) {
            result = result + (light->color * surf.finish.specular * lightTr * pow(max(0, H * surf.n), shininess));
        }

        return result;
    }

}

