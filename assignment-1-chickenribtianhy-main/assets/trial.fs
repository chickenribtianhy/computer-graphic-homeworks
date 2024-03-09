#version 330 core
out vec4 frag_color;

// in VS_OUT{
//     vec3 normal;
//     vec3 position;
// } gs_out;


in vec3 normal;
in vec3 frag_pos;

uniform vec3 viewPos;
uniform vec3 view_front;
uniform vec3 objectColor;

uniform vec3 lightColor;
uniform vec3 lightPos; 

uniform float k0;
uniform float k1;
uniform float k2;

uniform float cos_cut;
uniform float cos_outtercut;

void main()
{
    // vec3 frag_pos;
    // vec3 normal;
    // normal = gs_out.normal;
    // frag_pos = gs_out.position;
    // normal = vs_normal;
    // frag_pos = vs_frag_pos;
    // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;


    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - frag_pos);

    float theta = dot(lightDir, normalize(-view_front));
    
    vec3 result;
    if (theta-cos_outtercut < 0) 
    {       
        // not lighted
        result = ambient * objectColor;
        
    }
    else { // lighted
        
        float diff = max(dot(norm, lightDir), 0.0);
        // diff = 0.0;
        vec3 diffuse = diff * lightColor;
        // diffuse = vec3(0.0, 0.0, 0.0);

        // specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - frag_pos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor; 

        if (theta-cos_cut < 0)
        {
            // soft
            float epsilon   = cos_cut - cos_outtercut;
            float intensity = clamp((theta - cos_outtercut) / epsilon, 0.0, 1.0);  
            diffuse  *= intensity;
            specular *= intensity;
        }
        
        

        // attenuation
        float dist = length(lightPos - frag_pos);
        float attenuation = 1.0 / (k0 + k1 * dist + k2 * dist * dist);    
        diffuse *= attenuation;
        specular *= attenuation;

        result = (ambient + diffuse + specular) * objectColor;

    }
    frag_color = vec4(result, 1.0);
}