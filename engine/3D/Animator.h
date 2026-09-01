#pragma once
#include "ModelTypes.h"
#include "Skeleton.h"



class Animator {

public:

    void Play(const AnimationData& anim, bool loop = true, float startTime = 0.0f);

    void Update(float deltaTime, Skeleton& skeleton);


    void Stop() { currentAnim_ = nullptr; time_ = 0.0f; }
    void Pause(bool p) { paused_ = p; }
    void SetSpeed(float s) { speed_ = (s < 0.0f ? 0.0f : s); }
    void SetLoop(bool l) { loop_ = l; }


    bool IsPlaying()           const { return currentAnim_ != nullptr && !paused_; }
    bool IsFinished()          const { return !loop_ && finished_; }
    void SetTime(float t) { time_ = t; finished_ = false; }
    float GetTime()            const { return time_; }
    float GetSpeed()           const { return speed_; }
    bool  GetLoop()            const { return loop_; }
    float GetDuration()        const; 

private:
    
    const AnimationData* currentAnim_ = nullptr;

    float time_ = 0.0f;   // Current playback time (seconds)
    float speed_ = 1.0f;   // Playback speed multiplier
    bool  loop_ = true;   // Whether to loop
    bool  paused_ = false;  // Whether playback is paused
    bool  finished_ = false;  // Non-looping playback completion flag
};

