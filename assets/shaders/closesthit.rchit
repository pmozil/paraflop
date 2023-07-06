#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require
#include "utils.glsl"

layout(location = 0) rayPayloadInEXT RayPayload hitValue;
layout(location = 2) rayPayloadInEXT bool shadowed;
hitAttributeEXT vec2 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2, set = 0) uniform UBO 
{
	mat4 viewInverse;
	mat4 projInverse;
	int vertexSize;
    int lightsCount;
    float dTime;
} ubo;
layout(binding = 3, set = 0) buffer Lights { vec4 l[]; } lights;
layout(binding = 4, set = 0) buffer Vertices { vec4 v[]; } vertices;
layout(binding = 5, set = 0) buffer Indices { uint i[]; } indices;
layout(binding = 6, set = 0) uniform sampler samp;
layout(binding = 7, set = 0) uniform texture2D textures[];

Vertex unpack(uint index)
{
	// Unpack the vertices from the SSBO using the glTF vertex structure
	// The multiplier is the size of the vertex divided by four float components (=16 bytes)
	const int m = ubo.vertexSize / 16;

	vec4 d0 = vertices.v[m * index + 0];
	vec4 d1 = vertices.v[m * index + 1];
	vec4 d2 = vertices.v[m * index + 2];
	vec4 d3 = vertices.v[m * index + 3];

	Vertex v;
	v.pos = d0.xyz;
	v.normal = vec3(d0.w, d1.x, d1.y);
	v.color = vec4(d2.x, d2.y, d2.z, 1.0);
    v.uv = d1.zw;
    v.texId = vec4(d2.w, d3.x, d3.y, d3.z);

	return v;
}

uint random(int seed) {
    uint rand = seed;
    for (int i = 0; i < 3; i++) {
        rand = (1140671485 * rand + 128201163) % uint(pow(2, 23));
    }
    return rand;
}

void main()
{
	ivec3 index = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1], indices.i[3 * gl_PrimitiveID + 2]);

	Vertex v0 = unpack(index.x);
	Vertex v1 = unpack(index.y);
	Vertex v2 = unpack(index.z);

	// Interpolate normal
	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    vec2 uv = v0.uv * barycentricCoords.x + v1.uv * barycentricCoords.y + v2.uv * barycentricCoords.z;
    vec3 color = vec3(0.0F);

    if (v0.texId.y > EPSILON && length(v0.color) > EPSILON) {
        vec3 tex_col = texture(sampler2D(textures[uint(v0.texId.y)], samp), uv).xyz;
	    color = tex_col * 3 + v0.color.xyz;
    } else if (length(v0.color) > EPSILON) {
        color = v0.color.xyz;
    } else if (v0.texId.y > EPSILON) {
        vec3 tex_col = texture(sampler2D(textures[uint(v0.texId.y)], samp), uv).xyz;
	    color = tex_col * 3;
    }

    // Ambient lighting
    float lighting = AMBIENT_LIGHT;

	const float tmin = 0.001;
	const float tmax = 10000.0;
	const vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

	// Diffuse +  Blihn-Phong lighting
    for(int i = 0; i < LIGHT_SAMPLES; i++) {
        uint idx = random(ubo.lightsCount - i) % ubo.lightsCount;
        vec3 col = vec3(0.0F);
        vec4 lightPos = lights.l[i];
	    vec3 lightVector = normalize(lightPos.xyz);

	    // Trace shadow ray and offset indices to match shadow hit/miss shader group indices
	    shadowed = true;  
	    traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 0, 0, 1, origin, tmin, lightVector, tmax, 2);
        float dist = distance(origin, lightPos.xyz);
        float light = LIGHT_MULTIPLIER * lightPos.w / (dist * dist);
	    if (shadowed || light < EPSILON) {
            continue;
	    }

	    // Shadow casting
        vec3 halfway = normalize(normalize(lightPos.xyz) - normalize(gl_WorldRayOriginEXT) - normalize(gl_WorldRayDirectionEXT));
	    float halfway_dot = clamp(dot(halfway, normal), 0.0F, 1.0F);
        lighting  += 4 * halfway_dot * light / LIGHT_SAMPLES_SQRT;
    }

    hitValue.emission = vec3(lighting);
    hitValue.color = color;
    hitValue.distance = gl_RayTmaxEXT;
    hitValue.normal = normal;
    hitValue.reflector = length(color) / 9.0F;
}
