#pragma once

namespace daisysp
{

class Sample
{
  public:
     Sample();
    ~Sample() {;}

    void init(float * pBuf, uint32_t size);
    void setNumSamples(uint32_t n)      { numSamples = n; }
    uint32_t getNumSamples(void)        { return numSamples; }
    void start(void);
    void stop(void)                     { playing = false; }
    void setLoopFlag(bool looping)      { loopFlag = looping; }
    bool getSamples(float * pDst, uint32_t n);
 
  private:

    bool valid;
    bool playing;
    bool loopFlag;
    uint32_t readPtr;
    uint32_t numSamples;
    float* pBuffer;

};

} // namespace daisysp
