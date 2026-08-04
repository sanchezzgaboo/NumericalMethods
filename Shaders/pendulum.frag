#version 330 core

out vec4 FragColor;

uniform vec2 uResolution;
uniform float uTime;
uniform float uTheta;

float sdSegment(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a;
    vec2 ba = b - a;

    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);

    return length(pa - ba * h);
}

void main()
{
    vec2 p =
    (2.0 * gl_FragCoord.xy - uResolution)
    / uResolution.y;

    vec2 pivot = vec2(0.2, 0.5);
    float L = 0.6;

    vec2 bob = pivot + L * vec2(sin(uTheta), -cos(uTheta));

    float d = sdSegment(p, pivot, bob);

    float thickness = 0.01;

    float rod = 1.0 - smoothstep(
        thickness,
        thickness + 0.002,
        d);

    float bobRadius = 0.04;

    float circle =
        1.0 -
        smoothstep(
            bobRadius,
            bobRadius + 0.002,
            length(p - bob));
    float pivotCircle =
        1.0 -
        smoothstep(
            0.02,
            0.022,
            length(p - pivot));
    float image = max(rod, circle);
    image = max(image, pivotCircle);

    FragColor = vec4(vec3(image), 1.0); 
}