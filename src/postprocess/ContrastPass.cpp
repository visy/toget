/*
 *  ContrastPass.h
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
#include "ContrastPass.h"
#include "ofMain.h"

namespace itg
{
    ContrastPass::ContrastPass(const ofVec2f& aspect, bool arb, float contrast, float brightness) :
        contrast(contrast), brightness(brightness), RenderPass(aspect, arb, "contrast")
    {
        multiple = 1.0f;
        string fragShaderSrc = STRINGIFY(uniform sampler2D tex0;
                                         uniform float contrast;
                                         uniform float brightness;
                                         uniform float multiple;
                                         uniform float time;

                                        float rand(vec2 co)
                                        {
                                            float a = 12.9898;
                                            float b = 78.233;
                                            float c = 43758.5453;
                                            float dt= dot(co.xy ,vec2(a,b));
                                            float sn= mod(dt,3.14);
                                            return fract(tan(sn) * c);
                                        }

                                         
                                         void main(){
                                            
                                             vec2 uv = gl_TexCoord[0].st;
                                             
                                             float iGlobalTime = time;
                                             
                                             float magnitude = 0.0009;
                                             
                                             // Set up offset
                                             vec2 offsetRedUV = uv;
                                             offsetRedUV.x = uv.x + rand(vec2(iGlobalTime*0.03,uv.y*0.42)) * 0.001;
                                             offsetRedUV.x +=sin(rand(vec2(iGlobalTime*0.2, uv.y)))*magnitude;
                                             
                                             vec2 offsetGreenUV = uv;
                                             offsetGreenUV.x = uv.x + rand(vec2(iGlobalTime*0.004,uv.y*0.002)) * 0.004;
                                             offsetGreenUV.x += cos(iGlobalTime*9.0)*magnitude;
                                             
                                             vec2 offsetBlueUV = uv;
                                             offsetBlueUV.x = uv.y;
                                             offsetBlueUV.x += rand(vec2(atan(iGlobalTime*0.01),atan(uv.y)));
                                             
                                             // Load Texture
                                             float r = texture2D(tex0, offsetRedUV).r;
                                             float g = texture2D(tex0, offsetGreenUV).g;
                                             float b = texture2D(tex0, uv).b;
                                             
                                             float n = rand(vec2(uv.x+tan(time*0.05),uv.y+tan(time*0.05)))*0.2;
                                             
                                             gl_FragColor =  vec4(r-n, g-n, b-n, 1.0);
                                         }
                                         );
        
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShaderSrc);
        shader.linkProgram();
        
    }
    

    void ContrastPass::render(ofFbo& readFbo, ofFbo& writeFbo)
    {
        writeFbo.begin();
        
        ofClear(0, 0, 0, 255);
        
        shader.begin();
        
        shader.setUniformTexture("tex0", readFbo, 0);
        shader.setUniform1f("contrast", contrast);
        shader.setUniform1f("brightness", brightness);
        shader.setUniform1f("multiple", multiple);
        shader.setUniform1f("time", ofGetElapsedTimef());

        texturedQuad(0, 0, writeFbo.getWidth(), writeFbo.getHeight());
        
        shader.end();
        writeFbo.end();
    }
}