#ifndef __WICKEDDISTORTION__
#define __WICKEDDISTORTION__

#include "IPlug_include_in_plug_hdr.h"

class WickedDistortion : public IPlug
{
public:
  WickedDistortion(IPlugInstanceInfo instanceInfo);
  ~WickedDistortion();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mThreshold;
  double mDelayTime;
  double mDelayFeedback;
};

#endif
