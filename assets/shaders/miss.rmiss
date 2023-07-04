#version 460
#extension GL_EXT_ray_tracing : require


struct RayPayload {
	vec3 color;
    vec3 emission;
	float distance;
	vec3 normal;
	float reflector;
};

layout(location = 0) rayPayloadInEXT RayPayload hitValue;

void main()
{
    hitValue.color = vec3(0.0, 0.0, 0.2);
    hitValue.emission = vec3(0.0, 0.0, 0.0);
    hitValue.distance = 10000;
    hitValue.normal = vec3(0.0, 0.0, 0.0);
    hitValue.reflector = 0;
}
