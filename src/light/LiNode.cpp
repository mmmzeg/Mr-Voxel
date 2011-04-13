/* 
 * File:   Octree.cpp
 * Author: cgibson
 * 
 * Created on March 2, 2011, 7:05 PM
 */

#include "LiNode.h"
#include "../scene/Geometry.h"
#include "Surfel.h"
#include "sh.h"

#include "../system/config.h"

OctreeNode::OctreeNode( const Vec3 &min, const Vec3 &max ): _min(min), _max(max) {

    // Clear all children
    for(int i = 0; i < 8; i++)
        m_children[i] = NULL;
}

OctreeNode::OctreeNode(const OctreeNode& orig) {

    // Copy all children
    for(int i = 0; i < 8; i++)
        m_children[i] = orig.m_children[i];
}

OctreeNode::~OctreeNode() {
}

void
OctreeNode::cascadeDelete() {

    // Copy all children
    for(int i = 0; i < 8; i++) {
        if(m_children[i] != 0) {
            m_children[i]->cascadeDelete();
            delete m_children[i];
        }
    }
}

int
LiNode::getTestCount( const Ray &ray, int *testCount ) {
    double tmin = -1;
    double thit;
    int tmp;
    Vec3 n;

    if(!OctreeNode::test_intersect( ray, &thit, &n )){
        *testCount = 1;
        return 0;
    }


    if(hasChildren()) {
        vector<pair<double, int> > ch_hit;
        vector<pair<double, int> >::iterator ch_It;
        double tmp_t;
        Vec3 tmp_n;
        for(int i = 0; i < 8; i++) {
            if(child(i)->test_intersect(ray, &tmp_t, &tmp_n)) {
                ch_hit.push_back(pair<double, int>(tmp_t,i));
            }
        }
/*
        int test_tot = 0;
        int tmp_test = 0;
        tmp = 0;
        for(int i = 0; i < 8; i++) {
            tmp += child(i)->getTestCount( ray, &tmp_test);
            test_tot += tmp_test;
        }

        if(tmp > 0) {
            *testCount = test_tot;
            return 1;
        }else{
            *testCount = test_tot;
            return 0;
        }
        //*/
//*
        sort( ch_hit.begin(), ch_hit.end());

        int test_tot = 0;
        int tmp_test = 0;
        for(ch_It = ch_hit.begin(); ch_It != ch_hit.end(); ch_It++) {
            //printf("testing child %d\n", ch_It->second);
            assert(child(ch_It->second) != NULL);
            tmp = child(ch_It->second)->getTestCount( ray, &tmp_test);
            test_tot += tmp_test;

            if(tmp > 0) {
                *testCount = test_tot;
                return 1;
            }
        }
        *testCount = test_tot;
        return 0;
//*/
    }else{
        /**/
        if((_min.x() < ray.start.x() && _max.x() > ray.start.x()) &&
           (_min.y() < ray.start.y() && _max.y() > ray.start.y()) &&
           (_min.z() < ray.start.z() && _max.z() > ray.start.z())) {
           *testCount = 1;
           return 0;
        }
        *testCount = 1;

        for(int i = 0; i < _surfelCount; i++) {

            Ray tmpRay;
            assert(_surfelData[i].get() != NULL);
            shared_ptr<Surfel> s(_surfelData[i]);
            tmpRay.start = s->matrix * Vec4(ray.start, 1);
            tmpRay.direction = s->matrix * Vec4(ray.direction, 0);
            tmpRay.direction.norm();

            // Would be usefull if we wanted to suck at cheating
            if(s->normal() * (ray.direction * -1) > 0) {
                if(s->test_intersect(tmpRay, &thit, &n)) {
                    if(thit < tmin || tmin < 0.) {
                        return 1;
                    }
                }
            }
        }
    }

    *testCount = 1;

    return false;
}

inline int
OctreeNode::test_intersect( const Ray &ray, double *t, Vec3 * const n ) {

    double tmp_t;

    return test_intersect_region(ray, _min, _max, t, &tmp_t);
    
}

