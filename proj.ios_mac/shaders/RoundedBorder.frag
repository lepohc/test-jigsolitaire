#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
varying vec2 v_pos;

uniform vec2 u_size;
uniform float u_radius;
uniform float u_borderWidth;
uniform vec4 u_borderColor;

// Signed Distance Function for a rounded box
float sdRoundedBox(vec2 p, vec2 b, float r)
{
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main()
{
    vec2 halfSize = u_size / 2.0;
    
    // Calculate signed distance
    // v_pos is expected to be relative to the center of the sprite
    float dist = sdRoundedBox(v_pos, halfSize, u_radius);
    
    // Discard pixels outside the rounded rectangle
    if (dist > 0.0) {
        discard;
    }
    
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);
    
    // Calculate border factor
    // dist goes from negative (inside) to 0 (edge)
    // We want border when dist is between -u_borderWidth and 0
    
    // smoothstep for anti-aliased transition between texture and border
    float borderFactor = smoothstep(-u_borderWidth - 1.0, -u_borderWidth, dist);
    
    // Mix texture color and border color
    vec4 finalColor = mix(texColor, u_borderColor, borderFactor);
    
    // Apply vertex color (opacity)
    finalColor = finalColor * v_fragmentColor;
    
    // Anti-aliasing at the outer edge
    float alpha = 1.0 - smoothstep(-1.0, 0.0, dist);
    finalColor.a *= alpha;
    
    gl_FragColor = finalColor;
}
