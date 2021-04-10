#pragma once
#include "Common.h"
#include "CVector3.h"
#include "CMatrix4x4.h"
#include "Input.h"

class Model;



class Light
{
private:
    Model* mModel = nullptr;
    CVector3 mColour = CVector3{ 0.0f,0.0f,0.0f };
    float    mStrength = 0.0f;

public:
    Light(){}
    Model* GetModel() { return mModel; }
    CVector3 GetColour() { return mColour; }
    float GetStrength() { return mStrength; }

    void SetModel(Model* Model) { mModel = Model; }
    void SetColour(CVector3 Colour) { mColour = Colour; }
    void SetStrength(float Strength) { mStrength = Strength; }
};

