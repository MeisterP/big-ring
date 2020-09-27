#version 110
#extension GL_ARB_texture_rectangle : enable
/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * This code was inspired by the YUV-to-RGB shader in http://slouken.blogspot.nl/2011/02/mpeg-acceleration-with-glsl.html
 */

uniform sampler2DRect yTex;
uniform sampler2DRect uTex, vTex;

// YUV offset
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

// RGB coefficients
const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);

void main(void)
{
    vec2 tcoord;
    vec3 yuv, rgb;

    tcoord.x = gl_TexCoord[0].x;
    tcoord.y = gl_TexCoord[0].y;

    // Get the Y value
    yuv.x = texture2DRect(yTex, tcoord).r;

    // Get the U and V values
    tcoord *= 0.5;
    yuv.y = texture2DRect(uTex, tcoord).r;
    yuv.z = texture2DRect(vTex, tcoord).r;

    // Do the color transform
    yuv += offset;
    rgb.r = dot(yuv, Rcoeff);
    rgb.g = dot(yuv, Gcoeff);
    rgb.b = dot(yuv, Bcoeff);

    // assign the color (with alpha 1.0).
    gl_FragColor = vec4(rgb, 1.0);
}

