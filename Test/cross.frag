#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 mvp;
    vec2 resolution;
    float time;
} ubo;

vec3 ivo_cross(vec2 uv, int div, vec3 col, vec3 nocol)
{
    int iter = 0;

    bool useCross = true;
    int prevCross = int(useCross);

    while (iter < div)
    {
        int localHor = int(floor(uv.x * 3.0f)) % 3;
        int localVer = int(floor(uv.y * 3.0f)) % 3; 
        bool _cross = (localHor % 2 > 0 || localVer % 2 > 0);
        bool _hole = (localHor % 2 == 0 || localVer % 2 == 0);   
        bool colored = useCross ? _cross : _hole;
        if (!colored)
            break;

        iter++;
        if (iter == div)
            return colored ? col : nocol; 

        useCross = ((prevCross + 3 * localVer + localHor) % 2) == 0;
        prevCross = int(useCross);
        uv.x = uv.x * 3.0 - float(localHor);
        uv.y = uv.y * 3.0 - float(localVer);       
    }
    
    return nocol;
}

void main()
{
    float iTime = ubo.time;
    
    float secondsPerTime = 1.0;    
    float time = iTime * secondsPerTime;
    float prevTime = floor(time);
    float curTime = pow(time - prevTime, 1.25);
    float curScale = 1.0 + curTime * 2.;
    float curPos = curTime;
    
    vec4 fragCoord = vec4(gl_FragCoord.x, gl_FragCoord.y, 0, 1.0);
    fragCoord.y += 0.5 * (ubo.resolution.x - ubo.resolution.y);
    fragCoord.xy /= ubo.resolution.x;
    if ((fragCoord.y < fragCoord.x) == (1.0 - fragCoord.y > fragCoord.x))
    {
        float temp = fragCoord.y;
        fragCoord.y = fragCoord.x;
        fragCoord.x = temp;
    }
    vec2 uv = vec2(abs(fragCoord.x - 0.5), fragCoord.y + curPos);
    
   	uv /= curScale;
    
    float theta = curTime * 2.0 * 3.14159;
    
    vec3 col = vec3(gl_FrontFacing ? 1.0 : 0, 0, gl_FrontFacing ? 0 : 1.0);
    int power = 1 + int(floor(log(ubo.resolution.x)/log(3)));
    col = ivo_cross(uv, power, col, 0.69 * col);

    outColor = vec4(col, 1.0);
}