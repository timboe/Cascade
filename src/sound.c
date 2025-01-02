#include "sound.h"
#include "io.h"
#include "board.h"

bool m_doSfx = true;
bool m_doMusic = true;

bool m_hasMusic = true;

bool m_doingExplosion = false;

int8_t m_trackPlaying = -1;
int8_t m_wfPlaying = -1;

uint8_t m_plingID = 0;
int32_t m_plingTimer = 0;

FilePlayer* m_music[N_MUSIC_TRACKS];
FilePlayer* m_waterfalls[N_WF_TRACKS];

SamplePlayer* m_samplePlayer[kNSFX];
AudioSample* m_audioSample[kNSFX];

void musicStopped(SoundSource* _unused, void* _unused2);

/// ///

void soundSetDoingExplosion(const bool expOn) {
  m_doingExplosion = expOn;
}

void soundResetPling() {
  m_plingTimer = 0;
  m_doingExplosion = false; // backup
}

void soundSetDoMusic(const bool doit) {
  if (!m_hasMusic) return;
  m_doMusic = doit;
  if (!m_doMusic) {
    for (int32_t i = 0; i < N_WF_TRACKS; ++i) {
      pd->sound->fileplayer->stop(m_waterfalls[i]);
    }
    for (int32_t i = 0; i < N_MUSIC_TRACKS; ++i) {
      pd->sound->fileplayer->stop(m_music[i]);
    }
    m_wfPlaying = -1;
    m_trackPlaying = -1;
  } else {
    soundDoWaterfall( IOGetCurrentLevel() );
  }
}

void soundSetDoSfx(const bool doit) {
  m_doSfx = doit;
}

void soundWaterfallDoInit() {
  if (!m_hasMusic) return;
  pd->sound->fileplayer->play(m_waterfalls[0], 0);
  m_wfPlaying = 0;
}

void soundDoWaterfall(const uint8_t id) {
  #ifdef DEV
  pd->system->logToConsole("soundDoWaterfall called for %i", (int)id);
  #endif
  if (!m_hasMusic || (id % N_WF_TRACKS) == m_wfPlaying) return;
  for (int32_t i = 0; i < N_WF_TRACKS; ++i) {
    pd->sound->fileplayer->stop(m_waterfalls[i]);
  }
  m_wfPlaying = id % N_WF_TRACKS;
  pd->sound->fileplayer->play(m_waterfalls[m_wfPlaying], 0);
}

void soundDoMusic() {
  if (!m_hasMusic) return;
  int8_t track = -1;
  while (track == -1 || track == m_trackPlaying) {
    track = rand() % N_MUSIC_TRACKS;
  }
  m_trackPlaying = track;
  for (int32_t i = 0; i < N_MUSIC_TRACKS; ++i) {
    pd->sound->fileplayer->stop(m_music[i]);
  }
  pd->sound->fileplayer->play(m_music[m_trackPlaying], 1); 
}

void musicStopped(SoundSource* _unused, void* _unused2) {
  if (!m_hasMusic) return;
  if (IOIsCredits()) { soundDoMusic(); }
}

void soundPlayMusic(const uint8_t id) {
  pd->sound->fileplayer->play(m_music[id % N_MUSIC_TRACKS], 1);
}

