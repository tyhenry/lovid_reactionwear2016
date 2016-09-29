
#version 120

uniform sampler2DRect u_tex0; // tex
uniform sampler2DRect u_tex1; // mask
uniform vec2 u_resolution;

void main () {
	vec2 st = gl_FragCoord.xy/u_resolution;
	st.y = 1.-st.y;
	st *= u_resolution;
    vec4 color = vec4(st.x,st.y,0.,1.);

    color = texture2DRect(u_tex0,st);
    color.a = texture2DRect(u_tex1,st).g;

    gl_FragColor = color;
}