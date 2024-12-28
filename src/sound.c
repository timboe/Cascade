#include "sound.h"
#include "io.h"

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

void soundDoInit() {

  m_audioSample[kPlingSfx1] = pd->sound->sample->load("fx/pling_1");
  m_audioSample[kPlingSfx2] = pd->sound->sample->load("fx/pling_2");
  m_audioSample[kPlingSfx3] = pd->sound->sample->load("fx/pling_3");
  m_audioSample[kPlingSfx4] = pd->sound->sample->load("fx/pling_4");
  m_audioSample[kPlingSfx5] = pd->sound->sample->load("fx/pling_5");
  m_audioSample[kPlingSfx6] = pd->sound->sample->load("fx/pling_6");
  m_audioSample[kPlingSfx7] = pd->sound->sample->load("fx/pling_7");

  m_audioSample[kSplashSfx1] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__1");
  m_audioSample[kSplashSfx2] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__2");
  m_audioSample[kSplashSfx3] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__3");
  m_audioSample[kSplashSfx4] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__4");
  m_audioSample[kSplashSfx5] = pd->sound->sample->load("fx/737644__kraftaggregat__rocks-thrown-in-water__5");

  m_audioSample[kPopSfx] = pd->sound->sample->load("fx/pop");

  m_audioSample[kExplosionSfx] = pd->sound->sample->load("fx/explosion");

  for (int32_t i = 0; i < kNSFX; ++i) {
    m_samplePlayer[i] = pd->sound->sampleplayer->newPlayer();
    pd->sound->sampleplayer->setSample(m_samplePlayer[i], m_audioSample[i]);
  }

  m_hasMusic = true;
  for (int32_t i = 0; i < N_MUSIC_TRACKS; ++i) {
    m_music[i] = pd->sound->fileplayer->newPlayer();
    switch (i) {
      case 0: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517962__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-15"); break;
      case 1: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517963__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-14"); break;
      case 2: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517964__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-13"); break;
      case 3: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517968__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-7"); break;
      case 4: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517969__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-8"); break;
      case 5: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517971__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-3"); break;
      case 6: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517974__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-5"); break;
      case 7: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517977__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-4"); break;
      case 8: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517979__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-2"); break;
      case 9: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517983__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-23"); break;
      case 10: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517985__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-22"); break;
      case 11: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "tracks/517990__doctor_dreamchip__doctor-dreamchip-lofi-keyboard-pack-rhodes80bpm-c-major-9"); break;
    }
    pd->sound->fileplayer->setBufferLength(m_music[i], 1.0f); 
  }

  const float WF_VOLUMES[] = {0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f};
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
  if (!m_doSfx) return;

  if (sample == kPlingSfx1) {
    if (m_doingExplosion) {
      return; // Disable plings during the explosion
    }
    if (m_plingTimer == 0 || (gameGetFrameCount() - m_plingTimer) > TICK_FREQUENCY*2) {
      m_plingID = 0;
    }
    sample += m_plingID;
    if (m_plingID < N_PLINGS_SFX-1) m_plingID++;
    m_plingTimer = gameGetFrameCount();
  }

  if (sample == kSplashSfx1) {
    sample += rand() % N_SPLASHES_SFX;
  }

  pd->sound->sampleplayer->play(m_samplePlayer[sample], 1, 1.0f);
}

///////////////
