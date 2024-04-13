#include "sound.h"

bool m_doSfx = false;

bool m_hasMusic = true;

int8_t m_trackPlaying = -1;

FilePlayer* m_music[N_MUSIC_TRACKS + 1];

SamplePlayer* m_samplePlayer[kNSFX + 1]; // TODO remove these +1
AudioSample* m_audioSample[kNSFX + 1];

void soundDoMusicStopped(SoundSource* c, void* userdata);

/// ///

void musicDoInit() {
  if (!m_hasMusic) return;
  const bool musicOn = true;//(p->m_soundSettings & 1);
  if (musicOn) {
    if (m_trackPlaying == -1) {
      pd->sound->fileplayer->play(m_music[0], 1);
      m_trackPlaying = 0;
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

void soundDoChooseMusic(const int8_t id) {
  if (!m_hasMusic) return;
  if (m_trackPlaying == -1) {
    // music is off
    return;
  } 
  #ifdef DEV
  pd->system->logToConsole("Stopping %i", m_trackPlaying);
  #endif
  const int8_t toStop = m_trackPlaying;
  m_trackPlaying = -1; // Caution: there will be a callback
  pd->sound->fileplayer->stop(m_music[toStop]);
  pd->sound->fileplayer->play(m_music[id], 1); 
  m_trackPlaying = id;
}

void soundDoMusicStopped(SoundSource* _c, void* userdata) {
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
}

void soundDoUpdate() {
  m_doSfx = false; //(p->m_soundSettings & 2);
}

void soundDoInit() {

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
    pd->sound->fileplayer->setFinishCallback(m_music[i], soundDoMusicStopped, NULL);
  }
}

void soundDoSfx(const enum SfxSample sample) {
  if (!m_doSfx) return;
  pd->sound->sampleplayer->play(m_samplePlayer[sample], 1, 1.0f);
}

///////////////
