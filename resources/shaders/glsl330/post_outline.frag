

#version 330

// Input vertex attributes from raylib's internal 2D quad shader
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniforms
uniform sampler2D texture0; // The rendered scene texture canvas
uniform vec2 renderSize;    // Screen resolution e.g. vec2(800.0, 450.0)

// Output fragment color
out vec4 finalColor;

void main()
{
    // Step size for 1 pixel width offset
    vec2 texelSize = 1.0 / renderSize;
    
    // Explicit array to store our 3x3 pixel kernel grid intensities
    float kernel[9];
    int idx = 0;
    
    // Sample the surrounding pixels 
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec3 c = texture(texture0, fragTexCoord + vec2(x, y) * texelSize).rgb;
            // Convert RGB to luminance/grayscale intensity
            kernel[idx] = dot(c, vec3(0.2126, 0.7152, 0.0722));
            idx++;
        }
    }
    
    // Sobel Matrix Horizontal (G_x) operators applied to our kernel positions:
    // [0] [1] [2]      [-1,  0,  1]
    // [3] [4] [5]  =>  [-2,  0,  2]
    // [6] [7] [8]      [-1,  0,  1]
    float gx = -1.0*kernel[0] + 1.0*kernel[2] 
               -2.0*kernel[3] + 2.0*kernel[5] 
               -1.0*kernel[6] + 1.0*kernel[8];
               
    // Sobel Matrix Vertical (G_y) operators applied to our kernel positions:
    // [0] [1] [2]      [-1, -2, -1]
    // [3] [4] [5]  =>  [ 0,  0,  0]
    // [6] [7] [8]      [ 1,  2,  1]
    float gy = -1.0*kernel[0] - 2.0*kernel[1] - 1.0*kernel[2] 
               +1.0*kernel[6] + 2.0*kernel[7] + 1.0*kernel[8];
    
    // Calculate total edge gradient magnitude
    float edge = sqrt(gx * gx + gy * gy);
    
    // Get the base color from the 3D scene texture pass
    // Multiply by fragColor to respect any raylib 2D tint colors passed to DrawTexture
    vec4 baseColor = texture(texture0, fragTexCoord) * fragColor;
    
    // Adjust this threshold (0.0 to 1.0) to make lines more or less sensitive
    float threshold = 0.4; 
    
    if (edge > threshold) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0); // Draw pure black outline edge
    } else {
        finalColor = baseColor; // Output normal cel-shaded pixel
    }
}
