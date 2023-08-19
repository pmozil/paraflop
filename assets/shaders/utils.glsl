#define EPSILON 0.01F

#define SAMPLES 3
#define MAX_REFLECTIONS 3
#define LIGHT_SAMPLES 32
#define LIGHT_SAMPLES_SQRT 7 // = SQRT(LIGHT_SAMPLES)
#define AMBIENT_LIGHT 0.24F
#define LIGHT_MULTIPLIER 36

struct RayPayload {
	vec3 color;
    vec3 emission;
	float distance;
	vec3 normal;
	float reflector;
    float material;
};

struct Vertex {
  vec3 pos;
  vec3 normal;
  vec2 uv;
  vec4 color;
  vec4 texId;
  vec4 normalId;
  vec4 _pad2;
 };

uint random_uint(int seed) {
    uint rand = seed;
    for (int i = 0; i < 3; i++) {
        rand = (1140671485 * rand + 128201163) % uint(pow(2, 23));
    }
    return rand;
}
