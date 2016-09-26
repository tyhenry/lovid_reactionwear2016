#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.14159265359

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

float random (float x){
    return fract(sin(x)*10e5);
}

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    return mix(random(i),random(i+1.),smoothstep(0.,1.,f));
}

vec2 random2(vec2 st){
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

//Value Noise by Inigo Quilez - iq/2013
//https://www.shadertoy.com/view/lsf3WH
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ), 
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ), 
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

void main(){
	vec2 st = gl_FragCoord.xy/u_resolution; //normalize coords
	st = st*2.-1.; // move to center (-1 to 1 coords)
	float pct = length(max(abs(st)-0.3,0.)); // round rect

	// add wave
	float a = atan(st.x,st.y);
	//a = fract(a);

	//float f = noise(abs(st.x+a*10.)+u_time*2.)*0.1+0.1;
	//float f = smoothstep(-.5,1., noise(a*10.+u_time*10.))*0.1+0.01;
	//float f = noise(a*5.2+u_time)*.05;
	//float f = noise(st.x+a*10.+u_time*2.)*0.1+0.1;
	float f = /*noise(st.x*10.+u_time)*0.1+noise(st.y*10.-u_time*1.)*0.1*//*noise(st*vec2(0,-10.)-u_time)*0.01+*/noise(st*noise(u_time*0.3)*20.+ u_time*.2)*0.1;
	pct += f;

	pct = smoothstep(0.5,0.51,pct);
	pct = 1. - pct; // invert

	gl_FragColor = vec4(vec3(pct),1.);
}