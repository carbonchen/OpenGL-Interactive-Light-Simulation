#version 330 core

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note you don't have access to the vertex shader's default output, gl_Position.

in vec3 normalOutput;
in vec3 posOutput;

uniform mat4 model;

uniform vec3 lightPos;

uniform int switchRender;
uniform int rabbit;
uniform int sandal;
uniform int bear;
uniform int sphere;

// final color of the pixel
out vec4 fragColor;

void main()
{
    // material attributes
    vec3 ambChart;
    vec3 diffChart;
    vec3 specChart;
    float shininess;

    vec3 lightColor;

    // normal calculation for phong illumination
    vec3 normal = mat3(transpose(inverse(model))) * normalOutput;

    // chrome material rabbit, red light
    if (rabbit == 1) {
        ambChart = vec3(0.25, 0.25, 0.25);
        diffChart = vec3(0.4, 0.4, 0.4);
        specChart = vec3(0.774597, 0.774597, 0.774597);
        shininess = 0.6;
        lightColor = vec3(0.75, 0, 0);
        // white light to see material clearly
        // lightColor = vec3(1, 1, 1);
    }
    // yellow plastic material sandal, green light
    else if (sandal == 1) {
        ambChart = vec3(0.0, 0.0, 0.0);
        diffChart = vec3(0.5, 0.5, 0.0);
        specChart = vec3(0.6, 0.6, 0.6);
        shininess = 0.25;
        lightColor = vec3(0, 1, 0);
        // white light to see material clearly
        // lightColor = vec3(1, 1, 1);
    }
    // obsidian material bear, blue light
    else {
        ambChart = vec3(0.05375, 0.05, 0.06625);
        diffChart = vec3(0.18275, 0.17, 0.22525);
        specChart = vec3(0.332741, 0.328634, 0.346435);
        shininess = 0.3;
        lightColor = vec3(0, 0, 1);
        // white light to see material clearly
        // lightColor = vec3(1, 1, 1);
    }

    // light sphere gets no diffuse/specular
    if (sphere == 1) {
        ambChart = lightColor;
        diffChart = vec3(0.0, 0.0, 0.0);
        specChart = vec3(0.0, 0.0, 0.0);
    }

    // quadratic light attenuation
    float dist = length(lightPos - posOutput);
    float attenuation = 2.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));

    //ambient
    vec3 ambient = lightColor * ambChart;

    //diffuse
    vec3 lightDir = normalize(lightPos - posOutput);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * diffChart);

    //specular
    vec3 viewPos = vec3(0, 0, 20);
    vec3 viewDir = normalize(viewPos - posOutput);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightColor * (spec * specChart);

    vec3 result = attenuation * (ambient + diffuse + specular);

    // normal coloring
    if (switchRender == 0) {
        fragColor = vec4(result, 1.0);
    }
    // phong illumination coloring
    else {
        fragColor = vec4(vec3(normalOutput.x, normalOutput.y, normalOutput.z), 1.0);
    }
}