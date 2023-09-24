#include "sound.h"

bool m_sfxOn = false;

bool m_hasMusic = true;

int8_t m_trackPlaying = -1;

FilePlayer* m_music[N_MUSIC_TRACKS + 1];

SamplePlayer* m_samplePlayer[kNSFX];
AudioSample* m_audioSample[kNSFX];

void musicStopped(SoundSource* _c);

/// ///

void pauseMusic() {
  if (!m_hasMusic) return;
  if (m_trackPlaying == -1) return;
  pd->sound->fileplayer->pause(m_music[m_trackPlaying]);
}

void resumeMusic() {
  if (!m_hasMusic) return;
  if (m_trackPlaying == -1) return;
  pd->sound->fileplayer->play(m_music[m_trackPlaying], 1);
}

void updateMusicVol() {
  if (!m_hasMusic) return;
  if (m_trackPlaying == -1) return;
  int8_t i = 100;//getPlayer()->m_musicVol;
  if (!i) i = 100;
  float v = i * 0.01f;
  pd->sound->fileplayer->setVolume(m_music[m_trackPlaying], v, v);
}

void updateMusic(bool _isTitle) {
  if (!m_hasMusic) return;
  if (_isTitle) {
    if (m_trackPlaying != -1) {
      int8_t toStop = m_trackPlaying;
      m_trackPlaying = -1; // Caution: there will be a callback
      pd->sound->fileplayer->stop(m_music[toStop]);
    }
    pd->sound->fileplayer->play(m_music[0], 1);
    m_trackPlaying = 0;
    updateMusicVol();
  } else {
    const bool musicOn = true;//(p->m_soundSettings & 1);
    if (musicOn) {
      if (m_trackPlaying == -1) {
        pd->sound->fileplayer->play(m_music[0], 1);
        m_trackPlaying = 0;
        updateMusicVol();
      } else {
        // music is already playing
      }
    } else { // music off
      if (m_trackPlaying == -1) {
        // music is already off
      } else {
        #ifdef DEV
        pd->system->logToConsole("Stopping %i", m_trackPlaying);
        #endif
        int8_t toStop = m_trackPlaying;
        m_trackPlaying = -1; // Caution: there will be a callback
        pd->sound->fileplayer->stop(m_music[toStop]);
      }
    }
  }
}

void chooseMusic(int8_t _id) {
  if (!m_hasMusic) return;
  if (m_trackPlaying == -1) {
    // music is off
    return;
  } 
  #ifdef DEV
  pd->system->logToConsole("Stopping %i", m_trackPlaying);
  #endif
  int8_t toStop = m_trackPlaying;
  m_trackPlaying = -1; // Caution: there will be a callback
  pd->sound->fileplayer->stop(m_music[toStop]);
  pd->sound->fileplayer->play(m_music[_id], 1); 
  m_trackPlaying = _id;
  updateMusicVol();
}

void musicStopped(SoundSource* _c) {
  if (!m_hasMusic) return;
  if (m_trackPlaying == -1) {
    return;
  }
  int8_t next = m_trackPlaying;
  while (next == m_trackPlaying) {
    next = rand() % N_MUSIC_TRACKS;
  }
  pd->sound->fileplayer->play(m_music[next], 1);
  m_trackPlaying = next;
  updateMusicVol();
}

void updateSfx() {
  m_sfxOn = false; //(p->m_soundSettings & 2);
}

void initSound() {

  // m_audioSample[kSfxDestroy] = pd->sound->sample->load("sounds/destroy");

  for (int32_t i = 0; i < kNSFX; ++i) {
    m_samplePlayer[i] = pd->sound->sampleplayer->newPlayer();
    pd->sound->sampleplayer->setSample(m_samplePlayer[i], m_audioSample[i]);
  }

  m_hasMusic = true;
  for (int32_t i = 0; i < N_MUSIC_TRACKS + 1; ++i) {
    m_music[i] = pd->sound->fileplayer->newPlayer();
    pd->sound->fileplayer->setBufferLength(m_music[i], 5.0f); 
    switch (i) {
      case 0: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "music/1"); break;
      case 1: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "music/2"); break;
      case 2: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "music/3"); break;
      case 3: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "music/4"); break;
      case 4: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "music/5"); break;
      case 5: m_hasMusic &= pd->sound->fileplayer->loadIntoPlayer(m_music[i], "music/6"); break;
    }
    pd->sound->fileplayer->setFinishCallback(m_music[i], musicStopped);
  }
}

void sfx(enum SfxSample _sample) {
  if (!m_sfxOn) return;
  pd->sound->sampleplayer->play(m_samplePlayer[_sample], 1, 1.0f);
}

///////////////
