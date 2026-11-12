
#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Custom lighting uniforms
uniform vec3 lightDir = vec3(0.5, 1.0, 0.5); // Direction of the light source
uniform vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);

// Output fragment color
out vec4 finalColor;

void main()
{
    // Fetch texture color and combine with diffuse tint
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;
    
    // Normalize light direction and vertex normal
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    
    // Calculate light intensity (Lambertian reflection)
    float NdotL = max(dot(N, L), 0.0);
    
    // Map the smooth gradient to hard, discrete bands
    float intensity = 0.3; // Ambient base layer
    if (NdotL > 0.85) intensity = 1.0;
    else if (NdotL > 0.5) intensity = 0.7;
    else if (NdotL > 0.25) intensity = 0.5;

    // Apply quantized intensity to final output
    finalColor = texelColor * lightColor * intensity;
}