Color
LiNode::gather( const Ray &ray, double *t ) {
    double tmin = -1;
    double thit;
    Color closest = config::background;
    Color tmp;
    Vec3 n;

    if(!OctreeNode::test_intersect( ray, &thit, &n )){
        *t = -1;
        return config::background;
    }


    if(hasChildren()) {
        vector<pair<double, int> > ch_hit;
        vector<pair<double, int> >::iterator ch_It;
        double tmp_t;
        Vec3 tmp_n;
        for(int i = 0; i < 8; i++) {
            if(child(i)->test_intersect(ray, &tmp_t, &tmp_n)) {
                ch_hit.push_back(pair<double, int>(tmp_t,i));
            }
        }

        sort( ch_hit.begin(), ch_hit.end());

        for(ch_It = ch_hit.begin(); ch_It != ch_hit.end(); ch_It++) {
            //printf("testing child %d\n", ch_It->second);
            assert(child(ch_It->second) != NULL);
            tmp = child(ch_It->second)->gather( ray, &thit);
            /*if(thit < tmin) {
                tmin = thit;
                closest = tmp;
            }*/
            if(thit >= 0) {
                *t = thit;
                return tmp;
            }
        }

    }else{
        /**/
        if((_min.x() < ray.start.x() && _max.x() > ray.start.x()) &&
           (_min.y() < ray.start.y() && _max.y() > ray.start.y()) &&
           (_min.z() < ray.start.z() && _max.z() > ray.start.z())) {
           *t = -1;
           return config::background;
        }

        for(int i = 0; i < _surfelCount; i++) {

            Ray tmpRay;
            assert(_surfelData[i].get() != NULL);
            shared_ptr<Surfel> s(_surfelData[i]);
            tmpRay.start = s->matrix * Vec4(ray.start, 1);
            tmpRay.direction = s->matrix * Vec4(ray.direction, 0);
            tmpRay.direction.norm();

            // Would be usefull if we wanted to suck at cheating
            if(s->normal() * (ray.direction * -1) > 0) {
                if(s->test_intersect(tmpRay, &thit, &n)) {
                    if(thit < tmin || tmin < 0.) {
                        tmin = thit;
                        closest = s->diffuse();
                        //light::sh::reconstruct(ray.direction * -1, 2, sh_c);//
                    }
                }
            }
        }
    }

    *t = tmin;

    return closest;
}

int
LiNode::subdivide() {

    Vec3 size = (_max - _min) * 0.5;

    if(size.length() < 0.00000001) {
        printf("SIZE: %d\n", _surfelCount);
        for(int i = 0; i < _surfelCount; i++) {
            Surfel surf = *_surfelData[i];
            printf("\t%s\n", surf.position().str());
        }
        printf("ERROR\n\tMIN: %s\n\tMAX: %s\n", _min.str(), _max.str());
        exit(1);
    }
    assert(size.length() > 0.00000001);
    Vec3 min, max;
    int x,y,z;

    // Loop over children
    for(int i = 0; i < 8; i++) {
        // Find out which child this is (in terms of what quadrant)
        x = i%2;
        y = (i/2)%2;
        z = (i/4);

        // Define minimum by which quadrant it is in
        min = Vec3(_min.x() + (size.x() * x),
                     _min.y() + (size.y() * y),
                     _min.z() + (size.z() * z));

        // Max is always the minimum plus the size of the node
        max = min + size;

        
        m_children[i] = new LiNode(min, max);
    }

    int surfCount = _surfelCount;

    //_surfelCount = 0;

    // Iterate over all existing surfel data
    for(int i = 0; i < surfCount; i++) {
        //printf("Re-Adding %s!\n", _surfelData[i]->position().str());
        // Re-add the surfel, allowing it to recurse into the children
        add(_surfelData[i]);
    }
    // Clear all surfel data in this node
    return 0;
}

int
LiNode::size_of() {
    int size = 0;

    if(hasChildren()) {
        for(int i = 0; i < 8; i++) {
            size += child(i)->size_of();
        }
            size += sizeof(m_children);
            return size;
    }else{
        return sizeof(_surfelData) + (sizeof(shared_ptr<Surfel>) + sizeof(Surfel)) * _surfelCount;
    }

    return size;
}

