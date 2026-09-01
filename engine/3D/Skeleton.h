#pragma once
#include "Bone.h"
#include <vector>
#include <unordered_map>
#include <assimp/scene.h>
class Skeleton {


public:

    void Initialize(const aiScene* scene); 
    void UpdateWorldMatrix();              


    Bone* FindBone(const std::string& name);
    const Bone* FindBone(const std::string& name) const;

public:

    std::vector<Bone> bones;
    std::unordered_map<std::string, int> boneIndexMap; 
    int rootIndex = -1;
};

