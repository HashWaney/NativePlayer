/**
 * Created by Hash on 2020-04-21.
 */


#include "PcmSplitEntity.h"

PcmSplitEntity::PcmSplitEntity(SAMPLETYPE *sampleBuffer, int size) {
    this->buffer = static_cast<char *>(malloc(size));
    this->bufferSize = size;
    memcpy(buffer, sampleBuffer, size);

}

PcmSplitEntity::~PcmSplitEntity() {
    free(buffer);
    buffer = NULL;


}