void soundDoInit() {

  m_audioSample[kPlingSfx1] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__1");
  m_audioSample[kPlingSfx2] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__2");
  m_audioSample[kPlingSfx3] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__3");
  m_audioSample[kPlingSfx4] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__4");
  m_audioSample[kPlingSfx5] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__5");
  m_audioSample[kPlingSfx6] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__6");
  m_audioSample[kPlingSfx7] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__7");
  m_audioSample[kPlingSfx8] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__8");
  m_audioSample[kPlingSfx9] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__9");
  m_audioSample[kPlingSfx10] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__10");
  m_audioSample[kPlingSfx11] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__11");
  m_audioSample[kPlingSfx12] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__12");
  m_audioSample[kPlingSfx13] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__13");
  m_audioSample[kPlingSfx14] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__14");
  m_audioSample[kPlingSfx15] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__15");
  m_audioSample[kPlingSfx16] = pd->sound->sample->load("fx/760664__gutertonwav__guterton-cup-ding__16");

  m_audioSample[kDingSfx1] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__1");
  m_audioSample[kDingSfx2] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__2");
  m_audioSample[kDingSfx3] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__3");
  m_audioSample[kDingSfx4] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__4");
  m_audioSample[kDingSfx5] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__5");
  m_audioSample[kDingSfx6] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__6");
  m_audioSample[kDingSfx7] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__7");
  m_audioSample[kDingSfx8] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__8");
  m_audioSample[kDingSfx9] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__9");
  m_audioSample[kDingSfx10] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__10");
  m_audioSample[kDingSfx11] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__11");
  m_audioSample[kDingSfx12] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__12");
  m_audioSample[kDingSfx13] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__13");
  m_audioSample[kDingSfx14] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__14");
  m_audioSample[kDingSfx15] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__15");
  m_audioSample[kDingSfx16] = pd->sound->sample->load("fx/760661__gutertonwav__guterton-bowl-ding__16");

  m_audioSample[kSplashSfx1] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__1");
  m_audioSample[kSplashSfx2] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__2");
  m_audioSample[kSplashSfx3] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__3");
  m_audioSample[kSplashSfx4] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__4");
  m_audioSample[kSplashSfx5] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__5");

  m_audioSample[kWhooshSfx1] = pd->sound->sample->load("fx/682473__simosc__whoosh");
  m_audioSample[kWhooshSfx2] = pd->sound->sample->load("fx/682473__simosc__whoosh");
  m_audioSample[kWhooshSfx3] = pd->sound->sample->load("fx/682473__simosc__whoosh");
  m_audioSample[kWhooshSfx4] = pd->sound->sample->load("fx/682473__simosc__whoosh");

  m_audioSample[kPopSfx1] = pd->sound->sample->load("fx/613608__pellepyb__pop-sound__1");
  m_audioSample[kPopSfx2] = pd->sound->sample->load("fx/613608__pellepyb__pop-sound__2");
  m_audioSample[kPopSfx3] = pd->sound->sample->load("fx/613608__pellepyb__pop-sound__3");
  m_audioSample[kExplosionSfx] = pd->sound->sample->load("fx/explosion");
  m_audioSample[kBallClinkSfx] = pd->sound->sample->load("fx/ballClink");
  m_audioSample[kDrumRollSfx1] = pd->sound->sample->load("fx/705223__therandomsoundbyte2637__snare-drum-buzz-and-cymbal__1");
  m_audioSample[kDrumRollSfx2] = pd->sound->sample->load("fx/705223__therandomsoundbyte2637__snare-drum-buzz-and-cymbal__2");
  m_audioSample[kTeleportSfx] = pd->sound->sample->load("fx/teleport");
  m_audioSample[kSplitSfx] = pd->sound->sample->load("fx/ballSplit");
  m_audioSample[kChargeSfx] = pd->sound->sample->load("fx/587620__chungus43a__8-bit-laser-charging");
  m_audioSample[kPootSfx] = pd->sound->sample->load("fx/441373__matrixxx__heavy-artillery-shot");
  m_audioSample[kRelocateTurretSfx] =  pd->sound->sample->load("fx/134935__ztrees1__whoosh");
  m_audioSample[kTumblerClickSfx] =  pd->sound->sample->load("fx/tumblerClick");
  m_audioSample[kCrankClickSfx] =  pd->sound->sample->load("fx/crankClick");
  m_audioSample[kFizzleSfx] =  pd->sound->sample->load("fx/133448__chaosportal__cigarette-sizzle-01");
  m_audioSample[kBoingSfx1] =  pd->sound->sample->load("fx/201260__empraetorius__water-bottle-boing__1");
  m_audioSample[kBoingSfx2] =  pd->sound->sample->load("fx/201260__empraetorius__water-bottle-boing__2");

  for (int32_t i = 0; i < kNSFX; ++i) {
    m_samplePlayer[i] = pd->sound->sampleplayer->newPlayer();
    pd->sound->sampleplayer->setSample(m_samplePlayer[i], m_audioSample[i]);
  }

  m_hasMusic = true;
  for (int32_t i = 0; i < N_MUSIC_TRACKS; ++i) {
    m_music[i] = pd->sound->fileplayer->newPlayer();
    switch (i) {
      case 0:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517962__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-15"); break;
      case 1:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517963__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-14"); break;
      case 2:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517964__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-13"); break;
      case 3:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517968__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-7"); break;
      case 4:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517969__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-8"); break;
      case 5:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517971__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-3"); break;
      case 6:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517974__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-5"); break;
      case 7:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517977__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-4"); break;
      case 8:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517979__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-2"); break;
      case 9:  m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517983__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-23"); break;
      case 10: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517985__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-22"); break;
      case 11: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517990__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-9"); break;
    }
    pd->sound->fileplayer->setFinishCallback(m_music[i], musicStopped, NULL);
    pd->sound->fileplayer->setBufferLength(m_music[i], 1.0f); 
  }

  const float WF_VOLUMES[] = {0.25f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
  for (int32_t i = 0; i < N_WF_TRACKS; ++i) {
    m_waterfalls[i] = pd->sound->fileplayer->newPlayer();
    switch (i) {
      case 0: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_waterfalls[i], "tracks/690211__nox_sound__ambiance_stream_big_seljalandsfoss_loop_stereo_02"); break;
      case 1: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_waterfalls[i], "tracks/690213__nox_sound__ambiance_stream_light_skaftafell_loop_stereo_02"); break;
      case 2: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_waterfalls[i], "tracks/690214__nox_sound__ambiance_stream_moderate_seljalandsfoss_loop_stereo"); break;
      case 3: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_waterfalls[i], "tracks/690218__nox_sound__ambiance_stream_moderate_skogafoss_close_loop_stereo_02"); break;
      case 4: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_waterfalls[i], "tracks/690221__nox_sound__ambiance_waterfall_big_seljalandsfoss_loop_stereo"); break;
      case 5: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_waterfalls[i], "tracks/690224__nox_sound__ambiance_waterfall_big_skogafoss_loop_stereo"); break;
    }
    pd->sound->fileplayer->setVolume(m_waterfalls[i], WF_VOLUMES[i], WF_VOLUMES[i]);
    pd->sound->fileplayer->setBufferLength(m_waterfalls[i], 1.0f); 
  }

}