int
LiNode::count() {
    int size = 0;

    if(hasChildren()) {
        for(int i = 0; i < 8; i++) {
            size += child(i)->count();
        }
        return size;
        
    }else{
        return _surfelCount;
    }
}

LiNode::LiNode(Vec3 min, Vec3 max):OctreeNode(min,max), _surfelCount(0) {
    
    // Allocate data enough for the maximum amount of surfels
    _surfelData = new shared_ptr<Surfel>[MAX_SURFEL_COUNT];

   // printf("GENERATING NODE: %s - %s\n", m_min.str(), m_max.str());
    
}

int
LiNode::add(const shared_ptr<Surfel> obj) {

        //printf("Adding to NODE: %s - %s\n", m_min.str(), m_max.str());

    if(!inside(obj))
        return 0;
    
    // If this is a branch node
    if(hasChildren()) {
        //printf("Recursing\n");
        int res = 0;
        // Try adding it to every child node
        for(int i = 0; i < 8; i++) {
            
            res += child(i)->add(obj);
        }

        // Keep track of how many surfels are underneath a given node
        _surfelCount++;
        //printf("Surfel count: %d\n", _surfelCount);
        // Any non-zero failures will be recorded
        return res;

    }else{
        //printf("Hit a leaf node\n");
    // Otherwise we are a leaf node
        // Check if there are more surfels than we can hold
        if(_surfelCount >= MAX_SURFEL_COUNT) {
            subdivide();
            // Re-add this element.  It will recurse in this time
            return add(obj);
        }else{
            //printf("Adding %s!\n", obj->position().str());
            _surfelData[_surfelCount] = obj;
            _surfelCount++;
            //printf("Surfel count: %d\n", _surfelCount);

            return 0;
        }
    }
}


bool
LiNode::inside(const shared_ptr<Surfel> obj) {
    float dmin = 0;

    const Vec3 sphere_pos = obj->position();

    if(sphere_pos.x() > _min.x() && sphere_pos.x() < _max.x() &&
            sphere_pos.y() > _min.y() && sphere_pos.y() < _max.y() &&
            sphere_pos.z() > _min.z() && sphere_pos.z() < _max.z())
        return true;

    // X axis
    if( sphere_pos.x() < _min.x()) dmin += sqrt( sphere_pos.x() - _min.x()); else
    if( sphere_pos.x() > _max.x()) dmin += sqrt( sphere_pos.x() - _max.x());

    // Y axis
    if( sphere_pos.y() < _min.y()) dmin += sqrt( sphere_pos.y() - _min.y()); else
    if( sphere_pos.y() > _max.y()) dmin += sqrt( sphere_pos.y() - _max.y());

    // Z axis
    if( sphere_pos.z() < _min.z()) dmin += sqrt( sphere_pos.z() - _min.z()); else
    if( sphere_pos.z() > _max.z()) dmin += sqrt( sphere_pos.z() - _max.z());

    if( dmin <= obj->area() ) return true;

    return false;
}


int
LiNode::clear() {
    if(_surfelData != NULL) {
        delete _surfelData;
    }
}

void
LiNode::postprocess() {

    // If a branch
    if(hasChildren()) {
        // Process children first
        for( int i = 0; i < 8; i++) {
            child(i)->postprocess();
        }
        // Sum children SH coefficients
        // TODO: Later

    // If a leaf
    }else{
        //printf("SURF: %d\n", _surfelCount);
        if(_surfelCount == 0) {
            return;
        }

        assert(_surfelData[0] != NULL);

        // Process lighting using surfel list
        vector<shared_ptr<Surfel> > surf_list;
        for(int i = 0; i < _surfelCount; i++) {
            
            surf_list.push_back(shared_ptr<Surfel>());
            surf_list[i] = _surfelData[i];
            assert(_surfelData[i] != NULL);
        }
        light::sh::SHProject(surf_list, 2, sh_c);

        assert(_surfelData[0] != NULL);
        /*
        printf("NODE FINISHED:\n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s \n\t%s\n",
                sh_c[0].str(),sh_c[1].str(),sh_c[2].str(),sh_c[3].str(),sh_c[4].str(),sh_c[5].str(),sh_c[6].str(),sh_c[7].str(),sh_c[8].str());
        //* */
    }
}