#ifndef PARTICLE_H
#define PARTICLE_H

#include "lib/partikel.h"

extern ParticleSystem *particleSystem;

void RegisterEmitterConfigs(void);
int RegisterParticleEmitter(Vector2 position, void* emitterConfig);
void AddParticlesToDrawList(Vector2 position, EmitterConfig config, Texture2D* texture);
void DrawParticles(void);

typedef struct {
  EmitterConfig *breakBlock;
} EmitterConfigs;

extern EmitterConfigs emitterConfigs;

#endif
