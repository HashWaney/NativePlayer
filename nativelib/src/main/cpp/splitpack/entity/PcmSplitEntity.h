/**
 * Created by Hash on 2020-04-21.
 */


#ifndef AUDIOPLAYER_PCMSQLITEENTITY_H
#define AUDIOPLAYER_PCMSQLITEENTITY_H

#include "SoundTouch.h"

using namespace soundtouch;

class PcmSplitEntity {

public:
    char *buffer;
    int bufferSize;



public:
    PcmSplitEntity(SAMPLETYPE* sampleBuffer,int size);

    ~PcmSplitEntity();

};


#endif //AUDIOPLAYER_PCMSQLITEENTITY_H
