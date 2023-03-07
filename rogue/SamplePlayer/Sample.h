#pragma once

namespace daisysp
{

class Sample
{
  public:
     Sample() {}
    ~Sample() {}

    void Init(float * pBuf)     { pBuffer = pBuf; }
    void Load(void);
 
  private:
    float* pBuffer;

};

} // namespace daisysp
