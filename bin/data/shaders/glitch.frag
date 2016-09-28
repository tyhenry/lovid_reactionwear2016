#version 120

const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721); //BrCoSa const

uniform sampler2DRect tex0; //video texture
uniform float u_time; //ofGetElapsedTimef()

//Pixelate
uniform float textureW; //vid texture width (pixels)
uniform float textureH; //texture height
uniform float divX; //# of "pixel" divisions, default = 10.f
uniform float divY; //default = 10.

//BrCoSa
uniform vec3 avgluma; //default vec3(0.62f)
uniform float saturation;
uniform float contrast;
uniform float brightness;
uniform float alpha; //default = 1.f

//Glitch
uniform float glitch;

void main () {

  vec2 st = gl_TexCoord[0].st;

  

  //Pixelate
  float pixelW = textureW/divX; //default 64 (@ 640x480)
  float pixelH = textureH/divY; //default 48 (@ 640x480)
  vec2 pixCoord = vec2(pixelW*floor(st_glitch.x/pixelW),
                  pixelH*floor(st_glitch.y/pixelH));
  vec3 pixColor = vec3(texture2DRect(tex0, pixCoord));

  //Glitch
  vec2 st_glitch = st;
  vec2 res = vec2(textureW, textureH);
  st_glitch = st/(res*1.2); //normalize (w/ offset?)
  st_glitch.y = 1.-st_glitch.y; //invert y

  st_glitch *= 400.; //multiply space

  vec2 st_glitchIndex = floor(st_glitch);

  //st_glitch = fract(st_glitch); //repeat space

  if (mod(st_glitchIndex.x,2.) == 0.) { //if even
    st_glitch.y -= sin(u_time+5.*glitch*10.)*10.*glitch;
  } else {
    st_glitch.y += cos(u_time+7.*glitch*10.)*10.*glitch;
  }

  if (mod(st_glitchIndex.y,floor(abs(sin(glitch*10.)*10.))) == 0.) { //if even
    st_glitch.x -= 100.*abs(sin(st_glitch.y*mod(u_time,3.+glitch)*0.01))*glitch;
    
  } else {
    st_glitch.x += 100.*abs(cos(st_glitch.y*mod(u_time,2.+glitch)*0.01))*glitch;
    
  }

  st_glitch /= 400.;

  st_glitch.y = 1. - st_glitch.y;
  st_glitch *= res*1.2;

  vec3 glitchColor = vec3(0);
  glitchColor = vec3(texture2DRect(tex0,st_glitch));





  //BrCosa
  vec3 intensity  = vec3(dot(pixColor, LumCoeff));
  vec3 color      = mix(intensity, pixColor, saturation);
  color           = mix(avgluma, color, contrast);
  color          *= brightness;


  //output
  gl_FragColor = vec4(color, alpha);
}