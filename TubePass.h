//
//  TubePass.h
//  shaderview
//
//  Created by Joseph Wilk on 18/09/2017.
//
//

#pragma once

#include "RenderPass.h"
#include "ofShader.h"

namespace itg
{
  class TubePass : public RenderPass
  {
  public:

    typedef shared_ptr<TubePass> Ptr;

    TubePass(const ofVec2f& aspect, bool arb, float centerX = 0.5, float centerY = 0.5,
                 float exposure = 0.48, float decay = 0.9, float density = 0.25,
                 float weight = 0.25, float clamp = 1);

    void render(ofFbo& readFbo, ofFbo& writeFbo, ofTexture& depth);

    void setCenterX(float v){ centerX = v; }
    float getCenterX() { return centerX; }

    void setCenterY(float v){ centerY = v; }
    float getCenterY() { return centerY; }

    void setExposure(float v){ exposure = v; }
    float getExposure() { return exposure; }

    void setDecay(float v){ decay = v; }
    float getDecay() { return decay; }

    void setDensity(float v){ density = v; }
    float getDensity() { return density; }

    void setWeight(float v){ weight = v; }
    float getWeight() { return weight; }

    void setClamp(float v){ clamp = v; }
    float getClamp() { return clamp; }
  private:

    ofShader shader;

    float centerX;
    float centerY;
    float exposure;
    float decay;
    float density;
    float weight;
    float clamp;

  };
}
