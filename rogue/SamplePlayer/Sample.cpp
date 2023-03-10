#include "Sample.h"
#include <string.h>
#include "daisy_seed.h"

using namespace daisysp;

Sample::Sample(void) {

    numSamples = 0;
    pBuffer = nullptr;
    playing = false;
}

void Sample::init( float * pBuf, uint32_t size) {

    pBuffer = pBuf;
    playing = false;
    loopFlag = false;
    readPtr = 0;
    for (uint32_t s = 0; s < size; s++)
        pBuffer[s] = 0.0f;
}

void Sample::start(void) {

    if (pBuffer != nullptr) {
        readPtr = 0;
        playing = true;
    }
}

bool Sample::getSamples(float * pDst, uint32_t n) {

    float * destPtr = pDst;
    uint32_t remainingSamples = numSamples - readPtr;
    if (n <= remainingSamples) {
        memcpy(destPtr, (float *)&pBuffer[readPtr], n * sizeof(float));
        readPtr += n;
        return false;
    }
    else {
        memcpy(destPtr, (float *)&pBuffer[readPtr], remainingSamples * sizeof(float));
        destPtr += remainingSamples;
        if (loopFlag) {
            memcpy(destPtr, (float *)&pBuffer[0], (n - remainingSamples) * sizeof(float));
            readPtr = n - remainingSamples;
            return false;
        }
        else {
            memset(destPtr, 0, (n - remainingSamples) * sizeof(float));
            playing = false;
            return true;
        }
    }   
}