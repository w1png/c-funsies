#include "texture.h"
#include <stdio.h>
#define LIBPARTIKEL_IMPLEMENTATION
#include "lib/partikel.h"

#include "particle.h"
#include <stddef.h>

ParticleSystem* particleSystem = NULL;
EmitterConfigs emitterConfigs = {0};

bool BreakBlockDeactivator(Particle *particle) {
  return Particle_DeactivatorAge(particle);
}

int RegisterParticleEmitter(Vector2 position, void* emitterConfigTemplate) {
    EmitterConfig config = *(EmitterConfig*)emitterConfigTemplate;
    config.origin = position;

    Emitter *emitter = Emitter_New(config);
    if (!emitter) {
      TraceLog(LOG_ERROR, "Failed to register emitter");
      return -1;
    }

    Emitter_Start(emitter);

    ParticleSystem_Register(particleSystem, emitter);
    return 0;
}

void RegisterEmitterConfigs() {
  TraceLog(LOG_INFO, "===Registering particle emitters===");
  particleSystem = ParticleSystem_New();
  if (particleSystem == NULL) {
    TraceLog(LOG_ERROR, "Failed to create particle system");
    exit(1);
  }

  emitterConfigs.breakBlock = malloc(sizeof(EmitterConfig));
  emitterConfigs.breakBlock->capacity = 600;
  emitterConfigs.breakBlock->emissionRate = 800;
  emitterConfigs.breakBlock->origin = (Vector2){0, 0};
  emitterConfigs.breakBlock->originAcceleration = (FloatRange){.min = 0, .max = 0};
  emitterConfigs.breakBlock->direction = (Vector2){0, -1};
  emitterConfigs.breakBlock->directionAngle = (FloatRange){.min = -30, .max = 30};
  emitterConfigs.breakBlock->velocity = (FloatRange){.min = 80, .max = 140};
  emitterConfigs.breakBlock->externalAcceleration = (Vector2){0, 400};
  emitterConfigs.breakBlock->startColor = (Color){255, 80, 80, 255};
  emitterConfigs.breakBlock->endColor = (Color){255, 255, 100, 0};
  emitterConfigs.breakBlock->age = (FloatRange){.min = 2.5f, .max = 4.0f};
  emitterConfigs.breakBlock->blendMode = BLEND_ALPHA;
  emitterConfigs.breakBlock->particle_Deactivator = BreakBlockDeactivator;
  emitterConfigs.breakBlock->texture = textures.chest;

  ParticleSystem_SetOrigin(particleSystem, (Vector2){0, 0});
  ParticleSystem_Start(particleSystem);

  TraceLog(LOG_INFO, "===Particle emitters registered===");
}
