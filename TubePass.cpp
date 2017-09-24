/*
 *  ZoomBlurPass.cpp
 *
 *  Copyright (c) 2013, satcy, http://satcy.net
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Neil Mendoza nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "TubePass.h"
#include "ofMain.h"

namespace itg
{
  TubePass::TubePass(const ofVec2f& aspect, bool arb, float centerX, float centerY,
                             float exposure, float decay, float density,
                             float weight, float clamp) :
  centerX(centerX), centerY(centerY), exposure(exposure), decay(decay), density(density), weight(weight), clamp(clamp), RenderPass(aspect, arb, "zoomblur")
  {

    string fragShaderSrc = STRINGIFY(
                                     uniform sampler2D tex0;
                                     uniform float iGlobalTime;

                                     void main()
                                     {
                                       vec2 uv = gl_TexCoord[0].st;
                                       vec4 FragColor = vec4(0.0);
                                       vec4 color = texture2D(tex0,gl_TexCoord[0].st);

                                       float sCount = 900.;
                                       float nIntensity=0.8;
                                       float sIntensity=0.8;
                                       float noiseEntry = 0.0001;
                                       float accelerator= 1000.0;

                                       float x = uv.x * uv.y * iGlobalTime * accelerator;
                                       x = mod( x, 13.0 ) * mod( x, 123.0 );
                                       float dx = mod( x, 0.05 );
                                       vec3 cResult = color.rgb + color.rgb * clamp( 0.1 + dx * 100.0, 0.0, 1.0 );
                                       vec2 sc = vec2( sin( uv.y * sCount ), cos( uv.y * sCount ) );
                                       cResult += color.rgb * vec3( sc.x, sc.y, sc.x ) * sIntensity;
                                       cResult = color.rgb + clamp(nIntensity, noiseEntry,1.0 ) * (cResult - color.rgb);
                                         
                                       gl_FragColor = vec4(cResult, color.a);
                                     }
                                     );
    
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShaderSrc);
    shader.linkProgram();
    
  }
  
  void TubePass::render(ofFbo& readFbo, ofFbo& writeFbo, ofTexture& depthTex)
  {
    writeFbo.begin();
    shader.begin();
    
    shader.setUniformTexture("tex0", readFbo.getTexture(), 0);
    shader.setUniform1f("iGlobalTime", ofGetElapsedTimef() );
    texturedQuad(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
    
    shader.end();
    writeFbo.end();
  }
}
