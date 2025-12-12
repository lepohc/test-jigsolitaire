attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying mediump vec2 v_texCoord;
varying mediump vec4 v_fragmentColor;
varying mediump vec2 v_pos;
#else
varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
varying vec2 v_pos;
#endif

void main()
{
    // Sprite uses QuadCommand which transforms vertices to View Space on CPU.
    // So we only need to apply the Projection Matrix.
    gl_Position = CC_PMatrix * a_position;
    v_fragmentColor = a_color;
    v_texCoord = a_texCoord;
    // v_pos is not used anymore in frag shader for position calculation
    v_pos = a_position.xy; 
}
