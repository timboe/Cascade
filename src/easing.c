#include "easing.h"
#include <stdlib.h>

float getEasing(const enum EasingFunction_t e, const float t) {
  switch (e) {
    case EaseLinear: return t;
    case EaseInSine: return easeInSine(t);
    case EaseOutSine: return easeOutSine(t);
    case EaseInOutSine: return easeInOutSine(t);
    case EaseInQuad: return easeInQuad(t);
    case EaseOutQuad: return easeOutQuad(t);
    case EaseInOutQuad: return easeInOutQuad(t);
    case EaseInCubic: return easeInCubic(t);
    case EaseOutCubic: return easeOutCubic(t);
    case EaseInOutCubic: return easeInOutCubic(t);
    case EaseInQuart: return easeInQuart(t);
    case EaseOutQuart: return easeOutQuad(t);
    case EaseInOutQuart: return easeInOutQuart(t);
    case EaseInQuint: return easeInQuint(t);
    case EaseOutQuint: return easeOutQuint(t);
    case EaseInOutQuint: return easeInOutQuint(t);
    case EaseInExpo: return easeInExpo(t);
    case EaseOutExpo: return easeOutExpo(t);
    case EaseInOutExpo: return easeInOutExpo(t);
    case EaseInCirc: return easeInCirc(t);
    case EaseOutCirc: return easeOutCirc(t);
    case EaseInOutCirc: return easeInOutCirc(t);
    case EaseInBack: return easeInBack(t);
    case EaseOutBack: return easeOutBack(t);
    case EaseInOutBack: return easeInOutBack(t);
    case EaseInElastic: return easeInElastic(t);
    case EaseOutElastic: return easeOutElastic(t);
    case EaseInOutElastic: return easeInOutElastic(t);
    case EaseInBounce: return easeInBounce(t);
    case EaseOutBounce: return easeOutBounce(t);
    case EaseInOutBounce: return easeInOutBounce(t);
    default: return t;
  }
  return t;
}

float easeInSine(float t) {
  return sinf(1.5707963f * t);
}

float easeOutSine(float t) {
  return 1.0f + sinf(1.5707963f * (--t));
}

float easeInOutSine(float t) {
  return 0.5f * (1.0f + sinf(3.1415926f * (t - 0.5f)));
}

float easeInQuad(float t) {
  return t * t;
}

float easeOutQuad(float t) { 
  return t * (2.0f - t);
}

float easeInOutQuad(float t) {
  return t < 0.5f ? 2.0f * t * t : t * (4.0f - 2.0f * t) - 1.0f;
}

float easeInCubic(float t) {
  return t * t * t;
}

float easeOutCubic(float t) {
  return 1.0f + (--t) * t * t;
}

float easeInOutCubic(float t) {
  return t < 0.5f ? 4.0f * t * t * t : 1.0f + (--t) * (2.0f * (--t)) * (2.0f * t);
}

float easeInQuart(float t) {
  t *= t;
  return t * t;
}

float easeOutQuart(float t) {
  t = (--t) * t;
  return 1.0f - t * t;
}

float easeInOutQuart(float t) {
  if(t < 0.5f) {
    t *= t;
    return 8.0f * t * t;
  } else {
    t = (--t) * t;
    return 1.0f - 8.0f * t * t;
  }
}

float easeInQuint(float t) {
  float t2 = t * t;
  return t * t2 * t2;
}

float easeOutQuint(float t) {
  float t2 = (--t) * t;
  return 1.0f + t * t2 * t2;
}

float easeInOutQuint(float t) {
  float t2 = 0.0f;
  if(t < 0.5f) {
    t2 = t * t;
    return 16.0f * t * t2 * t2;
  } else {
    t2 = (--t) * t;
    return 1.0f + 16.0f * t * t2 * t2;
  }
}

float easeInExpo(float t) {
  return (powf(2.0f, 8.0f * t) - 1.0f) / 255.0f;
}

float easeOutExpo(float t) {
  return 1.0f - powf(2.0f, -8.0f * t);
}

float easeInOutExpo(float t) {
  if(t < 0.5f) {
    return (powf(2.0f, 16.0f * t) - 1.0f) / 510.0f;
  } else {
    return 1.0f - 0.5f * powf(2.0f, -16.0f * (t - 0.5f));
  }
}

float easeInCirc(float t) {
  return 1.0f - sqrtf(1.0f - t);
}

float easeOutCirc(float t) {
  return sqrtf(t);
}

float easeInOutCirc(float t) {
  if(t < 0.5f) {
    return (1.0f - sqrtf(1.0f - 2.0f * t)) * 0.5f;
  } else {
    return (1.0f + sqrtf(2.0f * t - 1.0f)) * 0.5f;
  }
}

float easeInBack(float t) {
  return t * t * (2.70158f * t - 1.70158f);
}

float easeOutBack(float t) {
  return 1.0f + (--t) * t * (2.70158f * t + 1.70158f);
}

float easeInOutBack(float t) {
  if(t < 0.5f) {
    return t * t * (7.0f * t - 2.5f) * 2.0f;
  } else {
    return 1.0f + (--t) * t * 2.0f * (7.0f * t + 2.5f);
  }
}

float easeInElastic(float t) {
  float t2 = t * t;
  return t2 * t2 * sinf(t * M_PIf * 4.5f);
}

float easeOutElastic(float t) {
  float t2 = (t - 1.0f) * (t - 1.0f);
  return 1.0f - t2 * t2 * cosf(t * M_PIf * 4.5f);
}

float easeInOutElastic(float t) {
  float t2 = 0.0f;
  if(t < 0.45f) {
    t2 = t * t;
    return 8.0f * t2 * t2 * sinf(t * M_PIf * 9.0f);
  } else if(t < 0.55f) {
    return 0.5f + 0.75f * sinf(t * M_PIf * 4.0f);
  } else {
    t2 = (t - 1.0f) * (t - 1.0f);
    return 1.0f - 8.0f * t2 * t2 * sinf(t * M_PIf * 9.0f);
  }
}

float easeInBounce(float t) {
  return powf(2.0f, 6.0f * (t - 1.0f)) * fabsf(sinf(t * M_PIf * 3.5f));
}

float easeOutBounce(float t) {
  return 1.0f - powf(2.0f, -6.0f * t) * fabsf(cosf(t * M_PIf * 3.5f));
}

float easeInOutBounce(float t) {
  if(t < 0.5f) {
    return 8.0f * powf(2.0f, 8.0f * (t - 1.0f)) * fabsf(sinf(t * M_PIf * 7.0f));
  } else {
    return 1.0f - 8.0f * powf(2.0f, -8.0f * t) * fabsf(sinf(t * M_PIf * 7.0f));
  }
}