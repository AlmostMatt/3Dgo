uniform sampler2D fbo_texture;
varying vec2 f_texcoord;
 
const static vec2 offset[8] = {
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, -1.0),
    vec2(0.0, 1.0),
};

void main(void) {
    vec2 pixel = vec2(1.0/1200.0, 1.0/900.0);
    vec4 col = texture2D(fbo_texture, f_texcoord);
    
    // compare adjacent pixels
    float diff = abs(texture2D(fbo_texture, f_texcoord + offset[0] * pixel).r - texture2D(fbo_texture, f_texcoord + offset[1] * pixel).r) + 
                     abs(texture2D(fbo_texture, f_texcoord + offset[2] * pixel).r - texture2D(fbo_texture, f_texcoord + offset[3] * pixel).r) + 
                     abs(texture2D(fbo_texture, f_texcoord + offset[4] * pixel).r - texture2D(fbo_texture, f_texcoord + offset[5] * pixel).r) + 
                     abs(texture2D(fbo_texture, f_texcoord + offset[6] * pixel).r - texture2D(fbo_texture, f_texcoord + offset[7] * pixel).r);
    diff = clamp((diff) , 0.0, 1.0) ;
    //col.gb = min(col.gb, 1 - diff);
    col = clamp(col, 0.0, 1.0); 
    gl_FragColor = col;
}

//    vec4 col = gl_Color;
