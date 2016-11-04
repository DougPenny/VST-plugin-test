#include "WickedDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;
const int size = 88200;
int cursor;
float *buffer;

enum EParams
{
  kThreshold = 0,
  kDelayTime = 1,
  kDelayFeedback = 2,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kThresholdX = 100,
  kThresholdY = 100,
  kKnobFrames = 60,
  
  kDelayTimeX = 200,
  kDelayTimeY = 100,
  kKnobFrames2 = 60,
  
  kDelayFeedbackX = 200,
  kDelayFeedbackY = 200,
  kKnobFrames3 = 60
};

WickedDistortion::WickedDistortion(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mThreshold(1.)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kThreshold)->InitDouble("Threshold", 100.0, 0.01, 100.0, 0.01, "%");
  GetParam(kThreshold)->SetShape(1.0);
  
  GetParam(kDelayTime)->InitDouble("DelayTime", 50.0, 0.01, 99.0, 1.0, "%");
  GetParam(kDelayTime)->SetShape(1.0);
  
  GetParam(kDelayFeedback)->InitDouble("DelayFeedback", 50.0, 0.0, 110.0, 0.01, "%");
  GetParam(kDelayFeedback)->SetShape(1.0);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  //init delay buffer
  buffer = new float[size];
  cursor = 0;
  
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap delay_time_knob = pGraphics->LoadIBitmap(KNOB_ID_2, KNOB_FN_2, kKnobFrames2);
  IBitmap delay_feedback_knob = pGraphics->LoadIBitmap(KNOB_ID_3, KNOB_FN_3, kKnobFrames3);
  
  pGraphics->AttachControl(new IKnobMultiControl(this, kThresholdX, kThresholdY, kThreshold, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kDelayTimeX, kDelayTimeY, kDelayTime, &delay_time_knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kDelayFeedbackX, kDelayFeedbackY, kDelayFeedback, &delay_feedback_knob));
  
  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

WickedDistortion::~WickedDistortion() {}

void WickedDistortion::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  int const channelCount = 2;
  
  for (int i = 0; i < channelCount; i++) {
    double* input = inputs[i];
    double* output = outputs[i];
    
    for (int s = 0; s < nFrames; ++s, ++input, ++output) {
      
      //Distortion
      if(*input >= 0) {
        // Make sure positive values can't go above the threshold:
        *output = fmin(*input, mThreshold);
      } else {
        // Make sure negative values can't go below the threshold:
        *output = fmax(*input, -mThreshold);
      }
      *output /= mThreshold;
      
      //Delay
      int delayTime = round((mDelayTime * size)/2);
      if (cursor >= size) {
        cursor = 0;
      }
      int j = cursor - delayTime;
      if( j < 0 ) {
        j += size;
      }
      buffer[cursor] = *input + (buffer[j] * mDelayFeedback);
      
      (*output) += buffer[cursor];
      
      cursor++;
    }
  }
}

void WickedDistortion::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void WickedDistortion::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kThreshold:
      mThreshold = GetParam(kThreshold)->Value() / 100.0;
      break;

    case kDelayTime:
      buffer = new float[size];
      mDelayTime = GetParam(kDelayTime)->Value() / 100.0;
      break;
      
    case kDelayFeedback:
      mDelayFeedback = GetParam(kDelayFeedback)->Value() / 100.0;
      break;
      
    default:
      break;
  }
}
