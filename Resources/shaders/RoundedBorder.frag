#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
varying vec2 v_pos;

uniform vec2 u_size;
uniform float u_borderWidth;
uniform vec4 u_borderColor;
uniform vec4 u_uvRect; // x: minU, y: minV, z: widthU, w: heightV
uniform vec4 u_cornerRadii; // x: TR, y: BR, z: TL, w: BL
uniform vec4 u_borderSides; // x: Top, y: Right, z: Bottom, w: Left (1.0 = show, 0.0 = hide)

// Signed Distance Function for a rounded box with varying radii
// r: TR, BR, TL, BL
float sdRoundedBox(vec2 p, vec2 b, vec4 r)
{
    // Select radius based on quadrant
    // p is centered at 0,0
    // x>0, y>0 -> TR (r.x)
    // x>0, y<0 -> BR (r.y)
    // x<0, y>0 -> TL (r.z)
    // x<0, y<0 -> BL (r.w)
    
    float radius = 0.0;
    if (p.x > 0.0) {
        if (p.y > 0.0) radius = r.x;
        else radius = r.y;
    } else {
        if (p.y > 0.0) radius = r.z;
        else radius = r.w;
    }
    
    vec2 q = abs(p) - b + radius;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

void main()
{
    // Calculate local UV (0.0 to 1.0)
    vec2 localUV = (v_texCoord - u_uvRect.xy) / u_uvRect.zw;
    
    // Calculate local position centered at (0,0)
    // FIX: Invert Y because Cocos2d texture coordinates V goes down (0 at top, 1 at bottom)
    // but we want localPos.y to go up (positive at top) to match Cartesian coordinates.
    vec2 localPos = vec2(localUV.x - 0.5, 0.5 - localUV.y) * u_size;

    vec2 halfSize = u_size / 2.0;
    
    // Calculate signed distance
    float dist = sdRoundedBox(localPos, halfSize, u_cornerRadii);
    
    // Discard pixels outside the rounded rectangle
    if (dist > 0.0) {
        discard;
    }
    
    // FIX: Prevent anti-aliasing seams on connected sides
    // We force the distance to be "deep inside" (-100.0) to ensure opaque texture.
    // However, if we are near a corner that HAS a border, we must force "border inside" (-2.5)
    // to ensure the outline remains continuous.
    float margin = u_borderWidth;
    float edgeThreshold = u_borderWidth + 1.0; 

    // Top Connected
    if (u_borderSides.x < 0.5) {
        if (localPos.y > halfSize.y - edgeThreshold) {
            bool leftBorder = u_borderSides.w > 0.5;
            bool rightBorder = u_borderSides.y > 0.5;
            bool inLeftCorner = leftBorder && (localPos.x < -halfSize.x + margin);
            bool inRightCorner = rightBorder && (localPos.x > halfSize.x - margin);
            
            if (inLeftCorner || inRightCorner) dist = min(dist, -2.5);
            else dist = -100.0;
        }
    }
    // Right Connected
    if (u_borderSides.y < 0.5) {
        if (localPos.x > halfSize.x - edgeThreshold) {
            bool topBorder = u_borderSides.x > 0.5;
            bool bottomBorder = u_borderSides.z > 0.5;
            bool inTopCorner = topBorder && (localPos.y > halfSize.y - margin);
            bool inBottomCorner = bottomBorder && (localPos.y < -halfSize.y + margin);
            
            if (inTopCorner || inBottomCorner) dist = min(dist, -2.5);
            else dist = -100.0;
        }
    }
    // Bottom Connected
    if (u_borderSides.z < 0.5) {
        if (localPos.y < -halfSize.y + edgeThreshold) {
            bool leftBorder = u_borderSides.w > 0.5;
            bool rightBorder = u_borderSides.y > 0.5;
            bool inLeftCorner = leftBorder && (localPos.x < -halfSize.x + margin);
            bool inRightCorner = rightBorder && (localPos.x > halfSize.x - margin);
            
            if (inLeftCorner || inRightCorner) dist = min(dist, -2.5);
            else dist = -100.0;
        }
    }
    // Left Connected
    if (u_borderSides.w < 0.5) {
        if (localPos.x < -halfSize.x + edgeThreshold) {
            bool topBorder = u_borderSides.x > 0.5;
            bool bottomBorder = u_borderSides.z > 0.5;
            bool inTopCorner = topBorder && (localPos.y > halfSize.y - margin);
            bool inBottomCorner = bottomBorder && (localPos.y < -halfSize.y + margin);
            
            if (inTopCorner || inBottomCorner) dist = min(dist, -2.5);
            else dist = -100.0;
        }
    }

    vec4 texColor = texture2D(CC_Texture0, v_texCoord);
    
    // Calculate border factor
    float borderFactor = smoothstep(-u_borderWidth - 1.0, -u_borderWidth, dist);
    
    // Mask borders based on u_borderSides
    // x: Top, y: Right, z: Bottom, w: Left
    
    // Top
    if (u_borderSides.x < 0.5 && localPos.y > halfSize.y - u_borderWidth) {
        bool keepLeft = (u_borderSides.w > 0.5) && (localPos.x < -halfSize.x + u_borderWidth);
        bool keepRight = (u_borderSides.y > 0.5) && (localPos.x > halfSize.x - u_borderWidth);
        if (!keepLeft && !keepRight) borderFactor = 0.0;
    }
    // Right
    if (u_borderSides.y < 0.5 && localPos.x > halfSize.x - u_borderWidth) {
        bool keepTop = (u_borderSides.x > 0.5) && (localPos.y > halfSize.y - u_borderWidth);
        bool keepBottom = (u_borderSides.z > 0.5) && (localPos.y < -halfSize.y + u_borderWidth);
        if (!keepTop && !keepBottom) borderFactor = 0.0;
    }
    // Bottom
    if (u_borderSides.z < 0.5 && localPos.y < -halfSize.y + u_borderWidth) {
        bool keepLeft = (u_borderSides.w > 0.5) && (localPos.x < -halfSize.x + u_borderWidth);
        bool keepRight = (u_borderSides.y > 0.5) && (localPos.x > halfSize.x - u_borderWidth);
        if (!keepLeft && !keepRight) borderFactor = 0.0;
    }
    // Left
    if (u_borderSides.w < 0.5 && localPos.x < -halfSize.x + u_borderWidth) {
        bool keepTop = (u_borderSides.x > 0.5) && (localPos.y > halfSize.y - u_borderWidth);
        bool keepBottom = (u_borderSides.z > 0.5) && (localPos.y < -halfSize.y + u_borderWidth);
        if (!keepTop && !keepBottom) borderFactor = 0.0;
    }

    // Mix texture color and border color
    vec4 finalColor = mix(texColor, u_borderColor, borderFactor);
    
    // Apply vertex color (opacity)
    finalColor = finalColor * v_fragmentColor;
    
    // Anti-aliasing at the outer edge
    float alpha = 1.0 - smoothstep(-1.0, 0.0, dist);
    finalColor.a *= alpha;
    
    gl_FragColor = finalColor;
}
