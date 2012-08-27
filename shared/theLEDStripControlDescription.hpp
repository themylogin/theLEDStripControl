#pragma once

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <stdint.h>
#include <string>
#include <vector>

struct theLEDStripControlDescription
{
    typedef struct {
        uint8_t r, g, b;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            (void) version;

            ar & r;
            ar & g;
            ar & b;
        }
    } color;

    std::vector<color> colors;
    int glowSpeed;
    std::string filter;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        (void) version;

        ar & colors;
        ar & glowSpeed;
        ar & filter;
    }
};
BOOST_CLASS_VERSION(theLEDStripControlDescription, 1)