void soundDoSfx(enum SfxSample sample) {
  if (!m_doSfx || IOGetIsPreloading()) return;

  if (sample == kPlingSfx1 || sample == kDingSfx1) {
    if (m_doingExplosion) {
      return; // Disable plings during the explosion
    }
    if (m_plingTimer == 0 || (gameGetFrameCount() - m_plingTimer) > TICK_FREQUENCY*2) {
      m_plingID = 0;
    }

    //TODO try and match the pitches better
    if (sample == kDingSfx1 && m_plingID >= 2) { sample -= 2; }
    else if (sample == kDingSfx1 && m_plingID >= 1) { sample -= 1; }

    sample += m_plingID;

    if (m_plingID < N_PLINGS_SFX-1) m_plingID++;
    m_plingTimer = gameGetFrameCount();
    if (boardGetCurrentSpecial() == kPegSpecialBounce) { sample = rand() % 2 ? kBoingSfx1 : kBoingSfx2; }
  } else  if (sample == kWhooshSfx1) { // Round robin
    static uint8_t offset = 0;
    offset = (offset + 1) % 4;
    sample += offset;
  } else if (sample == kSplashSfx1) {
    sample += rand() % N_SPLASHES_SFX;
  } else if (sample == kPopSfx1) {
    sample += rand() % N_POPS_SFX;
  }

  pd->sound->sampleplayer->play(m_samplePlayer[sample], 1, 1.0f);
}

void soundStopSfx(enum SfxSample sample) {
  if (!m_doSfx) return;
  pd->sound->sampleplayer->stop(m_samplePlayer[sample]);
}

///////////////
