#version 330
uniform sampler2D u_nearestDepthBuffer;
uniform sampler2D u_farthestDepthBuffer;
uniform sampler2D u_forwardColorBuffer;
uniform sampler2D u_forwardAlphasBuffer;
uniform float u_vpWidth;
uniform float u_vpHeight;
uniform vec4 u_diffuse;

#ifndef EDGE

#ifdef NEGATO

vec4 negato();

#else


#ifdef PIXEL_LIGHTING
in vec3 oPosition_WS;
in vec3 oNormal_WS;
in vec3 oLight_WS;

vec4 lighting(vec3 _normal, vec3 _position);
#else

#ifdef FLAT
flat in vec4 oColor;
#else
in vec4 oColor;
#endif // FLAT

#endif // PIXEL_LIGHTING

vec4 fetch_texture();

#endif // NEGATO

#else

vec4 oColor = vec4(0,0,0,u_diffuse.a);

#endif // EDGE

layout(location = 0) out vec4 frontDepth;
layout(location = 1) out vec4 backDepth;
layout(location = 2) out vec4 frontColorRG;
layout(location = 3) out vec4 backColorRG;
layout(location = 4) out vec4 frontColorBA;
layout(location = 5) out vec4 backColorBA;

float packColor(vec2 color);
vec2 unpackColor(float buffer);

#define MAX_DEPTH 1.0

/** Peel shader :
  * Only write fragment ahead from next peel
  **/
void main()
{
    vec2 texCoord = gl_FragCoord.xy / vec2( u_vpWidth, u_vpHeight );

    // window-space depth interpolated linearly in screen space
    float fragDepth = gl_FragCoord.z;

    float nearestDepth = - texture(u_nearestDepthBuffer, texCoord).r;
    float farthestDepth = texture(u_farthestDepthBuffer, texCoord).r;
    float forwardColorBufferRGTemp = texture(u_forwardColorBuffer, texCoord).r;
    float forwardColorBufferBATemp = texture(u_forwardAlphasBuffer, texCoord).r;

    //	g_dualDepthTexId[1]
    // Depths and 1.0-alphaMult always increase
    // so we can use pass-through by default with MAX blending
    frontDepth.r = - nearestDepth;
    backDepth.r = farthestDepth;

    //	g_dualFrontBlenderTexId[1]
    // Front colors always increase (DST += SRC*ALPHA_MULT)
    // so we can use pass-through by default with MAX blending
    frontColorRG.r = forwardColorBufferRGTemp;
    frontColorBA.r = forwardColorBufferBATemp;
    vec2 forwardColorBufferBA = unpackColor(forwardColorBufferBATemp);

    //	g_dualBackTempTexId[1]
    // Because over blending makes color increase or decrease,
    // we cannot pass-through by default.
    // Each pass, only one fragment writes a color greater than 0
    backColorRG.r = 0.;
    backColorBA.r = 0.;

    float alphaMultiplier = 1.0 - forwardColorBufferBA.g;

    //	CASE : fragment not interesting (previous peels)
    if (fragDepth < nearestDepth || fragDepth > farthestDepth)
    {
        // Skip this depth in the peeling algorithm
        frontDepth.r = -MAX_DEPTH;
        backDepth.r = -MAX_DEPTH;
        return;
    }

    //	CASE : fragment depth inside geometry => Peeling !
    if (fragDepth > nearestDepth && fragDepth < farthestDepth)
    {
        // This fragment needs to be peeled again
        frontDepth.r = -fragDepth;
        backDepth.r = fragDepth;
        return;
    }

#ifdef NEGATO
        vec4 colorOut = negato();
#else

#ifdef PIXEL_LIGHTING
        vec4 colorOut = lighting(normalize(oNormal_WS), oPosition_WS);
#else
        vec4 colorOut = oColor;
#endif
#ifndef EDGE
        colorOut *= fetch_texture();
#endif

#endif // NEGATO

    // If we made it here, this fragment is on the peeled layer from last pass
    // therefore, we need to shade it, and make sure it is not peeled any farther
    frontDepth.r = -MAX_DEPTH;
    backDepth.r = -MAX_DEPTH;

    if (fragDepth == nearestDepth)
    {
        colorOut.rgb *= colorOut.a * alphaMultiplier;
        colorOut.a = 1.0 - alphaMultiplier * (1.0 - colorOut.a);

        vec4 color;
        color.rg = unpackColor(frontColorRG.r);
        color.ba = forwardColorBufferBA;

        colorOut.rgb += color.rgb;

        frontColorRG.r = packColor(colorOut.rg);
        frontColorBA.r = packColor(vec2(colorOut.b, colorOut.a));
    }
    else
    {
        // fragDepth == farthestDepth
        //	Just send the back fragment color to the next shader/render
        backColorRG.r = packColor(colorOut.rg);
        backColorBA.r = packColor(colorOut.ba);
    }
}
