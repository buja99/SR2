#include "Animator.h"
#include <algorithm>
#include <cassert>
#include "Skeleton.h"
#include "MyMath.h"
#include "StaticModel.h"

void Animator::Play(const AnimationData& anim, bool loop, float startTime) {
    currentAnim_ = &anim;
    time_ = startTime;
    loop_ = loop;
    finished_ = false;
    paused_ = false;
}

void Animator::Update(float deltaTime, Skeleton& skeleton) {
    
    if (!currentAnim_ || paused_) {
        return;
    }

    const AnimationData& anim = *currentAnim_;

    if (anim.duration <= 0.0f) {
        return;
    }

    
    time_ += deltaTime * speed_;

   
    if (loop_) {
        time_ = fmodf(time_, anim.duration);
    } else if (time_ >= anim.duration) {
        time_ = anim.duration;
        finished_ = true;
        return;
    }

    
    for (const auto& channel : anim.channels) {
        auto it = skeleton.boneIndexMap.find(channel.nodeName);
        if (it == skeleton.boneIndexMap.end()) continue;

        int boneIndex = it->second;
        if (boneIndex < 0 || boneIndex >= skeleton.bones.size()) continue;

        Bone& bone = skeleton.bones[boneIndex]; 

        Vector3 pos = bone.transform.translate;
        Vector3 scl = bone.transform.scale;
        Quaternion rot = bone.transform.rotate;

        
        if (channel.translate.keyframes.size() >= 2) {
            const auto& keys = channel.translate.keyframes;
            size_t idx = 0;
            while (idx + 1 < keys.size() && keys[idx + 1].time < time_) ++idx;
            const auto& k0 = keys[idx];
            const auto& k1 = keys[(std::min<size_t>)(idx + 1, keys.size() - 1)];
            float t = (time_ - k0.time) / (k1.time - k0.time + 0.0001f);
            pos = MyMath::Lerp(k0.value, k1.value, t);
        }

        
        if (channel.scale.keyframes.size() >= 2) {
            const auto& keys = channel.scale.keyframes;
            size_t idx = 0;
            while (idx + 1 < keys.size() && keys[idx + 1].time < time_) ++idx;
            const auto& k0 = keys[idx];
            const auto& k1 = keys[(std::min<size_t>)(idx + 1, keys.size() - 1)];
            float t = (time_ - k0.time) / (k1.time - k0.time + 0.0001f);
            scl = MyMath::Lerp(k0.value, k1.value, t);
        }

        
        if (channel.rotate.keyframes.size() >= 2) {
            const auto& keys = channel.rotate.keyframes;
            size_t idx = 0;
            while (idx + 1 < keys.size() && keys[idx + 1].time < time_) ++idx;
            const auto& k0 = keys[idx];
            const auto& k1 = keys[(std::min<size_t>)(idx + 1, keys.size() - 1)];
            float t = (time_ - k0.time) / (k1.time - k0.time + 0.0001f);
            rot = MyMath::Slerp(k0.value, k1.value, t);
        }

        
        bone.transform.scale = scl;
        bone.transform.rotate = rot;
        bone.transform.translate = pos;

        
        bone.localMatrix = MyMath::MakeAffineMatrix(
            bone.transform.scale,
            MyMath::QuaternionToEuler(bone.transform.rotate),
            bone.transform.translate
        );
    }

    
    skeleton.UpdateWorldMatrix();
}

float Animator::GetDuration() const {
    return currentAnim_ ? currentAnim_->duration : 0.0f;
}
