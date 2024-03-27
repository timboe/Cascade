#pragma once
#include "game.h"

enum EasingFunction_t {
  EaseLinear,
  EaseInSine,
  EaseOutSine,
  EaseInOutSine,
  EaseInQuad,
  EaseOutQuad,
  EaseInOutQuad,
  EaseInCubic,
  EaseOutCubic,
  EaseInOutCubic,
  EaseInQuart,
  EaseOutQuart,
  EaseInOutQuart,
  EaseInQuint,
  EaseOutQuint,
  EaseInOutQuint,
  EaseInExpo,
  EaseOutExpo,
  EaseInOutExpo,
  EaseInCirc,
  EaseOutCirc,
  EaseInOutCirc,
  EaseInBack,
  EaseOutBack,
  EaseInOutBack,
  EaseInElastic,
  EaseOutElastic,
  EaseInOutElastic,
  EaseInBounce,
  EaseOutBounce,
  EaseInOutBounce,
  NEasingFunctions
};

float getEasing(const enum EasingFunction_t e, const float t);

float easeInSine(float time);
float easeOutSine(float time);
float easeInOutSine(float time);
float easeInQuad(float time);
float easeOutQuad(float time);
float easeInOutQuad(float time);
float easeInCubic(float time);
float easeOutCubic(float time);
float easeInOutCubic(float time);
float easeInQuart(float time);
float easeOutQuad(float time);
float easeInOutQuart(float time);
float easeInQuint(float time);
float easeOutQuint(float time);
float easeInOutQuint(float time);
float easeInExpo(float time);
float easeOutExpo(float time);
float easeInOutExpo(float time);
float easeInCirc(float time);
float easeOutCirc(float time);
float easeInOutCirc(float time);
float easeInBack(float time);
float easeOutBack(float time);
float easeInOutBack(float time);
float easeInElastic(float time);
float easeOutElastic(float time);
float easeInOutElastic(float time);
float easeInBounce(float time);
float easeOutBounce(float time);
float easeInOutBounce(float time);
